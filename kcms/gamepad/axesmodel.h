/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QAbstractTableModel>

#include "gamepad.h"

class AxesModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(Gamepad *device MEMBER m_device NOTIFY deviceChanged REQUIRED)

public:
    AxesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &) const override;
    int columnCount(const QModelIndex &) const override;

    QVariant data(const QModelIndex &index, int role) const override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

signals:
    void deviceChanged();

private:
    Gamepad *m_device = nullptr;
};
