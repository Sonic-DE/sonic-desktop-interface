/*
    SPDX-FileCopyrightText: Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm

QQC2.ApplicationWindow {
    id: root

    required property var device
    required property var tabletEvents

    property CalibrationTool tool: CalibrationTool {
        device: root.device
    }

    property real currentPenToolX: -1
    property real currentPenToolY: -1

    title: i18nc("@title", "Pen Calibration")

    onWidthChanged: tool.width = width
    onHeightChanged: tool.height = height

    visibility: Window.FullScreen

    Connections {
        target: tabletEvents

        function onToolDown(x, y) {
            const centerX = root.width / 2;
            const centerY = root.height / 2;

            let targetItem = null;
            if (x < centerX && y < centerY) {
                targetItem = topLeftAnchor;
            } else if (x < centerX && y > centerY) {
                targetItem = bottomLeftAnchor;
            } else if (x > centerX && y < centerY) {
                targetItem = topRightAnchor;
            } else {
                targetItem = bottomRightAnchor;
            }

            if (tool.finishedCalibration) {
                root.currentPenToolX = x;
                root.currentPenToolY = y;
            } else {
                tool.calibrate(x, y, targetItem.positionX, targetItem.positionY);
            }
        }

        function onToolUp() {
            root.currentPenToolX = -1;
            root.currentPenToolY = -1;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent

        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            horizontalAlignment: Text.AlignHCenter

            text: i18n("Tap the center of each target.")
        }

        QQC2.Button {
            visible: tool.finishedCalibration

            text: i18nc("@action:button", "Calibrate Again")
            icon.name: "edit-redo"

            onClicked: tool.reset()

            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.Button {
            text: i18nc("@action:button", "Reset Calibration")
            icon.name: "kt-restore-defaults"

            onClicked: tool.restoreDefaults();

            Layout.alignment: Qt.AlignHCenter
        }

        QQC2.Button {
            text: i18nc("@action:button", "Cancel Calibration")
            icon.name: "dialog-cancel"

            onClicked: root.close()

            Layout.alignment: Qt.AlignHCenter
        }
    }

    component Target: QQC2.ToolButton {
        readonly property real positionX: test.Kirigami.ScenePosition.x
        readonly property real positionY: test.Kirigami.ScenePosition.y

        icon.name: "crosshairs"

        down: {
            let isX = root.currentPenToolX >= Kirigami.ScenePosition.x && root.currentPenToolX <= Kirigami.ScenePosition.x + width;
            let isY = root.currentPenToolY >= Kirigami.ScenePosition.y && root.currentPenToolY <= Kirigami.ScenePosition.y + height;
            return isX && isY;
        }

        Item {
            id: test

            anchors.centerIn: parent
        }
    }

    component TargetPath: Shape {
        required property real sourceX
        required property real sourceY

        required property real targetX
        required property real targetY

        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        Kirigami.Theme.inherit: false
        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.lighter(Kirigami.Theme.backgroundColor, 1.5)
            startX: sourceX; startY: sourceY
            PathLine { x: targetX; y: targetY }
        }
    }

    component PairTargetPath: Item {
        required property var anchor

        visible: anchor.visible

        TargetPath {
            sourceX: anchor.positionX
            sourceY: 0
            targetX: anchor.positionX
            targetY: root.height
        }

        TargetPath {
            sourceX: 0
            sourceY: anchor.positionY
            targetX: root.width
            targetY: anchor.positionY
        }
    }

    Target {
        id: topLeftAnchor

        visible: tool.finishedCalibration || tool.currentTarget === 0

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.gridUnit * 4
            top: parent.top
            topMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: topLeftAnchor
    }

    Target {
        id: topRightAnchor

        visible: tool.finishedCalibration || tool.currentTarget === 1

        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit * 4
            top: parent.top
            topMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: topRightAnchor
    }

    Target {
        id: bottomLeftAnchor

        visible: tool.finishedCalibration || tool.currentTarget === 2

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.gridUnit * 4
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: bottomLeftAnchor
    }

    Target {
        id: bottomRightAnchor

        visible: tool.finishedCalibration || tool.currentTarget === 3

        anchors {
            right: parent.right
            rightMargin: Kirigami.Units.gridUnit * 4
            bottom: parent.bottom
            bottomMargin: Kirigami.Units.gridUnit * 4
        }
    }

    PairTargetPath {
        anchor: bottomRightAnchor
    }
}