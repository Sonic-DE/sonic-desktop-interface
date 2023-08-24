/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.private.desktopcontainment.folder 0.1 as Folder

import org.kde.qqc2desktopstyle.private as StylePrivate // Can it be avoided?

ColumnLayout {
    id: configIcons

    property alias cfg_filterMode: filterMode.currentIndex
    property alias cfg_filterPattern: filterPattern.text
    property alias cfg_filterMimeTypes: mimeTypesModel.checkedTypes
    property alias cfg_showHiddenFiles: showHiddenFiles.checked

    PlasmaCore.SortFilterModel {
        id: filteredMimeTypesModel

        sourceModel: Folder.MimeTypesModel {
            id: mimeTypesModel
        }

        // SortFilterModel doesn't have a case-sensitivity option
        // but filterRegExp always causes case-insensitive sorting.
        filterRegExp: mimeFilter.text
        filterRole: "name"

        sortRole: "name"
        property int sortOrder: mimeTypesView.sortIndicatorOrder //FIXME: workaround for PlasmaCore.SortFilterModel, look again after porting to KItemModels

        function checkFiltered() {
            var types = [];

            for (var i = 0; i < count; ++i) {
                types.push(get(i).name);
            }

            mimeTypesModel.checkedTypes = types;
        }

        function uncheckFiltered() {
            var types = [];

            for (var i = 0; i < count; ++i) {
                types.push(get(i).name);
            }

            mimeTypesModel.checkedTypes = mimeTypesModel.checkedTypes
                .filter(x => types.indexOf(x) === -1);
        }
    }
    Kirigami.FormLayout {
        ComboBox {
            id: filterMode
            Kirigami.FormData.label: i18n("Files:")
            model: [i18n("Show all"), i18n("Show matching"), i18n("Hide matching")]
        }

        TextField {
            id: filterPattern
            Kirigami.FormData.label: i18n("File name pattern:")
            enabled: (filterMode.currentIndex > 0)
            inputMethodHints: Qt.ImhNoPredictiveText
        }

        Kirigami.SearchField {
            id: mimeFilter
            Kirigami.FormData.label: i18n("File types:")
            enabled: (filterMode.currentIndex > 0)
        }

        CheckBox {
            id: showHiddenFiles
            Kirigami.FormData.label: i18n("Show hidden files:")
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        CheckBox { // Purely for metrics.
            id: metricsCheckBox
            visible: false
        }

        ScrollView {
            id: scrollView
            enabled: (filterMode.currentIndex > 0)
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: 100//parent.height - metricsCheckBox.height //- selectLayout.height
            leftPadding: LayoutMirroring.enabled ? ScrollBar.vertical.width : 2
            topPadding: 2
            rightPadding: LayoutMirroring.enabled ? 2 : ScrollBar.vertical.width
            bottomPadding: 2
            background: StylePrivate.StyleItem {
                control: scrollView
                elementType: "edit"

                sunken: true
                hasFocus: mimeTypesView.activeFocus
                hover: scrollView.hovered
            }
            ListView {
                id: mimeTypesView
                clip: true

                // Signal the delegates listen to when user presses space to toggle current row.
                signal toggleCurrent()


                model: filteredMimeTypesModel
                property real columnSize: Kirigami.Units.gridUnit * 10
                headerPositioning: ListView.OverlayHeader

                Keys.onSpacePressed: toggleCurrent()

                function adjustColumns() {return
                    // Resize description column to take whatever space is left.
                    var w = width;
                    for (var i = 0; i < columns - 1; ++i) {
                        w -= getColumn(i).width;
                    }
                    descriptionColumn.width = w;
                }

                onWidthChanged: adjustColumns()
                // Component.onCompleted is too early to do this...
                onCountChanged: adjustColumns()

                header: RowLayout {
                    z: 9
                    width: mimeTypesView.width
                    spacing: 0
                    Button {
                        Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                        onClicked: {
                            if (filteredMimeTypesModel.sortRole == "checked") {
                                filteredMimeTypesModel.sortOrder = filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder;
                                filteredMimeTypesModel.sort(0, filteredMimeTypesModel.sortOrder)
                                filteredMimeTypesModel.sortOrderChanged()
                            } else {
                                filteredMimeTypesModel.sortRole = "checked"
                            }
                        }
                        background: StylePrivate.StyleItem {
                            control: parent
                            elementType: "header"
                            sunken: control.down
                            activeControl: filteredMimeTypesModel.sortRole == "checked" ? (filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? "down" : "up") : ""
                            properties: {
                                "headerpos": "beginning"
                            }
                        }
                    }
                    Button {
                        text: i18n("File type")
                        Layout.preferredWidth: mimeTypesView.columnSize + Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing * 2
                        onClicked: {
                            if (filteredMimeTypesModel.sortRole == "name") {
                                filteredMimeTypesModel.sortOrder = filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder;
                                filteredMimeTypesModel.sort(0, filteredMimeTypesModel.sortOrder)
                                filteredMimeTypesModel.sortOrderChanged()
                            } else {
                                filteredMimeTypesModel.sortRole = "name"
                            }
                        }
                        background: StylePrivate.StyleItem {
                            control: parent
                            text: control.text
                            elementType: "header"
                            sunken: control.down
                            activeControl: filteredMimeTypesModel.sortRole == "name" ? (filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? "down" : "up") : ""
                            properties: {
                                "headerpos": "middle"
                            }
                        }
                    }
                    Button {
                        text: i18n("Description")
                        Layout.fillWidth: true
                        onClicked: {
                            if (filteredMimeTypesModel.sortRole == "description") {
                                filteredMimeTypesModel.sortOrder =  filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder;
                                filteredMimeTypesModel.sort(0, filteredMimeTypesModel.sortOrder)
                                filteredMimeTypesModel.sortOrderChanged()
                            } else {
                                filteredMimeTypesModel.sortRole = "description"
                            }
                        }
                        background: StylePrivate.StyleItem {
                            control: parent
                            text: control.text
                            elementType: "header"
                            sunken: control.down
                            activeControl: filteredMimeTypesModel.sortRole == "description" ? (filteredMimeTypesModel.sortOrder == Qt.AscendingOrder ? "down" : "up") : ""
                            properties: {
                                "headerpos": "end"
                            }
                        }
                    }
                }
                delegate: ItemDelegate {
                    id: delegate
                    width: mimeTypesView.width
                    height: Kirigami.Units.iconSizes.small + padding * 2
                    required property string name
                    required property string comment
                    required property var decoration

                    contentItem: RowLayout {
                        CheckBox{
                            Layout.fillHeight: true
                            checked: mimeTypesModel.checkedTypes.indexOf(name) >= 0
                            onToggled: {
                                let idx = mimeTypesModel.checkedTypes.indexOf(name);
                                if (idx >= 0) {
                                    mimeTypesModel.checkedTypes.splice(idx, 1);
                                } else {
                                    mimeTypesModel.checkedTypes.push(name)
                                }
                            }
                        }
                        Kirigami.Icon {
                            Layout.fillHeight: true
                            implicitWidth: Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small
                            animated: false // TableView re-uses delegates, avoid animation when sorting/filtering.
                            source: decoration
                        }
                        Label {
                            text: name
                            elide: Text.ElideRight
                            Layout.preferredWidth: mimeTypesView.columnSize
                            Layout.fillHeight: true
                        }
                        Label {
                            text: comment
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }
            }
        }

        RowLayout {
            id: selectLayout
            Button {
                id: selectAllButton
                enabled: (filterMode.currentIndex > 0)
                icon.name: "edit-select-all"
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
                ToolTip.text: i18n("Select All")
                onClicked: filteredMimeTypesModel.checkFiltered()
            }

            Button {
                id: deselectAllButton
                enabled: (filterMode.currentIndex > 0)
                icon.name: "edit-select-none"
                ToolTip.delay: Kirigami.Units.toolTipDelay
                ToolTip.visible: (Kirigami.Settings.isMobile ? pressed : hovered) && ToolTip.text.length > 0
                ToolTip.text: i18n("Deselect All")
                onClicked: filteredMimeTypesModel.uncheckFiltered()
            }
        }
    }
}
