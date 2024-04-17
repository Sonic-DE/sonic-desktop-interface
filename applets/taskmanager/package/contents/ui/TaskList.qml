/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0
import "code/layout.js" as LayoutMetrics

GridLayout {
    property bool animating: false

    layoutDirection: (Plasmoid.configuration.reverseMode && !tasks.vertical)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection

    rowSpacing: 0
    columnSpacing: 0
    rows: tasks.vertical ? -1 : Math.max(1, Math.floor(tasks.height / children[0].implicitHeight))
    columns: tasks.vertical
        ? Math.max(1, Math.floor(tasks.width / children[0].implicitWidth))
        : (tasks.width/tasksModel.count <= LayoutMetrics.preferredMinWidth()/tasks.plasmoid.configuration.maxStripes ? Math.ceil(tasksModel.count/tasks.plasmoid.configuration.maxStripes) : -1)
onChildrenChanged:
    print("AAAAA"+(tasks.width/tasksModel.count)+" "+ LayoutMetrics.preferredMinWidth()/2+" "+columns+" "+children[0].implicitHeight+" "+rows)
    /*move: Transition {
        SequentialAnimation {
            PropertyAction { target: taskList; property: "animating"; value: true }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: Kirigami.Units.longDuration
            }

            PropertyAction { target: taskList; property: "animating"; value: false }
        }
    }*/
   /* Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.rowSpan: 2//parent.rows
        Layout.row: 0
        Layout.column:parent.children.length - 1
    }*/
}
