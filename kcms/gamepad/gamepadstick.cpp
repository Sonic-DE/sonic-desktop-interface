/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadstick.h"

#include <QDebug>

GamepadStick::GamepadStick(QString name, int index, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_name(name)
{
}

GamepadStick::GamepadStick()
    : m_index(0)
{
}

void GamepadStick::setX(float x)
{
    m_gridValue.setX(x);
    emit gridValueChanged();
}

void GamepadStick::setY(float y)
{
    m_gridValue.setY(y);
    emit gridValueChanged();
}

QVector2D GamepadStick::getGridValue()
{
    return m_gridValue;
}

void GamepadStick::setGridValue(QVector2D gridValue)
{
    //    qDebug() << "stick " << m_name << " value: " << gridValue;
    m_gridValue = gridValue;
    emit gridValueChanged();
}
