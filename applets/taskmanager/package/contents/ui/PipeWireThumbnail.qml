/*
    SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15

import org.kde.pipewire 0.1 as PipeWire
import org.kde.taskmanager 0.1 as TaskManager
import org.kde.kirigami 2.20 as Kirigami

// opacity doesn't work in the root item
Item {
    anchors.fill: parent

    readonly property bool hasThumbnail: pipeWireSourceItem.nodeId > 0

    TaskManager.ScreencastingRequest {
        id: waylandItem
        uuid: thumbnailSourceItem.winId
    }

    PipeWire.PipeWireSourceItem {
        id: pipeWireSourceItem

        nodeId: waylandItem.nodeId

        anchors.fill: parent
    }
}
