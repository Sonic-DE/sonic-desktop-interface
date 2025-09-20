/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.20 as Kirigami
import org.kde.private.desktopcontainment.folder as Folder
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: configIcons

    property alias cfg_filterMode: filterMode.currentIndex
    property alias cfg_filterPattern: filterPattern.text
    property alias cfg_filterMimeTypes: mimeTypesModel.checkedTypes
    property alias cfg_showHiddenFiles: showHiddenFiles.checked

    extraFooterTopPadding: true // add separator at end of view

    KItemModels.KSortFilterProxyModel {
        id: filteredMimeTypesModel

        sourceModel: Folder.MimeTypesModel {
            id: mimeTypesModel
        }

        filterRegularExpression: RegExp(mimeTypesView.headerItem?.filterText ?? "", "i")
        filterRoleName: "name"

        sortRoleName: "name"
        sortOrder: Qt.AscendingOrder

        function toggleSortOrder() : void {
            sortOrder = sortOrder === Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder
        }

        function setSortRoleOrToggleOrder(roleName: string) : void {
            if (sortRoleName === roleName) {
                toggleSortOrder()
            } else {
                sortRoleName = roleName
                sortOrder = Qt.AscendingOrder
            }
        }

        function getTypes(model, onlyChecked = false) : list<string> {
            let types = [];

            for (let i = 0; i < count; ++i) {
                if (!onlyChecked || index(i, 0).data(Qt.CheckStateRole)) {
                    types.push(index(i, 0).data(Qt.UserRole));
                }
            }
            return types
        }

        function getAllTypes() : list<string> {
            return getTypes(mimeTypesModel, false)
        }

        function getFilteredTypes() : list<string> {
            return getTypes(filteredMimeTypesModel, false)
        }

        function getCheckedTypes() : list<string> {
            mimeTypesModel.checkedTypes // just to depend on it for bindings
            return getTypes(mimeTypesModel, true)
        }

        function checkTypes (types: list<string>) : void {
            mimeTypesModel.checkedTypes = [...mimeTypesModel.checkedTypes, ...types];
        }

        function checkFiltered() : void {
            checkTypes(getFilteredTypes());
        }

        function checkAll() : void {
            checkTypes(getAllTypes());
        }

        function uncheckFiltered() : void {
            let types = getFilteredTypes();
            mimeTypesModel.checkedTypes = mimeTypesModel.checkedTypes.filter(x => !types.includes(x));
        }

        function restore(types: list<string>): void {
            uncheckFiltered();
            checkTypes(types);
        }
    }

    Shortcut {
        sequences: ["Ctrl+I"] // Dolphin's filter shortcut
        onActivated: mimeTypesView.headerItem.filterField.forceActiveFocus(Qt.TabFocusReason)
    }

    Shortcut {
        sequences: [StandardKey.SelectAll]
        onActivated: {
            mimeTypesView.headerItem.allCheckBox.check()
        }
    }

    Shortcut {
        sequences: [StandardKey.Deselect]
        onActivated: {
            mimeTypesView.headerItem.allCheckBox.uncheck()
        }
    }

    header: ColumnLayout {
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            visible: mimeTypesModel.rowCount() > mimeTypesModel.checkedTypes.length && filterPattern.length > 0
            icon.source: "view-filter"
            text: i18nc("@info:usagetip inlinemessage", "The file name pattern will only apply to files of the chosen file types.")
            actions: [
                Kirigami.Action {
                    text: i18nc("@action:button enable all file types", "Enable All Types")
                    onTriggered: {
                        mimeTypesView.headerItem.allCheckBox.storeStateIfPartial()
                        filteredMimeTypesModel.checkAll()
                        mimeTypesView.headerItem.allCheckBox.updateCheckState()
                    }
                }
            ]
        }

        Kirigami.FormLayout {

            CheckBox {
                id: showHiddenFiles
                Kirigami.FormData.label: i18nc("@label form label", "Hidden files:")
                text: i18nc("@option:check for hidden files", "Show like regular files")
            }

            ComboBox {
                id: filterMode
                Kirigami.FormData.label: i18nc("@label form label", "Show files:")
                model: [i18nc("@item:inlistbox show files", "All files"), i18nc("@item:inlistbox show files", "Only files that match"), i18nc("@item:inlistbox show files", "Only files that do not match")]
            }

            TextField {
                id: filterPattern
                Kirigami.FormData.label: i18nc("@label form label", "File name patterns:")
                placeholderText: i18nc("@info:placeholder file name pattern", "Separate with space to use multiple patterns")
                enabled: (filterMode.currentIndex > 0)
                inputMethodHints: Qt.ImhNoPredictiveText
            }
        }
    }

    component SortButton: ToolButton {
        required property string roleName
        hoverEnabled: filteredMimeTypesModel.sortRoleName === roleName
        activeFocusOnTab: true
        Accessible.description: {
            if (filteredMimeTypesModel.sortRoleName != roleName) {
                return i18nc("@action:button accessible description current button state", "Not sorting by this column")
            }
            return filteredMimeTypesModel.sortOrder === Qt.AscendingOrder
                ? i18nc("@action:button accessible description current button state", "Current order: Alphabetical")
                : i18nc("@action:button accessible description current button state", "Current order: Reverse alphabetical")
        }
        display: AbstractButton.IconOnly
        icon.height: Kirigami.Units.iconSizes.small
        icon.width: Kirigami.Units.iconSizes.small
        icon.source: {
            if (hoverEnabled || activeFocus) {
                return filteredMimeTypesModel.sortOrder === Qt.AscendingOrder ? "arrow-up-symbolic" : "arrow-down"
            }
            return ""
        }
        onClicked: filteredMimeTypesModel.setSortRoleOrToggleOrder(roleName)
    }


    view: ListView {
        id: mimeTypesView
        clip: true
        enabled: (filterMode.currentIndex > 0)

        model: filteredMimeTypesModel
        currentIndex: -1
        property real columnSize: Kirigami.Units.gridUnit * 15
        property list<string> previouslyCheckedTypes: []
        headerPositioning: ListView.OverlayHeader
        Accessible.role: Accessible.List
        activeFocusOnTab: true

        Keys.onUpPressed: {
            if (currentIndex === 0) {
                headerItem.filterField.forceActiveFocus(Qt.TabFocusReason)
            } else {
                event.accepted = false
            }
        }

        Keys.onSpacePressed: {
            currentItem.checkBox.toggle()
            currentItem.checkBox.toggled()
        }

        header: Rectangle {
            property alias filterField: mimeFilter
            property alias filterText: mimeFilter.text
            property alias allCheckBox: allCheckBox
            width: mimeTypesView.width
            clip: true
            height: listHeaderLayout.implicitHeight + listHeaderLayout.anchors.topMargin + listHeaderLayout.anchors.rightMargin
            color: Kirigami.Theme.backgroundColor
            z: 8
            opacity:1

            ColumnLayout {
                id: listHeaderLayout
                anchors.fill: parent

                Row {
                    id: headerRow
                    z: 9
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.smallSpacing

                    RowLayout {
                        width: Math.round(headerRow.width / 2)
                        height: Kirigami.Units.gridUnit * 2


                        CheckBox {
                            id: allCheckBox
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            Layout.fillHeight: true
                            Layout.leftMargin: Kirigami.Units.smallSpacing * 2
                            tristate: true

                            Accessible.name: i18nc("@option:check tri-state all mimetypes checkbox in header, accessible only", "All File Types")
                            Accessible.description: checkState === Qt.PartiallyChecked ? i18nc("@option:check accessible description %1 numer currently checked %2 number total types", "%1 of %2 types checked", filteredMimeTypesModel.getCheckedTypes().length, mimeTypesModel.rowCount()) : ""

                            property int actualCheckState // using checkState directly breaks the logic

                            function storeStateIfPartial() {
                                if (actualCheckState == Qt.PartiallyChecked) {
                                    mimeTypesView.previouslyCheckedTypes = filteredMimeTypesModel.getCheckedTypes()
                                }
                            }

                            function check() {
                                storeStateIfPartial()
                                filteredMimeTypesModel.checkFiltered()
                                updateCheckState()
                            }

                            function uncheck() {
                                storeStateIfPartial()
                                filteredMimeTypesModel.uncheckFiltered()
                                updateCheckState()
                            }

                            function updateCheckState () {
                                let checkedTypes = filteredMimeTypesModel.getCheckedTypes()
                                if (checkedTypes.length === mimeTypesView.count) {
                                    actualCheckState = Qt.Checked
                                } else if (checkedTypes.length === 0) {
                                    actualCheckState = Qt.Unchecked
                                } else {
                                    actualCheckState = Qt.PartiallyChecked
                                }
                                checkState = actualCheckState
                            }
                            Component.onCompleted: updateCheckState()

                            onClicked: {
                                if (actualCheckState === Qt.Unchecked && mimeTypesView.previouslyCheckedTypes.length > 0) {
                                    filteredMimeTypesModel.restore(mimeTypesView.previouslyCheckedTypes)
                                    updateCheckState()
                                } else if (actualCheckState === Qt.Checked) {
                                    uncheck()
                                } else {
                                    check()
                                }
                            }

                            nextCheckState: function() {
                                if (actualCheckState === Qt.PartiallyChecked){
                                    return Qt.Checked
                                } else if (actualCheckState === Qt.Checked) {
                                    return Qt.Unchecked
                                } else if (mimeTypesView.previouslyCheckedTypes.length > 0) {
                                    return Qt.PartiallyChecked
                                } else {
                                    return Qt.Checked
                                }
                            }
                        }

                        Kirigami.Heading {
                            Layout.fillHeight: true
                            verticalAlignment: Text.AlignVCenter
                            text: i18nc("@title:column", "File Type")
                        }

                        TapHandler {
                            onTapped: filteredMimeTypesModel.setSortRoleOrToggleOrder("name")
                        }

                        SortButton {
                            roleName: "name"
                            text: i18nc("@action:button icon-only button for Accessible", 'Sort by column "File Type"')
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                    }

                    RowLayout {
                        width: Math.round(headerRow.width / 2)
                        height: parent.height

                        Kirigami.Heading {
                            Layout.fillHeight: true

                            verticalAlignment: Text.AlignVCenter
                            text: i18nc("@title:column", "Description")

                        }

                        TapHandler {
                            onTapped: filteredMimeTypesModel.setSortRoleOrToggleOrder("comment")
                        }

                        SortButton {
                            roleName: "comment"
                            text: i18nc("@action:button icon-only button for Accessible", 'Sort by column "Description"')
                        }

                        Kirigami.SearchField {
                            Layout.leftMargin: Kirigami.Units.largeSpacing * 3
                            Layout.rightMargin: Kirigami.Units.smallSpacing

                            id: mimeFilter
                            Layout.alignment: Qt.AlignVCenter
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            enabled: (filterMode.currentIndex > 0)
                            placeholderText: i18nc("@info:placeholder filter list of file types", "Filter types…")
                            onActiveFocusChanged: if (activeFocus) mimeTypesView.currentIndex = -1
                        }
                    }
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                }
            }
        }

        delegate: ItemDelegate {
            id: delegate
            width: mimeTypesView.width
            required property string name
            required property string comment
            required property var decoration
            required property var model
            required property int index

            property alias checkBox: typeCheckBox

            Accessible.role: Accessible.CheckBox
            Accessible.name: name
            Accessible.description: comment

            Keys.onSpacePressed: {
                typeCheckBox.toggle()
                typeCheckBox.toggled()
            }

            onClicked: {
                typeCheckBox.toggle()
                typeCheckBox.toggled()
                mimeTypesView.currentIndex = index
            }

            contentItem: Row {
                id: delegateContentItem
                width: delegate.availableWidth
                height: Math.max(delegateColumn1.implicitHeight, delegateColumn2.implicitHeight)

                RowLayout {
                    id: delegateColumn1
                    width: Math.round (parent.width / 2)

                    CheckBox {
                        id: typeCheckBox
                        Layout.fillHeight: true
                        checked: delegate.model.checked
                        activeFocusOnTab: false
                        onToggled: {
                            delegate.model.checked = this.checked
                            mimeTypesView.previouslyCheckedTypes = []
                            mimeTypesView.headerItem.allCheckBox.updateCheckState()
                        }
                    }
                    Kirigami.Icon {
                        id: typeIcon
                        Layout.fillHeight: true
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small
                        animated: false // TableView re-uses delegates, avoid animation when sorting/filtering.
                        source: delegate.decoration
                    }
                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: delegate.name
                        elide: Text.ElideRight
                    }
                }
                Label {
                    id: delegateColumn2
                    width: Math.round (parent.width / 2)
                    height: parent.height
                    text: delegate.comment
                    elide: Text.ElideRight
                }
            }
        }
    }
}
