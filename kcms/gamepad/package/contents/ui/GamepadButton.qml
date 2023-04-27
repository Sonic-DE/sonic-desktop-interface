/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt5Compat.GraphicalEffects
import QtQuick.Controls 2.0 as QQC2
import org.kde.plasma.gamepad.kcm 1.0

/* This is for a single gamepad button
   In order to make it relatively simpler to show
   a gamepad for showing button state and arrangement.
   This widget shows a single button of a given vendor
   using images from images/ folder.
*/
Item {
    id: root

    // Which button this is
    required property var idx
    required property var device

    readonly property var button: root.device.buttons[idx]

    required property var posX
    required property var posY
    required property var sizeWidth
    required property var sizeHeight
    required property var controllerWidth
    required property var controllerHeight
    required property var image

    Image {
        id: icon
        x: (root.posX * root.controllerWidth)
        y: (root.posY * root.controllerHeight)
        width: (root.sizeWidth * root.controllerWidth)
        height: (root.sizeHeight * root.controllerHeight)
        source: root.image

        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.text: button.name

        HoverHandler {
            id: hoverHandler
            acceptedDevices: PointerDevice.Mouse
        }

        Component.onCompleted: {
            console.log("button with index " + idx)
            console.log("width :" + width + " height : " + height)
            console.log("sizeWidth: " + root.sizeWidth)
            console.log("x: " + x + " y: " + y)
            console.log("source: " + source)
        }
    }

    ColorOverlay {
        visible: root.button.state
        anchors.fill: icon
        source: icon
        color: "#444444"
    }
}
