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
    Q_PROPERTY(QStringList buttonNames READ getButtonNames CONSTANT)
    Q_PROPERTY(QStringList buttonState READ getButtonState NOTIFY buttonStateChanged)
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

    QStringList getButtonState()
    {
        QStringList data;
        for (auto button : m_buttonState) {
            data.push_back(button ? "1" : "0");
        }
        return data;
    }

    QStringList getButtonNames()
    {
        QStringList data;
        for (auto code : m_buttonCodes) {
            data.push_back(buttonName(code));
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
    void buttonStateChanged();
    void axisStateChanged();

    // Possible when going from USB to Bluetooth, or vice versa
    void connectionTypeChanged();

private:
    // Give a button name for given ev code
    QString buttonName(int code);
    QString axisName(int code);
    void processEvent(struct input_event &ev);
    float normalize(int code, __s32 value);

    libevdev *m_device = nullptr;

    QString m_name;
    QString m_vendor;
    QString m_model;
    int m_numButtons = 0;
    int m_numAxes = 0;
    bool m_hasRumble = false;

    QVector<bool> m_buttonState;
    // List of the same buttons codes from evdev
    QVector<int> m_buttonCodes;
    QVector<QVector2D> m_axisState;
    // List of axis starting points for naming
    QVector<int> m_axisCodes;
    bool m_hasTouchPad = false;
    ConnectionType m_connectionType = UnknownType;
};
