/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami

import org.kde.private.desktopcontainment.folder as Folder

Folder.SearchDialog {
    id: dialog

    height: mainItem.implicitHeight + dialog.margins.top + + dialog.margins.bottom
    Behavior on height { NumberAnimation { duration: Kirigami.Units.shortDuration } }

    location: PlasmaCore.Types.Floating
    hideOnWindowDeactivate: true

    searchSensitivity: Plasmoid.configuration.searchSensitivity ? Qt.CaseSensitive : Qt.CaseInsensitive
    matchWholeWord: Plasmoid.configuration.matchWholeWord
    useRegularExpression: Plasmoid.configuration.useRegularExpression

    onVisibleChanged: {
        if (visible) {
            forceFocusToField();
        } else {
            searchField.text = "";
            searchField.accepted();
        }
    }

    onRegularExpressionChanged: {
        if (Plasmoid.configuration.useRegularExpression && !dialog.isValidRegularExpression()) {
            searchField.hasError = true;
            return;
        }

        searchField.hasError = false;
        dir.search(dialog.regularExpression);
    }

    function forceFocusToField() {
        searchField.forceActiveFocus();
    }

    ColumnLayout {
        width: Kirigami.Units.gridUnit * 30

        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop

            Kirigami.SearchField {
                id: searchField

                focusSequence: undefined // To not display the shortcut tooltip
                Layout.fillWidth: true

                property bool hasError: false

                background: Item {}
                color: hasError ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor

                onAccepted: dialog.searchString = text.trim();
            }

            QQC2.ToolButton {
                icon.name: "edit-select-all"
                text: i18nc("@action:button", "Select Found")
                display: QQC2.ToolButton.IconOnly

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                onClicked: dir.selectFound()
            }
        

            QQC2.ToolButton {
                icon.name: "edit-select-none"
                text: i18nc("@action:button", "Deselect Found")
                display: QQC2.ToolButton.IconOnly

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                onClicked: dir.clearSelection()
            }
        

            QQC2.ToolButton {
                id: powerSettings

                icon.name: "settings-configure"
                text: i18nc("@action:button", "Power Search")
                display: QQC2.ToolButton.IconOnly

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                action: Kirigami.Action {
                    checkable: true
                    shortcut: "Alt+S"
                }

                // To indicate about enabled search options
                Rectangle {
                    width: Kirigami.Units.largeSpacing
                    height: Kirigami.Units.largeSpacing

                    anchors {
                        right: parent.right
                        top: parent.top
                        margins: -Kirigami.Units.largeSpacing / 4
                    }

                    radius: width
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    color: Kirigami.Theme.neutralTextColor
                    visible: dialog.searchSensitivity || dialog.matchWholeWord || dialog.useRegularExpression
                }
            }

            QQC2.ToolButton {
                text: i18nc("@action:button", "Close (Escape)")
                display: QQC2.ToolButton.IconOnly

                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered && !pressed
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                action: Kirigami.Action {
                    icon.name: "window-close"
                    shortcut: StandardKey.Cancel
                    onTriggered: dialog.hide()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.smallSpacing * 2
            visible: powerSettings.checked

            QQC2.ToolButton {
                text: i18nc("@action:button", "Match Case")

                QQC2.ToolTip.text: i18nc("@info:tooltip", "Match Case (Alt+C)")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                action: Kirigami.Action {
                    checkable: true
                    checked: dialog.searchSensitivity
                
                    icon.name: checked ? "format-text-uppercase" : "format-text-lowercase"

                    shortcut: "Alt+C"

                    onTriggered: Plasmoid.configuration.searchSensitivity = checked
                }
            }

            QQC2.ToolButton {
                text: i18nc("@action:button", "Whole Word")

                QQC2.ToolTip.text: i18nc("@info:tooltip", "Match Whole Word (Alt+W)")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                action: Kirigami.Action {
                    checkable: true
                    checked: dialog.matchWholeWord

                    icon.name: "markasblank"

                    shortcut: "Alt+W"

                    onTriggered: Plasmoid.configuration.matchWholeWord = checked
                }
            }

            QQC2.ToolButton {
                text: i18nc("@action:button", "RegExp")

                QQC2.ToolTip.text: i18nc("@info:tooltip", "Use Regular Expression (Alt+R)")
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.timeout: Kirigami.Units.humanMoment

                action: Kirigami.Action {
                    checkable: true
                    checked: dialog.useRegularExpression

                    icon.name: "code-context"

                    shortcut: "Alt+R"

                    onTriggered: Plasmoid.configuration.useRegularExpression = checked
                }
            }
        }
    }
}
