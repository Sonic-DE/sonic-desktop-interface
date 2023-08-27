/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami

FocusScope {
    id: itemList

    property real minimumWidth: Kirigami.Units.gridUnit * 14
    property real maximumWidth: minimumWidth * 2

    width: minimumWidth
    height: listView.contentHeight

    signal keyNavigationAtListEnd

    property Item focusParent: null
    property QtObject dialog: null
    property QtObject childDialog: null
    property bool iconsEnabled: false
    property int itemHeight: Math.ceil((Math.max(Kirigami.Units.iconSizes.sizeForLabels, Kirigami.Units.iconSizes.small)
        + Math.max(highlightItemSvg.margins.top + highlightItemSvg.margins.bottom,
        listItemSvg.margins.top + listItemSvg.margins.bottom)) / 2) * 2
    property int separatorHeight: model.sorted === true ? 0 : lineSvg.horLineHeight + (2 * Kirigami.Units.smallSpacing)

    property alias currentIndex: listView.currentIndex
    property alias currentItem: listView.currentItem
    property alias keyNavigationWraps: listView.keyNavigationWraps
    property alias showChildDialogs: listView.showChildDialogs
    property alias model: listView.model
    property alias count: listView.count

    function spawnDialog(focusOnSpawn = false) {
        if (!kicker.expanded || model === undefined || currentIndex == -1) {
            return;
        }

        if (itemList.childDialog !== null) {
            return;
        }

        itemList.childDialog = itemListDialogComponent.createObject(itemList, {
            "focusParent": itemList,
        });

        if (focusOnSpawn) {
            itemList.childDialog.mainItem.showChildDialogs = false;
            itemList.childDialog.mainItem.currentIndex = 0;
            itemList.childDialog.mainItem.showChildDialogs = true;
        }
    }

    Connections {
        target: kicker
        function onExpandedChanged(expanded) {
            if (!expanded) {
                childDialog?.destroy();
            }
        }
    }

    ScrollView {
        anchors.fill: parent

        focus: true

        ListView {
            id: listView

            property bool showChildDialogs: true
            property int eligibleWidth: width

            currentIndex: -1

            boundsBehavior: Flickable.StopAtBounds
            snapMode: ListView.SnapToItem
            spacing: 0
            keyNavigationWraps: (dialog != null)

            delegate: ItemListDelegate {
                onFullTextWidthChanged: {
                    if (fullTextWidth > itemList.width) itemList.width = Math.min(fullTextWidth, itemList.maximumWidth);
                }
            }

            highlight: PlasmaExtras.Highlight {
                visible: listView.currentItem && !listView.currentItem.isSeparator
            }

            highlightMoveDuration: 0

            onCountChanged: {
                currentIndex = -1;
            }

            onCurrentIndexChanged: {
                if (!currentItem?.hasChildren || !kicker.expanded || currentIndex === -1) {
                    itemList.childDialog?.destroy();
                } else if (listView.showChildDialogs) {
                    itemList.spawnDialog();
                }
            }

            Keys.onPressed: event => {
                if (event.key === Qt.Key_Up) {
                    event.accepted = true;

                    if (!keyNavigationWraps && currentIndex == 0) {
                        itemList.keyNavigationAtListEnd();

                        return;
                    }

                    showChildDialogs = false;
                    decrementCurrentIndex();

                    if (currentItem.isSeparator) {
                        decrementCurrentIndex();
                    }

                    showChildDialogs = true;
                } else if (event.key === Qt.Key_Down) {
                    event.accepted = true;

                    if (!keyNavigationWraps && currentIndex == count - 1) {
                        itemList.keyNavigationAtListEnd();

                        return;
                    }

                    showChildDialogs = false;
                    incrementCurrentIndex();

                    if (currentItem.isSeparator) {
                        incrementCurrentIndex();
                    }

                    showChildDialogs = true;
                } else if ((event.key === Qt.Key_Right || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && itemList.childDialog != null) {
                    itemList.childDialog.mainItem.focus = true;
                    itemList.childDialog.mainItem.currentIndex = 0;
                } else if ((event.key === Qt.Key_Right || event.key === Qt.Key_Return || event.key === Qt.Key_Enter) && itemList.childDialog == null
                    && currentItem != null && currentItem.hasChildren) {
                    dialogSpawnTimer.focusOnSpawn = true;
                    dialogSpawnTimer.restart();
                } else if (event.key === Qt.Key_Left && dialog != null) {
                    dialog.destroy();
                } else if (event.key === Qt.Key_Escape) {
                    kicker.expanded = false;
                } else if (event.key === Qt.Key_Tab) {
                    //do nothing, and skip appending text
                }
            }
        }
    }
}
