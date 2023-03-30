/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepad.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>
#include <QTimer>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <Solid/Block>
#include <Solid/GenericInterface>
#include <csignal>
#include <fcntl.h>

#include "devicetypemodel.h"

Gamepad::Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent)
    : QObject(parent)
    , m_joystick(joystick)
    , m_gameController(controller)
    , m_model(i18n("Unknown Model"))
{
    m_name = SDL_JoystickName(m_joystick);

    m_vendor = SDL_JoystickGetVendor(m_joystick);

    qDebug() << "Gamepad vendor: " << Qt::hex << m_vendor;

    m_numButtons = SDL_JoystickNumButtons(joystick);
    m_numAxes = SDL_JoystickNumAxes(joystick);
    m_hasRumble = SDL_JoystickHasRumble(joystick);

    m_hasTouchPad = (SDL_GameControllerGetNumTouchpads(m_gameController) > 0);

    qDebug() << "Has touchpad: " << m_hasTouchPad;

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        if (SDL_GameControllerHasButton(m_gameController, (SDL_GameControllerButton)i)) {
            m_buttons.push_back(new GamepadButton(m_vendor, i, this));
        }
    }

    QMap<int, QString> axesToCheck = {// Only check if we have the X axes, since we should always have the Y as well
                                      {SDL_CONTROLLER_AXIS_LEFTX, i18nc("Left joystick name", "Left Thumb")},
                                      {SDL_CONTROLLER_AXIS_RIGHTX, i18nc("Right joystick name", "Right Thumb")}};

    QMap<int, QString> triggersToCheck = {{SDL_CONTROLLER_AXIS_TRIGGERLEFT, i18nc("Left trigger button name", "Left Trigger")},
                                          {SDL_CONTROLLER_AXIS_TRIGGERRIGHT, i18nc("Right trigger button name", "Right Trigger")}};

    for (int i : axesToCheck.keys()) {
        if (SDL_GameControllerHasAxis(m_gameController, (SDL_GameControllerAxis)i)) {
            QString name = axesToCheck.value(i);
            m_axes.insert(i, new GamepadStick(name, i, this));
        }
    }

    for (int i : triggersToCheck.keys()) {
        if (SDL_GameControllerHasAxis(m_gameController, (SDL_GameControllerAxis)i)) {
            QString name = triggersToCheck.value(i);
            m_triggers.insert(i, new GamepadTrigger(m_vendor, name, i, this));
        }
    }
}

Gamepad::Gamepad()
    : m_model(i18n("Unknown Model"))
{
}

SDL_GameControllerType Gamepad::gamepadType()
{
    // First check if user has specified one for this device by it's id

    // If not get the autodetected one from the devicetypemodel
    return SDL_GameControllerGetType(m_gameController);
}

void Gamepad::onButtonEvent(const SDL_ControllerButtonEvent sdlEvent)
{
    for (int i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i]->m_code == sdlEvent.button) {
            m_buttons[i]->setState(sdlEvent.type == SDL_CONTROLLERBUTTONDOWN);
            Q_EMIT buttonStateChanged(i);
            // Once we found the right button skip the rest
            return;
        }
    }
}

void Gamepad::onAxisEvent(const SDL_ControllerAxisEvent sdlEvent)
{
    switch (sdlEvent.axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_LEFTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_LEFTX)->setX((float)sdlEvent.value / (float)32767);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_LEFTX);
        }
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_LEFTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_LEFTX)->setY((float)sdlEvent.value / (float)32767);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_LEFTY);
        }
        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_RIGHTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_RIGHTX)->setX((float)sdlEvent.value / (float)32767);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_RIGHTX);
        }
        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_RIGHTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_RIGHTX)->setY((float)sdlEvent.value / (float)32767);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_RIGHTY);
        }
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        if (m_triggers.contains(sdlEvent.axis)) {
            m_triggers.value(sdlEvent.axis)->setValue((float)sdlEvent.value / (float)32767);
            Q_EMIT triggerStateChanged(sdlEvent.axis);
        }
        break;
    }
}
