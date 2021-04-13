/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import QtQml 2.15

EmptyScrollView {
    id: root
    property alias model: gridView.model
    property alias count: gridView.count
    property alias currentIndex: gridView.currentIndex
    property alias currentItem: gridView.currentItem
    property alias delegate: gridView.delegate

    PC3.ScrollBar.vertical.policy: PC3.ScrollBar.AlwaysOn

    horizontalPadding: 8//KickoffSingleton.backgroundMetrics.margins.left
    verticalPadding: 8//KickoffSingleton.backgroundMetrics.margins.top

    clip: gridView.interactive

    // implicitContentWidth returns -1 for some reason
    implicitWidth: gridView.cellHeight * 4 + leftPadding + rightPadding
    contentHeight: gridView.cellHeight * 4

    contentItem: GridView {
        id: gridView

        cellHeight: KickoffSingleton.gridCellSize
        cellWidth: KickoffSingleton.gridCellSize

        currentIndex: count > 0 ? 0 : -1
        focus: true
        interactive: height < contentHeight
        pixelAligned: true
        reuseItems: true
        boundsBehavior: Flickable.StopAtBounds
        keyNavigationEnabled: true
        keyNavigationWraps: false

        highlightMoveDuration: 0
        highlight: PlasmaCore.FrameSvgItem {
            opacity: gridView.activeFocus ? 1 : 0.5
            width: gridView.cellWidth
            height: gridView.cellHeight
            imagePath: "widgets/viewitem"
            prefix: "hover"
        }

        delegate: KickoffItemDelegate {
            id: itemDelegate
            icon.width: PlasmaCore.Units.iconSizes.large
            icon.height: PlasmaCore.Units.iconSizes.large
            display: PC3.AbstractButton.TextUnderIcon
            width: view && view.cellWidth || implicitWidth
        }

        // These have to be defined here because GridView will eat the up/down
        // events even if it can't go any further up or down
        Keys.priority: Keys.AfterItem
        // Not using KeyNavigation.up because otherwise pressing down when the
        // header or search field is focused will focus the contentAreaView
        // instead of the lastFocusedView.
        Keys.onUpPressed: {
            KickoffSingleton.searchField.forceActiveFocus(Qt.BacktabFocusReason)
        }
        // The issue above does not apply to this when pressing up in the footer
        KeyNavigation.down: KickoffSingleton.footer

        Binding {
            target: KickoffSingleton
            property: "lastFocusedView"
            value: gridView
            when: activeFocus
            restoreMode: Binding.RestoreBinding
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    gridView.positionViewAtBeginning()
                }
            }
        }
    }
}
