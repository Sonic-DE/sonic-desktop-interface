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

Gamepad::Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent)
    : QObject(parent)
    , m_joystick(joystick)
    , m_gameController(controller)
    , m_model(i18n("Unknown Model"))
{
    m_name = SDL_JoystickName(m_joystick);

    m_vendor = SDL_JoystickGetVendor(m_joystick);

    qDebug() << "Gamepad vendor: " << m_vendor;

    m_numButtons = SDL_JoystickNumButtons(joystick);
    m_numAxes = SDL_JoystickNumAxes(joystick);
    m_hasRumble = SDL_JoystickHasRumble(joystick);

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
            m_triggers.insert(i, new GamepadTrigger(name, i, this));
        }
    }
    // TODO: temporary event loop
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Gamepad::poll);
    timer->start(1);
}

Gamepad::Gamepad()
    : m_model(i18n("Unknown Model"))
{
}

void Gamepad::poll()
{
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) { }

    for (int i = 0; i < m_buttons.size(); i++) {
        if (SDL_GameControllerGetButton(m_gameController, (SDL_GameControllerButton)m_buttons[i]->m_code) == 1) {
            m_buttons[i]->setState(true);
        } else {
            m_buttons[i]->setState(false);
        }

        Q_EMIT buttonStateChanged(i);
    }

    for (int i : m_axes.keys()) {
        // Get grid data
        int16_t x = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)i);
        int16_t y = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)(i + 1));
        float xFloat = (float)x / (float)32767;
        float yFloat = (float)y / (float)32767;
        QVector2D gridValue(xFloat, yFloat);
        m_axes.value(i)->setGridValue(gridValue);

        Q_EMIT axisStateChanged(i);
    }

    for (int i : m_triggers.keys()) {
        // Get trigger data
        int16_t value = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)i);
        float floatValue = (float)value / 32767;
        m_triggers.value(i)->setValue(floatValue);
    }
}
