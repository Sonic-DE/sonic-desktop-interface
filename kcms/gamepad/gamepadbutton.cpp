/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadbutton.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>
#include <SDL2/SDL_gamecontroller.h>
#include <Solid/Block>
#include <Solid/GenericInterface>
#include <csignal>
#include <fcntl.h>

// TODO: confirm actual usb ids
const uint16_t USB_ID_SONY = 0x0a51;
const uint16_t USB_ID_SONY2 = 0x054c;
const uint16_t USB_ID_MICROSOFT = 0x045e;
const uint16_t USB_ID_NINTENDO = 0x057e;

GamepadButton::GamepadButton(uint16_t vendor, const int code, QObject *parent)
    : QObject(parent)
    , m_vendor(vendor)
    , m_code(code)
    , m_state(false)
{
    m_name = name(m_code);
}

GamepadButton::GamepadButton()
    : m_code(0)
    , m_state(false)
{
}

void GamepadButton::setState(bool state)
{
    qDebug() << "button with name: " << name(m_code) << " state changed: " << state;
    m_state = state;
    emit stateChanged();
}

bool GamepadButton::getState() const
{
    return m_state;
}

QString GamepadButton::name(int code)
{
    QString name = i18n("Unknown button %1", code);
    switch (code) {
    case SDL_CONTROLLER_BUTTON_X:
        name = i18nc("X button name", "X");
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        name = i18nc("Y button name", "Y");
        break;
    case SDL_CONTROLLER_BUTTON_B:
        name = i18nc("B button name", "B");
        break;
    case SDL_CONTROLLER_BUTTON_A:
        name = i18nc("A button name", "A");
        break;
    case SDL_CONTROLLER_BUTTON_BACK:
        name = i18nc("Back button name", "Back");
        break;
    case SDL_CONTROLLER_BUTTON_GUIDE:
        name = i18nc("Guide button name", "Guide");
        break;
    case SDL_CONTROLLER_BUTTON_START:
        name = i18nc("Start button name", "Start");
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        name = i18nc("Left stick button name", "Left stick");
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        name = i18nc("Right stick button name", "Right stick");
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        name = i18nc("Left shoulder button name", "Left shoulder");
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        name = i18nc("Right shoulder button name", "Right shoulder");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        name = i18nc("Up button name", "Up");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        name = i18nc("Down button name", "Down");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        name = i18nc("Left button name", "Left");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        name = i18nc("Right button name", "Right");
        break;
    case SDL_CONTROLLER_BUTTON_MISC1: /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
        name = i18nc("Miscellaneous button name", "Misc");
        break;
    case SDL_CONTROLLER_BUTTON_PADDLE1: /* Xbox Elite paddle P1 */
        name = i18nc("Paddle 1 button name", "Paddle 1");
        break;
    case SDL_CONTROLLER_BUTTON_PADDLE2: /* Xbox Elite paddle P3 */
        name = i18nc("Paddle 2 button name", "Paddle 2");
        break;
    case SDL_CONTROLLER_BUTTON_PADDLE3: /* Xbox Elite paddle P2 */
        name = i18nc("Paddle 3 button name", "Paddle 3");
        break;
    case SDL_CONTROLLER_BUTTON_PADDLE4: /* Xbox Elite paddle P4 */
        name = i18nc("Paddle 4 button name", "Paddle 4");
        break;
    case SDL_CONTROLLER_BUTTON_TOUCHPAD: /* PS4/PS5 touchpad button */
        name = i18nc("Touchpad button name", "Touchpad");
        break;
    }
    return name;
}

QString GamepadButton::image(int code)
{
    QString filename;
    QString pattern = QStringLiteral("images/%1_%2.png");
    QString prefix = "unknown";

    if (m_vendor == USB_ID_SONY || m_vendor == USB_ID_SONY2) {
        prefix = "sony";
    } else if (m_vendor == USB_ID_NINTENDO) {
        prefix = "nintendo";
    } else if (m_vendor == USB_ID_MICROSOFT) {
        prefix = "microsoft";
    } else {
        // Default to microsoft since xbox style is common for pc
        // gamepads
        prefix = "microsoft";
    }

    bool isNintendo = (m_vendor == USB_ID_NINTENDO);

    if (isNintendo) {
        // Nintendo puts X at north, Y at west, B at south and A at east
        // So switch them here
        if (code == SDL_CONTROLLER_BUTTON_X)
            code = SDL_CONTROLLER_BUTTON_Y;
        else if (code == SDL_CONTROLLER_BUTTON_Y)
            code = SDL_CONTROLLER_BUTTON_X;
        else if (code == SDL_CONTROLLER_BUTTON_A)
            code = SDL_CONTROLLER_BUTTON_B;
        else if (code == SDL_CONTROLLER_BUTTON_B)
            code = SDL_CONTROLLER_BUTTON_A;
    }

    switch (code) {
    case SDL_CONTROLLER_BUTTON_X:
        filename = pattern.arg(prefix).arg("west");
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        filename = pattern.arg(prefix).arg("north");
        break;
    case SDL_CONTROLLER_BUTTON_B:
        filename = pattern.arg(prefix).arg("east");
        break;
    case SDL_CONTROLLER_BUTTON_A:
        filename = pattern.arg(prefix).arg("south");
        break;
    case SDL_CONTROLLER_BUTTON_START:
        filename = pattern.arg(prefix).arg("start");
        break;
    case SDL_CONTROLLER_BUTTON_GUIDE:
        filename = pattern.arg(prefix).arg("mode");
        break;
    case SDL_CONTROLLER_BUTTON_BACK:
        filename = pattern.arg(prefix).arg("select");
        break;
    case SDL_CONTROLLER_BUTTON_MISC1:
        filename = pattern.arg(prefix).arg("misc");
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        filename = pattern.arg(prefix).arg("l1");
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        filename = pattern.arg(prefix).arg("r1");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        filename = pattern.arg(prefix).arg("up");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        filename = pattern.arg(prefix).arg("down");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        filename = pattern.arg(prefix).arg("left");
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        filename = pattern.arg(prefix).arg("right");
        break;
    }
    return filename;
}
