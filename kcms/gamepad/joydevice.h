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
    Q_PROPERTY(Brand brand READ getBrand CONSTANT)
    Q_PROPERTY(QString model READ getModel CONSTANT)
    Q_PROPERTY(int numButtons MEMBER m_numButtons CONSTANT)
    Q_PROPERTY(int numAxes MEMBER m_numAxes CONSTANT)
    Q_PROPERTY(bool hasRumble MEMBER m_hasRumble CONSTANT)
    Q_PROPERTY(bool hasTouchPad READ hasTouchPad CONSTANT)
    Q_PROPERTY(QStringList buttonState READ getButtonState NOTIFY buttonStateChanged)
    Q_PROPERTY(QStringList axisState READ getAxisState NOTIFY axisStateChanged)
    Q_PROPERTY(ConnectionType connectionType READ getConnectionType NOTIFY connectionTypeChanged)
public:
    explicit JoyDevice(const Solid::Device &device, QObject *parent = nullptr);
    // For QML usage of devices
    JoyDevice();

    enum Brand {
        UnknownBrand,
        // Playstation dualshock 3, 4 or 5, should have 2 sticks, 10 buttons, might have
        // a touchpad
        PlayStation,
        // Switch or switch pro, either connected with usb or bluetooth
        Switch,
        // Xbox controller, should have 2 sticks, 10 buttons
        XBox,
        // 8bitDo controller, could have between 6 and 12 button depending on model, etc.
        EightBitDo,
    };
    Q_ENUM(Brand)

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

    bool hasTouchPad() const
    {
        return m_hasTouchPad;
    }

    Brand getBrand() const
    {
        return m_brand;
    }

    QString getModel() const
    {
        return m_model;
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

    QStringList getAxisState()
    {
        QStringList data;
        for (auto axis : m_axisState) {
            data.push_back("(" + QString::number(axis.x()) + ", " + QString::number(axis.y()) + ")");
        }
        return data;
    }

    void poll();

    Q_INVOKABLE QString brandName(Brand brand)
    {
        switch (brand) {
        case PlayStation:
            return i18n("Sony PlayStation");
            break;
        case Switch:
            return i18n("Nintendo Switch");
            break;
        case XBox:
            return i18n("Microsoft XBox");
            break;
        case EightBitDo:
            return i18n("8bitDo");
            break;
        case UnknownBrand:
        default:
            return i18n("Unknown Brand");
        }
    }

signals:
    void buttonStateChanged();
    void axisStateChanged();

    // Possible when going from USB to Bluetooth, or vice versa
    void connectionTypeChanged();

private:
    void processEvent(struct input_event &ev);
    float normalize(int code, __s32 value);

    libevdev *m_device = nullptr;

    QString m_name;
    Brand m_brand;
    QString m_model;
    int m_numButtons = 0;
    int m_numAxes = 0;
    bool m_hasRumble = false;

    QVector<bool> m_buttonState;
    QVector<QVector2D> m_axisState;
    bool m_hasTouchPad = false;
    ConnectionType m_connectionType = UnknownType;
};
