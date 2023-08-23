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

        sortRole: mimeTypesView.getColumn(mimeTypesView.sortIndicatorColumn).role
        sortOrder: mimeTypesView.sortIndicatorOrder

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
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: 200
            //background.visible: true
            ListView {
                id: mimeTypesView
                clip: true

                // Signal the delegates listen to when user presses space to toggle current row.
                signal toggleCurrent()

                enabled: (filterMode.currentIndex > 0)

                model: filteredMimeTypesModel
                property real columnSize: Kirigami.Units.gridUnit * 15
    /*
                sortIndicatorVisible: true
                sortIndicatorColumn: 2 // Default to sort by "File type".

                onSortIndicatorColumnChanged: { // Disallow sorting by icon.
                    if (sortIndicatorColumn === 1) {
                        sortIndicatorColumn = 2;
                    }
                }*/

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
                    width: mimeTypesView.width
                    Label {
                        text: i18n("File type")
                        Layout.preferredWidth: mimeTypesView.columnSize
                    }
                    Label {
                        text: i18n("Description")
                        Layout.fillWidth: true
                    }
                }
                delegate: ItemDelegate {
                    id: delegate
                    width: mimeTypesView.width
                    required property string name
                    required property string comment
                    required property var decoration

                    contentItem: RowLayout {
                        CheckBox{
                            checked: delegate.checked
                        }
                        Kirigami.Icon {
                            width: Kirigami.Units.iconSizes.small
                            height: Kirigami.Units.iconSizes.small
                            animated: false // TableView re-uses delegates, avoid animation when sorting/filtering.
                            source: decoration
                        }
                        Label {
                            text: name
                            Layout.preferredWidth: mimeTypesView.columnSize
                        }
                        Label {
                            text: comment
                            Layout.fillWidth: true
                        }
                    }
                }
    /*
                QQC1.TableViewColumn {
                    role: "checked"
                    width: metricsCheckBox.width
                    resizable: false
                    movable: false

                    delegate: CheckBox {
                        id: checkBox

                        checked: styleData.value
                        activeFocusOnTab: false // only let the TableView as a whole get focus
                        onClicked: {
                            model.checked = checked
                            // Clicking it breaks the binding to the model value which becomes
                            // an issue during sorting as TableView re-uses delegates.
                            checked = Qt.binding(() => styleData.value);
                        }

                        Connections {
                            target: mimeTypesView
                            function onToggleCurrent() {
                                if (styleData.row === mimeTypesView.currentRow) {
                                    model.checked = !checkBox.checked;
                                }
                            }
                        }
                    }
                }

                QQC1.TableViewColumn {
                    role: "decoration"
                    width: Kirigami.Units.iconSizes.small
                    resizable: false
                    movable: false

                    delegate: Kirigami.Icon {
                        width: Kirigami.Units.iconSizes.small
                        height: Kirigami.Units.iconSizes.small
                        animated: false // TableView re-uses delegates, avoid animation when sorting/filtering.
                        source: styleData.value
                    }
                }

                QQC1.TableViewColumn {
                    id: nameColumn
                    role: "name"
                    title: i18n("File type")
                    width: Kirigami.Units.gridUnit * 10 // Assume somewhat reasonable default for mime type name.
                    onWidthChanged: mimeTypesView.adjustColumns()
                    movable: false
                }

                QQC1.TableViewColumn {
                    id: descriptionColumn
                    role: "comment"
                    title: i18n("Description")
                    movable: false
                    resizable: false
                }*/
            }
        }

        RowLayout {
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
