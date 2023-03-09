/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <QTimer>

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

#include "gamepad.h"

DeviceModel::DeviceModel()
{
    SDL_Init(SDL_INIT_GAMECONTROLLER);
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        addDevice(i);
    }

    // TODO: temporary event loop
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &DeviceModel::poll);
    // Use 100ms between polling for now
    timer->start(100);
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

void DeviceModel::poll()
{
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_CONTROLLERDEVICEADDED:
            addDevice(event.cdevice.which);
            break;

        case SDL_CONTROLLERDEVICEREMOVED:
            //            RemoveController( event.cdevice );
            break;

        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            // Tell that controller to process it's event
            m_devices.at(event.cbutton.which)->onButtonEvent(event.cbutton);
            break;

        case SDL_CONTROLLERAXISMOTION:
            // Tell that gamepad to process it's event
            m_devices.at(event.caxis.which)->onAxisEvent(event.caxis);
            break;
        }
    }

//    for (int i : m_triggers.keys()) {
        // Get trigger data
//        int16_t value = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)i);
//        float floatValue = (float)value / 32767;
//        m_triggers.value(i)->setValue(floatValue);
//    }
}

void DeviceModel::addDevice(const int deviceIndex)
{
    m_devices.push_back(new Gamepad(SDL_JoystickOpen(deviceIndex), SDL_GameControllerOpen(deviceIndex), this));
}
