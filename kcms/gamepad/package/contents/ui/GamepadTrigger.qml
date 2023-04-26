/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import Qt5Compat.GraphicalEffects
import QtQuick.Controls 2.0 as QQC2
import org.kde.plasma.gamepad.kcm 1.0

/* This is for showing a gamepad trigger button*/
Item {
    id: root

    // Which trigger this is
    required property var idx
    required property var device

    readonly property var trigger: root.device.triggers[idx]

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
        height: width
        source: root.image

        QQC2.ToolTip.visible: hoverHandler.hovered
        QQC2.ToolTip.text: trigger.name

        HoverHandler {
            id: hoverHandler
            acceptedDevices: PointerDevice.Mouse
        }
    }

    ColorOverlay {
        visible: true
        opacity: trigger.value
        anchors.fill: icon
        source: icon
        color: "#777777"
    }
}
