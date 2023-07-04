/*
    SPDX-FileCopyrightText: 2012-2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami 2 as Kirigami
import "code/tools.js" as TaskTools

ListView {
    id: taskList

    property bool animating: false

    delegate: Task {
        width: tasks.vertical ? tasks.width : (model.IsLauncher ? taskList.iconsOnlyTaskLength : taskList.taskLength)
        height: tasks.vertical ? taskList.iconsOnlyTaskLength : tasks.height
    }
    model: tasksModel
    layoutDirection: (plasmoid.configuration.reverseMode && !tasks.vertical)
        ? (Qt.application.layoutDirection === Qt.LeftToRight)
            ? Qt.RightToLeft
            : Qt.LeftToRight
        : Qt.application.layoutDirection
    orientation: tasks.vertical ? ListView.Vertical : ListView.Horizontal

    readonly property int spacingAdjustment: {
        if (plasmoid.pluginName === "org.kde.plasma.icontasks") {
            return Kirigami.Settings.tabletMode ? 3 : plasmoid.configuration.iconSpacing;
        }
        return 1;
    }
    readonly property real horizontalMargins: (taskFrame.margins.left + taskFrame.margins.right) * (tasks.vertical ? 1 : spacingAdjustment)
    readonly property real verticalMargins: (taskFrame.margins.left + taskFrame.margins.right) * (tasks.vertical ? 1 : spacingAdjustment)
    readonly property real iconsOnlyTaskLength: tasks.vertical ? tasks.width + verticalMargins : tasks.height + horizontalMargins
    property real taskLength: {
        if (tasks.iconsOnly) {
            return iconsOnlyTaskLength;
        } else {
            const leftLength = (tasks.vertical ? tasks.height : tasks.width) - tasksModel.launcherCount * iconsOnlyTaskLength;
            const expectedTaskLength = Math.min(iconsOnlyTaskLength + Kirigami.Units.iconSizes.sizeForLabels * 12, Math.floor(leftLength / ((taskList.count - tasksModel.launcherCount) || 1)));

            if (tasks.vertical) {
                return Math.max(iconsOnlyTaskLength, expectedTaskLength);
            }
            return Math.max(iconsOnlyTaskLength, expectedTaskLength);
        }
    }
    readonly property real totalLength: tasks.iconOnly ? taskList.count * taskList.iconsOnlyTaskLength : (taskList.count - tasksModel.launcherCount) * taskList.taskLength + tasksModel.launcherCount * taskList.iconsOnlyTaskLength

    move: Transition {
        SequentialAnimation {
            PropertyAction {
                target: taskList
                property: "animating"
                value: true
            }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: Kirigami.Units.longDuration
            }

            PropertyAction {
                target: taskList
                property: "animating"
                value: false
            }

            ScriptAction {
                script: {
                    publishIconGeometries(children, tasks);
                }
            }
        }
    }
    moveDisplaced: Transition {
        NumberAnimation {
            properties: "x, y"
            easing.type: Easing.OutQuad
            duration: Kirigami.Units.longDuration
        }
    }
    removeDisplaced: moveDisplaced

    function publishIconGeometries() {
        if (TaskTools.taskManagerInstanceCount >= 2) {
            return;
        }
        for (let i = 0; i < count - 1; ++i) {
            const task = itemAtIndex(i);

            if (!task.m.IsLauncher && !task.m.IsStartup) {
                tasks.tasksModel.requestPublishDelegateGeometry(tasks.tasksModel.makeModelIndex(task.itemIndex),
                    backend.globalRect(task), task);
            }
        }
    }

    Connections {
        target: plasmoid

        function onLocationChanged() {
            if (TaskTools.taskManagerInstanceCount >= 2) {
                return;
            }
            // This is on a timer because the panel may not have
            // settled into position yet when the location prop-
            // erty updates.
            iconGeometryTimer.start();
        }
    }

    Connections {
        target: plasmoid.configuration

        function onIconSpacingChanged() {
            iconGeometryTimer.start();
        }
    }

    Timer {
        id: iconGeometryTimer

        interval: 500
        repeat: false

        onTriggered: {
            publishIconGeometries();
        }
    }
}
