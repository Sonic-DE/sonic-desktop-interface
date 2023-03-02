/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtGraphicalEffects 1.0
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
    readonly property bool pressed: root.button.state

    required property var posX
    required property var posY
    required property var controllerImage

    Image {
        id: icon
        x: (root.posX * root.controllerImage.paintedWidth)
        y: (root.posY * root.controllerImage.paintedHeight)
        source: root.button.image

        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.text: button.name

        HoverHandler {
            id: hoverHandler
            acceptedDevices: PointerDevice.Mouse
        }
    }

    ColorOverlay {
        visible: root.button.state
        anchors.fill: icon
        source: icon
        color: "#777777"
    }
}
