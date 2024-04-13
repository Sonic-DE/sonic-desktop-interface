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
#include <QProcess>

#include <KPluginFactory>

#include "flags.h"
#include "keyboard_hardware.h"
#include "layout_memory_persister.h"
#include "x11_helper.h"
#include "xinput_helper.h"
#include "xkb_helper.h"
#include "xkb_rules.h"

K_PLUGIN_CLASS_WITH_JSON(KeyboardDaemon, "kded_keyboard.json")

KeyboardDaemon::KeyboardDaemon(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
    , keyboardConfig(new KeyboardConfig(this))
    , actionCollection(nullptr)
    , xEventNotifier(nullptr)
    , layoutMemory(*keyboardConfig)
    , rules(Rules::readRules(Rules::READ_EXTRAS))
{
    if (!X11Helper::xkbSupported(nullptr))
        return; // TODO: shut down the daemon?

    lastUsedLayouts = QList<uint>();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService(KEYBOARD_DBUS_SERVICE_NAME);
    dbus.registerObject(KEYBOARD_DBUS_OBJECT_PATH, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    dbus.connect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));

    LayoutNames::registerMetaType();

    configureKeyboard();
    registerListeners();

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

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.disconnect(QString(), KEYBOARD_DBUS_OBJECT_PATH, KEYBOARD_DBUS_SERVICE_NAME, KEYBOARD_DBUS_CONFIG_RELOAD_MESSAGE, this, SLOT(configureKeyboard()));
    dbus.unregisterObject(KEYBOARD_DBUS_OBJECT_PATH);
    dbus.unregisterService(KEYBOARD_DBUS_SERVICE_NAME);

    unregisterListeners();
    unregisterShortcut();

    delete xEventNotifier;
    delete rules;
}

void KeyboardDaemon::configureKeyboard()
{
    qCDebug(KCM_KEYBOARD) << "Configuring keyboard";
    init_keyboard_hardware();

    keyboardConfig->load();
    XkbHelper::initializeKeyboardLayouts(*keyboardConfig);
    layoutMemory.configChanged();

    unregisterShortcut();
    registerShortcut();
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
            switchToNextLayout();

            LayoutUnit newLayout = X11Helper::getCurrentLayout();
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                              QStringLiteral("/org/kde/osdService"),
                                                              QStringLiteral("org.kde.osdService"),
                                                              QStringLiteral("kbdLayoutChanged"));
            msg << Flags::getLongText(newLayout, rules);
            QDBusConnection::sessionBus().asyncCall(msg);
        });

        QAction *lastUsedLayoutAction = actionCollection->getLastUsedLayoutAction();
        connect(lastUsedLayoutAction, &QAction::triggered, this, [this]() {
            if (lastUsedLayouts.size() < 2) {
                switchToPreviousLayout();
            } else {
                setLayout(lastUsedLayouts.at(lastUsedLayouts.size()-2));
            }

            LayoutUnit newLayout = X11Helper::getCurrentLayout();
            QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"),
                                                              QStringLiteral("/org/kde/osdService"),
                                                              QStringLiteral("org.kde.osdService"),
                                                              QStringLiteral("kbdLayoutChanged"));
            msg << Flags::getLongText(newLayout, rules);
            QDBusConnection::sessionBus().asyncCall(msg);
        });

        actionCollection->loadLayoutShortcuts(keyboardConfig->layouts, rules);
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
    connect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
    connect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChangedSlot);
    xEventNotifier->start();
}

void KeyboardDaemon::unregisterListeners()
{
    if (xEventNotifier != nullptr) {
        xEventNotifier->stop();
        disconnect(xEventNotifier, &XInputEventNotifier::newPointerDevice, this, &KeyboardDaemon::configureInput);
        disconnect(xEventNotifier, &XInputEventNotifier::newKeyboardDevice, this, &KeyboardDaemon::configureKeyboard);
        disconnect(xEventNotifier, &XEventNotifier::layoutChanged, this, &KeyboardDaemon::layoutChangedSlot);
        disconnect(xEventNotifier, &XEventNotifier::layoutMapChanged, this, &KeyboardDaemon::layoutMapChanged);
    }
}

void KeyboardDaemon::layoutChangedSlot()
{
    layoutMemory.layoutChanged();
    updateLastUsedLayouts();
    Q_EMIT layoutChanged(getLayout());
}

void KeyboardDaemon::layoutMapChanged()
{
    keyboardConfig->load();
    updateLastUsedLayouts();
    layoutMemory.layoutMapChanged();
    Q_EMIT layoutListChanged();
}

void KeyboardDaemon::switchToNextLayout()
{
    X11Helper::scrollLayouts(1);
}

void KeyboardDaemon::switchToPreviousLayout()
{
    X11Helper::scrollLayouts(-1);
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
    QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
    const int posInCurSett = layouts.lastIndexOf(keyboardConfig->layouts.at(index));
    if (keyboardConfig->layoutLoopCount() != KeyboardConfig::NO_LOOPING && posInCurSett < 0) {
        layouts.takeLast();
        const uint indexOfLastMainLayoutInXKB = layouts.size();
        // spare layout preempts last one in the loop
        layouts.append(keyboardConfig->layouts.at(index));
        XkbHelper::initializeKeyboardLayouts(layouts);
        index = indexOfLastMainLayoutInXKB;
    } else if (keyboardConfig->layoutLoopCount() != KeyboardConfig::NO_LOOPING && posInCurSett >= 0) {
        index = posInCurSett;
    }
    return X11Helper::setGroup(index);
}

uint KeyboardDaemon::getLayout() const
{
    QList<LayoutUnit> layouts = X11Helper::getLayoutsList();
    const int pos = keyboardConfig->layouts.lastIndexOf(layouts.at(X11Helper::getGroup()));
    return ((pos >= 0) ? pos : 0);
}

QList<LayoutNames> KeyboardDaemon::getLayoutsList() const
{
    QList<LayoutNames> ret;

    auto layoutsList = keyboardConfig->layouts;
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
        ret.append({layoutUnit.layout(), displayName, Flags::getLongText(layoutUnit, rules)});
    }
    return ret;
}

void KeyboardDaemon::updateLastUsedLayouts() {
    qCDebug(KCM_KEYBOARD) << "KeyboardDaemon::updateLastUsedLayouts()_1:" << lastUsedLayouts;
    if (lastUsedLayouts.size() == 0) {
        lastUsedLayouts.append(0);
    }
    if (lastUsedLayouts.at(lastUsedLayouts.size() - 1) != getLayout()) {
        lastUsedLayouts.append(getLayout());
    }
    if (lastUsedLayouts.size() > 2) {
        lastUsedLayouts.removeFirst();
    }
    qCWarning(KCM_KEYBOARD) << "KeyboardDaemon::updateLastUsedLayouts()_1:" << lastUsedLayouts;
}

#include "keyboard_daemon.moc"
