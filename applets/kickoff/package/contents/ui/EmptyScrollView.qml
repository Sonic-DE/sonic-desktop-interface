/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.16 as Kirigami

/**
 * This is meant to be a very basic ScrollView that behaves like most ScrollViews do,
 * but inherits no externally defined content or behavior, except for the PC3 ScrollBars.
 */
T.ScrollView {
    id: root

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

//     horizontalPadding: PC3.ScrollBar.vertical.width
//     verticalPadding: PC3.ScrollBar.horizontal.height
    //leftPadding: {
        //if (mirrored && PC3.ScrollBar.vertical.visible) {
            //return PC3.ScrollBar.vertical.width// + horizontalPadding
        //} else {
            //return horizontalPadding
        //}
    //}
    //rightPadding: {
        //if (!mirrored && PC3.ScrollBar.vertical.visible) {
            //return PC3.ScrollBar.vertical.width// + horizontalPadding
        //} else {
            //return horizontalPadding
        //}
    //}
    //topPadding: verticalPadding
    //bottomPadding: PC3.ScrollBar.vertical.visible ? PC3.ScrollBar.horizontal.height : verticalPadding

    PC3.ScrollBar.vertical: PC3.ScrollBar {
        visible: size < 1 && policy !== PC3.ScrollBar.AlwaysOff
        parent: root
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            topMargin: root.topPadding
            bottomMargin: root.bottomPadding
        }
        height: root.availableHeight
    }

    PC3.ScrollBar.horizontal: PC3.ScrollBar {
        visible: size < 1 && policy !== PC3.ScrollBar.AlwaysOff
        parent: root
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: root.leftPadding
            rightMargin: root.rightPadding
        }
    }

    //Kirigami.WheelHandler {
        //target: root.contentItem
    //}
}
