/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QString>

#include <QDebug>
#include <QVector2D>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_joystick.h>
#include <Solid/Device>
#include <libevdev-1.0/libevdev/libevdev.h>

#include <KLocalizedString>
#include <SDL2/SDL_gamecontroller.h>

#include "gamepadbutton.h"
#include "gamepadstick.h"
#include "gamepadtrigger.h"

class Gamepad : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QString model READ getModel CONSTANT)
    Q_PROPERTY(int numButtons MEMBER m_numButtons CONSTANT)
    Q_PROPERTY(int numAxes MEMBER m_numAxes CONSTANT)
    Q_PROPERTY(int deviceType READ gamepadType CONSTANT)
    Q_PROPERTY(bool hasRumble MEMBER m_hasRumble CONSTANT)
    Q_PROPERTY(bool hasTouchPad READ hasTouchPad CONSTANT)
    Q_PROPERTY(QVariantList buttons READ getButtons NOTIFY buttonStateChanged)
    Q_PROPERTY(QVariantList axes READ getAxes NOTIFY axisStateChanged)
    Q_PROPERTY(QVariantList triggers READ getTriggers NOTIFY triggerStateChanged)
    Q_PROPERTY(ConnectionType connectionType READ getConnectionType NOTIFY connectionTypeChanged)
public:
    explicit Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent = nullptr);
    // For QML usage of devices
    Gamepad();

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
        for (auto button : m_buttons) {
            data.push_back(QVariant::fromValue(button));
        }
        return data;
    }

    QVariantList getAxes() const
    {
        QVariantList data;
        for (auto axis : m_axes.values()) {
            data.push_back(QVariant::fromValue(axis));
        }
        return data;
    }

    QVariantList getTriggers() const
    {
        QVariantList data;
        for (auto trigger : m_triggers.values()) {
            data.push_back(QVariant::fromValue(trigger));
        }
        return data;
    }

    SDL_Joystick *getJoystick() const
    {
        return m_joystick;
    }

    friend class DeviceModel;

    SDL_GameControllerType gamepadType();

signals:
    void buttonStateChanged(int index);
    void axisStateChanged(int index);
    void triggerStateChanged(int index);

    // Possible when going from USB to Bluetooth, or vice versa
    void connectionTypeChanged();

private:
    void onButtonEvent(const SDL_ControllerButtonEvent sdlEvent);
    void onAxisEvent(const SDL_ControllerAxisEvent sdlEvent);

    SDL_Joystick *m_joystick = nullptr;
    SDL_GameController *m_gameController = nullptr;

    QVector<GamepadButton *> m_buttons;
    QMap<int, GamepadStick *> m_axes;
    QMap<int, GamepadTrigger *> m_triggers;

    QString m_name;
    uint16_t m_vendor;
    QString m_model;
    int m_numButtons = 0;
    int m_numAxes = 0;
    bool m_hasRumble = false;

    bool m_hasTouchPad = false;
    ConnectionType m_connectionType = UnknownType;
};
