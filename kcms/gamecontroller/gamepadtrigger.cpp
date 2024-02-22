/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadtrigger.h"

#include <SDL2/SDL_gamecontroller.h>

GamepadTrigger::GamepadTrigger(const int vendor, const QString &name, const int index, QObject *parent)
    : QObject(parent)
    , m_vendor(vendor)
    , m_index(index)
    , m_name(name)
    , m_value(0.0)
{
}

QString GamepadTrigger::name() const
{
    return m_name;
}

float GamepadTrigger::value() const
{
    return m_value;
}

void GamepadTrigger::setValue(const float value)
{
    if (value != m_value) {
        m_value = value;
        Q_EMIT valueChanged();
    }
}
