/*
 *   Copyright (C) 2016 Ivan Cukic <ivan.cukic(at)kde.org>
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

#ifndef SORTED_ACTIVITY_MODEL
#define SORTED_ACTIVITY_MODEL

// Qt
#include <QSortFilterProxyModel>
#include <KActivities/ActivitiesModel>
#include <KActivities/Consumer>
#include <KActivities/Info>

class SortedActivitiesModel : public QSortFilterProxyModel {
    Q_OBJECT

    Q_PROPERTY(bool sortByLastUsedTime READ sortByLastUsedTime WRITE setSortByLastUsedTime NOTIFY sortByLastUsedTimeChanged)

public:
    SortedActivitiesModel(QVector<KActivities::Info::State> states, QObject *parent = 0);
    virtual ~SortedActivitiesModel();

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const;

    QString relativeActivity(int relative) const;

protected:
    uint lastUsedTime(const QString &activity) const;
    bool lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const;

    enum AdditionalRoles {
        LastTimeUsed = Qt::UserRole + 6,
        LastTimeUsedString = Qt::UserRole + 7
    };

public Q_SLOTS:
    bool sortByLastUsedTime() const;
    void setSortByLastUsedTime(bool sortByLastUsedTime);

    void backgroundsUpdated(const QStringList &changedBackgrounds);

    QString activityIdForIndex(const QModelIndex &index) const;
    QString activityIdForRow(int row) const;

Q_SIGNALS:
    void sortByLastUsedTimeChanged(bool sortByLastUsedTime);

private:
    bool m_sortByLastUsedTime;
    KActivities::ActivitiesModel *m_activitiesModel;
    KActivities::Consumer *m_activities;
};

#endif // SORTED_ACTIVITY_MODEL

