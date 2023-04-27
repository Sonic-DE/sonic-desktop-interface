/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt5Compat.GraphicalEffects
import QtQuick.Controls 2.0 as QQC2
import org.kde.plasma.gamepad.kcm 1.0

/* This is for showing a gamepad axis */
Item {
    id: root

    // Which axis this is
    required property var idx
    required property var buttonidx
    required property var device

    readonly property var axis: root.device.axes[idx]
    readonly property var button: root.device.buttons[buttonidx]

    required property var posX
    required property var posY
    required property var sizeWidth
    required property var sizeHeight
    required property var controllerWidth
    required property var controllerHeight

    Rectangle {
        id: outerRing
        border.color: "black"
        color: "transparent"
        x: (root.posX * root.controllerWidth)
        y: (root.posY * root.controllerHeight)
        width: (root.sizeWidth * root.controllerWidth)
        height: width
        radius: 0.5 * width
    }

    Rectangle {
        id: icon
        x: (root.posX * root.controllerWidth) + (root.axis.gridValue.x * 50) + (width * 0.1)
        y: (root.posY * root.controllerHeight) + (root.axis.gridValue.y * 50) + (width * 0.1)
        width: (root.sizeWidth * root.controllerWidth * 0.8) // Slightly smaller
        height: width
        radius: 0.5 * width

        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.text: axis.name

        HoverHandler {
            id: hoverHandler
            acceptedDevices: PointerDevice.Mouse
        }
    }

    ColorOverlay {
        visible: root.button.state
        anchors.fill: icon
        source: icon
        color: "#444444"
    }
}
