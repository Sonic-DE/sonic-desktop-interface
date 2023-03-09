/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QStandardItemModel>
#include <Solid/Device>

class Gamepad;

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CustomRoles { NameRole = Qt::UserRole + 1, DeviceRole };

    DeviceModel();

    Q_INVOKABLE Gamepad *device(int index) const;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private slots:
    void poll();

private:
    void addDevice(const int deviceIndex);

    // Map of devices by their sdl index
    QList<Gamepad *> m_devices;
};
