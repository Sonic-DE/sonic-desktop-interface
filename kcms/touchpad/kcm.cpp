/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"

#include "logging.h"
#include "touchpadbackend.h"
#include "touchpadmoduledata.h"
#include <config-build-options.h>

#include <KLocalizedString>
#include <KPluginFactory>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QQmlEngine>
#include <QQmlError>
#include <QTimer>

#include <algorithm>

using namespace Qt::StringLiterals;

K_PLUGIN_FACTORY_WITH_JSON(KCMTouchpadFactory, "kcm_touchpad.json", registerPlugin<KCMTouchpad>(); registerPlugin<TouchpadModuleData>();)

extern "C" {
Q_DECL_EXPORT void kcminit()
{
#if BUILD_KCM_TOUCHPAD_X11
    KCMTouchpad::kcmInit();
#endif
}
}

Message::Message() = default;

Message::Message(MessageType::MessageType type, const QString &text)
    : type(type)
    , text(text)
{
}

Message Message::error(const QString &text)
{
    return Message(MessageType::MessageType::Error, text);
}

Message Message::information(const QString &text)
{
    return Message(MessageType::MessageType::Information, text);
}

bool Message::operator==(const Message &other) const = default;

KCMTouchpad::KCMTouchpad(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
{
    registerQmlTypes();

    m_backend = TouchpadBackend::implementation();

    if (!m_backend) {
        m_initError = true;
        setSaveLoadMessage(Message::error(i18n("Not able to select appropriate backend")));
        qCCritical(KCM_TOUCHPAD) << "Not able to select appropriate backend.";
        return;
    }

    // A valid backend with zero devices is not an initialization error.
    // Hotplug notifications remain active so the KCM can recover when a
    // device appears later.
    m_initError = false;

    connect(m_backend, &TouchpadBackend::needsSaveChanged, this, &KCMTouchpad::updateKcmNeedsSave);
    connect(m_backend, &TouchpadBackend::deviceAdded, this, &KCMTouchpad::onDeviceAdded);
    connect(m_backend, &TouchpadBackend::deviceRemoved, this, &KCMTouchpad::onDeviceRemoved);

    connect(this, &KQuickConfigModule::mainUiReady, this, &KCMTouchpad::onMainUiReady);

    setButtons(KQuickConfigModule::Default | KQuickConfigModule::Apply);
    setCurrentDeviceIndex(0);
}

void KCMTouchpad::registerQmlTypes()
{
    static bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    const auto uri = "org.kde.plasma.private.kcm_touchpad";
    qmlRegisterUncreatableType<LibinputCommon>(uri, 1, 0, "InputDevice", QString());
    qmlRegisterUncreatableType<Message>(uri, 1, 0, "message", QString());
    qmlRegisterUncreatableMetaObject(MessageType::staticMetaObject, uri, 1, 0, "MessageType", QString());
    qmlRegisterUncreatableType<KCMTouchpad>(uri, 1, 0, "KCMTouchpad", QString());
    qmlRegisterUncreatableType<TouchpadBackend>(uri, 1, 0, "TouchpadBackend", QString());
}

TouchpadBackend *KCMTouchpad::backend() const
{
    return m_backend;
}

int KCMTouchpad::currentDeviceIndex() const
{
    return m_currentDeviceIndex;
}

void KCMTouchpad::setCurrentDeviceIndex(int index)
{
    // Should be at least zero, even if there are no devices. Thus it can't be
    // a clamp() because that might crash when low is greater than high.
    const int maxIndex = (m_backend ? m_backend->deviceCount() - 1 : 0);
    index = std::max(0, std::min(index, maxIndex));
    if (m_currentDeviceIndex != index) {
        m_currentDeviceIndex = index;
        Q_EMIT currentDeviceIndexChanged();
    }
}

void KCMTouchpad::kcmInit()
{
#if BUILD_KCM_TOUCHPAD_X11
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (!backend) {
        return;
    }
    if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        backend->load();
        backend->save();
    }
#endif
}

void KCMTouchpad::load()
{
    if (m_initError || !m_backend) {
        return;
    }

    if (!m_backend->deviceCount()) {
        // No device is not a load error; the placeholder handles display.
        setNeedsSave(false);
        return;
    }

    if (!m_backend->load()) {
        setSaveLoadMessage(Message::error(i18n("Error while loading values. See logs for more information. Please restart this configuration module.")));
    } else {
        setSaveLoadMessage();
    }
    setNeedsSave(false);
}

void KCMTouchpad::save()
{
    if (m_initError || !m_backend) {
        return;
    }

    if (!m_backend->deviceCount()) {
        setNeedsSave(false);
        return;
    }

    if (!m_backend->save()) {
        setSaveLoadMessage(
            Message::error(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again.")));
        // Do not reload or notify KDED on failure.
        // Note: KCModuleQml::save() will unconditionally clear needsSave after
        // this returns. The backend retains its dirty state for diagnostics,
        // but the shell's Apply indicator cannot be preserved by this KCM.
        setNeedsSave(m_backend->isSaveNeeded());
        return;
    }

    setSaveLoadMessage();

    // Notify KDED to reload settings
    QDBusMessage reload = QDBusMessage::createMethodCall(QStringLiteral("org.kde.touchpad"),
                                                         QStringLiteral("/modules/touchpad"),
                                                         QStringLiteral("org.kde.touchpad"),
                                                         QStringLiteral("reloadSettings"));
    QDBusConnection::sessionBus().asyncCall(reload);

    // load newly written values
    load();
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::defaults()
{
    if (m_initError || !m_backend) {
        return;
    }

    if (!m_backend->deviceCount()) {
        return;
    }

    if (!m_backend->defaults()) {
        setSaveLoadMessage(Message::error(i18n("Error while loading default values. Failed to set some options to their default values.")));
    }
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::onMainUiReady()
{
    if (m_warningsConnected) {
        return;
    }

    QQmlEngine *engine = this->engine().get();
    if (!engine) {
        return;
    }

    connect(engine, &QQmlEngine::warnings, this, &KCMTouchpad::onQmlEngineWarnings);
    m_warningsConnected = true;
}

void KCMTouchpad::onQmlEngineWarnings(const QList<QQmlError> &warnings)
{
    static bool reentrancyGuard = false;
    if (reentrancyGuard) {
        return;
    }

    const QString touchpadPrefix = QStringLiteral("qrc:/kcm/kcm_touchpad/");

    for (const QQmlError &warning : warnings) {
        const QString url = warning.url().toString();
        if (!url.startsWith(touchpadPrefix)) {
            continue;
        }

        qCWarning(KCM_TOUCHPAD).noquote() << "QML warning:" << url << "line:" << warning.line() << "column:" << warning.column()
                                          << "type:" << warning.messageType() << "description:" << warning.description();
    }

    // Collect touchpad-scoped warnings for display
    QList<QQmlError> touchpadWarnings;
    for (const QQmlError &warning : warnings) {
        if (warning.url().toString().startsWith(touchpadPrefix)) {
            touchpadWarnings.append(warning);
        }
    }

    if (touchpadWarnings.isEmpty()) {
        return;
    }

    // Deduplicate by message type, URL, line, column, and description
    QList<QQmlError> distinct;
    for (const QQmlError &w : std::as_const(touchpadWarnings)) {
        bool found = false;
        for (const QQmlError &d : std::as_const(distinct)) {
            if (d.messageType() == w.messageType() && d.url() == w.url() && d.line() == w.line() && d.column() == w.column()
                && d.description() == w.description()) {
                found = true;
                break;
            }
        }
        if (!found) {
            distinct.append(w);
        }
    }

    if (distinct.isEmpty()) {
        return;
    }

    reentrancyGuard = true;

    const QString firstDesc = distinct.first().description();
    QString text = firstDesc;
    if (distinct.size() > 1) {
        text += QStringLiteral("\n(%1 more warning%2)").arg(distinct.size() - 1).arg(distinct.size() > 2 ? QStringLiteral("s") : QString());
    }
    setUiRuntimeMessage(Message::error(text));

    reentrancyGuard = false;
}

void KCMTouchpad::updateKcmNeedsSave()
{
    if (!m_backend) {
        setNeedsSave(false);
        return;
    }
    if (!m_backend->isSaveNeeded()) {
        setSaveLoadMessage();
    }
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::onDeviceAdded(bool success)
{
    if (!success) {
        setHotplugMessage(Message::error(i18n("Error while adding newly connected device. Please reconnect it and restart this configuration module.")));
        return;
    }

    if (m_backend && m_backend->deviceCount() > 0) {
        setHotplugMessage();
    }
}

void KCMTouchpad::onDeviceRemoved(int index)
{
    if (m_currentDeviceIndex == index) {
        if (m_backend && m_backend->deviceCount() > 0) {
            setHotplugMessage(Message::information(i18n("Touchpad disconnected. Closed its setting dialog.")));
        } else {
            setHotplugMessage(Message::information(i18n("Touchpad disconnected. No other touchpads found.")));
        }
    }

    // Update current device index based on remaining devices
    if (m_backend && m_currentDeviceIndex >= index && m_currentDeviceIndex > 0) {
        setCurrentDeviceIndex(m_currentDeviceIndex - 1);
    } else {
        setCurrentDeviceIndex(m_currentDeviceIndex);
    }

    setNeedsSave(m_backend ? m_backend->isSaveNeeded() : false);
}

void KCMTouchpad::setSaveLoadMessage(const Message &message)
{
    if (m_saveLoadMessage != message) {
        m_saveLoadMessage = message;
        Q_EMIT saveLoadMessageChanged();
    }
}

void KCMTouchpad::setHotplugMessage(const Message &message)
{
    if (m_hotplugMessage != message) {
        m_hotplugMessage = message;
        Q_EMIT hotplugMessageChanged();
    }
}

void KCMTouchpad::setUiRuntimeMessage(const Message &message)
{
    if (m_uiRuntimeMessage != message) {
        m_uiRuntimeMessage = message;
        Q_EMIT uiRuntimeMessageChanged();
    }
}

#include "kcm.moc"
