/*
    Copyright (C) 2020  Mikel Johnson <mikel5764@gmail.com>
    Copyright (C) 2021  Kai Uwe Broulik <kde@broulik.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.private.kicker 0.1 as Kicker
import org.kde.plasma.components 2.0 as PlasmaComponents // for Menu + MenuItem
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.16 as Kirigami

RowLayout {
    id: root
    property alias leave: leaveButton
    spacing: PlasmaCore.Units.smallSpacing// * 2

    Kicker.SystemModel {
        id: systemModel
        favoritesModel: KickoffSingleton.rootModel.systemFavoritesModel
    }

    Row {
        id: favoriteButtonsRow
        spacing: parent.spacing
        Layout.fillWidth: true
        Repeater {
            model: systemModel
            delegate: PC3.ToolButton {
                id: buttonDelegate
                text: model.display
                icon.name: model.decoration
                // TODO: Don't generate items that will never be seen. Maybe DelegateModel can help?
                visible: String(plasmoid.configuration.systemFavorites).includes(model.favoriteId)
                onClicked: {
                    model.trigger(index, "", "")
                }
            }
        }
    }

    PC3.ToolButton {
        id: leaveButton
        readonly property int currentId: plasmoid.configuration.primaryActions
        display: PC3.AbstractButton.IconOnly
        icon.width: PlasmaCore.Units.iconSizes.smallMedium
        icon.height: PlasmaCore.Units.iconSizes.smallMedium
        icon.name: ["system-log-out", "system-shutdown", "view-more-symbolic"][currentId];
        text: [i18n("Leave…"), i18n("Power…"), i18n("More…")][currentId]
        // Make it look pressed while the menu is open
        down: contextMenu.status === PlasmaComponents.DialogStatus.Open || pressed
        PC3.ToolTip.text: [i18n("Leave"), i18n("Power"), i18n("More")][leaveButton.currentId]
        PC3.ToolTip.visible: leaveButton.display === PC3.AbstractButton.IconOnly && leaveButton.hovered
        PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
        onPressed: contextMenu.openRelative()
    }

    Instantiator {
        model: systemModel
        // Not a QQC1 MenuItem. It's actually a custom QQuickItem.
        delegate: PlasmaComponents.MenuItem {
            text: model.display
            icon: model.decoration
            // TODO: Don't generate items that will never be seen. Maybe DelegateModel can help?
            visible: !String(plasmoid.configuration.systemFavorites).includes(model.favoriteId)

            onClicked: model.trigger(index, "", "")
        }

        onObjectAdded: {
            contextMenu.addMenuItem(object);
        }
    }

    // Not a QQC1 Menu. It's actually a custom QObject that uses a QMenu.
    PlasmaComponents.Menu {
        id: contextMenu
        visualParent: leaveButton
        placement: {
            switch (plasmoid.location) {
            case PlasmaCore.Types.LeftEdge:
            case PlasmaCore.Types.RightEdge:
            case PlasmaCore.Types.TopEdge:
                return PlasmaCore.Types.BottomPosedRightAlignedPopup;
            case PlasmaCore.Types.BottomEdge:
            default:
                return PlasmaCore.Types.TopPosedRightAlignedPopup;
            }
        }
    }
}
