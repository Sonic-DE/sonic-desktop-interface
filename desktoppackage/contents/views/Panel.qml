/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.taskmanager 0.1 as TaskManager
import org.kde.kwindowsystem 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    property Item containment

    property bool floatingPrefix: floatingPanelSvg.usedPrefix === "floating"
    property bool floating: panel.floating
    readonly property bool screenCovered: visibleWindowsModel.count > 0 && !kwindowsystem.showingDesktop

    property var panelMask: opaqueItem.mask
    onPanelMaskChanged: {
        console.log('---------------------------------------------------------------------->', panelMask)
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

    readonly property int bottomFloatingPadding: floating && containment && containment.location !== PlasmaCore.Types.TopEdge ? (floatingPrefix ? floatingPanelSvg.fixedMargins.bottom : 8) : 0
    readonly property int leftFloatingPadding: floating && containment && containment.location !== PlasmaCore.Types.RightEdge ? (floatingPrefix ? floatingPanelSvg.fixedMargins.left   : 8) : 0
    readonly property int rightFloatingPadding: floating && containment && containment.location !== PlasmaCore.Types.LeftEdge ? (floatingPrefix ? floatingPanelSvg.fixedMargins.right  : 8) : 0
    readonly property int topFloatingPadding: floating && containment && containment.location !== PlasmaCore.Types.BottomEdge ? (floatingPrefix ? floatingPanelSvg.fixedMargins.top    : 8) : 0

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
        visible: false
        enabledBorders: panel.enabledBorders
        anchors.fill: parent
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }


    PlasmaCore.FrameSvgItem {
        id: floatingTranslucentItem
        // Number between 0 (not floating) - 1 (floating)
        property double floatingness: 0
        visible: false
        anchors {
            fill: parent
            bottomMargin: bottomFloatingPadding * floatingness
            leftMargin: leftFloatingPadding * floatingness
            rightMargin: rightFloatingPadding * floatingness
            topMargin: topFloatingPadding * floatingness
        }
        /*states: State {
            name: 'fill'; when: screenCovered
            PropertyChanges {
                target: floatingTranslucentItem.anchors; bottomMargin: 0; leftMargin: 0;
                rightMargin: 0; topMargin: 0;
            }
        }
        transitions: Transition {
            from: ""; to: "fill"; reversible: true
            NumberAnimation {
                properties: "bottomMargin,topMargin,leftMargin,rightMargin"
                duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad
            }
        }*/

        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }


    PlasmaCore.FrameSvgItem {
        id: floatingOpaqueItem
        visible: false
        property double floatingness: 0
        anchors {
            fill: parent
            bottomMargin: bottomFloatingPadding * floatingness
            leftMargin: leftFloatingPadding * floatingness
            rightMargin: rightFloatingPadding * floatingness
            topMargin: topFloatingPadding * floatingness
        }
        /*states: State {
            name: 'fill'; when: screenCovered
            PropertyChanges {
                target: floatingTranslucentItem.anchors; bottomMargin: 0; leftMargin: 0;
                rightMargin: 0; topMargin: 0;
            }
        }
        transitions: Transition {
            from: ""; to: "fill"; reversible: true
            NumberAnimation {
                properties: "bottomMargin,topMargin,leftMargin,rightMargin"
                duration: PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad
            }
        }*/

        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }

    PlasmaCore.FrameSvgItem {
        id: opaqueItem
        visible: false
        enabledBorders: panel.enabledBorders
        anchors.fill: parent
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }

    Keys.onEscapePressed: {
        root.parent.focus = false
    }

    /*transitions: [
        Transition {
            from: "*"
            to: "transparent"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        if (currentlyFloating) {
                            floatingTranslucentItem.visible = true
                        } else {
                            translucentItem.visible = true
                        }
                        root.panelMask = translucentItem.mask
                    }
                }
                NumberAnimation {
                    target: floating ? floatingOpaqueItem : opaqueItem
                    properties: "opacity"
                    to: 0
                    duration: floating ? 2 : PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        floatingOpaqueItem.visible = opaqueItem.visible = false
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
                        if (currentlyFloating) {
                            floatingOpaqueItem.visible = true
                        } else {
                            opaqueItem.visible = true
                        }
                    }
                }
                NumberAnimation {
                    target: floating ? floatingOpaqueItem : opaqueItem
                    properties: "opacity"
                    to: 1
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        translucentItem.visible = floatingTranslucentItem.visible = false
                        root.panelMask = opaqueItem.mask
                    }
                }
            }
        }
    ]*/

    transitions: [
        Transition {
            from: "opaque"
            to: "transparent"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        translucentItem.visible = true
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
                        root.panelMask = translucentItem.mask
                    }
                }
            }
        },
        Transition {
            from: "transparent"
            to: "opaque"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        opaqueItem.visible = translucentItem.visible = true
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
                        root.panelMask = opaqueItem.mask
                    }
                }
            }
        },
        Transition {
            from: "transparent"
            to: "floatingtransparent"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        translucentItem.visible = false
                        floatingTranslucentItem.visible = true
                        root.panelMask = floatingTranslucentItem.mask
                    }
                }
                NumberAnimation {
                    target: floatingTranslucentItem
                    properties: "floatingness"
                    to: 1
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
            }
        },
        Transition {
            from: "floatingtransparent"
            to: "transparent"
            SequentialAnimation {
                NumberAnimation {
                    target: floatingTranslucentItem
                    properties: "floatingness"
                    to: 0
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        translucentItem.visible = true
                        floatingTranslucentItem.visible = false
                        root.panelMask = translucentItem.mask
                    }
                }
            }
        },
        Transition {
            from: "opaque"
            to: "floatingopaque"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        opaqueItem.visible = false
                        floatingOpaqueItem.visible = true
                        root.panelMask = floatingOpaqueItem.mask
                    }
                }
                NumberAnimation {
                    target: floatingOpaqueItem
                    properties: "floatingness"
                    to: 1
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
            }
        },
        Transition {
            from: "floatingopaque"
            to: "opaque"
            SequentialAnimation {
                NumberAnimation {
                    target: floatingOpaqueItem
                    properties: "floatingness"
                    to: 0
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        opaqueItem.visible = true
                        floatingOpaqueItem.visible = false
                        root.panelMask = opaqueItem.mask
                    }
                }
            }
        },
        Transition {
            from: "opaque"
            to: "floatingtransparent"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        opaqueItem.visible = false
                        floatingTranslucentItem.visible = floatingOpaqueItem.visible = true
                        floatingTranslucentItem.floatingness = floatingOpaqueItem.floatingness = 0
                        root.panelMask = floatingTranslucentItem.mask
                    }
                }
                ParallelAnimation {
                    NumberAnimation {
                        target: floatingTranslucentItem
                        properties: "floatingness"
                        to: 1
                        duration: PlasmaCore.Units.veryLongDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: floatingOpaqueItem
                        properties: "floatingness"
                        to: 1
                        duration: PlasmaCore.Units.veryLongDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: floatingOpaqueItem
                        properties: "opacity"
                        to: 0
                        duration: PlasmaCore.Units.veryLongDuration
                        easing.type: Easing.InOutQuad
                    }
                }
                ScriptAction {
                    script: {
                        floatingOpaqueItem.visible = false
                    }
                }
            }
        },
        Transition {
            from: "floatingtransparent"
            to: "opaque"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        floatingTranslucentItem.visible = floatingOpaqueItem.visible = true
                        floatingOpaqueItem.opacity = 1 - floatingTranslucentItem.opacity
                        floatingOpaqueItem.floatingness = floatingTranslucentItem.floatingness
                        root.panelMask = opaqueItem.mask
                    }
                }
                ParallelAnimation {
                    NumberAnimation {
                        target: floatingTranslucentItem
                        properties: "floatingness"
                        to: 0
                        duration: PlasmaCore.Units.veryLongDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: floatingOpaqueItem
                        properties: "floatingness"
                        to: 0
                        duration: PlasmaCore.Units.veryLongDuration
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        target: floatingOpaqueItem
                        properties: "opacity"
                        to: 1
                        duration: PlasmaCore.Units.veryLongDuration
                        easing.type: Easing.InOutQuad
                    }
                }
                ScriptAction {
                    script: {
                        opaqueItem.visible = true
                        floatingTranslucentItem.visible = floatingOpaqueItem.visible = false
                        floatingTranslucentItem.opacity = floatingOpaqueItem.opacity = 1
                    }
                }
            }
        },
        Transition {
            from: "floatingopaque"
            to: "floatingtransparent"
            ScriptAction {
                script: {
                    floatingOpaqueItem.visible = false
                    floatingTranslucentItem.visible = true
                    floatingTranslucentItem.floatingness = 1
                    floatingTranslucentItem.opacity = 1
                    root.panelMask = floatingTranslucentItem.mask
                }
            }
        },
        Transition {
            from: "floatingtransparent"
            to: "floatingopaque"
            ScriptAction {
                script: {
                    floatingOpaqueItem.visible = true
                    floatingTranslucentItem.visible = false
                    floatingOpaqueItem.floatingness = 1
                    floatingOpaqueItem.opacity = 1
                    root.panelMask = floatingOpaqueItem.mask
                }
            }
        }
    ]

    property bool isOpaque: panel.opacityMode === 1
    property bool isTransparent: panel.opacityMode === 2
    property bool isAdaptive: panel.opacityMode === 0
    property var stateTriggers: [floating, screenCovered, isOpaque, isAdaptive, isTransparent]
    states: [
        State {name: "opaque"},
        State {name: "transparent"},
        State {name: "floatingtransparent"},
        State {name: "floatingopaque"}
    ]
    onStateTriggersChanged: {
        if ((!floating || screenCovered) && (isOpaque || (screenCovered && isAdaptive))) {
            root.state = "opaque"
        } else if ((!floating || screenCovered) && (isTransparent || (!screenCovered && isAdaptive))) {
            root.state = "transparent"
        } else if ((floating && !screenCovered) && (isTransparent || isAdaptive)) {
            root.state = "floatingtransparent"
        } else if (floating && !screenCovered && isOpaque) {
            root.state = "floatingopaque"
        }
        if ((root.state == "opaque" || root.state == "floatingopaque") && containment) {
            containment.containmentDisplayHints |= PlasmaCore.Types.DesktopFullyCovered
        } else {
            containment.containmentDisplayHints &= ~PlasmaCore.Types.DesktopFullyCovered
        }
        console.log(root.state)
    }

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
        translucentItem.prefix = opaqueItem.prefix = floatingTranslucentItem.prefix = floatingOpaqueItem.prefix = [pre, ""];
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
        delayed: true
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

    Connections {
        target: containment
        function onActivated() {
            containment.status = PlasmaCore.Types.AcceptingInputStatus;
            // When the containment is set to AcceptingInputStatus he window will be given focus and
            // will try to give focus to an itemof the scene, but not the one we wnant. if we call immediately
            // forceActiveFocus on the one we want we'll have a race condition that won't happen if we do it later
            Qt.callLater(root.nextItemInFocusChain().forceActiveFocus);
        }
    }

    Connections {
        target: parent
        function onActiveFocusChanged() {
            if (!parent.activeFocus) {
                containment.status = PlasmaCore.Types.PassiveStatus
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        x: root.verticalPanel || !panel.activeFocusItem
            ? 0
            : Math.max(panel.activeFocusItem.Kirigami.ScenePosition.x, panel.activeFocusItem.Kirigami.ScenePosition.x)
        y: root.verticalPanel && panel.activeFocusItem
            ? Math.max(panel.activeFocusItem.Kirigami.ScenePosition.y, panel.activeFocusItem.Kirigami.ScenePosition.y)
            : 0

        width: panel.activeFocusItem
            ? (root.verticalPanel ? root.width : Math.min(panel.activeFocusItem.width, panel.activeFocusItem.width))
            : 0
        height: panel.activeFocusItem
            ? (root.verticalPanel ?  Math.min(panel.activeFocusItem.height, panel.activeFocusItem.height) : root.height)
            : 0

        visible: panel.active && panel.activeFocusItem

        imagePath: "widgets/tabbar"
        prefix: {
            var prefix = ""
            switch (root.containment.location) {
                case PlasmaCore.Types.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case PlasmaCore.Types.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case PlasmaCore.Types.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
            }
            if (!hasElementPrefix(prefix)) {
                prefix = "active-tab";
            }
            return prefix;
        }
    }
    Item {
        id: containmentParent
        anchors.centerIn: isOpaque ? floatingOpaqueItem : floatingTranslucentItem
        width: root.width - leftFloatingPadding - rightFloatingPadding
        height: root.height - topFloatingPadding - bottomFloatingPadding
    }
}
