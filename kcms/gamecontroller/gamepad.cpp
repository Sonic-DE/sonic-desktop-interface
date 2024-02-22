/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepad.h"

#include <QTimer>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>

#include "devicetypemodel.h"
#include "logging.h"

Gamepad::Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent)
    : QObject(parent)
    , m_joystick(joystick)
    , m_gameController(controller)
    , m_model(i18nc("@label", "Unknown Model"))
    , m_connectionType(UnknownType)
{
    m_name = QString::fromLocal8Bit(SDL_JoystickName(m_joystick));
    m_path = QString::fromLocal8Bit(SDL_JoystickPath(m_joystick));

    m_vendor = SDL_JoystickGetVendor(m_joystick);

    m_numButtons = SDL_JoystickNumButtons(joystick);
    m_numAxes = SDL_JoystickNumAxes(joystick);
    m_hasRumble = SDL_JoystickHasRumble(joystick);

    m_hasTouchPad = (SDL_GameControllerGetNumTouchpads(m_gameController) > 0);

    const auto powerLevel = SDL_JoystickCurrentPowerLevel(m_joystick);
    m_connectionType = (powerLevel == SDL_JOYSTICK_POWER_WIRED ? USBType : BluetoothType);

    for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        if (SDL_GameControllerHasButton(m_gameController, static_cast<SDL_GameControllerButton>(i))) {
            m_buttons.insert(i, new GamepadButton(m_vendor, i, this));
        }
    }

    const QMap<int, QString> axesToCheck = {// Only check if we have the X axes, since we should always have the Y as well
                                            {SDL_CONTROLLER_AXIS_LEFTX, i18nc("@label Left joystick name", "Left Thumb")},
                                            {SDL_CONTROLLER_AXIS_RIGHTX, i18nc("@label Right joystick name", "Right Thumb")}};

    const QMap<int, QString> triggersToCheck = {{SDL_CONTROLLER_AXIS_TRIGGERLEFT, i18nc("@label Left trigger button name", "Left Trigger")},
                                                {SDL_CONTROLLER_AXIS_TRIGGERRIGHT, i18nc("@label Right trigger button name", "Right Trigger")}};

    for (const int i : axesToCheck.keys()) {
        if (SDL_GameControllerHasAxis(m_gameController, (SDL_GameControllerAxis)i)) {
            QString name = axesToCheck.value(i);
            qCDebug(KCM_GAMECONTROLLER) << "Adding axes with id" << i;
            m_axes.insert(i, new GamepadStick(name, this));
        }
    }

    for (const int i : triggersToCheck.keys()) {
        if (SDL_GameControllerHasAxis(m_gameController, (SDL_GameControllerAxis)i)) {
            const QString &name = triggersToCheck.value(i);
            qCDebug(KCM_GAMECONTROLLER) << "Adding trigger with id" << i;
            m_triggers.insert(i, new GamepadTrigger(m_vendor, name, i, this));
        }
    }
}

QString Gamepad::name() const
{
    return m_name;
}

QString Gamepad::path() const
{
    return m_path;
}

QString Gamepad::model() const
{
    return m_model;
}

int Gamepad::numButtons() const
{
    return m_numButtons;
}

int Gamepad::numAxes() const
{
    return m_numAxes;
}

SDL_GameControllerType Gamepad::gamepadType() const
{
    auto type = SDL_GameControllerGetType(m_gameController);
    qCDebug(KCM_GAMECONTROLLER) << "GamepadType: " << type;
    return type;
}

bool Gamepad::hasRumble() const
{
    return m_hasRumble;
}

bool Gamepad::hasTouchPad() const
{
    return m_hasTouchPad;
}

Gamepad::ConnectionType Gamepad::connectionType() const
{
    return m_connectionType;
}

GamepadButton *Gamepad::button(const int sdlButtonType)
{
    if (hasButton(sdlButtonType)) {
        return m_buttons.value(sdlButtonType);
    } else {
        qCWarning(KCM_GAMECONTROLLER) << "Button with type " << sdlButtonType << " requested, but not found.";
        return nullptr;
    }
}

bool Gamepad::hasButton(const int sdlButtonType) const
{
    return m_buttons.contains(sdlButtonType);
}

GamepadStick *Gamepad::axis(const int sdlAxis)
{
    if (hasAxis(sdlAxis)) {
        return m_axes.value(sdlAxis);
    } else {
        qCWarning(KCM_GAMECONTROLLER) << "Axis with type " << sdlAxis << " requested, but not found.";
        return nullptr;
    }
}

bool Gamepad::hasAxis(const int sdlAxis) const
{
    return m_axes.contains(sdlAxis);
}

GamepadTrigger *Gamepad::trigger(const int sdlTrigger)
{
    if (hasTrigger(sdlTrigger)) {
        return m_triggers.value(sdlTrigger);
    } else {
        qCWarning(KCM_GAMECONTROLLER) << "Trigger with type " << sdlTrigger << " requested, but not found.";
        return nullptr;
    }
}

bool Gamepad::hasTrigger(const int sdlTrigger) const
{
    return m_triggers.contains(sdlTrigger);
}

void Gamepad::onButtonEvent(const SDL_ControllerButtonEvent sdlEvent)
{
    if (m_buttons.contains(sdlEvent.button)) {
        m_buttons.value(sdlEvent.button)->setState(sdlEvent.type == SDL_CONTROLLERBUTTONDOWN);
        Q_EMIT buttonStateChanged(SDL_GameControllerButton(sdlEvent.button));
    }
}

void Gamepad::onAxisEvent(const SDL_ControllerAxisEvent sdlEvent)
{
    // SDL2 documentation states this the maximum value.
    constexpr float VALUE_MAX = 32767.0f;

    switch (sdlEvent.axis) {
    case SDL_CONTROLLER_AXIS_LEFTX:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_LEFTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_LEFTX)->setX(static_cast<float>(sdlEvent.value) / VALUE_MAX);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_LEFTX);
        }
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_LEFTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_LEFTX)->setY(static_cast<float>(sdlEvent.value) / VALUE_MAX);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_LEFTY);
        }
        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_RIGHTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_RIGHTX)->setX(static_cast<float>(sdlEvent.value) / VALUE_MAX);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_RIGHTX);
        }
        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        if (m_axes.contains(SDL_CONTROLLER_AXIS_RIGHTX)) {
            m_axes.value(SDL_CONTROLLER_AXIS_RIGHTX)->setY(static_cast<float>(sdlEvent.value) / VALUE_MAX);
            Q_EMIT axisStateChanged(SDL_CONTROLLER_AXIS_RIGHTY);
        }
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        if (m_triggers.contains(sdlEvent.axis)) {
            m_triggers.value(sdlEvent.axis)->setValue(static_cast<float>(sdlEvent.value) / VALUE_MAX);
            Q_EMIT triggerStateChanged(sdlEvent.axis);
        }
        break;
    }
}

SDL_Joystick *Gamepad::joystick() const
{
    return m_joystick;
}

SDL_GameController *Gamepad::gamecontroller() const
{
    return m_gameController;
}
