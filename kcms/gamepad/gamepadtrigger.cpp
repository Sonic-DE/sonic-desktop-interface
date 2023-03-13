/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadtrigger.h"

#include <QDebug>

#include <SDL2/SDL_gamecontroller.h>

const uint16_t USB_ID_SONY = 0x0a51;
const uint16_t USB_ID_SONY2 = 0x054c;
const uint16_t USB_ID_MICROSOFT = 0x045e;
const uint16_t USB_ID_NINTENDO = 0x057e;

GamepadTrigger::GamepadTrigger(int vendor, QString name, int index, QObject *parent)
    : QObject(parent)
    , m_vendor(vendor)
    , m_index(index)
    , m_name(name)
{
}

GamepadTrigger::GamepadTrigger()
    : m_index(0)
{
}

float GamepadTrigger::getValue()
{
    return m_value;
}

QString GamepadTrigger::getImage()
{
    return image(m_index);
}

QString GamepadTrigger::image(int index)
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

    switch (index) {
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        filename = pattern.arg(prefix).arg("l2");
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        filename = pattern.arg(prefix).arg("r2");
        break;
    }
    return filename;
}

void GamepadTrigger::setValue(float value)
{
    //    qDebug() << "trigger " << m_name << " value: " << value;
    m_value = value;
    emit valueChanged();
}
