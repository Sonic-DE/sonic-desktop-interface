/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>
#include <QVector2D>

class GamepadTrigger : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    // For 1D axis
    Q_PROPERTY(int value READ getValue NOTIFY valueChanged)
public:
    explicit GamepadTrigger(QString name, int index, QObject *parent = nullptr);
    // For QML usage of axis objects
    GamepadTrigger();

    void setValue(int value);
    int getValue();

signals:
    void valueChanged();

private:
    int m_index;
    QString m_name;

    int m_value;
};
