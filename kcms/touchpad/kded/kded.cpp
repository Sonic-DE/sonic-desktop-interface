/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kded.h"

#include <KConfigGroup>
#include <KPluginFactory>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDebug>

using namespace Qt::Literals::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(TouchpadDisabler, "kded_touchpad.json")

void migrateConfig(TouchpadBackend *backend)
{
    if (!backend->isTouchpadAvailable()) {
        return;
    }

#if BUILD_KCM_TOUCHPAD_X11
    // on Wayland, KWin handles the on/off/toggle shortcuts, the kded is not used and X11 settings are not adopted
    if (backend->getMode() != TouchpadInputBackendMode::XLibinput) {
        return;
    }
#else
    return;
#endif

    KSharedConfig::Ptr oldConfig = KSharedConfig::openConfig(u"touchpadrc"_s);
    // TODO KF7 (or perhaps earlier): Remove Plasma 5->6 migration code and delete touchpadrc

    const KConfigGroup oldGroup = oldConfig->group(u"autodisable"_s);
    if (!oldGroup.exists()) {
        // avoid writing a new config file just with migration data
        return;
    }

    bool disableWhenMousePluggedIn = oldGroup.readEntry("DisableWhenMousePluggedIn", false);
    bool disableOnKeyboardActivity = oldGroup.readEntry("DisableOnKeyboardActivity", true);

    // touchpadxlibinputrc uses the device name in the config group - use backend API to write to the correct one
    backend->load();

    for (LibinputCommon *device : backend->inputDevices()) { // only touchpads, because TouchpadBackend
        if (device->supportsDisableEventsOnExternalMouse()) {
            device->setDisableEventsOnExternalMouse(disableWhenMousePluggedIn);
        }
        // DisableWhileTyping has been configurable since early on, so keep the migration defensive
        // by only performing migrations from `true` (default) to `false` (manually changed in prior backend).
        // i.e. users who stuck with the default, or who changed it to `false` only for libinput, are unaffected
        if (!disableOnKeyboardActivity && device->supportsDisableWhileTyping()) {
            device->setDisableWhileTyping(false);
        }
    }

    backend->save();

    oldConfig->deleteGroup(u"autodisable"_s);
    oldConfig->sync();
}

void TouchpadDisabler::serviceRegistered(const QString &service)
{
    if (!m_dependencies.removeWatchedService(service)) {
        return;
    }

    if (m_dependencies.watchedServices().isEmpty()) {
        lateInit();
    }
}

TouchpadDisabler::TouchpadDisabler(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
    , m_backend(TouchpadBackend::implementation())
{
    if (!m_backend) {
        return;
    }
    migrateConfig(m_backend);

    QDBusConnection::sessionBus().registerService(u"org.kde.touchpad"_s);

    m_dependencies.addWatchedService("org.kde.plasmashell");
    connect(&m_dependencies, &QDBusServiceWatcher::serviceRegistered, this, &TouchpadDisabler::serviceRegistered);

    connect(m_backend, &TouchpadBackend::touchpadStateChanged, this, &TouchpadDisabler::updateCurrentState);
    connect(m_backend, &TouchpadBackend::touchpadReset, this, &TouchpadDisabler::handleReset);
    connect(m_backend, &TouchpadBackend::keyboardActivityStarted, this, &TouchpadDisabler::keyboardActivityStarted);
    connect(m_backend, &TouchpadBackend::keyboardActivityFinished, this, &TouchpadDisabler::keyboardActivityFinished);
    m_keyboardActivityTimer.setSingleShot(true);
    connect(&m_keyboardActivityTimer, &QTimer::timeout, this, [this] {
        m_keyboardActivitySuspend = false;
        applySuspendReasons();
    });
    m_externalMouseTimer.setInterval(1000);
    connect(&m_externalMouseTimer, &QTimer::timeout, this, [this] {
        const bool present = m_disableWhenMousePluggedIn && m_backend->externalMousePresent();
        if (present != m_externalMouseSuspend) {
            m_externalMouseSuspend = present;
            applySuspendReasons();
        }
    });
    m_externalMouseTimer.start();
    reloadSettings();

    updateCurrentState();
    m_userRequestedSuspend = m_touchpadSuspended;

    m_dependencies.setWatchMode(QDBusServiceWatcher::WatchForRegistration);
    m_dependencies.setConnection(QDBusConnection::sessionBus());
    QDBusPendingCall async = QDBusConnection::sessionBus().interface()->asyncCall(QLatin1String("ListNames"));
    QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(async, this);
    connect(callWatcher, &QDBusPendingCallWatcher::finished, this, &TouchpadDisabler::serviceNameFetchFinished);

    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.login1"),
                                         QStringLiteral("/org/freedesktop/login1"),
                                         QStringLiteral("org.freedesktop.login1.Manager"),
                                         QStringLiteral("PrepareForSleep"),
                                         this,
                                         SLOT(onPrepareForSleep(bool)));
}

void TouchpadDisabler::serviceNameFetchFinished(QDBusPendingCallWatcher *callWatcher)
{
    QDBusPendingReply<QStringList> reply = *callWatcher;
    callWatcher->deleteLater();

    if (reply.isError() || reply.value().isEmpty()) {
        qWarning() << "Error: Couldn't get registered services list from session bus";
        return;
    }

    const QStringList allServices = reply.value();
    const QStringList watchedList = m_dependencies.watchedServices();
    for (const QString &service : watchedList) {
        if (allServices.contains(service)) {
            serviceRegistered(service);
        }
    }
}

void TouchpadDisabler::updateCurrentState()
{
    if (!m_backend->isTouchpadAvailable()) {
        return;
    }
    m_touchpadSuspended = m_backend->isTouchpadSuspended();
}

void TouchpadDisabler::toggle()
{
    m_userRequestedSuspend = !m_userRequestedSuspend;
    applySuspendReasons();

    showOsd();
}

void TouchpadDisabler::disable()
{
    m_userRequestedSuspend = true;
    applySuspendReasons();

    showOsd();
}

void TouchpadDisabler::enable()
{
    m_userRequestedSuspend = false;
    applySuspendReasons();

    showOsd();
}

void TouchpadDisabler::lateInit()
{
    updateCurrentState();
}

void TouchpadDisabler::handleReset()
{
    updateCurrentState();
    if (!m_backend->isTouchpadAvailable()) {
        return;
    }
    // Apply persisted configuration to a newly attached XLibinput device
    // that was connected after session kcminit.
#if BUILD_KCM_TOUCHPAD_X11
    if (m_backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        if (!m_backend->load()) {
            qWarning() << "Failed to load touchpad config on attach";
        } else if (!m_backend->save()) {
            qWarning() << "Failed to save touchpad config on attach";
        }
    }
#endif
    applySuspendReasons();
}

void TouchpadDisabler::reloadSettings()
{
#if BUILD_KCM_TOUCHPAD_X11 && HAVE_SYNAPTICS
    if (m_backend->getMode() == TouchpadInputBackendMode::XSynaptics) {
        KConfigGroup group(KSharedConfig::openConfig(u"touchpadrc"_s), u"autodisable"_s);
        m_disableOnKeyboardActivity = group.readEntry("DisableOnKeyboardActivity", true);
        m_disableWhenMousePluggedIn = group.readEntry("DisableWhenMousePluggedIn", false);
        m_keyboardActivityTimeoutMs = qMax(0, group.readEntry("KeyboardActivityTimeoutMs", 300));
    }
#endif
}

void TouchpadDisabler::keyboardActivityStarted()
{
    if (!m_disableOnKeyboardActivity) {
        return;
    }
    m_keyboardActivityTimer.stop();
    m_keyboardActivitySuspend = true;
    applySuspendReasons();
}

void TouchpadDisabler::keyboardActivityFinished()
{
    if (m_keyboardActivitySuspend) {
        m_keyboardActivityTimer.start(m_keyboardActivityTimeoutMs);
    }
}

void TouchpadDisabler::applySuspendReasons()
{
    m_backend->setTouchpadSuspended(m_userRequestedSuspend || m_keyboardActivitySuspend || m_externalMouseSuspend);
}

void TouchpadDisabler::onPrepareForSleep(bool sleep)
{
    m_preparingForSleep = sleep;
}

void TouchpadDisabler::showOsd()
{
    if (m_preparingForSleep) {
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                      QStringLiteral("/org/kde/osdService"),
                                                      QStringLiteral("org.kde.osdService"),
                                                      QStringLiteral("touchpadEnabledChanged"));

    msg.setArguments({!m_backend->isTouchpadSuspended()});

    QDBusConnection::sessionBus().asyncCall(msg);
}

#include "kded.moc"
