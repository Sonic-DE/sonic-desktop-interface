/*
    SPDX-FileCopyrightText: 2023 Taro Tanaka <mkrmdk@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.plasma.plasmoid

KSvg.SvgItem {
    id: runningIndicator

    readonly property bool inTopPanel: Plasmoid.location === PlasmaCore.Types.TopEdge
    readonly property bool inBottomPanel: Plasmoid.location === PlasmaCore.Types.BottomEdge
    readonly property bool inLeftPanel: Plasmoid.location === PlasmaCore.Types.LeftEdge
    readonly property bool inRightPanel: Plasmoid.location === PlasmaCore.Types.RightEdge
    readonly property bool inPanel: (inTopPanel || inBottomPanel || inLeftPanel || inRightPanel)

    readonly property bool onScreenEdge: svg.hasElement("hint-running-indicator-on-screen-edge")

    readonly property bool atTop: (inTopPanel && onScreenEdge) || (inBottomPanel && !onScreenEdge) || (!inPanel && !onScreenEdge)
    readonly property bool atBottom: (inBottomPanel && onScreenEdge) || (inTopPanel && !onScreenEdge) || (!inPanel && onScreenEdge)
    readonly property bool atLeft: (inLeftPanel && onScreenEdge) || (inRightPanel && !onScreenEdge)
    readonly property bool atRight: (inRightPanel && onScreenEdge) || (inLeftPanel && !onScreenEdge)

    visible: parent.model.IsWindow

    states: [
        State {
            name: "top"
            when: atTop
            AnchorChanges {
                target: runningIndicator
                anchors.top: runningIndicator.parent.top
                anchors.left: undefined
                anchors.right: undefined
                anchors.bottom: undefined
                anchors.horizontalCenter: iconBox.horizontalCenter
                anchors.verticalCenter: undefined
            }
        },
        State {
            name: "bottom"
            when: atBottom
            AnchorChanges {
                target: runningIndicator
                anchors.top: undefined
                anchors.left: undefined
                anchors.right: undefined
                anchors.bottom: runningIndicator.parent.bottom
                anchors.horizontalCenter: iconBox.horizontalCenter
                anchors.verticalCenter: undefined
            }
        },
        State {
            name: "left"
            when: atLeft
            AnchorChanges {
                target: runningIndicator
                anchors.top: undefined
                anchors.left: runningIndicator.parent.left
                anchors.right: undefined
                anchors.bottom: undefined
                anchors.horizontalCenter: undefined
                anchors.verticalCenter: iconBox.verticalCenter
            }
        },
        State {
            name: "right"
            when: atRight
            AnchorChanges {
                target: runningIndicator
                anchors.top: undefined
                anchors.left: undefined
                anchors.right: runningIndicator.parent.right
                anchors.bottom: undefined
                anchors.horizontalCenter: undefined
                anchors.verticalCenter: iconBox.verticalCenter
            }
        }
    ]

    implicitWidth: Math.min(naturalSize.width, iconBox.width)
    implicitHeight: Math.min(naturalSize.height, iconBox.width)

    imagePath: "widgets/tasks"
    elementId: {
        let position;
        if (atBottom) {
            position = "bottom";
        } else if (atLeft) {
            position = "left";
        } else if (atRight) {
            position = "right";
        } else {
            position = "top";
        }
        const attention = parent.model.IsDemandingAttention || task.smartLauncherItem?.urgent ? "-attention" : "";
        const state = parent.model.IsActive ? "-focus" : parent.model.IsMinimized ? "-minimized" : "";
        const grouped = parent.model.IsGroupParent ? "-grouped" : "";

        return `running-indicator-${position}${attention}${state}${grouped}`;
    }
}
