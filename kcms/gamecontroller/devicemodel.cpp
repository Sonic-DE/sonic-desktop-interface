/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <KLocalizedString>
#include <QScopedValueRollback>
#include <QSet>
#include <QTimer>

#include <SDL2/SDL.h>

#include <iterator>

#include "logging.h"

// 100 ms while we have devices
const int kShortPollTime = 100;
// 2 seconds while we don't have any devices
const int kLongPollTime = 2000;

static int s_sdlLeaseCount = 0;
// Module relevance and the QML page each create a DeviceModel, but SDL has only
// one process-wide event queue. Whichever model polls must notify all of them.
static QList<QPointer<DeviceModel>> s_deviceModels;
static bool s_polling = false;

DeviceModel::DeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (s_sdlLeaseCount > 0 || SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0) {
        ++s_sdlLeaseCount;
        m_hasSdlLease = true;
        s_deviceModels.append(this);
    } else {
        qCWarning(KCM_GAMECONTROLLER) << "Could not initialise SDL game controller subsystem:" << SDL_GetError();
    }

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &DeviceModel::poll);
    // Only poll once per 2 seconds until we have a device
    m_timer->start(kLongPollTime);

    // Discovery must not depend on queued ADDED events: another SDL consumer
    // may already have taken them, including this KCM's relevance model.
    if (m_hasSdlLease) {
        SDL_PumpEvents();
        refreshDevices();
    }
}

DeviceModel::~DeviceModel()
{
    s_deviceModels.removeAll(this);
    qDeleteAll(m_devices);
    m_devices.clear();
    if (m_hasSdlLease && --s_sdlLeaseCount == 0) {
        qCDebug(KCM_GAMECONTROLLER) << "Releasing SDL game controller subsystem";
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }
}

Device *DeviceModel::device(SDL_JoystickID id) const
{
    return m_devices.value(id, nullptr);
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid)) {
        return {};
    }

    const SDL_JoystickID id = m_devices.keys().at(index.row());

    switch (role) {
    case CustomRoles::TextRole:
        return i18nc("Device name and path", "%1 (%2)", m_devices.value(id)->name(), m_devices.value(id)->path());
    case CustomRoles::IDRole:
        return id;
    }

    return {};
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{CustomRoles::TextRole, QByteArrayLiteral("text")}, {CustomRoles::IDRole, QByteArrayLiteral("id")}};
}

void DeviceModel::poll()
{
    if (!m_hasSdlLease || s_polling) {
        return;
    }
    QScopedValueRollback<bool> polling(s_polling, true);

    SDL_PumpEvents();
    // QPointer snapshots also tolerate a signal handler destroying a model.
    const auto models = s_deviceModels;
    for (const auto &model : models) {
        if (model) {
            model->refreshDevices();
        }
    }

    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        for (const auto &model : models) {
            if (model) {
                model->processEvent(event);
            }
        }
    }
}

void DeviceModel::refreshDevices()
{
    const int count = SDL_NumJoysticks();
    if (count < 0) {
        return;
    }
    QSet<SDL_JoystickID> connected;
    for (int index = 0; index < count; ++index) {
        const auto id = SDL_JoystickGetDeviceInstanceID(index);
        if (id >= 0) {
            connected.insert(id);
            if (!m_devices.contains(id)) {
                addDevice(index);
            }
        }
    }
    const auto knownIds = m_devices.keys();
    for (auto id : knownIds) {
        if (!connected.contains(id)) {
            removeDevice(id);
        }
    }
}

void DeviceModel::processEvent(const SDL_Event &event)
{
    switch (event.type) {
    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
        // Added events contain transient array indices, not stable IDs.
        // Reconcile current devices rather than opening a stale index.
        refreshDevices();
        break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        if (m_devices.contains(event.jbutton.which) && !m_devices.value(event.jbutton.which)->isController()) {
            m_devices.value(event.jbutton.which)->onButtonEvent(event.jbutton);
        }
        break;
    case SDL_JOYAXISMOTION:
        if (m_devices.contains(event.jaxis.which) && !m_devices.value(event.jaxis.which)->isController()) {
            m_devices.value(event.jaxis.which)->onAxisEvent(event.jaxis);
        }
        break;
    case SDL_JOYHATMOTION:
        if (m_devices.contains(event.jhat.which)) {
            m_devices.value(event.jhat.which)->onHatEvent(event.jhat);
        }
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
        if (m_devices.contains(event.cbutton.which) && m_devices.value(event.cbutton.which)->isController()) {
            m_devices.value(event.cbutton.which)->onControllerButtonEvent(event.cbutton);
        }
        break;
    case SDL_CONTROLLERAXISMOTION:
        if (m_devices.contains(event.caxis.which) && m_devices.value(event.caxis.which)->isController()) {
            m_devices.value(event.caxis.which)->onControllerAxisEvent(event.caxis);
        }
        break;
    }
}

void DeviceModel::addDevice(const int deviceIndex)
{
    const auto candidateId = SDL_JoystickGetDeviceInstanceID(deviceIndex);
    if (candidateId < 0 || m_devices.contains(candidateId) || SDL_JoystickIsVirtual(deviceIndex)) {
        return;
    }
    auto device = std::make_unique<Device>(deviceIndex, this);
    if (!device->open()) {
        const QString error = QString::fromLocal8Bit(SDL_GetError());
        qCCritical(KCM_GAMECONTROLLER).nospace() << "Could not open device " << deviceIndex << ": " << error;
        return;
    }

    const SDL_JoystickID id = device->id();
    if (m_devices.contains(id)) {
        qCWarning(KCM_GAMECONTROLLER) << "Ignoring a duplicate device ID" << id << "from add event";
        return;
    }

    qCDebug(KCM_GAMECONTROLLER) << "Adding device" << deviceIndex << "with ID" << id;

    const int row = std::distance(m_devices.begin(), m_devices.lowerBound(id));
    beginInsertRows(QModelIndex(), row, row);
    m_devices.insert(id, device.release());
    endInsertRows();

    // Now that we have a device poll every short poll time
    m_timer->setInterval(kShortPollTime);
    Q_EMIT devicesChanged();
}

void DeviceModel::removeDevice(const SDL_JoystickID id)
{
    if (!m_devices.contains(id)) {
        qCWarning(KCM_GAMECONTROLLER) << "Ignoring an invalid device ID" << id << "from removal event";
        return;
    }

    const int index = m_devices.keys().indexOf(id);

    qCDebug(KCM_GAMECONTROLLER) << "Removing device with ID" << id;

    beginRemoveRows(QModelIndex(), index, index);
    Device *device = m_devices.take(id);
    endRemoveRows();
    device->deleteLater();

    if (m_devices.count() == 0) {
        // Go back to long timeout polling now that we don't have a device
        m_timer->setInterval(kLongPollTime);
    }
    Q_EMIT devicesChanged();
}

int DeviceModel::count() const
{
    return m_devices.size();
}
