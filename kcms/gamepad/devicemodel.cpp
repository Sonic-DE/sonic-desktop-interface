/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

#include "gamepad.h"

DeviceModel::DeviceModel()
{
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        addDevice(i);
    }
}

Gamepad *DeviceModel::device(int index) const
{
    if (index < 0 || index > m_devices.count())
        return nullptr;

    return m_devices.at(index);
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case CustomRoles::NameRole:
        return m_devices[index.row()]->getName();
    case CustomRoles::DeviceRole:
        return QVariant::fromValue(m_devices[index.row()]);
    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{CustomRoles::NameRole, "name"}, {CustomRoles::DeviceRole, "device"}};
}

void DeviceModel::addDevice(const int deviceIndex)
{
    m_devices.push_back(new Gamepad(SDL_JoystickOpen(deviceIndex), SDL_GameControllerOpen(deviceIndex), this));
}
