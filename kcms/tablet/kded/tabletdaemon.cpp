/*
    SPDX-FileCopyrightText: 2025 KDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabletdaemon.h"
#include "inputdevice.h"

#include <KGlobalAccel>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QAction>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QScreen>

Q_LOGGING_CATEGORY(KDED_TABLET, "kded_tabletdaemon")

// D-Bus service/interface constants
static constexpr QLatin1String KWIN_SERVICE{"org.kde.KWin"};
static constexpr QLatin1String DEVICE_MANAGER_PATH{"/org/kde/KWin/InputDevice"};
static constexpr QLatin1String DEVICE_MANAGER_INTERFACE{"org.kde.KWin.InputDeviceManager"};
static constexpr QLatin1String DEVICE_INTERFACE{"org.kde.KWin.InputDevice"};
static constexpr QLatin1String DBUS_PROPERTIES_INTERFACE{"org.freedesktop.DBus.Properties"};

K_PLUGIN_CLASS_WITH_JSON(TabletDaemon, "tabletdaemon.json")

TabletDaemon::TabletDaemon(QObject *parent, const QVariantList &args)
    : KDEDModule(parent)
{
    Q_UNUSED(args)

    setupShortcuts();
    loadDevices();

    // Monitor screen topology changes
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &TabletDaemon::onScreenAdded);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &TabletDaemon::onScreenRemoved);

    // Listen for device hotplug events
    bool deviceAddedConnected = QDBusConnection::sessionBus().connect(KWIN_SERVICE,
                                                                      DEVICE_MANAGER_PATH,
                                                                      DEVICE_MANAGER_INTERFACE,
                                                                      QStringLiteral("deviceAdded"),
                                                                      this,
                                                                      SLOT(onDeviceAdded(QString)));
    if (!deviceAddedConnected) {
        qCWarning(KDED_TABLET) << "Failed to connect to KWin deviceAdded signal";
    }

    bool deviceRemovedConnected = QDBusConnection::sessionBus().connect(KWIN_SERVICE,
                                                                        DEVICE_MANAGER_PATH,
                                                                        DEVICE_MANAGER_INTERFACE,
                                                                        QStringLiteral("deviceRemoved"),
                                                                        this,
                                                                        SLOT(onDeviceRemoved(QString)));
    if (!deviceRemovedConnected) {
        qCWarning(KDED_TABLET) << "Failed to connect to KWin deviceRemoved signal";
    }
}

TabletDaemon::~TabletDaemon()
{
    qDeleteAll(m_tablets);
}

void TabletDaemon::setupShortcuts()
{
    auto cycleAction = new QAction(i18nc("@action", "Cycle Tablet Between Screens"), this);
    cycleAction->setObjectName(QStringLiteral("cycleTabletScreen"));
    cycleAction->setProperty("componentName", QStringLiteral("tabletdaemon"));
    cycleAction->setProperty("componentDisplayName", i18nc("Component name", "Tablet Daemon"));
    KGlobalAccel::self()->setDefaultShortcut(cycleAction, {Qt::CTRL | Qt::SHIFT | Qt::Key_G});
    KGlobalAccel::self()->setShortcut(cycleAction, {Qt::CTRL | Qt::SHIFT | Qt::Key_G});
    connect(cycleAction, &QAction::triggered, this, &TabletDaemon::cycleScreen);

    for (int i = 0; i < MAX_SUPPORTED_SCREENS; ++i) {
        auto mapAction = new QAction(i18nc("@action %1 is the screen number", "Map Tablet to Screen %1", i + 1), this);
        mapAction->setObjectName(QStringLiteral("mapTabletToScreen%1").arg(i + 1));
        mapAction->setProperty("componentName", QStringLiteral("tabletdaemon"));
        mapAction->setProperty("componentDisplayName", i18nc("Component name", "Tablet Daemon"));
        KGlobalAccel::self()->setDefaultShortcut(mapAction, {});
        KGlobalAccel::self()->setShortcut(mapAction, {});

        connect(mapAction, &QAction::triggered, this, [this, i]() {
            mapToScreen(i);
        });
    }
}

void TabletDaemon::loadDevices()
{
    QDBusInterface kwinService(KWIN_SERVICE, QStringLiteral("/"), QString(), QDBusConnection::sessionBus());
    if (!kwinService.isValid()) {
        qCWarning(KDED_TABLET) << "KWin D-Bus service not available, tablet detection disabled";
        return;
    }

    QDBusInterface deviceManager(KWIN_SERVICE, DEVICE_MANAGER_PATH, DBUS_PROPERTIES_INTERFACE, QDBusConnection::sessionBus());

    QDBusReply<QVariant> reply = deviceManager.call(QStringLiteral("Get"), DEVICE_MANAGER_INTERFACE, QStringLiteral("devicesSysNames"));

    if (!reply.isValid()) {
        qCWarning(KDED_TABLET) << "Failed to query KWin for devices:" << reply.error().message();
        return;
    }

    const QStringList devices = reply.value().toStringList();

    for (const QString &sysName : devices) {
        if (m_tablets.contains(sysName)) {
            continue;
        }

        QDBusInterface deviceIface(KWIN_SERVICE, DEVICE_MANAGER_PATH + QLatin1Char('/') + sysName, DEVICE_INTERFACE, QDBusConnection::sessionBus());

        QVariant tabletToolProp = deviceIface.property("tabletTool");
        if (tabletToolProp.isValid() && tabletToolProp.toBool()) {
            auto device = new InputDevice(sysName, this);
            m_tablets.insert(sysName, device);
            qCDebug(KDED_TABLET) << "Loaded tablet device:" << sysName;
        }
    }
}

void TabletDaemon::onDeviceAdded(const QString &sysName)
{
    if (m_tablets.contains(sysName)) {
        qCDebug(KDED_TABLET) << "Device already loaded:" << sysName;
        return;
    }

    QDBusInterface deviceIface(KWIN_SERVICE, DEVICE_MANAGER_PATH + QLatin1Char('/') + sysName, DEVICE_INTERFACE, QDBusConnection::sessionBus());

    QVariant tabletToolProp = deviceIface.property("tabletTool");
    if (tabletToolProp.isValid() && tabletToolProp.toBool()) {
        auto device = new InputDevice(sysName, this);
        m_tablets.insert(sysName, device);
        qCDebug(KDED_TABLET) << "Added tablet device:" << sysName;
    }
}

void TabletDaemon::onDeviceRemoved(const QString &sysName)
{
    if (m_tablets.contains(sysName)) {
        delete m_tablets.take(sysName);
        qCDebug(KDED_TABLET) << "Removed tablet device:" << sysName;
    }
}

bool TabletDaemon::ensureTabletsLoaded()
{
    if (m_tablets.isEmpty()) {
        loadDevices();
    }

    if (m_tablets.isEmpty()) {
        showOSD(i18nc("@info", "No tablet device found"), QStringLiteral("dialog-warning"));
        qCWarning(KDED_TABLET) << "No tablet devices available";
        return false;
    }

    return true;
}

QString TabletDaemon::getScreenDisplayName(QScreen *screen) const
{
    if (!screen) {
        return QString();
    }

    QString name = screen->model();
    if (name.isEmpty()) {
        name = screen->name();
    }
    return name;
}

void TabletDaemon::cycleScreen()
{
    if (!ensureTabletsLoaded()) {
        return;
    }

    auto screens = qGuiApp->screens();
    if (screens.isEmpty()) {
        showOSD(i18nc("@info", "No screens found"), QStringLiteral("dialog-error"));
        qCWarning(KDED_TABLET) << "No screens available";
        return;
    }

    QString currentOutput = m_tablets.cbegin().value()->outputName();

    int currentIndex = -1;
    for (int i = 0; i < screens.size(); ++i) {
        if (screens[i]->name() == currentOutput) {
            currentIndex = i;
            break;
        }
    }

    int nextIndex = (currentIndex + 1) % screens.size();
    QString nextOutput = screens[nextIndex]->name();

    for (auto device : std::as_const(m_tablets)) {
        if (!device) {
            continue;
        }

        device->setOutputName(nextOutput);
        device->setMapToWorkspace(false);
        device->save();
    }

    QString screenName = getScreenDisplayName(screens[nextIndex]);
    showOSD(i18nc("@info %1 is the screen name", "Tablet(s) mapped to %1", screenName));
    qCDebug(KDED_TABLET) << "Cycled" << m_tablets.size() << "tablet(s) to screen:" << screenName;
}

void TabletDaemon::mapToScreen(int screenIndex)
{
    if (!ensureTabletsLoaded()) {
        return;
    }

    auto screens = qGuiApp->screens();
    if (screenIndex < 0 || screenIndex >= screens.size()) {
        showOSD(i18nc("@info %1 is the screen number", "Screen %1 not found", screenIndex + 1), QStringLiteral("dialog-warning"));
        qCWarning(KDED_TABLET) << "Invalid screen index:" << screenIndex << "(available screens:" << screens.size() << ")";
        return;
    }

    QString targetOutput = screens[screenIndex]->name();

    for (auto device : std::as_const(m_tablets)) {
        if (!device) {
            continue;
        }

        device->setOutputName(targetOutput);
        device->setMapToWorkspace(false);
        device->save();
    }

    QString screenName = getScreenDisplayName(screens[screenIndex]);
    showOSD(i18nc("@info %1 is the screen name", "Tablet(s) mapped to %1", screenName));
    qCDebug(KDED_TABLET) << "Mapped" << m_tablets.size() << "tablet(s) to screen:" << screenName;
}

void TabletDaemon::onScreenAdded(QScreen *screen)
{
    if (!screen) {
        return;
    }

    qCDebug(KDED_TABLET) << "Screen added:" << screen->name() << screen->model();

    if (!m_tablets.isEmpty()) {
        showOSD(i18nc("@info %1 is the screen name", "Screen added: %1", getScreenDisplayName(screen)), QStringLiteral("video-display"));
    }
}

void TabletDaemon::onScreenRemoved(QScreen *screen)
{
    if (!screen) {
        return;
    }

    QString removedScreenName = screen->name();
    qCDebug(KDED_TABLET) << "Screen removed:" << removedScreenName;

    auto screens = qGuiApp->screens();
    if (screens.isEmpty()) {
        qCWarning(KDED_TABLET) << "No screens remaining after screen removal";
        return;
    }

    QString firstScreenName = screens.first()->name();
    bool remapped = false;

    for (auto device : std::as_const(m_tablets)) {
        if (!device) {
            continue;
        }

        if (device->outputName() == removedScreenName) {
            device->setOutputName(firstScreenName);
            device->save();
            remapped = true;
        }
    }

    if (remapped) {
        showOSD(i18nc("@info %1 is the screen name", "Screen removed, tablets remapped to %1", getScreenDisplayName(screens.first())),
                QStringLiteral("video-display"));
    }
}

void TabletDaemon::showOSD(const QString &message, const QString &icon)
{
    QDBusInterface osd(QStringLiteral("org.kde.plasmashell"),
                       QStringLiteral("/org/kde/osdService"),
                       QStringLiteral("org.kde.osdService"),
                       QDBusConnection::sessionBus());

    if (osd.isValid()) {
        osd.call(QStringLiteral("showText"), icon, message);
    } else {
        qCWarning(KDED_TABLET) << "Failed to show OSD message:" << message;
    }
}

#include "tabletdaemon.moc"
