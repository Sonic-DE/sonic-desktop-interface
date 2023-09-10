/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.configuration 2.0
import "panelconfiguration"


//TODO: all of this will be done with desktop components
Item {
    id: dialogRoot

    signal closeContextMenu

//BEGIN Properties
    width: 640
    height: 64
    implicitWidth: Kirigami.Units.gridUnit * 2
    implicitHeight: Kirigami.Units.gridUnit * 2

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    state: {
        switch (panel.location) {
        case PlasmaCore.Types.TopEdge:
            return "TopEdge"
        case PlasmaCore.Types.LeftEdge:
            return "LeftEdge"
        case PlasmaCore.Types.RightEdge:
            return "RightEdge"
        case PlasmaCore.Types.BottomEdge:
        default:
            return "BottomEdge"
        }
    }

    property bool vertical: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge)
//END properties

//BEGIN Connections
    Connections {
        target: panel
        function onOffsetChanged() {
            ruler.offset = panel.offset
        }
        function onMinimumLengthChanged() {
            ruler.minimumLength = panel.minimumLength
        }
        function onMaximumLengthChanged() {
            ruler.maximumLength = panel.maximumLength
        }
    }

    Connections {
        target: Plasmoid
        function onImmutableChanged() {
            configDialog.close()
        }
    }
//END Connections


//BEGIN UI components

    Ruler {
        id: ruler
        visible: !fitContent && !fillWidth
        property bool fitContent: panel.minimumLength === 10 && panel.maximumLength === panel.screenToFollow.geometry.width
        property bool fillWidth: panel.minimumLength === panel.screenToFollow.geometry.width
        state: dialogRoot.state
    }

    PlasmaCore.Dialog {
        id: mainDialog
        visible: dialogRoot.visible

        property var targetWidth: Kirigami.Units.gridUnit * 27
        property var targetHeight: Kirigami.Units.gridUnit * 31

        x: panel.screenGeometry.width / 2 - targetWidth / 2
        y: panel.screenGeometry.height - panel.thickness - Kirigami.Units.gridUnit * 2 - mainItem.height - (ruler.visible ? ruler.height : 0)

        mainItem: DialogContent {
            width: mainDialog.targetWidth
        }

        location: PlasmaCore.Types.Floating
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint
    }

//END UI components

//BEGIN Animations
    //when EdgeHandle is released animate to old panel position
    ParallelAnimation {
        id: panelResetAnimation
        NumberAnimation {
            target: panel
            properties: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge) ? "x" : "y"
            to:  {
                switch (panel.location) {
                case PlasmaCore.Types.TopEdge:
                    return panel.screenGeometry.y + panel.distance
                case PlasmaCore.Types.LeftEdge:
                    return panel.screenGeometry.x + panel.distance
                case PlasmaCore.Types.RightEdge:
                    return panel.screenGeometry.x + panel.screenGeometry.width - panel.width - panel.distance
                case PlasmaCore.Types.BottomEdge:
                default:
                    return panel.screenGeometry.y + panel.screenGeometry.height - panel.height - panel.distance
                }
            }
            duration: Kirigami.Units.shortDuration
        }

        NumberAnimation {
            target: configDialog
            properties: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge) ? "x" : "y"
            to: {
                switch (panel.location) {
                case PlasmaCore.Types.TopEdge:
                    return panel.screenGeometry.y + panel.height + panel.distance
                case PlasmaCore.Types.LeftEdge:
                    return panel.screenGeometry.x + panel.width + panel.distance
                case PlasmaCore.Types.RightEdge:
                    return panel.screenGeometry.x + panel.screenGeometry.width - panel.width - configDialog.width - panel.distance
                case PlasmaCore.Types.BottomEdge:
                default:
                    return panel.screenGeometry.y + panel.screenGeometry.height - panel.height - configDialog.height - panel.distance
                }
            }
            duration: Kirigami.Units.shortDuration
        }
    }
//END Animations

//BEGIN States
states: [
        State {
            name: "TopEdge"
        },
        State {
            name: "BottomEdge"
        },
        State {
            name: "LeftEdge"
        },
        State {
            name: "RightEdge"
        }
    ]
//END States
}
