/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kcm 1.4 as KCM
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Layouts 1.3 as Layouts
import org.kde.plasma.gamepad.kcm 1.0
import QtQuick.Shapes 1.15

import ".."

Image {
    id: root

    required property var device

    source: "../images/genericgamepad.svg"

    //anchors.top: formLayout.bottom
    y: 300

    width: parent.width
    height: parent.width * (sourceSize.height / sourceSize.width)

    //sourceSize.width: paintedWidth
    //sourceSize.height: paintedHeight

    /*Rectangle {
        width: 0.08 * controllerImage.paintedWidth
        height: width
        radius: width * 0.5
        x: (0.3 * controllerImage.paintedWidth) + currentDevice.axes[0].gridValue.x * 50
        y: (controllerImage.paintedHeight / 2) + currentDevice.axes[0].gridValue.y * 50
    }

    Rectangle {
        width: 0.08 * controllerImage.paintedWidth
        height: width
        radius: width * 0.5
        x: (0.6 * controllerImage.paintedWidth) + currentDevice.axes[1].gridValue.x * 50
        y: (controllerImage.paintedHeight / 2) + currentDevice.axes[1].gridValue.y * 50
    }*/

    // left shoulder
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_LEFTSHOULDER
        device: root.device
        controllerImage: root

        posX: 0.27
        posY: 0.08
    }

    // right shoulder
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
        device: root.device
        controllerImage: root

        posX: 0.8
        posY: 0.08
    }

    // left d-pad
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_LEFT
        device: root.device
        controllerImage: root

        posX: 0.38
        posY: 0.54
    }

    // right d-pad
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_RIGHT
        device: root.device
        controllerImage: root

        posX: 0.42
        posY: 0.54
    }

    // top d-pad
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_UP
        device: root.device
        controllerImage: root

        posX: 0.40
        posY: 0.52
    }

    // bottom d-pad
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_DOWN
        device: root.device
        controllerImage: root

        posX: 0.40
        posY: 0.56
    }

    // left stick
    GamepadStick {
        idx: 0
        buttonidx: GamepadButton.SDL_CONTROLLER_BUTTON_LEFTSTICK
        device: root.device
        controllerImage: root

        posX: 0.3
        posY: 0.3
    }

    // right stick
    GamepadStick {
        idx: 1
        buttonidx: GamepadButton.SDL_CONTROLLER_BUTTON_RIGHTSTICK
        device: root.device
        controllerImage: root

        posX: 0.67
        posY: 0.56
    }

    // X button
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_X
        device: root.device
        controllerImage: root

        posX: 0.78
        posY: 0.2
    }

    // Y button
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_Y
        device: root.device
        controllerImage: root

        posX: 0.73
        posY: 0.3
    }

    // B button
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_B
        device: root.device
        controllerImage: root

        posX: 0.78
        posY: 0.42
    }

    // A button
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_A
        device: root.device
        controllerImage: root

        posX: 0.82
        posY: 0.3
    }

    // select
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_BACK
        device: root.device
        controllerImage: root

        posX: 0.46
        posY: 0.24
    }

    // capture
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_MISC1
        device: root.device
        controllerImage: root

        posX: 0.48
        posY: 0.33
    }

    // home
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_GUIDE
        device: root.device
        controllerImage: root

        posX: 0.52
        posY: 0.33
    }

    // start
    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_START
        device: root.device
        controllerImage: root

        posX: 0.54
        posY: 0.24
    }

}
