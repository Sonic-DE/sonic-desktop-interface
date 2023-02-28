/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joybutton.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>
#include <SDL2/SDL_gamecontroller.h>
#include <Solid/Block>
#include <Solid/GenericInterface>
#include <csignal>
#include <fcntl.h>

// Distance in pixels between buttons near each other both X and Y
const int kSpaceBetweenButtons = 35;
const int kDPadSpacing = 20;

// Positions of buttons
// DPad positions
const int kLeftX = 10;
const int kMidX = kLeftX + kDPadSpacing;
const int kRightX = kMidX + kDPadSpacing;

const int kDPadTopY = 24;
const int kDPadMidY = kDPadTopY + kDPadSpacing;
const int kDPadBottomY = kDPadMidY + kDPadSpacing;

// Center button positions
const int kSelectX = kRightX + (kSpaceBetweenButtons * 2);
const int kMiscX = kSelectX + kSpaceBetweenButtons;
const int kModeX = kMiscX + kSpaceBetweenButtons;
const int kStartX = kModeX + kSpaceBetweenButtons;

// right quad positions
const int kButtonsX = kStartX + (kSpaceBetweenButtons * 2);
const int kButtonsMidX = kButtonsX + kSpaceBetweenButtons;
const int kButtonsRightX = kButtonsMidX + kSpaceBetweenButtons;

// y positions
const int kTopY = 10;
const int kMidY = kTopY + kSpaceBetweenButtons;
const int kBottomY = kMidY + kSpaceBetweenButtons;

// TODO: confirm actual usb ids
const uint16_t USB_ID_SONY = 0x0a51;
const uint16_t USB_ID_SONY2 = 0x054c;
const uint16_t USB_ID_MICROSOFT = 0x045e;
const uint16_t USB_ID_NINTENDO = 0x057e;

JoyButton::JoyButton(uint16_t vendor, const int code, QObject *parent)
    : QObject(parent)
    , m_vendor(vendor)
    , m_code(code)
    , m_state(false)
{
    m_name = name(m_code);
}

JoyButton::JoyButton()
    : m_code(0)
    , m_state(false)
{
}

void JoyButton::setState(bool state)
{
    qDebug() << "button with name: " << name(m_code) << " state changed: " << state;
    m_state = state;
    emit stateChanged();
}

bool JoyButton::getState() const
{
    return m_state;
}

QPoint JoyButton::position(int code)
{
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

    QPoint point = QPoint(0, 0);
    switch (code) {
    case SDL_CONTROLLER_BUTTON_X:
        point.setX(kButtonsX);
        break;
    case SDL_CONTROLLER_BUTTON_B:
        point.setX(kButtonsRightX);
        break;
    case SDL_CONTROLLER_BUTTON_Y:
    case SDL_CONTROLLER_BUTTON_A:
        point.setX(kButtonsMidX);
        break;
    case SDL_CONTROLLER_BUTTON_BACK:
        point.setX(kSelectX);
        break;
    case SDL_CONTROLLER_BUTTON_MISC1:
        point.setX(kMiscX);
        break;
    case SDL_CONTROLLER_BUTTON_GUIDE:
        point.setX(kModeX);
        break;
    case SDL_CONTROLLER_BUTTON_START:
        point.setX(kStartX);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        point.setX(kLeftX);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        point.setX(kMidX);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        point.setX(kRightX);
        break;
    }

    switch (code) {
    case SDL_CONTROLLER_BUTTON_X:
    case SDL_CONTROLLER_BUTTON_B:
        point.setY(kMidY);
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        point.setY(kTopY);
        break;
    case SDL_CONTROLLER_BUTTON_A:
        point.setY(kBottomY);
        break;
    case SDL_CONTROLLER_BUTTON_BACK:
    case SDL_CONTROLLER_BUTTON_MISC1:
    case SDL_CONTROLLER_BUTTON_GUIDE:
    case SDL_CONTROLLER_BUTTON_START:
        point.setY(kMidY);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        point.setY(kDPadTopY);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        point.setY(kDPadMidY);
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        point.setY(kDPadBottomY);
        break;
    }
    return point;
}

QString JoyButton::name(int code)
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

QString JoyButton::image(int code)
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
        /* case BTN_C:
                name = i18nc("C button", "C");
                break;
            case BTN_Z:
                name = i18nc("Z button", "Z");
                break;
            case BTN_TR:
                name = i18nc("R1 button", "R1");
                break;
            case BTN_TR2:
                name = i18nc("R2 button", "R2");
                break;
            case BTN_TL:
                name = i18nc("L1 button", "L1");
                break;
            case BTN_TL2:
                name = i18nc("L2 button", "L2");
                break;
            case BTN_THUMBL:
                name = i18nc("Left thumb stick button", "Left Thumb");
                break;
            case BTN_THUMBR:
                name = i18nc("Right thumb stick button", "Right Thumb");
                break; */
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
        /*    case BTN_TRIGGER:
                name = i18nc("Trigger button", "Trigger");
                break;
            case BTN_THUMB:
                name = i18nc("Thumb button", "Thumb");
                break;
            case BTN_THUMB2:
                name = i18nc("Second thumb button", "Thumb 2");
                break;
            case BTN_TOP:
                name = i18nc("Top button", "Top");
                break;
            case BTN_TOP2:
                name = i18nc("Second top button", "Top 2");
                break;
            case BTN_PINKIE:
                name = i18nc("Pinkie (smallest finger) button", "Pinkie");
                break;
            case BTN_BASE:
                name = i18nc("First base button", "Base 1");
                break;
            case BTN_BASE2:
                name = i18nc("Second base button", "Base 2");
                break;
            case BTN_BASE3:
                name = i18nc("Third base button", "Base 3");
                break;
            case BTN_BASE4:
                name = i18nc("Fourth base button", "Base 4");
                break;
            case BTN_BASE5:
                name = i18nc("Fifth base button", "Base 5");
                break;
            case BTN_BASE6:
                name = i18nc("Sixfth base button", "Base 6");
                break;*/
    }
    return filename;
}
