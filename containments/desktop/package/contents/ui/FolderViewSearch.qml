/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami

import org.kde.private.desktopcontainment.folder as Folder

Folder.SearchDialog {
    id: dialog

    location: PlasmaCore.Types.Floating
    type: PlasmaCore.Dialog.Dock
    flags: Qt.WindowStaysOnTopHint

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

    function forceFocusToField() {
        searchField.forceActiveFocus();
    }

    mainItem:
        RowLayout {
        width: Kirigami.Units.gridUnit * 30

        Kirigami.SearchField {
            id: searchField

            focusSequence: undefined // To not display the shortcut tooltip
            Layout.fillWidth: true

            property bool hasError: false

            background: Item {}
            color: hasError ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            Keys.onEscapePressed: dialog.visible = false

            onAccepted: {
                dialog.searchString = text.trim();

                if (Plasmoid.configuration.useRegularExpression && !dialog.isValidRegularExpression()) {
                    searchField.hasError = true;
                    return;
                }

                searchField.hasError = false;
                dir.search(dialog.getRegularExpression());
            }
        }

        QQC2.ToolButton {
            checkable: true
            checked: dialog.searchSensitivity
            icon.name: checked ? "format-text-uppercase" : "format-text-lowercase"
            display: QQC2.AbstractButton.IconOnly

            QQC2.ToolTip.text: i18n("Match Case")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            onClicked: {
                Plasmoid.configuration.searchSensitivity = checked;
                searchField.accepted();
            }
        }

        QQC2.ToolButton {
            checkable: true
            checked: dialog.matchWholeWord
            icon.name: "markasblank"
            display: QQC2.AbstractButton.IconOnly

            QQC2.ToolTip.text: i18n("Match Whole Word")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            onClicked: {
                Plasmoid.configuration.matchWholeWord = checked;
                searchField.accepted();
            }
        }

        QQC2.ToolButton {
            checkable: true
            checked: dialog.useRegularExpression
            icon.name: "code-context"
            display: QQC2.AbstractButton.IconOnly

            QQC2.ToolTip.text: i18n("Use Regular Expression")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            onClicked: {
                Plasmoid.configuration.useRegularExpression = checked;
                searchField.accepted();
            }
        }

        QQC2.ToolButton {
            id: closeButton
            icon.name: "window-close"
            display: QQC2.AbstractButton.IconOnly

            QQC2.ToolTip.text: i18n("Close")
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

            onClicked: dialog.visible = false
        }
    }
}