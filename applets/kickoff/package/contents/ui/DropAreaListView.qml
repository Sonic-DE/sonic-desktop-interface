/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.private.kicker 0.1 as Kicker

KickoffListView {
    id: root
    delegate: DraggableItemDelegate {
        id: itemDelegate
        extendHoverMargins: true
        width: itemDelegate.view.availableWidth
        mouseArea.drag.axis: Drag.YAxis
    }
    DropArea {
        id: dropArea
        z: -1
        parent: root
        anchors.fill: parent
        enabled: plasmoid.immutability !== PlasmaCore.Types.SystemImmutable
        onPositionChanged: if (drag.source instanceof DraggableItemDelegate) {
            const source = drag.source
            const view = drag.source.view
            if (view === root.view && !view.move.running && !view.moveDisplaced.running) {
                const pos = mapToItem(view.contentItem, drag.x, drag.y)
                const targetIndex = view.indexAt(pos.x, pos.y)
                if (targetIndex >= 0 && targetIndex !== source.index) {
                    view.model.moveRow(source.index, targetIndex)
                    // itemIndex changes directly after moving,
                    // we can just set the currentIndex to it then.
                    view.currentIndex = source.index
                }
            }
        }
    }
    SmoothedAnimation {
        id: autoScrollUp
        target: root.view
        property: "contentY"
        to: 0
        velocity: 200 * PlasmaCore.Units.devicePixelRatio
        running: dropArea.containsDrag && dropArea.drag.y <= root.header.height * 2
    }
    SmoothedAnimation {
        id: autoScrollDown
        target: root.view
        property: "contentY"
        to: root.view.contentHeight - root.view.height
        velocity: 200 * PlasmaCore.Units.devicePixelRatio
        running: dropArea.containsDrag && dropArea.drag.y > root.height - root.footer.height * 2
    }
}
