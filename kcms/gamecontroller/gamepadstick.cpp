/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadstick.h"

GamepadStick::GamepadStick(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{
}

QString GamepadStick::name() const
{
    return m_name;
}

QVector2D GamepadStick::gridValue() const
{
    return m_gridValue;
}

void GamepadStick::setX(const float x)
{
    if (x != m_gridValue.x()) {
        m_gridValue.setX(x);
        Q_EMIT gridValueChanged();
    }
}

void GamepadStick::setY(const float y)
{
    if (y != m_gridValue.y()) {
        m_gridValue.setY(y);
        Q_EMIT gridValueChanged();
    }
}
