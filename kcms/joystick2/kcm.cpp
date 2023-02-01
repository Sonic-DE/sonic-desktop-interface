/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kcm.h"

#include <KPluginFactory>

#include <QProcess>
#include <QStandardPaths>

#include "devicemodel.h"

K_PLUGIN_FACTORY_WITH_JSON(KCMJoystickFactory, "kcm_joystick2.json", registerPlugin<KCMJoystick>();)

const QLatin1String s_nonePluginName("None");

KCMJoystick::KCMJoystick(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, metaData, args)
{
    qmlRegisterType<DeviceModel>("org.kde.plasma.joystick.kcm", 1, 0, "DeviceModel");
}

#include "kcm.moc"
#include "moc_kcm.cpp"
