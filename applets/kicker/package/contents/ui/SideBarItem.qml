/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.plasmoid 2.0

import "code/tools.js" as Tools

Item {
    id: item

    width: root.width
    height: root.width

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList !== null)))
    property int itemIndex: model.index

    onAboutToShowActionMenu: actionMenu => {
        const actionList = (model.hasActionList !== null) ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, repeater.model, model.favoriteId);
    }

    onActionTriggered: (actionId, actionArgument) => {
        if (Tools.triggerAction(repeater.model, model.index, actionId, actionArgument) === true) {
            kicker.expanded = false;
        }
    }

    function openActionMenu(visualParent, x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: {
            repeater.model.trigger(index, "", null);
            kicker.expanded = false;
        }
    }

    TapHandler {
        enabled: item.hasActionList
        acceptedButtons: Qt.RightButton
        gesturePolicy: TapHandler.WithinBounds // Release grab when menu appears
        onPressedChanged: if (pressed) {
            item.openActionMenu(item, point.position.x, point.position.y);
        }
    }

    Kirigami.Icon {
        id: icon
        anchors.fill: parent

        active: toolTip.containsMouse

        source: model.decoration

        DragHandler {
            id: dragHandler
            onActiveChanged: if (active) {
                icon.grabToImage((result) => {
                    if (!dragHandler.active) {
                        return;
                    }
                    dragSource.Drag.imageSource = result.url;
                    dragSource.Drag.mimeData = {
                        "text/uri-list": model.url.toString(),
                    };
                    dragSource.sourceItem = item;
                    dragSource.Drag.active = dragHandler.active;
                });
            } else {
                dragSource.Drag.active = false;
            }
        }
    }

    PlasmaCore.ToolTipArea {
        id: toolTip

        property string text: model.display

        anchors {
            fill: parent
            leftMargin: - sideBar.margins.left
            rightMargin: - sideBar.margins.right
        }

        interactive: false
        location: (((Plasmoid.location === PlasmaCore.Types.RightEdge)
            || (Qt.application.layoutDirection === Qt.RightToLeft))
            ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge)

        mainItem: toolTipDelegate
    }
}
