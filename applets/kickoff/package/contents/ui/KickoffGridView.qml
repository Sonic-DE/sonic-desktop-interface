/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
 *   Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

EmptyPage {
    id: root
    property alias model: view.model
    property alias count: view.count
    property alias currentIndex: view.currentIndex
    property alias currentItem: view.currentItem
    property alias delegate: view.delegate

    clip: view.interactive

    header: MouseArea {
        implicitHeight: KickoffSingleton.backgroundMetrics.margins.top
        hoverEnabled: true
        onEntered: {
            listView.currentIndex = listView.indexAt(0,0)
        }
    }

    footer: MouseArea {
        implicitHeight: KickoffSingleton.backgroundMetrics.margins.bottom
        hoverEnabled: true
        onEntered: {
            listView.currentIndex = listView.indexAt(0, listView.height)
        }
    }

    contentItem: GridView {
        id: view

        property real viewWidth: width - leftMargin - rightMargin
        property real viewHeight: height - topMargin - bottomMargin

        implicitWidth: view.cellHeight * 4 + leftMargin + rightMargin
        implicitHeight: view.cellHeight * 4 + topMargin + bottomMargin

        leftMargin: verticalScrollBar.width
        rightMargin: verticalScrollBar.width

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
            opacity: view.activeFocus ? 1 : 0.5
            width: view.cellWidth
            height: view.cellHeight
            imagePath: "widgets/viewitem"
            prefix: "hover"
        }

        delegate: KickoffItemDelegate {
            id: itemDelegate
            icon.width: PlasmaCore.Units.iconSizes.large
            icon.height: PlasmaCore.Units.iconSizes.large
            display: PC3.AbstractButton.TextUnderIcon
            width: view.cellWidth
            Item {
                parent: itemDelegate
                anchors.fill: parent
                anchors.margins: 1
                enabled: !verticalScrollBar.active
                HoverHandler {
                    onPointChanged: {
                        if (hovered) {
                            itemDelegate.forceActiveFocus(Qt.MouseFocusReason)
                            view.currentIndex = index
                        }
                    }
                }
            }
        }

        move: normalTransition
        moveDisplaced: normalTransition

        Transition {
            id: normalTransition
            NumberAnimation {
                duration: Plasma.Units.shortDuration
                properties: "x, y"
                easing.type: Easing.OutCubic
            }
        }

        PC3.ScrollBar.vertical: PC3.ScrollBar {
            id: verticalScrollBar
            visible: size < 1 && policy !== PC3.ScrollBar.AlwaysOff
            z: 2
        }

        //MouseArea {
            //id: marginMouseArea
            //parent: view
            //anchors.fill: parent
            //z: 1
            //enabled: !verticalScrollBar.active
            //hoverEnabled: true
            //propagateComposedEvents: true
            //onEntered: {
                //view.forceActiveFocus(Qt.MouseFocusReason)
            //}
            //onPositionChanged: {
                //let oldIndex = view.currentIndex
                //let newIndex = view.indexAt(mouseX + view.contentX, mouseY + view.contentY)
                //view.currentIndex = newIndex >= 0 ? newIndex : oldIndex
            //}
        //}

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
            value: view
            when: activeFocus
            restoreMode: Binding.RestoreBinding
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    view.positionViewAtBeginning()
                }
            }
        }
    }
}
