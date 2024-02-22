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

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(float value READ value NOTIFY valueChanged)

public:
    explicit GamepadTrigger(int vendor, const QString &name, int index, QObject *parent = nullptr);

    QString name() const;
    float value() const;

    void setValue(float value);

Q_SIGNALS:
    void valueChanged();

private:
    int m_vendor;
    int m_index;
    QString m_name;

    float m_value;
};
