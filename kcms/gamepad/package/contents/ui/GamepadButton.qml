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
    // Which button this is
    property var ix
    property var device
    property var joybutton: device.buttons[ix]
    property bool pressed: joybutton.state

    Image {
        id: icon
        x: joybutton.position.x
        y: joybutton.position.y
        source: joybutton.image
    }

    ColorOverlay {
        visible: joybutton.state
        anchors.fill: icon
        source: icon
        color: "#777777"
    }
}
