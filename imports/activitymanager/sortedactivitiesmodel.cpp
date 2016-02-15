/*
 *   Copyright (C) 2016 <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Self
#include "sortedactivitiesmodel.h"

// Qt
#include <QDebug>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KActivities/Consumer>

SortedActivitiesModel::SortedActivitiesModel(QVector<KActivities::Info::State> states, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_sortByLastUsedTime(true)
    , m_activitiesModel(new KActivities::ActivitiesModel(states, this))
    , m_activities(new KActivities::Consumer(this))
{
    qDebug() << "SortedActivitiesModel" << (void*)this << "::constructor";

    setSourceModel(m_activitiesModel);

    setDynamicSortFilter(true);
    setSortRole(LastTimeUsed);
    sort(0, Qt::DescendingOrder);
}

SortedActivitiesModel::~SortedActivitiesModel()
{
}

bool SortedActivitiesModel::sortByLastUsedTime() const
{
    return m_sortByLastUsedTime;
}

void SortedActivitiesModel::setSortByLastUsedTime(bool sortByLastUsedTime)
{
    if (m_sortByLastUsedTime != sortByLastUsedTime) {
        // qDebug() << "SortedActivitiesModel" << (void*)this << "::setSortByLastUsedTime" << sortByLastUsedTime;

        m_sortByLastUsedTime = sortByLastUsedTime;

        if (m_sortByLastUsedTime) {
            setSortRole(LastTimeUsed);
        } else {
            setSortRole(Qt::DisplayRole);
        }
    }
}

uint SortedActivitiesModel::lastUsedTime(const QString &activity) const
{
    if (m_activities->currentActivity() == activity) {
        return 0;

    } else {
        KConfig config("kactivitymanagerd-switcher");
        KConfigGroup times(&config, "LastUsed");

        return times.readEntry(activity, (uint)0);
    }
}

bool SortedActivitiesModel::lessThan(const QModelIndex &sourceLeft,
                                     const QModelIndex &sourceRight) const
{
    if (m_sortByLastUsedTime) {
        const auto activityLeft  = sourceModel()->data(sourceLeft, KActivities::ActivitiesModel::ActivityId);
        const auto activityRight = sourceModel()->data(sourceRight, KActivities::ActivitiesModel::ActivityId);

        const auto timeLeft  = lastUsedTime(activityLeft.toString());
        const auto timeRight = lastUsedTime(activityRight.toString());

        // qDebug() << "SortedActivitiesModel" << (void*)this << " times " << timeLeft << timeRight;

        return timeLeft < timeRight;

    } else {
        const auto titleLeft  = sourceModel()->data(sourceLeft, KActivities::ActivitiesModel::ActivityName);
        const auto titleRight = sourceModel()->data(sourceRight, KActivities::ActivitiesModel::ActivityName);

        // qDebug() << "SortedActivitiesModel" << (void*)this << " titles " << titleLeft << titleRight;

        return titleLeft < titleRight;
    }
}

QHash<int, QByteArray> SortedActivitiesModel::roleNames() const
{
    if (!sourceModel()) return QHash<int, QByteArray>();

    auto roleNames = sourceModel()->roleNames();

    roleNames[LastTimeUsed]       = "lastTimeUsed";
    roleNames[LastTimeUsedString] = "lastTimeUsedString";

    return roleNames;
}

QVariant SortedActivitiesModel::data(const QModelIndex &index, int role) const
{
    if (role == LastTimeUsed || role == LastTimeUsedString) {
        const auto activity =
            QSortFilterProxyModel::data(index, Qt::UserRole).toString();

        const auto time = lastUsedTime(activity);

        // qDebug() << "SortedActivitiesModel" << (void*)this << " time for" << activity << " " << time;

        if (role == LastTimeUsed) {
            return QVariant(time);

        } else {
            const auto now = QDateTime::currentDateTime().toTime_t();

            if (time == 0) return i18n("Used some time ago");

            auto diff = now - time;

            // We do not need to be precise
            diff /= 60;
            const auto minutes = diff % 60; diff /= 60;
            const auto hours   = diff % 24; diff /= 24;
            const auto days    = diff % 30; diff /= 30;
            const auto months  = diff % 12; diff /= 12;
            const auto years   = diff;

            return (years > 0)   ? i18n("Used a long time ago")
                 : (months > 0)  ? i18ncp("amount in months",  "Used a month ago",  "Used %1 months ago", months)
                 : (days > 0)    ? i18ncp("amount in days",    "Used a day ago",    "Used %1 days ago",   days)
                 : (hours > 0)   ? i18ncp("amount in hours",   "Used an hour ago",  "Used %1 hours ago",  hours)
                 : (minutes > 0) ? i18ncp("amount in minutes", "Used a minute ago", "Used %1 minutes ago",  minutes)
                 :                 i18n("Used a moment ago");

        }

    } else {
        return QSortFilterProxyModel::data(index, role);
    }
}

QString SortedActivitiesModel::relativeActivity(int relative) const
{
    qDebug() << "SortedActivitiesModel" << (void*)this << ": relative" << relative;
    const auto currentActivity = m_activities->currentActivity();
    qDebug() << "SortedActivitiesModel" << (void*)this << ": sourceModel" << sourceModel() << " c act" << currentActivity;

    if (!sourceModel()) return QString();

    const auto currentRowCount = sourceModel()->rowCount();

    int currentActivityIndex = 0;
    qDebug() << "SortedActivitiesModel" << (void*)this << ": rowCount" << currentRowCount;

    for (; currentActivityIndex < currentRowCount; currentActivityIndex++) {
        qDebug() << "SortedActivitiesModel" << (void*)this
            << ": currentActivityIndex" << currentActivityIndex
            << data(index(currentActivityIndex, Qt::UserRole)).toString();
        if (data(index(currentActivityIndex, Qt::UserRole)).toString() == currentActivity) break;
    }

    qDebug() << "SortedActivitiesModel" << (void*)this << ": curr" << currentActivityIndex;

    currentActivityIndex = (currentActivityIndex + relative) % currentRowCount;

    qDebug() << "SortedActivitiesModel" << (void*)this << ": curr" << currentActivityIndex;

    return data(index(currentActivityIndex, Qt::UserRole)).toString();
}

