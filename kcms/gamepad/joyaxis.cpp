/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joyaxis.h"

#include <QDebug>

JoyAxis::JoyAxis(QString name, int index, bool is2D, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_is2D(is2D)
    , m_name(name)
{
}

JoyAxis::JoyAxis()
    : m_index(0)
    , m_is2D(false)
{
}

bool JoyAxis::getIs2DAxis()
{
    return m_is2D;
}

int JoyAxis::getValue()
{
    return m_value;
}

QVector2D JoyAxis::getGridValue()
{
    return m_gridValue;
}

void JoyAxis::setValue(int value)
{
    qDebug() << "trigger " << m_name << " value: " << value;
    m_value = value;
    emit valueChanged();
}

void JoyAxis::setGridValue(QVector2D gridValue)
{
    qDebug() << "stick " << m_name << " value: " << gridValue;
    m_gridValue = gridValue;
    emit gridValueChanged();
}
