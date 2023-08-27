/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.plasmoid 2.0

FocusScope {
    id: root

    Layout.minimumWidth: sideBar.width
        + (sideBar.visible ? mainRow.anchors.leftMargin : 0) + Math.max(searchField.defaultWidth, runnerColumns.visible ? runnerColumns.implicitWidth : 0)
    Layout.maximumWidth: Math.max(rootList.minimumWidth, Layout.minimumWidth)

    Layout.minimumHeight: Math.max(
        ((rootModel.count - rootModel.separatorCount) * rootList.itemHeight) + (rootModel.separatorCount * rootList.separatorHeight) + searchField.height + 2 * Kirigami.Units.smallSpacing,
        sideBar.margins.top + sideBar.margins.bottom + favoriteApps.contentHeight + favoriteSystemActions.contentHeight + sidebarSeparator.height + 4 * Kirigami.Units.smallSpacing
    )
    Layout.maximumHeight: Layout.minimumHeight

    readonly property alias searchField: searchField // Used to reset text
    readonly property alias rootList: rootList // Used to reset currentIndex
    readonly property Item focusItem: {
        if (!kicker.expanded) {
            return null;
        } else if (runnerColumns.activeFocus) {
            return runnerColumns;
        } else if (favoriteApps.activeFocus) {
            return favoriteApps;
        } else if (favoriteApps.activeFocus) {
            return favoriteApps;
        } else if (favoriteSystemActions.activeFocus) {
            return favoriteSystemActions;
        } else if (rootList.currentIndex !== -1) {
            return rootList;
        } else {
            return searchField;
        }
    }

    Keys.forwardTo: searchField
    Keys.onEscapePressed: {
        kicker.expanded = false;
    }

    KSvg.FrameSvgItem {
        id: sideBar

        visible: width > 0

        anchors.top: parent.top
        anchors.left: parent.left
        width: (globalFavorites && systemFavorites
            && (globalFavorites.count + systemFavorites.count)
            ? Kirigami.Units.iconSizes.medium + margins.left + margins.right : 0)
        height: parent.height
        focus: true

        imagePath: "widgets/frame"
        prefix: "plain"

        SideBarSection {
            id: favoriteApps

            anchors.top: parent.top
            anchors.topMargin: sideBar.margins.top

            height: (sideBar.height - sideBar.margins.top - sideBar.margins.bottom
                - favoriteSystemActions.height - sidebarSeparator.height - (4 * Kirigami.Units.smallSpacing))
            focus: root.focusItem === favoriteApps

            model: globalFavorites

            Binding {
                target: globalFavorites
                property: "iconSize"
                value: Kirigami.Units.iconSizes.medium
                restoreMode: Binding.RestoreBinding
            }
        }

        KSvg.SvgItem {
            id: sidebarSeparator

            anchors.bottom: favoriteSystemActions.top
            anchors.bottomMargin: (2 * Kirigami.Units.smallSpacing)
            anchors.horizontalCenter: parent.horizontalCenter

            width: Kirigami.Units.iconSizes.medium
            height: lineSvg.horLineHeight

            visible: (favoriteApps.model && favoriteApps.model.count
                && favoriteSystemActions.model && favoriteSystemActions.model.count)

            svg: lineSvg
            elementId: "horizontal-line"

            states: [ State {
                name: "top"
                when: (Plasmoid.location === PlasmaCore.Types.TopEdge)

                AnchorChanges {
                    target: sidebarSeparator
                    anchors.top: favoriteSystemActions.bottom
                    anchors.bottom: undefined

                }

                PropertyChanges {
                    target: sidebarSeparator
                    anchors.topMargin: (2 * Kirigami.Units.smallSpacing)
                    anchors.bottomMargin: undefined
                }
            }]
        }

        SideBarSection {
            id: favoriteSystemActions

            anchors.bottom: parent.bottom
            anchors.bottomMargin: sideBar.margins.bottom

            focus: root.focusItem === favoriteSystemActions
            model: systemFavorites

            states: [ State {
                name: "top"
                when: (Plasmoid.location === PlasmaCore.Types.TopEdge)

                AnchorChanges {
                    target: favoriteSystemActions
                    anchors.top: parent.top
                    anchors.bottom: undefined
                }

                PropertyChanges {
                    target: favoriteSystemActions
                    anchors.topMargin: sideBar.margins.top
                    anchors.bottomMargin: undefined
                }
            }]
        }
    }

    Item {
        id: mainRow

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: sideBar.right
            leftMargin: sideBar.visible ? Kirigami.Units.smallSpacing : 0
            right: parent.right
        }

        Row {
            id: runnerColumns

            anchors {
                top: parent.top
                left: parent.left
                bottom: searchField.top
            }
            visible: searchField.text !== "" && runnerModel.count > 0
            focus: root.focusItem === runnerColumns

            Repeater {
                id: runnerColumnsRepeater

                model: runnerModel

                delegate: RunnerResultsList {
                    id: runnerMatches
                    visible: runnerModel.modelForRow(index).count > 0

                    Keys.onLeftPressed: event => {
                        let leftIndex = index - 1;
                        while (leftIndex >= 0 && !runnerColumnsRepeater.itemAt(leftIndex).firstItem) {
                            leftIndex -= 1;
                        }
                        if (leftIndex >= 0) {
                            currentIndex = -1;
                            runnerColumnsRepeater.itemAt(leftIndex).firstItem.forceActiveFocus(Qt.TabFocusReason);
                        }
                    }
                    Keys.onRightPressed: event => {
                        let rightIndex = index + 1;
                        while (rightIndex <= runnerColumnsRepeater.count - 1 && !runnerColumnsRepeater.itemAt(rightIndex).firstItem) {
                            rightIndex += 1;
                        }
                        if (rightIndex <= runnerColumnsRepeater.count - 1) {
                            currentIndex = -1;
                            runnerColumnsRepeater.itemAt(rightIndex).firstItem.forceActiveFocus(Qt.TabFocusReason);
                        }
                    }
                }
            }
        }

        ItemListView {
            id: rootList

            anchors {
                top: parent.top
                left: parent.left
                bottom: searchField.top
            }
            minimumWidth: root.Layout.minimumWidth - sideBar.width - mainRow.anchors.leftMargin
            height: ((rootModel.count - rootModel.separatorCount) * itemHeight) + (rootModel.separatorCount * separatorHeight)

            visible: searchField.text === ""
            focus: root.focusItem === rootList
            model: rootModel

            iconsEnabled: Plasmoid.configuration.showIconsRootLevel
        }

        PlasmaExtras.SearchField {
            id: searchField

            anchors.bottom: mainRow.bottom
            anchors.left: parent.left

            readonly property real defaultWidth: Kirigami.Units.gridUnit * 14
            width: (runnerColumnsRepeater.count !== 0 ? runnerColumnsRepeater.itemAt(0).width
                                                    : (rootList.visible ? rootList.width : defaultWidth))

            focus: !Kirigami.InputMethod.willShowOnActive && root.focusItem === searchField

            KeyNavigation.up: if (runnerColumns.visible) {
                return runnerColumnsRepeater.itemAt(0).lastItem;
            } else {
                return rootList.lastItem;
            }

            KeyNavigation.down: if (runnerColumns.visible) {
                return runnerColumnsRepeater.itemAt(0).firstItem;
            } else {
                return rootList.firstItem; // Skip the first item because it's selected by default
            }

            onTextChanged: {
                runnerModel.query = text;
            }

            onFocusChanged: {
                if (focus) {
                    // FIXME: Cleanup arbitration between rootList/runnerCols here and in Keys.
                    if (rootList.visible) {
                        rootList.currentIndex = -1;
                    }

                    if (runnerColumns.visible) {
                        runnerColumnsRepeater.itemAt(0).currentIndex = -1;
                    }
                }
            }

            Keys.onUpPressed: event => {
                if (runnerColumns.visible) {
                    const lastList = runnerColumnsRepeater.itemAt(runnerColumnsRepeater.count - 1);
                    lastList.currentIndex = lastList.count - 1;
                } else {
                    // Create child dialogs only after a key is pressed
                    rootList.showChildDialogs = false;
                    rootList.currentIndex = rootList.count - 1;
                }

                event.accepted = false; // Pass the event to KeyNavigation.up
            }

            Keys.onDownPressed: event => {
                if (runnerColumns.visible) {
                    const lastList = runnerColumnsRepeater.itemAt(runnerColumnsRepeater.count - 1);
                    lastList.currentIndex = 0;
                } else {
                    // Create child dialogs only after a key is pressed
                    rootList.showChildDialogs = false;
                    rootList.currentIndex = 0;
                }

                event.accepted = false; // Pass the event to KeyNavigation.down
            }

            Keys.onReturnPressed: event => {
                if (runnerColumns.visible) {
                    KeyNavigation.down.Keys.onReturnPressed(event);
                } else {
                    event.accepted = false;
                }
            }
            Keys.onEnterPressed: event => Keys.onReturnPressed(event)
        }
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
