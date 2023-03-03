/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.0 as QQC2
import org.kde.plasma.gamepad.kcm 1.0

/* This is for showing a gamepad axis */
Item {
    id: root

    // Which axis this is
    required property var idx
    required property var device

    readonly property var axis: root.device.axes[idx]

    required property var posX
    required property var posY
    required property var controllerImage

    Rectangle {
        id: outerRing
        border.color: "black"
        color: "transparent"
        width: root.controllerImage.paintedWidth * 0.05
        height: width
        x: (root.posX * root.controllerImage.paintedWidth) - (root.controllerImage.paintedWidth * 0.005)
        y: (root.posY * root.controllerImage.paintedHeight) - (root.controllerImage.paintedWidth * 0.005)
        radius: 0.5 * width
    }

    Rectangle {
        id: icon
        width: root.controllerImage.paintedWidth * 0.04
        height: width
        x: (root.posX * root.controllerImage.paintedWidth) + (root.axis.gridValue.x * 50)
        y: (root.posY * root.controllerImage.paintedHeight) + (root.axis.gridValue.y * 50)
        radius: 0.5 * width

        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.text: axis.name

        HoverHandler {
            id: hoverHandler
            acceptedDevices: PointerDevice.Mouse
        }
    }
}
