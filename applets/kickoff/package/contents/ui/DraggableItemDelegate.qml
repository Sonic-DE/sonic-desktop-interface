/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

KickoffItemDelegate {
    id: root
    property real initialX: x
    property real initialY: y
    readonly property bool dragActive: mouseArea.drag.active
    // The default Z value for delegates is 1. The default Z value for the section delegate is 2.
    // The highlight gets a value of 3 while the drag is active and then goes back to the default value of 0.
    z: dragActive ? 4 : 1
    mouseArea.drag {
        target: plasmoid.immutability !== PlasmaCore.Types.SystemImmutable ? root : undefined
        minimumX: 0
        maximumX: root.view.availableWidth - root.width
        minimumY: 0
        maximumY: root.view.availableHeight - root.height
    }
    Binding {
        target: KickoffSingleton
        property: "dragSource"
        value: root
        when: root.dragActive
        restoreMode: Binding.RestoreBindingOrValue
    }
    Binding {
        target: KickoffSingleton.dragHelper
        property: "dragIconSize"
        value: root.icon.height
        when: root.dragActive
        restoreMode: Binding.RestoreBindingOrValue
    }
    onDragActiveChanged: if (dragActive) {
        // Fixes warning: "Passing incompatible arguments to C++ functions from JavaScript is dangerous and deprecated."
        const url = root.url ? root.url : ""
        KickoffSingleton.dragHelper.startDrag(root, url, decoration)
    }
}
