/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

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

#include "joybutton.h"

class JoyDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QString vendor READ getVendor CONSTANT)
    Q_PROPERTY(QString model READ getModel CONSTANT)
    Q_PROPERTY(int numButtons MEMBER m_numButtons CONSTANT)
    Q_PROPERTY(int numAxes MEMBER m_numAxes CONSTANT)
    Q_PROPERTY(bool hasRumble MEMBER m_hasRumble CONSTANT)
    Q_PROPERTY(bool hasTouchPad READ hasTouchPad CONSTANT)
    Q_PROPERTY(QVariantList buttons READ getButtons CONSTANT)
    Q_PROPERTY(QStringList axisNames READ getAxisNames CONSTANT)
    Q_PROPERTY(QStringList axisState READ getAxisState NOTIFY axisStateChanged)
    Q_PROPERTY(ConnectionType connectionType READ getConnectionType NOTIFY connectionTypeChanged)
public:
    explicit JoyDevice(const Solid::Device &device, QObject *parent = nullptr);
    // For QML usage of devices
    JoyDevice();

    enum ConnectionType {
        UnknownType,
        USBType,
        BluetoothType,
    };
    Q_ENUM(ConnectionType)

    QString getName() const
    {
        return m_name;
    }

    QString getVendor() const
    {
        return m_vendor;
    }

    QString getModel() const
    {
        return m_model;
    }

    bool hasTouchPad() const
    {
        return m_hasTouchPad;
    }

    ConnectionType getConnectionType() const
    {
        return m_connectionType;
    }

    QVariantList getButtons() const
    {
        QVariantList data;
        for (auto button : m_buttons.values()) {
            data.push_back(QVariant::fromValue(button));
        }
        return data;
    }

    QStringList getAxisState()
    {
        QStringList data;
        for (auto axis : m_axisState) {
            data.push_back("(" + QString::number(axis.x()) + ", " + QString::number(axis.y()) + ")");
        }
        return data;
    }

    QStringList getAxisNames()
    {
        QStringList data;
        for (auto code : m_axisCodes) {
            data.push_back(axisName(code));
        }

        return data;
    }

    void poll();

signals:
    void axisStateChanged();

    // Possible when going from USB to Bluetooth, or vice versa
    void connectionTypeChanged();

private:
    QString axisName(int code);
    void processEvent(struct input_event &ev);
    float normalize(int code, __s32 value);

    libevdev *m_device = nullptr;

    QMap<int, JoyButton *> m_buttons;

    QString m_name;
    QString m_vendor;
    QString m_model;
    int m_numButtons = 0;
    int m_numAxes = 0;
    bool m_hasRumble = false;

    // List of the same buttons codes from evdev
    QVector<QVector2D> m_axisState;
    // List of axis starting points for naming
    QVector<int> m_axisCodes;
    bool m_hasTouchPad = false;
    ConnectionType m_connectionType = UnknownType;
};
