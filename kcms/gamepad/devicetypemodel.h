/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QList>
#include <QStandardItemModel>

class DeviceTypeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum CustomRoles {
        NameRole = Qt::UserRole + 1, // Name shown in the gui for selection
        QMLName, // Which gamepadtype qml file to load
    };

    DeviceTypeModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString qmlName(int row);

private:
    void addType(int type, const QString &guiName, const QString &qmlName);

    // Map of SDL type enum, to gamepad name and qml name
    QMap<int, std::pair<QString, QString>> m_types;
};
