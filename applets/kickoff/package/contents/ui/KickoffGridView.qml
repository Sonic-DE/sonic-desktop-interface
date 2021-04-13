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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

EmptyScrollView {
    id: root
    property alias model: gridView.model
    property alias count: gridView.count
    property alias currentIndex: gridView.currentIndex
    property alias currentItem: gridView.currentItem
    property alias delegate: gridView.delegate

    PC3.ScrollBar.vertical.policy: PC3.ScrollBar.AlwaysOn

    horizontalPadding: KickoffSingleton.backgroundMetrics.margins.left
//     rightPadding: KickoffSingleton.backgroundMetrics.margins.right
    verticalPadding: KickoffSingleton.backgroundMetrics.margins.top
//     bottomPadding: KickoffSingleton.backgroundMetrics.margins.bottom

    clip: gridView.interactive

    // implicitContentWidth returns -1 for some reason
    implicitWidth: gridView.cellHeight * 4 + leftPadding + rightPadding
    contentHeight: gridView.cellHeight * 4

    contentItem: GridView {
        id: gridView
        focus: true

        interactive: height < contentHeight

        cellHeight: KickoffSingleton.gridCellSize
        cellWidth: KickoffSingleton.gridCellSize

        pixelAligned: true
        reuseItems: true
        keyNavigationEnabled: true
        keyNavigationWraps: false
        boundsBehavior: Flickable.StopAtBounds

        delegate: KickoffItemDelegate {
            id: itemDelegate
            icon.width: PlasmaCore.Units.iconSizes.large
            icon.height: PlasmaCore.Units.iconSizes.large
            display: PC3.AbstractButton.TextUnderIcon
            width: view && view.cellWidth || implicitWidth
        }

        highlight: PlasmaCore.FrameSvgItem {
            width: gridView.cellWidth
            height: gridView.cellHeight
            imagePath: "widgets/viewitem"
            prefix: "hover"
        }

        highlightFollowsCurrentItem: true
        highlightMoveDuration: 0

        Connections {
            target: plasmoid
            function onExpandedChanged() {
                if(!plasmoid.expanded) {
                    root.currentIndex = 0
                }
            }
        }
    }
}
