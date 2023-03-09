/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <QTimer>

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

#include "gamepad.h"

DeviceModel::DeviceModel()
{
    SDL_Init(SDL_INIT_GAMECONTROLLER);

    poll();

    // TODO: temporary event loop
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DeviceModel::poll);
    // Use 100ms between polling for now
    timer->start(100);
}

Gamepad *DeviceModel::device(int index) const
{
    if (index < 0 || index > m_devices.count())
        return nullptr;

    int sdlIndex = m_devices.keys().at(index);
    return m_devices.value(sdlIndex);
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    int sdlIndex = m_devices.keys().at(index.row());
    switch (role) {
    case CustomRoles::NameRole:
        return m_devices.value(sdlIndex)->getName();
    case CustomRoles::DeviceRole:
        return QVariant::fromValue(m_devices.value(sdlIndex));
    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{CustomRoles::NameRole, "name"}, {CustomRoles::DeviceRole, "device"}};
}

void DeviceModel::poll()
{
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            addDevice(event.cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            removeDevice(event.cdevice.which);
            break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP: {
            // Tell that controller to process it's event
            qDebug() << "Got button event for device: " << event.cbutton.which;
            m_devices.value(event.cbutton.which)->onButtonEvent(event.cbutton);
        } break;

        case SDL_CONTROLLERAXISMOTION: {
            // Tell that gamepad to process it's event
            qDebug() << "Got axis event for device: " << event.caxis.which;
            m_devices.value(event.caxis.which)->onAxisEvent(event.caxis);
        } break;
        }
    }
}

void DeviceModel::addDevice(const int deviceIndex)
{
    if (m_devices.contains(deviceIndex)) {
        qDebug() << "Got a duplicate add event, ignoring";
        return;
    }

    int nextIndex = m_devices.count();
    qDebug() << "adding device: " << deviceIndex << " at row: " << nextIndex;

    beginInsertRows(QModelIndex(), nextIndex, nextIndex);
    m_devices.insert(deviceIndex, new Gamepad(SDL_JoystickOpen(deviceIndex), SDL_GameControllerOpen(deviceIndex), this));
    endInsertRows();
}

void DeviceModel::removeDevice(const int deviceIndex)
{
    if (!m_devices.contains(deviceIndex)) {
        qWarning() << "Invalid device index from removal event, ignoring";
        return;
    }

    int ourIndex = m_devices.keys().indexOf(deviceIndex);
    qDebug() << "removing device: " << deviceIndex << " from row " << ourIndex;

    beginRemoveRows(QModelIndex(), ourIndex, ourIndex);
    m_devices.value(deviceIndex)->deleteLater();
    m_devices.remove(deviceIndex);
    endRemoveRows();
}
