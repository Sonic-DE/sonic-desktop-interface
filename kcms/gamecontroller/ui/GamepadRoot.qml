/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import Qt5Compat.GraphicalEffects

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

import org.kde.plasma.gamecontroller.kcm
import "."

Rectangle {
    id: root
    color: "blue"

    required property var device
    required property string svgPath

    KSvg.Svg {
        id: svgitem

        imagePath: svgPath
        size.width: root.width
        size.height: root.height
    }

    ColorOverlay {
        anchors.fill: image

        source: image
        color: Kirigami.Theme.disabledTextColor
    }

    KSvg.SvgItem {
        id: image

        visible: false
        width: root.width
        height: root.height

        svg: svgitem
        elementId: "base"
    }

    GamepadTrigger {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_TRIGGERRIGHT
        device: root.device
        svgItem: svgitem
        elementId: "right-trigger"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_RIGHTSHOULDER
        device: root.device
        svgItem: svgitem
        elementId: "right-shoulder"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_LEFTSHOULDER
        device: root.device
        svgItem: svgitem
        elementId: "left-shoulder"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_START
        device: root.device
        svgItem: svgitem
        elementId: "mid-right"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_BACK
        device: root.device
        svgItem: svgitem
        elementId: "mid-left"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_UP
        device: root.device
        svgItem: svgitem
        elementId: "up"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_RIGHT
        device: root.device
        svgItem: svgitem
        elementId: "right"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_DOWN
        device: root.device
        svgItem: svgitem
        elementId: "down"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_DPAD_LEFT
        device: root.device
        svgItem: svgitem
        elementId: "left"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_GUIDE
        device: root.device
        svgItem: svgitem
        elementId: "center"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_X
        device: root.device
        svgItem: svgitem
        elementId: "x-button"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_Y
        device: root.device
        svgItem: svgitem
        elementId: "y-button"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_A
        device: root.device
        svgItem: svgitem
        elementId: "a-button"
    }

    GamepadButton {
        idx: GamepadButton.SDL_CONTROLLER_BUTTON_B
        device: root.device
        svgItem: svgitem
        elementId: "b-button"
    }

    GamepadTrigger {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_TRIGGERLEFT
        device: root.device
        svgItem: svgitem
        elementId: "left-trigger"
    }

    GamepadStick {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_LEFTX
        buttonidx: GamepadButton.SDL_CONTROLLER_BUTTON_LEFTSTICK
        device: root.device
        svgItem: svgitem
        elementId: "l-pad"
    }

    GamepadStick {
        idx: GamepadButton.SDL_CONTROLLER_AXIS_RIGHTX
        buttonidx: GamepadButton.SDL_CONTROLLER_BUTTON_RIGHTSTICK
        device: root.device
        svgItem: svgitem
        elementId: "r-pad"
    }
}
