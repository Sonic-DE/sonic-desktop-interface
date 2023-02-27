/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

#include <QDebug>
#include <QVector2D>
#include <Solid/Device>
#include <libevdev-1.0/libevdev/libevdev.h>

#include <KLocalizedString>

class JoyButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QString image READ getImage CONSTANT)
    Q_PROPERTY(QPoint position READ getPosition CONSTANT)
    Q_PROPERTY(bool state READ getState NOTIFY stateChanged)
public:
    explicit JoyButton(const QString &vendor, const int code, QObject *parent = nullptr);
    // For QML usage of buttons
    JoyButton();

    QString getName() const
    {
        return m_name;
    }

    void setState(bool state);

    bool getState() const;

    QString getName()
    {
        return name(m_code);
    }

    QString getImage()
    {
        return image(m_code);
    }

    QPoint getPosition()
    {
        return position(m_code);
    }

signals:
    void stateChanged();

private:
    // Give a button name for given ev code
    QString name(int code);

    // Give path to an image to show in the gui for the given button.
    QString image(int code);
    QPoint position(int code);

    QString m_vendor;
    int m_code;
    QString m_name;

    bool m_state;
};
