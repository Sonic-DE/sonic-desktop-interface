/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

#include "joybutton.h"

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

    // TODO: temporary event loop
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &JoyDevice::poll);
    timer->start(100);
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
    }
}
