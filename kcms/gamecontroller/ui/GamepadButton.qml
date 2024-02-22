/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

import org.kde.plasma.gamecontroller.kcm

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

    readonly property var button: root.device.button(idx)

    required property var svgItem
    required property var elementId

    visible: root.device.hasButton(idx)

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
        anchors.fill: icon

        source: icon
        color: (root.button && root.button.state) ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
    }
}
