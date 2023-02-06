/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

#include <QDebug>
#include <libevdev-1.0/libevdev/libevdev.h>
#include <libudev.h>

class JoyDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(int numButtons READ getNumButtons NOTIFY numButtonsChanged)
    Q_PROPERTY(QString brand READ getBrand NOTIFY brandChanged)
public:
    explicit JoyDevice(udev_device *device, QObject *parent = nullptr);
    JoyDevice();

    QString getName() const
    {
        return m_name;
    }

    QString getBrand() const
    {
        return m_brand;
    }

    int getNumButtons() const
    {
        return m_numButtons;
    }

signals:
    // These never happen in practice, but QML want's them.
    void numButtonsChanged();
    void brandChanged();

private:
    udev_device *m_device = nullptr;

    QString m_name;
    QString m_brand;
    int m_numButtons = 0;
};
