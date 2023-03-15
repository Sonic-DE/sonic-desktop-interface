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

    source: "./images/controller.svg"

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

    // left trigger
    GamepadButton {
        idx: 9
        device: root.device
        controllerImage: root

        posX: 0.27
        posY: 0.08
    }

    // right trigger
    GamepadButton {
        idx: 10
        device: root.device
        controllerImage: root

        posX: 0.8
        posY: 0.08
    }

    // left d-pad
    GamepadButton {
        idx: 13
        device: root.device
        controllerImage: root

        posX: 0.28
        posY: 0.3
    }

    // right d-pad
    GamepadButton {
        idx: 14
        device: root.device
        controllerImage: root

        posX: 0.32
        posY: 0.3
    }

    // top d-pad
    GamepadButton {
        idx: 11
        device: root.device
        controllerImage: root

        posX: 0.3
        posY: 0.26
    }

    // bottom d-pad
    GamepadButton {
        idx: 12
        device: root.device
        controllerImage: root

        posX: 0.3
        posY: 0.34
    }

    // left stick
    GamepadStick {
        idx: 0
        buttonidx: 7
        device: root.device
        controllerImage: root

        posX: 0.42
        posY: 0.56
    }

    // right stick
    GamepadStick {
        idx: 1
        buttonidx: 8
        device: root.device
        controllerImage: root

        posX: 0.67
        posY: 0.56
    }

    // west button
    GamepadButton {
        idx: 2
        device: root.device
        controllerImage: root

        posX: 0.73
        posY: 0.3
    }

    // east button
    GamepadButton {
        idx: 1
        device: root.device
        controllerImage: root

        posX: 0.82
        posY: 0.3
    }

    // north button
    GamepadButton {
        idx: 3
        device: root.device
        controllerImage: root

        posX: 0.78
        posY: 0.2
    }

    // south button
    GamepadButton {
        idx: 0
        device: root.device
        controllerImage: root

        posX: 0.78
        posY: 0.42
    }

    // guide
    GamepadButton {
        idx: 5
        device: root.device
        controllerImage: root

        posX: 0.5
        posY: 0.13
    }

    // select
    GamepadButton {
        idx: 4
        device: root.device
        controllerImage: root

        posX: 0.44
        posY: 0.24
    }

    // start
    GamepadButton {
        idx: 6
        device: root.device
        controllerImage: root

        posX: 0.54
        posY: 0.24
    }

    // misc1
    GamepadButton {
        idx: 15
        device: root.device
        controllerImage: root

        posX: 0.5
        posY: 0.33
    }
}
