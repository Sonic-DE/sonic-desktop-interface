/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joybutton.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>
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
const int kModeX = kSelectX + kSpaceBetweenButtons;
const int kStartX = kModeX + kSpaceBetweenButtons;

// right quad positions
const int kButtonsX = kStartX + (kSpaceBetweenButtons * 2);
const int kButtonsMidX = kButtonsX + kSpaceBetweenButtons;
const int kButtonsRightX = kButtonsMidX + kSpaceBetweenButtons;

// y positions
const int kTopY = 10;
const int kMidY = kTopY + kSpaceBetweenButtons;
const int kBottomY = kMidY + kSpaceBetweenButtons;

JoyButton::JoyButton(const QString &vendor, const int code, QObject *parent)
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
    m_state = state;
    emit stateChanged();
}

bool JoyButton::getState() const
{
    return m_state;
}

QPoint JoyButton::position(int code)
{
    QPoint point = QPoint(0, 0);
    switch (code) {
    case BTN_WEST:
        point.setX(kButtonsX);
        break;
    case BTN_EAST:
        point.setX(kButtonsRightX);
        break;
    case BTN_NORTH:
    case BTN_SOUTH:
        point.setX(kButtonsMidX);
        break;
    case BTN_SELECT:
        point.setX(kSelectX);
        break;
    case BTN_MODE:
        point.setX(kModeX);
        break;
    case BTN_START:
        point.setX(kStartX);
        break;
    case BTN_DPAD_LEFT:
        point.setX(kLeftX);
        break;
    case BTN_DPAD_UP:
    case BTN_DPAD_DOWN:
        point.setX(kMidX);
        break;
    case BTN_DPAD_RIGHT:
        point.setX(kRightX);
        break;
    }

    switch (code) {
    case BTN_WEST:
    case BTN_EAST:
        point.setY(kMidY);
        break;
    case BTN_NORTH:
        point.setY(kTopY);
        break;
    case BTN_SOUTH:
        point.setY(kBottomY);
        break;
    case BTN_SELECT:
    case BTN_MODE:
    case BTN_START:
        point.setY(kMidY);
        break;
    case BTN_DPAD_UP:
        point.setY(kDPadTopY);
        break;
    case BTN_DPAD_LEFT:
    case BTN_DPAD_RIGHT:
        point.setY(kDPadMidY);
        break;
    case BTN_DPAD_DOWN:
        point.setY(kDPadBottomY);
        break;
    }
    return point;
}

QString JoyButton::name(int code)
{
    QString name = i18n("Unknown button %1", code);
    switch (code) {
    case BTN_WEST:
        name = i18nc("Western direction button", "West");
        break;
    case BTN_NORTH:
        name = i18nc("Northern direction button", "North");
        break;
    case BTN_EAST:
        name = i18nc("Eastern direction button", "East");
        break;
    case BTN_SOUTH:
        name = i18nc("Southern direction button", "South");
        break;
    case BTN_START:
        name = i18nc("Start button", "Start");
        break;
    case BTN_SELECT:
        name = i18nc("Select button", "Select");
        break;
    case BTN_C:
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
    case BTN_MODE:
        name = i18nc("Mode button", "Mode");
        break;
    case BTN_THUMBL:
        name = i18nc("Left thumb stick button", "Left Thumb");
        break;
    case BTN_THUMBR:
        name = i18nc("Right thumb stick button", "Right Thumb");
        break;
    case BTN_DPAD_UP:
        name = i18nc("Up button", "Up");
        break;
    case BTN_DPAD_DOWN:
        name = i18nc("Down button", "Down");
        break;
    case BTN_DPAD_LEFT:
        name = i18nc("Left button", "Left");
        break;
    case BTN_DPAD_RIGHT:
        name = i18nc("Right button", "Right");
        break;
    case BTN_TRIGGER:
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
        break;
    }
    return name;
}

QString JoyButton::image(int code)
{
    QString filename = QStringLiteral("images/%1_%2.png");
    QString prefix = "unknown";

    if (m_vendor == "Sony") {
        prefix = "sony";
    } else if (m_vendor == "Nintendo") {
        prefix = "nintendo";
    } else {
        prefix = "sony";
    }

    switch (code) {
    case BTN_WEST:
        filename = filename.arg(prefix).arg("west");
        break;
    case BTN_NORTH:
        filename = filename.arg(prefix).arg("north");
        break;
    case BTN_EAST:
        filename = filename.arg(prefix).arg("east");
        break;
    case BTN_SOUTH:
        filename = filename.arg(prefix).arg("south");
        break;
    case BTN_START:
        filename = filename.arg(prefix).arg("start");
        break;
    case BTN_MODE:
        filename = filename.arg(prefix).arg("mode");
        break;
    case BTN_SELECT:
        filename = filename.arg(prefix).arg("select");
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
    case BTN_DPAD_UP:
        filename = filename.arg(prefix).arg("up");
        break;
    case BTN_DPAD_DOWN:
        filename = filename.arg(prefix).arg("down");
        break;
    case BTN_DPAD_LEFT:
        filename = filename.arg(prefix).arg("left");
        break;
    case BTN_DPAD_RIGHT:
        filename = filename.arg(prefix).arg("right");
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
