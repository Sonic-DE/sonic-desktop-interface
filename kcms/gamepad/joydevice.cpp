/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

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

JoyDevice::JoyDevice(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent)
    : QObject(parent)
    , m_model(i18n("Unknown Model"))
    , m_joystick(joystick)
    , m_gameController(controller)
{
    m_name = SDL_JoystickName(m_joystick);

    m_vendor = SDL_JoystickGetVendor(m_joystick);

    qDebug() << "joydevice vendor: " << m_vendor;

    m_numButtons = SDL_JoystickNumButtons(joystick);
    m_numAxes = SDL_JoystickNumAxes(joystick);
    m_hasRumble = SDL_JoystickHasRumble(joystick);

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        if (SDL_GameControllerHasButton(m_gameController, (SDL_GameControllerButton)i)) {
            m_buttons.push_back(new JoyButton(m_vendor, i, this));
        }
    }

    QMap<int, QString> axesToCheck = {// Only check if we have the X axes, since we should always have the Y as well
                                      {SDL_CONTROLLER_AXIS_LEFTX, i18nc("Left joystick name", "Left Thumb")},
                                      {SDL_CONTROLLER_AXIS_RIGHTX, i18nc("Right joystick name", "Right Thumb")},
                                      {SDL_CONTROLLER_AXIS_TRIGGERLEFT, i18nc("Left trigger button name", "Left Trigger")},
                                      {SDL_CONTROLLER_AXIS_TRIGGERRIGHT, i18nc("Right trigger button name", "Right Trigger")}};

    for (int i : axesToCheck.keys()) {
        if (SDL_GameControllerHasAxis(m_gameController, (SDL_GameControllerAxis)i)) {
            QString name = axesToCheck.value(i);
            bool is2D = (i < 3); // Triggers are only 1D
            m_axes.insert(i, new JoyAxis(name, i, is2D, this));
        }
    }

    // TODO: temporary event loop
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &JoyDevice::poll);
    timer->start(1);
}

JoyDevice::JoyDevice()
    : m_model(i18n("Unknown Model"))
{
}

void JoyDevice::poll()
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
        if (i < 3) {
            // Get grid data
            int16_t x = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)i);
            int16_t y = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)(i + 1));
            float xFloat = (float)x / (float)32767;
            float yFloat = (float)y / (float)32767;
            QVector2D gridValue(xFloat, yFloat);
            m_axes.value(i)->setGridValue(gridValue);
        } else {
            // Get trigger data
            int16_t value = SDL_GameControllerGetAxis(m_gameController, (SDL_GameControllerAxis)i);
            float floatValue = (float)value / 32767;
            m_axes.value(i)->setValue(floatValue);
        }

        Q_EMIT axisStateChanged(i);
    }
}
