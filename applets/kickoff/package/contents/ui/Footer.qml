/*
 *    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
 *    Copyright (C) 2021 by Noah Davis <noahadvs@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaExtras.PlasmoidHeading {
    id: root

    property alias tabBarCurrentIndex: tabBar.currentIndex
    readonly property real implicitTabBarWidth: tabBar.implicitWidth
    property real preferredTabBarWidth: 0

    contentWidth: tabBar.implicitWidth + root.spacing + separator.implicitWidth + root.spacing + leaveButtons.implicitWidth
    contentHeight: leaveButtons.implicitHeight
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    leftPadding: KickoffSingleton.backgroundMetrics.margins.left
    rightPadding: KickoffSingleton.backgroundMetrics.margins.right
    topPadding: KickoffSingleton.backgroundMetrics.margins.top
    bottomPadding: KickoffSingleton.backgroundMetrics.margins.bottom

    leftInset: 0
    rightInset: 0
    topInset: 0
    bottomInset: 0

//     height: (root.opacity == 0) ? 0 : implicitHeight
    location: KickoffSingleton.reverseVerticalLayout ? PlasmaExtras.PlasmoidHeading.Location.Header : PlasmaExtras.PlasmoidHeading.Location.Footer

    spacing: KickoffSingleton.backgroundMetrics.margins.left

    PC3.TabBar {
        id: tabBar
        property real tabWidth: Math.max(applicationsTab.implicitWidth, placesTab.implicitWidth)
        focus: true
        width: root.preferredTabBarWidth || implicitWidth
        implicitWidth: contentWidth + leftPadding + rightPadding
        implicitHeight: contentHeight + topPadding + bottomPadding
        contentHeight: root.height

        // This is needed to keep the sparators horizontally aligned
        leftPadding: mirrored ? root.spacing : 0
        rightPadding: !mirrored ? root.spacing : 0

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }

        topPadding: -root.topPadding
        bottomPadding: -root.bottomPadding

        position: KickoffSingleton.reverseVerticalLayout ? PC3.TabBar.Header : PC3.TabBar.Footer

        // Workaround contentItem not having `focus: true`
        Binding {
            target: tabBar.contentItem
            when: tabBar.contentItem !== null
            property: "focus"
            value: true
            restoreMode: Binding.RestoreBindingOrValue
        }

        PC3.TabButton {
            id: applicationsTab
            focus: true
            width: tabBar.tabWidth
            height: parent.height
            icon.width: PlasmaCore.Units.iconSizes.smallMedium
            icon.height: PlasmaCore.Units.iconSizes.smallMedium
            icon.name: "applications-other"
            text: i18n("Applications")
            KeyNavigation.backtab: KickoffSingleton.contentAreaView != null ? KickoffSingleton.contentAreaView : null
        }
        PC3.TabButton {
            id: placesTab
            width: tabBar.tabWidth
            height: parent.height
            icon.width: PlasmaCore.Units.iconSizes.smallMedium
            icon.height: PlasmaCore.Units.iconSizes.smallMedium
            icon.name: "compass"
            text: i18n("Places") //Explore?
        }
    }
    PlasmaCore.SvgItem {
        id: separator
        anchors {
            left: tabBar.right
            top: parent.top
            bottom: parent.bottom
        }
        implicitWidth: naturalSize.width
        implicitHeight: implicitWidth
        elementId: "vertical-line"
        svg: KickoffSingleton.lineSvg
    }
    LeaveButtons {
        id: leaveButtons
        anchors {
            left: separator.right
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: root.spacing
        }
    }

    Behavior on height {
        enabled: plasmoid.expanded
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InQuad
        }
    }

    // Using item containing WheelHandler instead of MouseArea because
    // MouseArea doesn't keep track to the total amount of rotation.
    // Keeping track of the total amount of rotation makes it work
    // better for touch pads.
    Item {
        id: mouseItem
        property bool filtered: false
        property real enterX
        property real enterY
        parent: root
        anchors.left: parent.left
        height: root.height
        width: tabBar.width
        z: 1 // Has to be above contentItem to recieve mouse wheel events
        WheelHandler {
            id: tabScrollHandler
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onWheel: {
                let shouldDec = rotation >= 15
                let shouldInc = rotation <= -15
                let shouldReset = (rotation > 0 && tabBar.currentIndex == 0) || (rotation < 0 && tabBar.currentIndex == tabBar.count-1)
                if (shouldDec) {
                    tabBar.decrementCurrentIndex();
                    rotation = 0
                } else if (shouldInc) {
                    tabBar.incrementCurrentIndex();
                    rotation = 0
                } else if (shouldReset) {
                    rotation = 0
                }
            }
        }
        HoverHandler {
            id: mouseFilterHandler
            grabPermissions: PointerHandler.TakeOverForbidden
            acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
            onHoveredChanged: if (hovered) {
                mouseItem.enterX = point.position.x
                mouseItem.enterY = point.position.y
            } else {
                // For LTR: If moved right and up, filter
                // For RTL: If moved left and up, filter
                // I think moving in the opposite direction cancels the filter with a slight delay.
                // This is desireable, but I'm not sure why it works.
                if ((root.mirrored ? mouseItem.enterX > point.position.x : mouseItem.enterX < point.position.x) && mouseItem.enterY > point.position.y) {
                    mouseItem.filtered = true
                    mouseFilterTimer.restart()
                } else {
                    mouseItem.filtered = false
                    mouseFilterTimer.stop()
                }
                mouseItem.enterX = -1
                mouseItem.enterY = -1
            }
        }
        Timer {
            id: mouseFilterTimer
            interval: root.height // Not sure, but maybe 1ms per pixel is a good way to pick a timer
            onTriggered: mouseItem.filtered = false
        }
        Binding {
            target: KickoffSingleton
            property: "filteringMouseHover"
            value: mouseItem.filtered
            restoreMode: Binding.RestoreBindingOrValue
        }
    }
}
