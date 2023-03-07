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
        NameRole = Qt::UserRole + 1,
        QMLName, // Which gamepadtype qml file to load
    };

    enum Type {
        Generic,
        NintendoSwitchPro,
        NintendoJoyCon,
        PS3,
        PS4,
        PS5,
        XBox,
        SteamDeck,
        SteamController,
    };
    Q_ENUMS(Type)

    DeviceTypeModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE QString qmlName(int row);

private:
    void addType(const QString &type, const QString &qmlName);

    // List to maintain order of insertion
    QStringList m_typeNames;
    // Map of type names to layout names
    QMap<QString, QString> m_types;
};
