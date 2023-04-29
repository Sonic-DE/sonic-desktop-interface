/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kcm_activities.h"
#include "activityconfig.h"

#include <KActivities/Controller>
#include <KAuthorized>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QtQml/QQmlModuleRegistration>

K_PLUGIN_CLASS_WITH_JSON(ActivitiesModule, "kcm_activities.json")

ActivitiesModule::ActivitiesModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickConfigModule(parent, metaData, args)
    , m_newActivityAuthorized(KAuthorized::authorize(QStringLiteral("plasma-desktop/add_activities")))
{
    qmlRegisterType<ActivityConfig>("org.kde.kcms.activities", 1, 0, "ActivityConfig");

    if (!args.isEmpty()) {
        m_firstArgument = args.first().toString();
    }
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
    if (!id.isEmpty() && !KActivities::Controller().activities().contains(id)) {
        qWarning() << "Cannot configure activity. There is no activity with id" << id;
        qWarning() << "List of Activities: " << KActivities::Controller().activities();
        return;
    }

    if (depth() > 1) {
        pop();
    }
    push(QStringLiteral("ActivityEditor.qml"), QVariantMap{{QStringLiteral("activityId"), id}});
}

void ActivitiesModule::newActivity()
{
    // Launch the editor with an empty string as activity id
    configureActivity(QString());
}

void ActivitiesModule::deleteActivity(const QString &id)
{
    if (!m_newActivityAuthorized) {
        return;
    }

    KActivities::Controller().removeActivity(id);
}

void ActivitiesModule::load()
{
    if (m_firstArgument.isEmpty()) {
        return;
    }

    if (m_firstArgument == QStringLiteral("newActivity")) {
        newActivity();
    } else {
        configureActivity(m_firstArgument);
    }

    m_firstArgument = QString();
}

#include "kcm_activities.moc"
