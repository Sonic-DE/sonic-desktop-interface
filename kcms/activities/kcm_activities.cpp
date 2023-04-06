/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kcm_activities.h"

#include <KAuthorized>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QMessageBox>
#include <QProcess>

#include "dialog.h"

#include <KActivities/Controller>

K_PLUGIN_CLASS_WITH_JSON(ActivitiesModule, "kcm_activities.json")

ActivitiesModule::ActivitiesModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickConfigModule(parent, metaData, args)
    , m_newActivityAuthorized(KAuthorized::authorize(QStringLiteral("plasma-desktop/add_activities")))
{
}

ActivitiesModule::~ActivitiesModule()
{
}

bool ActivitiesModule::newActivityAuthorized() const
{
    return m_newActivityAuthorized;
}

void ActivitiesModule::configureActivity(const QString &id)
{
    Dialog::showDialog(id);
}

void ActivitiesModule::newActivity()
{
    if (!m_newActivityAuthorized) {
        return;
    }
    Dialog::showDialog();
}

void ActivitiesModule::deleteActivity(const QString &id)
{
    if (!m_newActivityAuthorized) {
        return;
    }

    KActivities::Controller().removeActivity(id);
}

void ActivitiesModule::configureActivities()
{
    QProcess::startDetached(QStringLiteral("kcmshell5"), {QStringLiteral("activities")});
}

#include "kcm_activities.moc"
