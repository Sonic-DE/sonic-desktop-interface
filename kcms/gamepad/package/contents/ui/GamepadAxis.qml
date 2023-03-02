/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/* This is for a single gamepad button
   In order to make it relatively simpler to show
   a gamepad for showing button state and arrangement.
   This widget shows a single button of a given vendor
   using images from images/ folder.
*/

import QtQuick 2.15
import QtGraphicalEffects 1.0
import org.kde.plasma.gamepad.kcm 1.0

Item {
    id: button

    // Which axis this is
    property var idx
    property var device
    property var joybutton: device.axes[idx]
    //property bool pressed: joybutton.state

    property var posX
    property var posY
    property var controllerImage

    Rectangle {
        id: icon
        width: controllerImage.paintedWidth * 0.03
        height: width
        x: (posX * controllerImage.paintedWidth) + (joybutton.gridValue.x * 50)
        y: (posY * controllerImage.paintedHeight) + (joybutton.gridValue.y * 50)
        radius: 0.5 * width
    }

    /*ColorOverlay {
        visible: joybutton.state
        anchors.fill: icon
        source: icon
        color: "#777777"
    }*/
}
