/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "buttonmodel.h"

#include "joydevice.h"
#include <SDL2/SDL_joystick.h>

ButtonModel::ButtonModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // TODO: what is this?
    connect(this, &ButtonModel::deviceChanged, this, [this] {
        connect(m_device, &JoyDevice::buttonStateChanged, this, [this](int index) {
            const QModelIndex changedIndex = this->index(index, 1);
            Q_EMIT dataChanged(changedIndex, changedIndex, {Qt::DisplayRole});
        });
    });
}

int ButtonModel::rowCount(const QModelIndex &) const
{
    return SDL_JoystickNumButtons(m_device->getJoystick());
}

int ButtonModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant ButtonModel::data(const QModelIndex &index, int role) const
{
    if (index.column() == 0) {
        switch (role) {
        case Qt::DisplayRole:
            return QString::number(index.row());
        default:
            break;
        }
    } else {
        switch (role) {
        case Qt::DisplayRole:
            return SDL_JoystickGetButton(m_device->getJoystick(), index.row());
        default:
            break;
        }
    }

    return QVariant();
}

QHash<int, QByteArray> ButtonModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}};
}

QVariant ButtonModel::headerData(int section, Qt::Orientation orientation, int role) const
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