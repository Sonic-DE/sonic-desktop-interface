/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicetypemodel.h"

#include <KLocalizedString>

#include <QDebug>

DeviceTypeModel::DeviceTypeModel()
{
    addType(i18nc("Generic gamepad controller name", "Generic Gamepad"), "GenericGamepad");
    addType(i18nc("Nintendo Switch Pro controller name", "Nintendo Switch Pro"), "SwitchProGamepad");
    addType(i18nc("Nintendo Joy-Con controller name", "Nintendo Joy-Con"), "JoyConGamepad");
    addType(i18nc("Playstation 3 controller name", "DualShock 3"), "PS3Gamepad");
    addType(i18nc("Playstation 4 controller name", "DualShock 4"), "PS4Gamepad");
    addType(i18nc("Playstation 5 controller name", "DualSense"), "PS5Gamepad");
    addType(i18nc("XBox controller name", "XBox Gamepad"), "XBoxGamepad");
    addType(i18nc("Valve steam deck controller name", "Steam Deck"), "SteamDeck");
    addType(i18nc("Valve steam controller name", "Steam Controller"), "SteamController");
}

int DeviceTypeModel::rowCount(const QModelIndex &parent) const
{
    return m_types.count();
}

QVariant DeviceTypeModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case CustomRoles::NameRole:
        return m_typeNames.at(index.row());
    case CustomRoles::QMLName:
        return m_types.value(m_typeNames.at(index.row()));
    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceTypeModel::roleNames() const
{
    return {{CustomRoles::NameRole, "name"}, {CustomRoles::QMLName, "qmlname"}};
}

QString DeviceTypeModel::qmlName(int row)
{
    if (row >= 0 && row < m_typeNames.count()) {
        qDebug() << "Getting qml name for type: " << m_typeNames.at(row);
        qDebug() << "Should give: " << m_types.value(m_typeNames.at(row));
        return m_types.value(m_typeNames.at(row));
    }
    return QString();
}

void DeviceTypeModel::addType(const QString &name, const QString &qmlName)
{
    m_typeNames.append(name);
    m_types.insert(name, qmlName);
}
