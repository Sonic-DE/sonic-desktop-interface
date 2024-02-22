/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects

import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami

import org.kde.plasma.gamecontroller.kcm

/* This is for showing a gamepad trigger button*/
Item {
    id: root

    // Which trigger this is
    required property var idx
    required property var device

    readonly property var trigger: root.device.trigger(idx)

    required property var svgItem
    required property var elementId

    visible: root.device.hasTrigger(idx)

    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.text: trigger.name

    HoverHandler {
        id: hoverHandler

        acceptedDevices: PointerDevice.Mouse
    }

    KSvg.SvgItem {
        id: icon

        visible: false
        width: Math.round(elementRect.width)
        height: Math.round(elementRect.height)
        x: Math.round(elementRect.x)
        y: Math.round(elementRect.y)

        svg: svgItem
        elementId: root.elementId
    }

    ColorOverlay {
        opacity: 1 - trigger.value
        anchors.fill: icon
        source: icon
        color: Kirigami.Theme.textColor
    }

    ColorOverlay {
        opacity: trigger.value
        anchors.fill: icon
        source: icon
        color: Kirigami.Theme.highlightColor
    }
}
