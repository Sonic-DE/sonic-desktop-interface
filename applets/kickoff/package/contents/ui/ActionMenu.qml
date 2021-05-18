/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma Singleton

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import Qt.labs.platform 1.1 as Platform
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PC2
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: root
    visible: false

    property var modelData: null
    property var actionList: null

    // Not a QQC1 Menu. It's actually a custom QObject that uses a QMenu.
    property PC2.Menu menu: PC2.Menu {
        id: menu
        placement: root.placement
        visualParent: root.visualParent
        //onVisualParentChanged: {
            //if (visualParent != null) {
                //clearMenuItems()
            //}
        //}
    }

    Instantiator {
        id: instantiator
        model: 3
        delegate: PC2.MenuItem {
            text: "asdf " + index
        }
        onObjectAdded: menu.addMenuItem(object)
        onObjectRemoved: menu.removeMenuItem(object)
    }

    signal actionClicked(string actionId, var actionArgument)

    //onActionListChanged: refreshMenu();

    function refreshMenu() {
        if (menu != null) {
            menu.clearMenuItems();
        }

        if (!actionList) {
            return;
        }

        // actionList.forEach(function(actionItem) {
        //     var item = contextMenuItemComponent.createObject(menu, {
        //         "actionItem": actionItem,
        //     });
        // });

        fillMenu(menu, actionList);
    }

    function fillMenu(menu, items) {
        items.forEach((actionItem) => {
            if (actionItem.subActions) {
                // This is a menu
                var submenuItem = contextSubmenuItemComponent.createObject(menu, { "actionItem" : actionItem });
                fillMenu(submenuItem.submenu, actionItem.subActions);
            } else {
                var item = contextMenuItemComponent.createObject(menu, { "actionItem": actionItem });
            }
        });
    }

    Component {
        id: contextSubmenuItemComponent

        PC2.MenuItem {
            id: submenuItem

            property var actionItem

            text: actionItem.text ? actionItem.text : ""
            icon: actionItem.icon ? actionItem.icon : null

            property var submenu: PC2.Menu {
                id: submenu_
                visualParent: submenuItem.action
            }
        }
    }

    Component {
        id: contextMenuItemComponent

        PC2.MenuItem {
            property var actionItem

            text      : actionItem.text ? actionItem.text : ""
            enabled   : actionItem.type !== "title" && ("enabled" in actionItem ? actionItem.enabled : true)
            separator : actionItem.type === "separator"
            section   : actionItem.type === "title"
            icon      : actionItem.icon ? actionItem.icon : null
            checkable : actionItem.checkable ? actionItem.checkable : false
            checked   : actionItem.checked ? actionItem.checked : false

            onClicked: {
                root.actionClicked(actionItem.actionId, actionItem.actionArgument);
            }
        }
    }
}
