/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>
#include <QVector2D>

class GamepadStick : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QVector2D gridValue READ getGridValue NOTIFY gridValueChanged)
public:
    explicit GamepadStick(QString name, int index, QObject *parent = nullptr);
    // For QML usage of axis objects
    GamepadStick();

    void setX(float x);
    void setY(float y);
    void setGridValue(QVector2D gridValue);
    QVector2D getGridValue();

signals:
    void gridValueChanged();

private:
    int m_index;
    QString m_name;

    QVector2D m_gridValue;
};
