/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicetypemodel.h"

#include <KLocalizedString>

#include <QDebug>

#include <SDL2/SDL_gamecontroller.h>

// TODO: confirm actual usb ids
const uint16_t USB_ID_SONY = 0x0a51;
const uint16_t USB_ID_SONY2 = 0x054c;
const uint16_t USB_ID_MICROSOFT = 0x045e;
const uint16_t USB_ID_NINTENDO = 0x057e;

#define kGenericName "GenericGamepad"
#define kXBox360Name "XBox360Gamepad"
#define kXBoxName "XBoxGamepad"
#define kDS3Name "PS3Gamepad"
#define kDS4Name "PS4Gamepad"
#define kSwitchProName "SwitchProGamepad"
#define kVirtualName "VirtualGamepad"
#define kDS5Name "PS5Gamepad"
#define kLunaName "LunaGamepad"
#define kStadiaName "StadiaGamepad"
#define kShieldName "ShieldGamepad"
#define kLeftJoyconName "LeftJoyConGamepad"
#define kRightJoyconName "RightJoyConGamepad"
#define kJoyconPairName "JoyConPairGamepad"

// No SDL types, so unused for now
#define kWiimoteName "WiimoteGamepad"
#define kSteamDeckName "SteamDeck"
#define kSteamControllerName "SteamController"

DeviceTypeModel::DeviceTypeModel()
{
    addType(SDL_CONTROLLER_TYPE_UNKNOWN, i18nc("Generic gamepad controller name", "Generic Gamepad"), kGenericName);
    addType(SDL_CONTROLLER_TYPE_XBOX360, i18nc("XBox 360 controller name", "XBox 360 Gamepad"), kXBox360Name);
    addType(SDL_CONTROLLER_TYPE_XBOXONE, i18nc("XBox controller name", "XBox Gamepad"), kXBoxName);
    addType(SDL_CONTROLLER_TYPE_PS3, i18nc("Playstation 3 controller name", "DualShock 3"), kDS3Name);
    addType(SDL_CONTROLLER_TYPE_PS4, i18nc("Playstation 4 controller name", "DualShock 4"), kDS4Name);
    addType(SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO, i18nc("Nintendo Switch Pro controller name", "Nintendo Switch Pro"), kSwitchProName);
    addType(SDL_CONTROLLER_TYPE_VIRTUAL, i18nc("Virtual controller name", "Virtual Gamepad"), kVirtualName);
    addType(SDL_CONTROLLER_TYPE_PS5, i18nc("Playstation 5 controller name", "DualSense"), kDS5Name);
    addType(SDL_CONTROLLER_TYPE_AMAZON_LUNA, i18nc("Amazon Luna controller name", "Luna Gamepad"), kLunaName);
    addType(SDL_CONTROLLER_TYPE_GOOGLE_STADIA, i18nc("Google Stadia controller name", "Stadia Gamepad"), kStadiaName);
    addType(SDL_CONTROLLER_TYPE_NVIDIA_SHIELD, i18nc("NVIDIA Shield controller name", "NVIDIA Shield Gamepad"), kShieldName);
    addType(SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT, i18nc("Nintendo Left Joy-Con controller name", "Nintendo Joy-Con L"), kLeftJoyconName);
    addType(SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT, i18nc("Nintendo Right Joy-Con controller name", "Nintendo Joy-Con R"), kRightJoyconName);
    addType(SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR, i18nc("Nintendo Switch Joy-Con pair controller name", "Nintendo Joy-Con Pair"), kJoyconPairName);
    // SDL doesn't have a type for wiimotes...
    // addType(SDL_CONTROLLER_TYPE_i18nc("Nintendo Wiimote name", "Nintendo Wiimote"), kWiimoteName);
    // Or for these 2 steam controller types (yet)
    // addType(SDL_CONTROLLER_TYPE_STEAMDECK, i18nc("Valve steam deck controller name", "Steam Deck"), kSteamDeckName);
    // addType(i18nc("Valve steam controller name", "Steam Controller"), kSteamControllerName);
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
        return m_types.value(index.row()).first;
    case CustomRoles::QMLName:
        return m_types.value(index.row()).second;
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
    if (m_types.contains(row)) {
        qDebug() << "Getting qml name for type: " << m_types.value(row).second;
        return m_types.value(row).second;
    }
    return QString();
}

void DeviceTypeModel::addType(int type, const QString &guiName, const QString &qmlName)
{
    m_types.insert(type, std::pair<QString, QString>(guiName, qmlName));
}
