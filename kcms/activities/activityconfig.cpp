/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "activityconfig.h"

#include "common/dbus/common.h"
#include "features_interface.h"
#include "utils/continue_with.h"

#include <KActivities/Info>

#include <KGlobalAccel>
#include <KLocalizedString>

#include <QAction>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

ActivityConfig::ActivityConfig(QObject *parent)
    : QObject(parent)
    , m_features(new org::kde::ActivityManager::Features(QStringLiteral(KAMD_DBUS_SERVICE),
                                                         QStringLiteral(KAMD_DBUS_FEATURES_PATH),
                                                         QDBusConnection::sessionBus(),
                                                         this))
{
    reset();
}

ActivityConfig::~ActivityConfig()
{
    delete m_features;
}

void ActivityConfig::reset()
{
    m_name = QString();
    m_description = QString();
    m_iconName = QStringLiteral("activities");
    m_private = false;
    m_shortcut = QKeySequence();
}

QString ActivityConfig::activityId() const
{
    return m_activityId;
}

void ActivityConfig::setActivityId(const QString &activityId)
{
    if (activityId == m_activityId) {
        return;
    }

    m_activityId = activityId;
    Q_EMIT activityIdChanged();

    load();
}

void ActivityConfig::load()
{
    if (m_activityId.isEmpty()) {
        reset();
        Q_EMIT infoChanged();
        return;
    }

    KActivities::Info activityInfo(m_activityId);
    m_name = activityInfo.name();
    m_description = activityInfo.description();
    m_iconName = activityInfo.icon();

    // finding the key shortcut
    const auto shortcuts = KGlobalAccel::self()->globalShortcut(QStringLiteral("ActivityManager"), QStringLiteral("switch-to-activity-%1").arg(m_activityId));
    m_shortcut = shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();

    // is private?
    auto result = m_features->GetValue(QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/%1").arg(m_activityId));
    auto watcher = new QDBusPendingCallWatcher(result, this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [&](QDBusPendingCallWatcher *watcher) mutable {
        QDBusPendingReply<QDBusVariant> reply = *watcher;
        m_private = reply.value().variant().toBool();
        Q_EMIT infoChanged();
        watcher->deleteLater();
    });

    Q_EMIT infoChanged();
}

void ActivityConfig::save()
{
    if (m_activityId.isEmpty()) {
        createActivity();
        return;
    }

    m_activities.setActivityName(m_activityId, m_name);
    m_activities.setActivityDescription(m_activityId, m_description);
    m_activities.setActivityIcon(m_activityId, m_iconName);

    // setting the key shortcut
    QAction action(nullptr);
    action.setProperty("isConfigurationAction", true);
    action.setProperty("componentName", QStringLiteral("ActivityManager"));
    action.setObjectName(QStringLiteral("switch-to-activity-%1").arg(m_activityId));
    KGlobalAccel::self()->setShortcut(&action, {m_shortcut}, KGlobalAccel::NoAutoloading);

    // is private?
    m_features->SetValue(QStringLiteral("org.kde.ActivityManager.Resources.Scoring/isOTR/%1").arg(m_activityId), QDBusVariant(m_private));
}

void ActivityConfig::createActivity()
{
    using namespace kamd::utils;
    continue_with(KActivities::Controller().addActivity(m_name), [this](const optional_view<QString> &activityId) {
        if (activityId.is_initialized()) {
            m_activityId = activityId.get();
            save();
        }
    });
}
