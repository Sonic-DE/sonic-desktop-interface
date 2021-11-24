/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.taskmanager 0.1 as TaskManager
import org.kde.kwindowsystem 1.0

Item {
    id: root

    property Item containment

    property bool floating: floatingPanelSvg.usedPrefix == 'floating'
    property bool screenCovered: visibleWindowsModel.count > 0  && !kwindowsystem.showingDesktop

    property alias panelMask: privateSwapper.mask

    QtObject {
        id: privateSwapper
        property string completedState: ""
        // Work around the fact that we can't use a ternary if in an alias
        readonly property var mask: {
            if (completedState == "opaque") {
                return opaqueItem.mask
            } else {
                return translucentItem.mask
            }
        }
    }

    readonly property bool verticalPanel: containment && containment.formFactor === PlasmaCore.Types.Vertical

    readonly property real spacingAtMinSize: Math.round(Math.max(1, (verticalPanel ? root.width : root.height) - units.iconSizes.smallMedium)/2)
    PlasmaCore.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: 'thick'
        imagePath: "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: floatingPanelSvg
        visible: false
        prefix: ['floating', '']
        imagePath: "widgets/panel-background"
    }
    readonly property int topPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.top, spacingAtMinSize));
    readonly property int bottomPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.bottom, spacingAtMinSize));
    readonly property int leftPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.left, spacingAtMinSize));
    readonly property int rightPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.right, spacingAtMinSize));

    readonly property int topFloatingPadding: floating && containment.location != PlasmaCore.Types.BottomEdge ? floatingPanelSvg.fixedMargins.top : 0
    readonly property int leftFloatingPadding: floating && containment.location != PlasmaCore.Types.RightEdge? floatingPanelSvg.fixedMargins.left : 0
    readonly property int rightFloatingPadding: floating && containment.location != PlasmaCore.Types.LeftEdge? floatingPanelSvg.fixedMargins.right : 0
    readonly property int bottomFloatingPadding: floating && containment.location != PlasmaCore.Types.TopEdge? floatingPanelSvg.fixedMargins.bottom : 0

    property int maskOffsetX: screenCovered ? 0 : leftFloatingPadding
    property int maskOffsetY: screenCovered ? 0 : topFloatingPadding
    Behavior on maskOffsetX {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
        }
    }
    Behavior on maskOffsetY {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
        }
    }

    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
    }

    PlasmaCore.SortFilterModel {
        id: visibleWindowsModel
        filterRole: 'IsMinimized'
        filterRegExp: 'false'
        sourceModel: TaskManager.TasksModel {
            filterByVirtualDesktop: true
            filterByActivity: true
            filterNotMaximized: true
            filterByScreen: true
            filterHidden: true

            screenGeometry: panel.screenGeometry
            virtualDesktop: virtualDesktopInfo.currentDesktop
            activity: activityInfo.currentActivity

            id: tasksModel
            groupMode: TaskManager.TasksModel.GroupDisabled
        }
    }

    KWindowSystem {
        id: kwindowsystem
    }

    PlasmaCore.FrameSvgItem {
        id: translucentItem
        enabledBorders: floating ? undefined : panel.enabledBorders
        anchors {
            fill: parent
            bottomMargin: bottomFloatingPadding; leftMargin: leftFloatingPadding
            rightMargin: rightFloatingPadding; topMargin: topFloatingPadding
        }
        states: State {
            name: 'fill'; when: screenCovered
            PropertyChanges {
                target: translucentItem.anchors; bottomMargin: 0; leftMargin: 0;
                rightMargin: 0; topMargin: 0;
            }
        }
        transitions: Transition {
            from: ""; to: "fill"; reversible: true
            NumberAnimation {
                properties: "bottomMargin,topMargin,leftMargin,rightMargin"
                duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad
            }
        }

        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }

    PlasmaCore.FrameSvgItem {
        id: opaqueItem
        enabledBorders: floating ? undefined : panel.enabledBorders
        anchors {
            fill: parent
            bottomMargin:  bottomFloatingPadding; leftMargin: leftFloatingPadding
            rightMargin: rightFloatingPadding; topMargin: topFloatingPadding
        }
        states: State {
            name: 'fill'; when: screenCovered
            PropertyChanges {
                target: opaqueItem.anchors; bottomMargin: 0; leftMargin: 0;
                rightMargin: 0; topMargin: 0;
            }
        }
        transitions: Transition {
            from: ""; to: "fill"; reversible: true
            NumberAnimation {
                properties: "bottomMargin,topMargin,leftMargin,rightMargin"
                duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad
            }
        }
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }

    transitions: [
        Transition {
            from: "*"
            to: "transparent"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        translucentItem.visible = true
                        containment.containmentDisplayHints &= ~PlasmaCore.Types.DesktopFullyCovered;
                    }
                }
                NumberAnimation {
                    target: opaqueItem
                    properties: "opacity"
                    to: 0
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        opaqueItem.visible = false
                        privateSwapper.completedState = "transparent"
                        root.panelMaskChanged()
                    }
                }
            }
        },
        Transition {
            from: "*"
            to: "opaque"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        opaqueItem.visible = true
                        containment.containmentDisplayHints |= PlasmaCore.Types.DesktopFullyCovered;
                    }
                }
                NumberAnimation {
                    target: opaqueItem
                    properties: "opacity"
                    to: 1
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        translucentItem.visible = false
                        privateSwapper.completedState = "opaque"
                        root.panelMaskChanged()
                    }
                }
            }
        }
    ]

    states: [
        State {
            name: "opaque"
            when: panel.opacityMode == 1 || (panel.opacityMode == 0 && screenCovered)
        },
        State {
            when: panel.opacityMode == 2 || (panel.opacityMode == 0 && !screenCovered)
            name: "transparent"
        }
    ]

    function adjustPrefix() {
        if (!containment) {
            return "";
        }
        var pre;
        switch (containment.location) {
        case PlasmaCore.Types.LeftEdge:
            pre = "west";
            break;
        case PlasmaCore.Types.TopEdge:
            pre = "north";
            break;
        case PlasmaCore.Types.RightEdge:
            pre = "east";
            break;
        case PlasmaCore.Types.BottomEdge:
            pre = "south";
            break;
        default:
            pre = "";
            break;
        }
        translucentItem.prefix = opaqueItem.prefix = [pre, ""];
    }

    onContainmentChanged: {
        if (!containment) {
            return;
        }
        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;
        containment.locationChanged.connect(adjustPrefix);
        adjustPrefix();
    }

    Binding {
        target: panel
        property: "length"
        when: containment
        value: {
            if (!containment) {
                return;
            }
            if (verticalPanel) {
                return containment.Layout.preferredHeight
            } else {
                return containment.Layout.preferredWidth
            }
        }
        restoreMode: Binding.RestoreBinding
    }

    Binding {
        target: panel
        property: "backgroundHints"
        when: containment
        value: {
            if (!containment) {
                return;
            }

            return containment.backgroundHints; 
        }
        restoreMode: Binding.RestoreBinding
    }

    Item {
        id: containmentParent
        anchors.centerIn: translucentItem
        width: root.width - leftFloatingPadding - rightFloatingPadding
        height: root.height - topFloatingPadding - bottomFloatingPadding
    }
}
