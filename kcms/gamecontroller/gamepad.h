/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>


    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector2D>

#include <KLocalizedString>

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_joystick.h>

#include "gamepadbutton.h"
#include "gamepadstick.h"
#include "gamepadtrigger.h"

class Gamepad : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString path READ path CONSTANT)
    Q_PROPERTY(QString model READ model CONSTANT)
    Q_PROPERTY(int numButtons READ numButtons CONSTANT)
    Q_PROPERTY(int numAxes READ numAxes CONSTANT)
    Q_PROPERTY(int gamepadType READ gamepadType CONSTANT)
    Q_PROPERTY(bool hasRumble READ hasRumble CONSTANT)
    Q_PROPERTY(bool hasTouchPad READ hasTouchPad CONSTANT)
    Q_PROPERTY(ConnectionType connectionType READ connectionType NOTIFY connectionTypeChanged)

public:
    Gamepad(SDL_Joystick *joystick, SDL_GameController *controller, QObject *parent = nullptr);

    enum ConnectionType {
        UnknownType,
        USBType,
        BluetoothType,
    };
    Q_ENUM(ConnectionType)

    QString name() const;
    QString path() const;
    QString model() const;
    int numButtons() const;
    int numAxes() const;
    SDL_GameControllerType gamepadType() const;
    bool hasRumble() const;
    bool hasTouchPad() const;
    ConnectionType connectionType() const;

    Q_INVOKABLE GamepadButton *button(int sdlButtonType);
    Q_INVOKABLE bool hasButton(int sdlButtonType) const;

    Q_INVOKABLE GamepadStick *axis(int sdlAxis);
    Q_INVOKABLE bool hasAxis(int sdlAxis) const;

    Q_INVOKABLE GamepadTrigger *trigger(int sdlTrigger);
    Q_INVOKABLE bool hasTrigger(int sdlTrigger) const;

    SDL_Joystick *joystick() const;
    SDL_GameController *gamecontroller() const;

Q_SIGNALS:
    void buttonStateChanged(SDL_GameControllerButton button);
    void axisStateChanged(int index);
    void triggerStateChanged(int index);

    // Possible when going from USB to Bluetooth, or vice versa
    void connectionTypeChanged();

private:
    friend class DeviceModel;

    void onButtonEvent(SDL_ControllerButtonEvent sdlEvent);
    void onAxisEvent(SDL_ControllerAxisEvent sdlEvent);

    SDL_Joystick *m_joystick = nullptr;
    SDL_GameController *m_gameController = nullptr;

    QMap<int, GamepadButton *> m_buttons;
    QMap<int, GamepadStick *> m_axes;
    QMap<int, GamepadTrigger *> m_triggers;

    QString m_name;
    QString m_path;
    uint16_t m_vendor = 0;
    QString m_model;
    int m_numButtons = 0;
    int m_numAxes = 0;
    bool m_hasRumble = false;

    bool m_hasTouchPad = false;
    ConnectionType m_connectionType = UnknownType;
};
