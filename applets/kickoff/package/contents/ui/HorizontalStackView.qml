/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.StackView {
    id: control
    property bool reverseTransitions: false
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    popEnter: enterTransition
    popExit: exitTransition
    pushEnter: enterTransition
    pushExit: exitTransition
    replaceEnter: enterTransition
    replaceExit: exitTransition
    clip: busy
    contentItem: currentItem
    // Using NumberAnimation instead of XAnimator because the latter wasn't always smooth enough
    Transition {
        id: enterTransition
        NumberAnimation {
            properties: "x"
            from: (control.reverseTransitions ? -0.5 : 0.5) * (control.mirrored ? -1 : 1) * -control.width
            to: 0
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation { property: "opacity"
            from: 0.0
            to: 1.0
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    Transition {
        id: exitTransition
        NumberAnimation {
            property: "x"
            from: 0
            to: (control.reverseTransitions ? -0.5 : 0.5) * (control.mirrored ? -1 : 1) * control.width
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
        NumberAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
}
