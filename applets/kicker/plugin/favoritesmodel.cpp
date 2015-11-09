/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "favoritesmodel.h"
#include "appentry.h"
#include "contactentry.h"
#include "fileentry.h"
#include "systementry.h"
#include "actionlist.h"

#include <QDebug>

#include <KLocalizedString>

#include <KActivitiesExperimentalStats/Terms>
#include <KActivitiesExperimentalStats/ResultModel>

namespace KAStats = KActivities::Experimental::Stats;

using namespace KAStats;
using namespace KAStats::Terms;

FavoritesModel::FavoritesModel(QObject *parent) : ForwardingModel(parent)
, m_enabled(true)
, m_maxFavorites(-1)
, m_dropPlaceholderIndex(-1)
{
    refresh();
}

FavoritesModel::~FavoritesModel()
{
}

QString FavoritesModel::description() const
{
    return i18n("Favorites");
}

QVariant FavoritesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const QString url = ForwardingModel::data(index, ResultModel::ResourceRole).toString();

    qDebug() << "URL" << url;

    // const casts are bad, but we can not achieve this
    // with the standard 'mutable' members for lazy evaluation
    const AbstractEntry *entry = const_cast<FavoritesModel*>(this)->favoriteFromId(url);

    if (!entry) {
        return "NULL!";
    }

    if (role == Qt::DisplayRole) {
        return entry->name();
    } else if (role == Qt::DecorationRole) {
        return entry->icon();
    } else if (role == Kicker::FavoriteIdRole) {
        return entry->id();
    } else if (role == Kicker::UrlRole) {
        return entry->url();
    } else if (role == Kicker::HasActionListRole) {
        return entry->hasActions();
    } else if (role == Kicker::ActionListRole) {
        return entry->actions();
    }

    return QVariant();
}

// int FavoritesModel::rowCount(const QModelIndex& parent) const
// {
// }

bool FavoritesModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= rowCount()) {
        return false;
    }

    const QString url = ForwardingModel::data(index(row, 0), ResultModel::ResourceRole).toString();

    return m_entries.contains(url) ? m_entries[url]->run(actionId, argument)
                                   : false;
}

bool FavoritesModel::enabled() const
{
    return m_enabled;
}

int FavoritesModel::maxFavorites() const
{
    return m_maxFavorites;
}

void FavoritesModel::setMaxFavorites(int max)
{
    if (m_maxFavorites != max)
    {
        m_maxFavorites = max;

        if (m_maxFavorites != -1 && m_favorites.count() > m_maxFavorites) {
            refresh();
        }

        emit maxFavoritesChanged();
    }
}

void FavoritesModel::setEnabled(bool enable)
{
    if (m_enabled != enable) {
        m_enabled = enable;

        emit enabledChanged();
    }
}

QStringList FavoritesModel::favorites() const
{
    return m_favorites;
}

void FavoritesModel::setFavorites(const QStringList& favorites)
{
    QStringList _favorites(favorites);
    _favorites.removeDuplicates();

    if (_favorites != m_favorites) {
        m_favorites = _favorites;
        refresh();
    }
}

bool FavoritesModel::isFavorite(const QString &id) const
{
    qDebug() << "isFavorite" << id;
    return m_entries.contains(validateUrl(id));
}

void FavoritesModel::addFavorite(const QString &id, int index)
{
    const QString url = validateUrl(id);

    qDebug() << "addFavorite" << url << index;
}

void FavoritesModel::removeFavorite(const QString &id)
{
    const QString url = validateUrl(id);

    qDebug() << "removeFavorite" << url;
}

// void FavoritesModel::moveRow(int from, int to)
// {
// }

int FavoritesModel::dropPlaceholderIndex() const
{
    return m_dropPlaceholderIndex;
}

void FavoritesModel::setDropPlaceholderIndex(int index)
{
    if (m_dropPlaceholderIndex != index) {
        m_dropPlaceholderIndex = index;

    }
}

AbstractModel *FavoritesModel::favoritesModel()
{
    return this;
}

void FavoritesModel::refresh()
{
    QObject *oldModel = sourceModel();

    auto query = LinkedResources
                    | Agent {
                        "org.kde.plasma.favorites.applications",
                        "org.kde.plasma.favorites.contacts",
                        "org.kde.plasma.favorites.system"
                      }
                    | Type::any()
                    | Activity::current()
                    | Limit(15);

    ResultModel *model = new ResultModel(query);

    QModelIndex index;

    if (model->canFetchMore(index)) {
        model->fetchMore(index);
    }

    setSourceModel(model);

    delete oldModel;
}

AbstractEntry *FavoritesModel::favoriteFromId(const QString &id)
{
    if (!m_entries.contains(id)) {
        const QUrl url(id);
        const QString &s = url.scheme();

        AbstractEntry *entry = nullptr;

        if (s == QStringLiteral("applications")
                || s == QStringLiteral("preferred")
                || (s.isEmpty() && id.contains(QStringLiteral(".desktop")))) {
            entry = new AppEntry(this, id);
        } else if (s == QStringLiteral("ktp")) {
            entry = new ContactEntry(this, id);
        } else if (url.isValid() && !url.scheme().isEmpty()) {
            entry = new FileEntry(this, url);
        } else {
            entry = new SystemEntry(this, id);
        }

        m_entries[id] = entry;
    }

    return m_entries[id];
}

QString FavoritesModel::validateUrl(const QString &url) const
{
    const QUrl qurl(url);
    const QString &s = qurl.scheme();

    return s.isEmpty() ? "applications://" + url : url;
}


