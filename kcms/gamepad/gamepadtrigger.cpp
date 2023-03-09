/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gamepadtrigger.h"

#include <QDebug>

GamepadTrigger::GamepadTrigger(QString name, int index, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_name(name)
{
}

GamepadTrigger::GamepadTrigger()
    : m_index(0)
{
}

int GamepadTrigger::getValue()
{
    return m_value;
}

void GamepadTrigger::setValue(int value)
{
    //    qDebug() << "trigger " << m_name << " value: " << value;
    m_value = value;
    emit valueChanged();
}
