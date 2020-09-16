
#include "keysdata.h"

#include <KGlobalAccel>
#include <kglobalaccel_interface.h>
#include <kglobalaccel_component_interface.h>
#include <KGlobalShortcutInfo>
#include <KPluginFactory>
#include <KStandardShortcut>

// Short timeout, rather fail than block isDefaults for to long which needs to be sync
constexpr int dbusTimeout = 5; //milliseconds

KeysData::KeysData(QObject *parent, const QVariantList &args)
{
}

bool KeysData::isDefaults() const
{
   for (int i = KStandardShortcut::AccelNone + 1; i < KStandardShortcut::StandardShortcutCount; ++i) {
        const auto id = static_cast<KStandardShortcut::StandardShortcut>(i);
        const QList<QKeySequence> activeShortcuts = KStandardShortcut::shortcut(id);
        const QList<QKeySequence> defaultShortcuts = KStandardShortcut::hardcodedDefaultShortcut(id);
        if (activeShortcuts != defaultShortcuts) {
            return false;
        }
    }

    // need to do this blocking
    KGlobalAccelInterface globalAccelInterface(QStringLiteral("org.kde.kglobalaccel"),
        QStringLiteral("/kglobalaccel"), QDBusConnection::sessionBus());
    globalAccelInterface.setTimeout(dbusTimeout);
    if (!globalAccelInterface.isValid()) {
        return true;
    }
    auto componentsCall = globalAccelInterface.allComponents();
    componentsCall.waitForFinished();
    if (componentsCall.isError()) {
        return true;
    }
    const auto components = componentsCall.value();
    for (const auto &componentPath : components) {
        KGlobalAccelComponentInterface component(globalAccelInterface.service(), componentPath.path(),
            QDBusConnection::sessionBus());
        component.setTimeout(dbusTimeout);
        if (!component.isValid()) {
            return true;
        }
        auto allShortcutsCall = component.allShortcutInfos();
        allShortcutsCall.waitForFinished();
        if (allShortcutsCall.isError()) {
            return true;
        }
        const auto allShortcuts = allShortcutsCall.value();
        for (const auto &shortcutInfo : allShortcuts) {
            if (shortcutInfo.defaultKeys() != shortcutInfo.keys()) {
                return false;
            }
        }
    }
    return true;
}


#include "keysdata.moc"
