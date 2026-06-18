/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "keyboard_daemon.h"
#include "debug.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>

#include <KPluginFactory>

#include "flags.h"
#include "ipc/KeyboardIpcServer.h"
#include "keyboard_hardware.h"
#include "keyboardsettings.h"
#include "layout_memory_persister.h"
#include "x11_helper.h"
#include "xinput_helper.h"
#include "xkb_helper.h"

K_PLUGIN_CLASS_WITH_JSON(KeyboardDaemon, "kded_keyboard.json")

KeyboardDaemon::KeyboardDaemon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , keyboardSettings(new KeyboardSettings(this))
    , keyboardConfig(new KeyboardConfig(keyboardSettings, this))
    , actionCollection(nullptr)
    , xEventNotifier(nullptr)
    , layoutMemory(*keyboardConfig)
{
    if (!X11Helper::xkbSupported(nullptr))
        return;

    configureKeyboard();
    registerListeners();

    m_ipcServer = new KeyboardIpcServer(
        this,
        [this]() {
            return getLayoutsList();
        },
        [this]() {
            return getLayout();
        },
        [this]() {
            switchToNextLayout();
        },
        [this]() {
            switchToPreviousLayout();
        },
        [this](uint i) {
            setLayout(i);
        });

    LayoutMemoryPersister layoutMemoryPersister(layoutMemory);
    if (layoutMemoryPersister.restore()) {
        if (layoutMemoryPersister.getGlobalLayout().isValid()) {
            X11Helper::setLayout(layoutMemoryPersister.getGlobalLayout());
        }
    }
}

KeyboardDaemon::~KeyboardDaemon()
{
    LayoutMemoryPersister layoutMemoryPersister(layoutMemory);
    layoutMemoryPersister.setGlobalLayout(X11Helper::getCurrentLayout());
    layoutMemoryPersister.save();

    delete m_ipcServer;
    m_ipcServer = nullptr;

    unregisterListeners();
    unregisterShortcut();

    delete xEventNotifier;
}

void KeyboardDaemon::configureKeyboard()
{
    qCDebug(KCM_KEYBOARD) << "Configuring keyboard";
    init_keyboard_hardware();

    // Force the shared KConfig to re-read kxkbrc from disk
    keyboardSettings->sharedConfig()->reparseConfiguration();
    keyboardConfig->load();
    XkbHelper::initializeKeyboardLayouts(*keyboardConfig);
    layoutMemory.configChanged();

    unregisterShortcut();
    registerShortcut();

    qCDebug(KCM_KEYBOARD) << "Emitting layoutListChanged (from configureKeyboard)";
    Q_EMIT layoutListChanged();
    broadcastLayoutsChanged();
}

void KeyboardDaemon::configureInput()
{
    QStringList modules;
    modules << QStringLiteral("kcm_mouse_init") << QStringLiteral("kcm_touchpad_init");
    QProcess::startDetached(QStringLiteral("kcminit"), modules);
}

void KeyboardDaemon::registerShortcut()
{
    if (actionCollection == nullptr) {
        actionCollection = new KeyboardLayoutActionCollection(this, false);

        QAction *toggleLayoutAction = actionCollection->getToggleAction();
        connect(toggleLayoutAction, &QAction::triggered, this, [this]() {
            setLastUsedLayoutValue(getLayout());
            switchToNextLayout();

            LayoutUnit newLayout = X11Helper::getCurrentLayout();
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                              QStringLiteral("/org/kde/osdService"),
                                                              QStringLiteral("org.kde.osdService"),
                                                              QStringLiteral("kbdLayoutChanged"));
            msg << Flags::getLongText(newLayout);
            QDBusConnection::sessionBus().asyncCall(msg);
        });

        QAction *lastUsedLayoutAction = actionCollection->getLastUsedLayoutAction();
        connect(lastUsedLayoutAction, &QAction::triggered, this, [this]() {
            auto layoutsList = X11Helper::getLayoutsList();
            if (!lastUsedLayout.has_value() || layoutsList.count() <= *lastUsedLayout) {
                switchToPreviousLayout();
            } else {
                setLayout(*lastUsedLayout);
            }

            LayoutUnit newLayout = X11Helper::getCurrentLayout();
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                              QStringLiteral("/org/kde/osdService"),
                                                              QStringLiteral("org.kde.osdService"),
                                                              QStringLiteral("kbdLayoutChanged"));
            msg << Flags::getLongText(newLayout);
            QDBusConnection::sessionBus().asyncCall(msg);
        });

        actionCollection->loadLayoutShortcuts(keyboardConfig->layouts());
        // clang-format off
    connect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        // clang-format on
    }
}

void KeyboardDaemon::unregisterShortcut()
{
    // register KDE keyboard shortcut for switching layouts
    if (actionCollection != nullptr) {
        // clang-format off
        disconnect(actionCollection, SIGNAL(actionTriggered(QAction*)), this, SLOT(setLayout(QAction*)));
        // clang-format on
        disconnect(actionCollection->getToggleAction(), &QAction::triggered, this, &KeyboardDaemon::switchToNextLayout);

        delete actionCollection;
        actionCollection = nullptr;
    }
}

void KeyboardDaemon::registerListeners()
{
    if (xEventNotifier == nullptr) {
        xEventNotifier = new XInputEventNotifier();
    }
    connect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureInput);
    connect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
    connect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, [this]() {
        layoutMapDebounce->start();
    });
    connect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChangedSlot);
    xEventNotifier->start();

    const QString kxkbrc = kxkbrcPath();
    if (kxkbrc.isEmpty()) {
        qCWarning(KCM_KEYBOARD) << "Could not determine config directory; kxkbrc will not be watched";
        return;
    }

    if (!keyboardFileWatcher) {
        keyboardFileWatcher = new QFileSystemWatcher(this);
        connect(keyboardFileWatcher, &QFileSystemWatcher::fileChanged, this, &KeyboardDaemon::kxkbrcChanged);
    }
    if (!keyboardFileDebounce) {
        keyboardFileDebounce = new QTimer(this);
        keyboardFileDebounce->setSingleShot(true);
        keyboardFileDebounce->setInterval(1000);
        connect(keyboardFileDebounce, &QTimer::timeout, this, &KeyboardDaemon::kxkbrcDebounceTimeout);
    }

    if (!layoutMapDebounce) {
        layoutMapDebounce = new QTimer(this);
        layoutMapDebounce->setSingleShot(true);
        layoutMapDebounce->setInterval(1000);
        connect(layoutMapDebounce, &QTimer::timeout, this, [this]() {
            keyboardSettings->sharedConfig()->reparseConfiguration();
            keyboardConfig->load();
            layoutMemory.layoutMapChanged();
            Q_EMIT layoutListChanged();
            broadcastLayoutsChanged();
        });
    }

    if (!keyboardFileWatcher->files().contains(kxkbrc)) {
        if (!keyboardFileWatcher->addPath(kxkbrc)) {
            qCWarning(KCM_KEYBOARD) << "Failed to watch kxkbrc at" << kxkbrc;
        } else {
            qCDebug(KCM_KEYBOARD) << "Watching kxkbrc at" << kxkbrc;
        }
    }
}

void KeyboardDaemon::unregisterListeners()
{
    if (xEventNotifier != nullptr) {
        xEventNotifier->stop();
        disconnect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureInput);
        disconnect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
        disconnect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChangedSlot);
        disconnect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, nullptr);
    }
    if (keyboardFileWatcher != nullptr) {
        disconnect(keyboardFileWatcher, &QFileSystemWatcher::fileChanged, this, &KeyboardDaemon::kxkbrcChanged);
    }
    if (keyboardFileDebounce != nullptr) {
        disconnect(keyboardFileDebounce, &QTimer::timeout, this, &KeyboardDaemon::kxkbrcDebounceTimeout);
        keyboardFileDebounce->stop();
    }
    if (layoutMapDebounce != nullptr) {
        layoutMapDebounce->stop();
    }
}

QString KeyboardDaemon::kxkbrcPath() const
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    if (configDir.isEmpty()) {
        return QString();
    }
    return QDir(configDir).filePath(QStringLiteral("kxkbrc"));
}

void KeyboardDaemon::kxkbrcChanged(const QString &path)
{
    Q_UNUSED(path);
    qCDebug(KCM_KEYBOARD) << "kxkbrc changed, restarting 1000 ms debounce timer";

    // QSaveFile replaces the file by rename, which can drop the watch.
    // Re-add immediately so we do not miss subsequent saves.
    const QString kxkbrc = kxkbrcPath();
    if (!kxkbrc.isEmpty() && !keyboardFileWatcher->files().contains(kxkbrc)) {
        if (!keyboardFileWatcher->addPath(kxkbrc)) {
            qCWarning(KCM_KEYBOARD) << "Failed to re-add kxkbrc watch at" << kxkbrc;
        } else {
            qCDebug(KCM_KEYBOARD) << "Re-added kxkbrc watch at" << kxkbrc;
        }
    }

    keyboardFileDebounce->start();
}

void KeyboardDaemon::kxkbrcDebounceTimeout()
{
    const QString kxkbrc = kxkbrcPath();
    if (kxkbrc.isEmpty() || !QFile::exists(kxkbrc)) {
        qCWarning(KCM_KEYBOARD) << "kxkbrc is missing at debounce timeout; skipping configureKeyboard";
        if (!kxkbrc.isEmpty() && !keyboardFileWatcher->files().contains(kxkbrc)) {
            keyboardFileWatcher->addPath(kxkbrc); // re-arm for when the file reappears
        }
        return;
    }

    qCDebug(KCM_KEYBOARD) << "kxkbrc debounce timer fired, calling configureKeyboard";
    configureKeyboard();

    // After configureKeyboard returns, make sure the watch is still in place.
    if (!keyboardFileWatcher->files().contains(kxkbrc)) {
        if (!keyboardFileWatcher->addPath(kxkbrc)) {
            qCWarning(KCM_KEYBOARD) << "Failed to re-add kxkbrc watch after configureKeyboard at" << kxkbrc;
        }
    }
}

void KeyboardDaemon::layoutChangedSlot()
{
    layoutMemory.layoutChanged();

    Q_EMIT layoutChanged(getLayout());
    broadcastLayoutChanged(getLayout());
}

void KeyboardDaemon::switchToNextLayout()
{
    setLastUsedLayoutValue(getLayout());
    X11Helper::scrollLayouts(1);
    Q_EMIT layoutChanged(getLayout());
    broadcastLayoutChanged(getLayout());
}

void KeyboardDaemon::switchToPreviousLayout()
{
    setLastUsedLayoutValue(getLayout());
    X11Helper::scrollLayouts(-1);
    Q_EMIT layoutChanged(getLayout());
    broadcastLayoutChanged(getLayout());
}

void KeyboardDaemon::broadcastLayoutsChanged()
{
    if (m_ipcServer) {
        m_ipcServer->broadcastLayoutsChanged(getLayoutsList());
    }
}

void KeyboardDaemon::broadcastLayoutChanged(uint index)
{
    if (m_ipcServer) {
        m_ipcServer->broadcastLayoutChanged(index);
    }
}

bool KeyboardDaemon::setLayout(QAction *action)
{
    if (action == actionCollection->getToggleAction())
        return false;

    if (action == actionCollection->getLastUsedLayoutAction())
        return false;

    return setLayout(action->data().toUInt());
}

bool KeyboardDaemon::setLayout(uint index)
{
    if (keyboardSettings->layoutLoopCount() != KeyboardConfig::NO_LOOPING && index >= uint(keyboardSettings->layoutLoopCount())) {
        QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
        const uint indexOfLastMainLayoutInConfig = keyboardConfig->layouts().lastIndexOf(layouts.takeLast());
        const uint indexOfLastMainLayoutInXKB = layouts.size();

        // Re-calculate indexes for layout switching Actions
        const auto &actions = actionCollection->actions();
        for (const auto &action : actions) {
            // clang-format off
            if (action->data().toUInt() == indexOfLastMainLayoutInXKB) {
                action->setData(indexOfLastMainLayoutInConfig < index ?
                                    indexOfLastMainLayoutInConfig + 1 :
                                    indexOfLastMainLayoutInConfig);
            } else if (action->data().toUInt() == index) {
                action->setData(indexOfLastMainLayoutInXKB);
            } else if (index < indexOfLastMainLayoutInConfig
                       && index < action->data().toUInt() && action->data().toUInt() <= indexOfLastMainLayoutInConfig) {
                action->setData(action->data().toUInt() - 1);
            } else if (indexOfLastMainLayoutInConfig < index
                       && indexOfLastMainLayoutInConfig < action->data().toUInt() && action->data().toUInt() < index) {
                action->setData(action->data().toUInt() + 1);
            }
            // clang-format on
        }

        if (index <= indexOfLastMainLayoutInConfig) {
            // got to a shifted diapason due to previously selected spare layout, so adjusting the index accordingly
            --index;
        }
        // spare layout preempts last one in the loop
        layouts.append(keyboardConfig->layouts().at(index));
        XkbHelper::initializeKeyboardLayouts(layouts);
        index = indexOfLastMainLayoutInXKB;
    }
    setLastUsedLayoutValue(getLayout());
    return X11Helper::setGroup(index);
}

uint KeyboardDaemon::getLayout() const
{
    return X11Helper::getGroup();
}

QList<LayoutNames> KeyboardDaemon::getLayoutsList() const
{
    QList<LayoutNames> ret;

    auto layoutsList = X11Helper::getLayoutsList();
    if (keyboardSettings->layoutLoopCount() != KeyboardConfig::NO_LOOPING) {
        // extra layouts list overlaps with the main layouts loop initially by 1 position
        auto extraLayouts = keyboardConfig->layouts().mid(keyboardSettings->layoutLoopCount() - 1);
        // spare layout currently placed in the loop is removed from the extra layouts
        // as it was already "moved" to the last loop position
        extraLayouts.removeOne(layoutsList.last());
        layoutsList.append(extraLayouts);
    }
    for (auto &layoutUnit : std::as_const(layoutsList)) {
        QString displayName = layoutUnit.getDisplayName();
        const auto configDefaultLayouts = keyboardConfig->getDefaultLayouts();
        auto it = std::find(configDefaultLayouts.begin(), configDefaultLayouts.end(), layoutUnit);
        if (it != configDefaultLayouts.end()) {
            displayName = it->getDisplayName();
        } else {
            const auto configExtraLayouts = keyboardConfig->getExtraLayouts();
            it = std::find(configExtraLayouts.begin(), configExtraLayouts.end(), layoutUnit);
            if (it != configExtraLayouts.end()) {
                displayName = it->getDisplayName();
            }
        }
        ret.append({layoutUnit.layout(), displayName, Flags::getLongText(layoutUnit)});
    }
    return ret;
}

void KeyboardDaemon::setLastUsedLayoutValue(uint newValue)
{
    auto layoutsList = X11Helper::getLayoutsList();
    if (layoutsList.count() > 1) {
        lastUsedLayout = std::optional<uint>{newValue};
    }
}

#include "keyboard_daemon.moc"
