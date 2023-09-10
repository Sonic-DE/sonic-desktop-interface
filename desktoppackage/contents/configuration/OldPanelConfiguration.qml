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
import org.kde.plasma.shell.panel 0.1 as Panel
import "panelconfiguration"


//TODO: all of this will be done with desktop components
Item {
    id: dialogRoot

    signal closeContextMenu

//BEGIN Properties
    //width: 640
    //height: 64
    //implicitWidth: ruler.implicitWidth
    //implicitHeight: ruler.implicitHeight TODO stuff, working on it
    implicitWidth: 1
    implicitHeight: 1
    width: 1
    height: 1

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
        visible: panel.lengthMode === Panel.Global.Custom
        property var shortSide: vertical ? width : height
        state: dialogRoot.state
    }

    PlasmaCore.Dialog {
        id: mainDialog
        visible: dialogRoot.visible

        property var targetWidth: Kirigami.Units.gridUnit * 27
        property var targetHeight: mainItem.height

        mainItem: Item {
            width: mainDialog.targetWidth
            height: mainContent.implicitHeight
            DialogContent {
                id: mainContent
                width: parent.width
            }

            Kirigami.PlaceholderMessage {
                width: parent.width - Kirigami.Units.largeSpacing * 2
                anchors.centerIn: parent
                visible: !mainContent.visible
                text: "Click on the arrows near the screen edge you want the panel to occupy."

                helpfulAction: Kirigami.Action {
                    icon.name: "dialog-conce"
                    text: "Cancel"
                    onTriggered: mainContent.visible = true
                }
            }
        }

        location: PlasmaCore.Types.Floating
        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint
    }

//END UI components

//BEGIN States
states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: mainDialog
                y: panel.screenGeometry.y + Kirigami.Units.gridUnit * 2 + (ruler.visible ? ruler.shortSide : 0)
                y: panel.screenGeometry.y + Kirigami.Units.gridUnit * 2 + (ruler.visible ? ruler.shortSide : 0) + panel.thickness
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: mainDialog
                x: panel.screenGeometry.x + panel.screenGeometry.width / 2 - mainDialog.targetWidth / 2
                y: panel.screenGeometry.y + panel.screenGeometry.height - panel.thickness - Kirigami.Units.gridUnit * 2 - mainDialog.targetHeight - (ruler.visible ? ruler.shortSide : 0)
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: mainDialog
                y: panel.screenGeometry.y + panel.screenGeometry.height / 2 - mainDialog.targetHeight / 2
                x: panel.screenGeometry.x + Kirigami.Units.gridUnit * 2 + (ruler.visible ? ruler.shortSide : 0) + panel.thickness
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: mainDialog
                y: panel.screenGeometry.y + panel.screenGeometry.height / 2 - mainDialog.targetHeight / 2
                x: panel.screenGeometry.x + panel.screenGeometry.width - panel.thickness - (ruler.visible ? ruler.shortSide : 0) - mainDialog.targetWidth - Kirigami.Units.gridUnit * 2
            }
        }
    ]
//END States
}
