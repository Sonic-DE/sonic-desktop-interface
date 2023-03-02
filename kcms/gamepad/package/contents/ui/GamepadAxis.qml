/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtGraphicalEffects 1.0
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
        id: icon
        width: root.controllerImage.paintedWidth * 0.03
        height: width
        x: (root.posX * root.controllerImage.paintedWidth) + (root.axis.gridValue.x * 50)
        y: (root.posY * root.controllerImage.paintedHeight) + (root.axis.gridValue.y * 50)
        radius: 0.5 * width
    }
}
