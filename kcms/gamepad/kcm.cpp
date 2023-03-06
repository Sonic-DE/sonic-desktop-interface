/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"

#include <KPluginFactory>

#include <QProcess>
#include <QStandardPaths>

#include "axesmodel.h"
#include "buttonmodel.h"
#include "devicemodel.h"
#include "gamepad.h"
#include "gamepadbutton.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMJoystickFactory, "kcm_gamepad.json", registerPlugin<KCMGamePad>();)

const QLatin1String s_nonePluginName("None");

KCMGamePad::KCMGamePad(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
{
    qmlRegisterType<DeviceModel>("org.kde.plasma.gamepad.kcm", 1, 0, "DeviceModel");
    qmlRegisterType<Gamepad>("org.kde.plasma.gamepad.kcm", 1, 0, "Gamepad");
    qmlRegisterType<GamepadButton>("org.kde.plasma.gamepad.kcm", 1, 0, "GamepadButton");
    qmlRegisterType<GamepadStick>("org.kde.plasma.gamepad.kcm", 1, 0, "GamepadStick");
    qmlRegisterType<AxesModel>("org.kde.plasma.gamepad.kcm", 1, 0, "AxesModel");
    qmlRegisterType<ButtonModel>("org.kde.plasma.gamepad.kcm", 1, 0, "ButtonModel");
}

#include "kcm.moc"
#include "moc_kcm.cpp"
