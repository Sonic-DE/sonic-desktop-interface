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
    Q_PROPERTY(QString image READ getImage CONSTANT)
    Q_PROPERTY(float value READ getValue NOTIFY valueChanged)
public:
    explicit GamepadTrigger(int vendor, QString name, int index, QObject *parent = nullptr);
    // For QML usage of axis objects
    GamepadTrigger();

    void setValue(float value);
    float getValue();

    QString getImage();
signals:
    void valueChanged();

private:
    // Give path to an image to show in the gui for the given button.
    QString image(int index);

    int m_vendor;
    int m_index;
    QString m_name;

    float m_value;
};
