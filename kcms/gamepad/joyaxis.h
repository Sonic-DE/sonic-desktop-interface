/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>
#include <QVector2D>

class JoyAxis : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(bool is2DAxis READ getIs2DAxis CONSTANT)
    // For 1D axis
    Q_PROPERTY(int value READ getValue NOTIFY valueChanged)
    // For 2D axis
    Q_PROPERTY(QVector2D gridValue READ getGridValue NOTIFY gridValueChanged)
public:
    explicit JoyAxis(QString name, int index, bool is2D = true, QObject *parent = nullptr);
    // For QML usage of axis objects
    JoyAxis();

    bool getIs2DAxis();

    void setGridValue(QVector2D gridValue);
    QVector2D getGridValue();

    void setValue(int value);
    int getValue();

signals:
    void valueChanged();
    void gridValueChanged();

private:
    int m_index;
    bool m_is2D;
    QString m_name;

    int m_value;
    QVector2D m_gridValue;
};
