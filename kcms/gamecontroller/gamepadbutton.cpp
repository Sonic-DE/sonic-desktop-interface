/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadbutton.h"

#include <KLocalizedString>

GamepadButton::GamepadButton(const uint16_t vendor, const int code, QObject *parent)
    : QObject(parent)
    , m_vendor(vendor)
    , m_code(code)
    , m_state(false)
{
}

void GamepadButton::setState(const bool state)
{
    if (state != m_state) {
        m_state = state;
        Q_EMIT stateChanged();
    }
}

bool GamepadButton::state() const
{
    return m_state;
}

QString GamepadButton::name(int code) const
{
    switch (code) {
    case SDL_CONTROLLER_BUTTON_X:
        return i18nc("@label X button name", "X");
    case SDL_CONTROLLER_BUTTON_Y:
        return i18nc("@label Y button name", "Y");
    case SDL_CONTROLLER_BUTTON_B:
        return i18nc("@label B button name", "B");
    case SDL_CONTROLLER_BUTTON_A:
        return i18nc("@label A button name", "A");
    case SDL_CONTROLLER_BUTTON_BACK:
        return i18nc("@label Back button name", "Back");
    case SDL_CONTROLLER_BUTTON_GUIDE:
        return i18nc("@label Guide button name", "Guide");
    case SDL_CONTROLLER_BUTTON_START:
        return i18nc("@label Start button name", "Start");
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        return i18nc("@label Left stick button name", "Left stick");
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        return i18nc("@label Right stick button name", "Right stick");
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        return i18nc("@label Left shoulder button name", "Left shoulder");
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        return i18nc("@label Right shoulder button name", "Right shoulder");
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        return i18nc("@label Up button name", "Up");
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        return i18nc("@label Down button name", "Down");
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        return i18nc("@label Left button name", "Left");
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        return i18nc("@label Right button name", "Right");
    case SDL_CONTROLLER_BUTTON_MISC1: /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
        return i18nc("@label Miscellaneous button name", "Misc");
    case SDL_CONTROLLER_BUTTON_PADDLE1: /* Xbox Elite paddle P1 */
        return i18nc("@label Paddle 1 button name", "Paddle 1");
    case SDL_CONTROLLER_BUTTON_PADDLE2: /* Xbox Elite paddle P3 */
        return i18nc("@label Paddle 2 button name", "Paddle 2");
    case SDL_CONTROLLER_BUTTON_PADDLE3: /* Xbox Elite paddle P2 */
        return i18nc("@label Paddle 3 button name", "Paddle 3");
    case SDL_CONTROLLER_BUTTON_PADDLE4: /* Xbox Elite paddle P4 */
        return i18nc("@label Paddle 4 button name", "Paddle 4");
    case SDL_CONTROLLER_BUTTON_TOUCHPAD: /* PS4/PS5 touchpad button */
        return i18nc("@label Touchpad button name", "Touchpad");
    default:
        return i18nc("@label", "Unknown button %1", code);
    }
}

QString GamepadButton::name() const
{
    return name(m_code);
}
