/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import QtQml 2.15

EmptyScrollView {
    id: root
    property alias model: listView.model
    property alias count: listView.count
    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias delegate: listView.delegate
    property alias section: listView.section

    clip: listView.interactive

    // If either uses a grid, use grid cells to determine size
    contentHeight: plasmoid.configuration.favoritesDisplay == 0 || plasmoid.configuration.applicationsDisplay == 0
        ? KickoffSingleton.gridCellSize * 4 : KickoffSingleton.listDelegateHeight * 10

    verticalPadding: KickoffSingleton.backgroundMetrics.margins.top
    horizontalPadding: KickoffSingleton.backgroundMetrics.margins.left

    contentItem: ListView {
        width: root.availableWidth
        id: listView

        // Currently narrator only notifies about focus changes
        // That means that if item has focus narrator won't notify about name/description changes, like count changing
        // which is most apparent with submenus
        // We work around this by having the first focused item having the list count and name as a description
        // When we unfocus it goes back to only reporting it's name
        // That way we create a seamless experience where when model changes we always report the new item count

        // Determines whether or not we tell the amount of items in the list
        property bool accessibilityCount: true

        currentIndex: count > 0 ? 0 : -1
        focus: true
        interactive: height < contentHeight
        pixelAligned: true
        reuseItems: true
        boundsBehavior: Flickable.StopAtBounds
        keyNavigationEnabled: true
        keyNavigationWraps: false
    //     verticalLayoutDirection: root.upsideDown ? ListView.BottomToTop : ListView.TopToBottom

        // This is actually needed. The highlight will animate from thin to wide otherwise.
        highlightResizeDuration: 0
        highlightMoveDuration: 0
        highlight: PlasmaCore.FrameSvgItem {
            opacity: listView.activeFocus ? 1 : 0.5
            imagePath: "widgets/viewitem"
            prefix: "hover"
            //opacity: navigationMethod.state != "keyboard" || (listView.hasKeyboardFocus && listView.activeFocus) ? 1 : 0.5
        }

        delegate: KickoffItemDelegate {
            id: itemDelegate
            viewScrolling: root.PC3.ScrollBar.vertical.active
            width: view && view.width || implicitWidth
        }

        section {
            property: "group"
            criteria: ViewSection.FullString
            delegate: PC3.Label {
                property real delegateContentHeight: Math.max(PlasmaCore.Units.iconSizes.smallMedium, PlasmaCore.Theme.mSize(root.font).height)
                width: section.length === 1 ? delegateContentHeight + leftPadding + rightPadding : listView.contentItem.width
                height: KickoffSingleton.listDelegateHeight
                leftPadding: listView.effectiveLayoutDirection === Qt.LeftToRight
                    ? KickoffSingleton.listItemMetrics.margins.left : 0
                rightPadding: listView.effectiveLayoutDirection === Qt.RightToLeft
                    ? KickoffSingleton.listItemMetrics.margins.right : 0
                // important for RTL (otherwise label won't reverse)
                horizontalAlignment: section.length === 1 ? Text.AlignHCenter : Text.AlignLeft
                verticalAlignment: Text.AlignVCenter
                maximumLineCount: 1
                elide: Text.ElideRight
                font.pixelSize: delegateContentHeight
                enabled: false
                // Force it to be uppercase or else if the first item in a section starts
                // with a lowercase letter, the header letter will be lowercase too!
                // Only applies to alphabetical characters in "All applications"
                text: section.length === 1 ? section.toUpperCase() : section
            }
        }

        //add: normalTransition
        //displaced: normalTransition
        move: normalTransition
        moveDisplaced: normalTransition
//         populate: normalTransition
        //remove: normalTransition
        Transition {
            id: normalTransition
            NumberAnimation {
                duration: Plasma.Units.shortDuration
                properties: "x, y"
                easing.type: Easing.OutCubic
            }
        }

        // These have to be defined here because ListView will eat the up/down
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
            value: listView
            when: activeFocus
            restoreMode: Binding.RestoreBinding
        }

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    listView.positionViewAtBeginning()
                }
            }
        }
    }
}
