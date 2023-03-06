/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "axesmodel.h"

#include "gamepad.h"
#include <SDL2/SDL_joystick.h>

AxesModel::AxesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // TODO: no seriously, what is this
    connect(this, &AxesModel::deviceChanged, this, [this] {
        connect(m_device, &Gamepad::axisStateChanged, this, [this](int index) {
            const QModelIndex changedIndex = this->index(index, 1);
            Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
        });
    });
}

int AxesModel::rowCount(const QModelIndex &) const
{
    return SDL_JoystickNumAxes(m_device->getJoystick());
}

int AxesModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant AxesModel::data(const QModelIndex &index, int role) const
{
    if (index.column() == 0) {
        switch (role) {
        case Qt::DisplayRole:
            qDebug() << index.row();
            return QString::number(index.row());
        default:
            break;
        }
    } else {
        switch (role) {
        case Qt::DisplayRole:
            return SDL_JoystickGetAxis(m_device->getJoystick(), index.row());
        default:
            break;
        }
    }

    return QVariant();
}

QHash<int, QByteArray> AxesModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}};
}

QVariant AxesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0:
            return QString("Index");
        case 1:
            return QString("State");
        }
    }
    return QVariant();
}
