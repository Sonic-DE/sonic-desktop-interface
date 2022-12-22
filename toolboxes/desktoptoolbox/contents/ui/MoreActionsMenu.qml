/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.plasmoid 2.0

PlasmaCore.Dialog {
    id: menuDialog

    visible: true

    flags: Qt.WindowStaysOnTopHint
    hideOnWindowDeactivate: true
    location: Plasmoid.location
    type: PlasmaCore.Dialog.PopupMenu

    mainItem: PlasmaComponents3.ScrollView {
        id: scrollView

        width: PlasmaCore.Units.gridUnit * 15
        height: Math.min(Screen.desktopAvailableHeight * 3 / 4, groupListView.contentHeight)

        ListView {
            id: groupListView

            readonly property TextMetrics textMetrics: TextMetrics {}

            model: {
                const actionList = Plasmoid.contextualActions;
                if (actionList && actionList.length > 0) { // Only available in KF5.102
                    return actionList.filter(action => action.visible);
                }
                return Plasmoid.actions.filter(action => action.visible);
            }
            reuseItems: true

            delegate: PlasmaComponents3.ItemDelegate {
                width: groupListView.width

                enabled: modelData.enabled && text.length > 0
                icon.name: modelData.iconText
                text: modelData.text

                onClicked: {
                    modelData.trigger();
                    menuDialog.visible = false;
                }
            }

            onCountChanged: Qt.callLater(updateMaxTextWidth);

            function updateMaxTextWidth() {
                let tempMaxTextWidth = 0;
                for (let i = 0; i < count; ++i) {
                    textMetrics.text = groupListView.itemAtIndex(i).text;
                    if (textMetrics.boundingRect.width > tempMaxTextWidth) {
                        tempMaxTextWidth = textMetrics.boundingRect.width;
                    }
                }
                if (scrollView.width < tempMaxTextWidth) {
                    scrollView.width = tempMaxTextWidth;
                }
            }
        }
    }

    onVisibleChanged: if (!visible) {
        destroy();
    }

    Component.onCompleted: {
        menuButton.menuDialog = this;
    }
}
