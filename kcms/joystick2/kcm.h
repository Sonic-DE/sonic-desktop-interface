/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2019 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include <KNSCore/EntryWrapper>
#include <KPackage/Package>
#include <KQuickAddons/ManagedConfigModule>

class QStandardItemModel;
class QSortFilterProxyModel;
class SplashScreenSettings;
class SplashScreenData;

class KCMJoystick : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

public:
    KCMJoystick(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args);
};
