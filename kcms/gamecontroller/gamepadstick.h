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

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QVector2D gridValue READ gridValue NOTIFY gridValueChanged)

public:
    explicit GamepadStick(const QString &name, QObject *parent = nullptr);

    QString name() const;
    QVector2D gridValue() const;

    void setX(float x);
    void setY(float y);

Q_SIGNALS:
    void gridValueChanged();

private:
    QString m_name;
    QVector2D m_gridValue;
};
