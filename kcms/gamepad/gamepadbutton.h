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

class GamepadButton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name MEMBER m_name CONSTANT)
    Q_PROPERTY(QString image READ getImage CONSTANT)
    Q_PROPERTY(bool state READ getState NOTIFY stateChanged)
public:
    explicit GamepadButton(uint16_t vendor, const int code, QObject *parent = nullptr);
    // For QML usage of buttons
    GamepadButton();

    enum SDL_Buttons {
        // From SDL_gamecontroller.h so we can use them in qml
        SDL_CONTROLLER_BUTTON_INVALID = -1,
        SDL_CONTROLLER_BUTTON_A,
        SDL_CONTROLLER_BUTTON_B,
        SDL_CONTROLLER_BUTTON_X,
        SDL_CONTROLLER_BUTTON_Y,
        SDL_CONTROLLER_BUTTON_BACK,
        SDL_CONTROLLER_BUTTON_GUIDE,
        SDL_CONTROLLER_BUTTON_START,
        SDL_CONTROLLER_BUTTON_LEFTSTICK,
        SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_DPAD_UP,
        SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        SDL_CONTROLLER_BUTTON_MISC1, /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
        SDL_CONTROLLER_BUTTON_PADDLE1, /* Xbox Elite paddle P1 */
        SDL_CONTROLLER_BUTTON_PADDLE2, /* Xbox Elite paddle P3 */
        SDL_CONTROLLER_BUTTON_PADDLE3, /* Xbox Elite paddle P2 */
        SDL_CONTROLLER_BUTTON_PADDLE4, /* Xbox Elite paddle P4 */
        SDL_CONTROLLER_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    };
    Q_ENUMS(SDL_Buttons)

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

    friend class Gamepad;

signals:
    void stateChanged();

private:
    // Give a button name for given ev code
    QString name(int code);

    // Give path to an image to show in the gui for the given button.
    QString image(int code);

    uint16_t m_vendor;
    int m_code;
    QString m_name;

    bool m_state;
};
