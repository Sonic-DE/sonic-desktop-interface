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

#include <KLocalizedString>

class JoyDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(int numButtons READ getNumButtons NOTIFY numButtonsChanged)
    Q_PROPERTY(int numSticks READ getNumSticks NOTIFY numSticksChanged)
    Q_PROPERTY(bool hasTouchPad READ hasTouchPad NOTIFY hasTouchPadChanged)
    Q_PROPERTY(Brand brand READ getBrand NOTIFY brandChanged)
    Q_PROPERTY(QString model READ getModel NOTIFY modelChanged)
    Q_PROPERTY(ConnectionType connectionType READ getConnectionType NOTIFY connectionTypeChanged)
public:
    explicit JoyDevice(udev_device *device, QObject *parent = nullptr);
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

    int getNumButtons() const
    {
        return m_numButtons;
    }

    int getNumSticks() const
    {
        return m_numSticks;
    }

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
    // These never happen in practice, but QML want's them.
    void numButtonsChanged();
    void numSticksChanged();
    void hasTouchPadChanged();
    void brandChanged();
    void modelChanged();
    // Possible when going from USB to Bluetooth, or vice versa
    void connectionTypeChanged();

private:
    udev_device *m_device = nullptr;

    QString m_name;
    Brand m_brand;
    QString m_model;
    int m_numButtons = 0;
    int m_numSticks = 0;
    bool m_hasTouchPad = false;
    ConnectionType m_connectionType = UnknownType;
};
