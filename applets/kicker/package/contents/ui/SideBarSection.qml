/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami 2 as Kirigami

// To reflect children's activeFocus
FocusScope {
    id: section

    anchors.horizontalCenter: parent.horizontalCenter
    width: Kirigami.Units.iconSizes.medium
    height: contentHeight

    property real contentHeight: model ? (model.count * Kirigami.Units.iconSizes.medium) + ((model.count - 1) * flow.spacing) : 0
    readonly property alias repeater: repeater
    property alias model: repeater.model

    Timer {
        id: resetAnimationDurationTimer

        interval: 150
        repeat: false

        onTriggered: {
            flow.animationDuration = interval - 20;
        }
    }

    DropArea {
        anchors.fill: parent
        property string favoriteId
        onEntered: event => {
            const containsFavoriteId = event.formats.indexOf("text/x-orgkdeplasmakicker_favoriteid") >= 0 && dragSource.sourceItem.parent !== flow;
            event.accpeted = (event.hasUrls && dragSource.sourceItem.parent === flow) || containsFavoriteId;
            if (containsFavoriteId) {
                favoriteId = event.getDataAsString("text/x-orgkdeplasmakicker_favoriteid");
            }
        }
        onPositionChanged: event => {
            if (flow.animating) {
                return;
            }

            const above = flow.childAt(event.x, event.y);

            if (above && above !== dragSource.sourceItem && dragSource.sourceItem.parent === flow) {
                repeater.model.moveRow(dragSource.sourceItem.itemIndex, above.itemIndex);
            }
        }
        onDropped: event => {
            if (favoriteId.length > 0) {
                kicker.globalFavorites.addFavorite(favoriteId);
                favoriteId = "";
            }
        }
    }

    Flow {
        id: flow

        anchors.fill: parent
        focus: true

        property bool animating: false
        property int animationDuration: resetAnimationDurationTimer.interval

        move: Transition {
            SequentialAnimation {
                PropertyAction { target: flow; property: "animating"; value: true }

                NumberAnimation {
                    duration: flow.animationDuration
                    properties: "x, y"
                    easing.type: Easing.OutQuad
                }

                PropertyAction { target: flow; property: "animating"; value: false }
            }
        }

        spacing: (2 * Kirigami.Units.smallSpacing)

        Repeater {
            id: repeater

            delegate: SideBarItem {
                width: section.width
                height: width
                KeyNavigation.right: section.KeyNavigation.right /* ListView will propagate focus to currentItem */

                // Prevent searchField from accepting events
                Keys.onUpPressed: event => {
                    if (index > 0) {
                        repeater.itemAt(index - 1).forceActiveFocus(Qt.TabFocusReason);
                    } else {
                        section.KeyNavigation.up?.repeater.itemAt(section.KeyNavigation.up.repeater.count - 1).forceActiveFocus(Qt.TabFocusReason);
                    }
                }
                Keys.onDownPressed: event => {
                    if (index < repeater.count - 1) {
                        repeater.itemAt(index + 1).forceActiveFocus(Qt.TabFocusReason);
                    } else {
                        section.KeyNavigation.down?.repeater.itemAt(0).forceActiveFocus(Qt.TabFocusReason);
                    }
                }
                Keys.onRightPressed: event => {
                    if (!runnerColumns.visible) {
                        rootList.showChildDialogs = false;
                        rootList.currentIndex = 0;
                    }
                    event.accepted = false;
                }
            }

            onCountChanged: {
                flow.animationDuration = 0;
                resetAnimationDurationTimer.start();
            }
        }
    }
}
