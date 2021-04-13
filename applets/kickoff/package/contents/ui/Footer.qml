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

        PC3.TabButton {
            id: applicationsTab
            width: tabBar.tabWidth
            height: parent.height
            icon.width: PlasmaCore.Units.iconSizes.smallMedium
            icon.height: PlasmaCore.Units.iconSizes.smallMedium
            icon.name: "applications-other"
            text: i18n("Applications")
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

        // Using item containing WheelHandler instead of MouseArea because
        // MouseArea doesn't keep track to the total amount of rotation.
        // Keeping track of the total amount of rotation makes it work
        // better for touch pads.
        Item {
            parent: tabBar
            anchors.fill: parent
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
        }

        //onCurrentIndexChanged: {
            //header.input.forceActiveFocus();
            //keyboardNavigation.state = "LeftColumn"
            //root.currentView.listView.positionAtBeginning();
        //}

        //Connections {
            //target: plasmoid
            //function onExpandedChanged() {
                //header.input.forceActiveFocus();
                //switchToInitial();
            //}
        //}
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
}
