/*
 *    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3

import "code/tools.js" as Tools

PlasmaComponents3.ItemDelegate {
    id: item

    required property int index
    required property bool isSeparator
    required property bool hasChildren
    required property bool isParent
    required property bool disabled
    required property bool hasActionList
    required property string favoriteId
    required property var /*QVariantList*/ actionList
    required property url url
    required property string description
    required property string decoration
    required property var model // for display, which would shadow ItemDelegate
    required property var favoritesModel
    required property var baseModel

    readonly property ActionMenu menu: actionMenu

    property bool dialogDefaultRight: Application.layoutDirection !== Qt.RightToLeft

    signal interactionConcluded
    signal openCategory(bool keyboardInitiated)

    text: model.display
    icon.name: decoration
    hoverEnabled: true

    action: Kirigami.Action {
        onTriggered: source => {
            if (item.dragActive) { return }
            if (!item.hasChildren) {
                item.baseModel.trigger(item.index, "", null);
                item.interactionConcluded()
            } else {
                item.openCategory(source instanceof KeyEvent)
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.RightButton

        onPressed: mouse => {
            if (item.hasActionList || item.favoriteId !== null) {
                item.openActionMenu(mouse.x, mouse.y);
            }
        }
    }

    contentItem: null

    Keys.onReturnPressed: event => action.trigger(event)
    Keys.onEnterPressed: event => action.trigger(event)
    Keys.onSpacePressed: event => action.trigger(event)
    Keys.onMenuPressed: {
        if (item.hasActionList || item.favoriteId !== null) {
            item.openActionMenu()
        }
    }
}
