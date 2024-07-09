/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.Dialog {
    id: actionDialog

    property InputSequence inputSequence

    signal gotInputSequence(sequence: InputSequence)

    title: i18nc("@title Select the action for the tablet button", "Select Button Action")
    modal: true

    maximumWidth: Kirigami.Units.gridUnit * 20
    maximumHeight: Kirigami.Units.gridUnit * 17

    topPadding: Kirigami.Units.largeSpacing
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    showCloseButton: false

    onAccepted: gotInputSequence(inputSequence)

    function refreshDialogData(): void {
        switch (inputSequence.type) {
            case InputSequence.Disabled:
                disabledRadio.checked = true;
                break;
            case InputSequence.Keyboard:
                keyboardRadio.checked = true;
                seq.keySequence = inputSequence.keySequence();
                break;
            case InputSequence.Mouse:
                mouseRadio.checked = true;
                clickCombo.currentIndex = indexOfValue(inputSequence.mouseButton())
                break;
            case InputSequence.ApplicationDefined:
                applicationRadio.checked = true;
                break;
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.ButtonGroup { id: radioGroup }

        QQC2.RadioButton {
            id: disabledRadio

            readonly property int index: 0

            icon.name: "action-unavailable-symbolic"
            text: i18nc("@option:radio Disable this action", "Disabled")

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = InputSequence.Disabled;
                refreshDialogData();
            }
        }

        QQC2.RadioButton {
            id: keyboardRadio

            readonly property int index: 1

            icon.name: "input-keyboard-symbolic"
            text: i18nc("@option:radio Set this action to a keyboard type", "Keyboard")

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = InputSequence.Keyboard;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: mouseRadio

            readonly property int index: 2

            text: i18nc("@option:radio Set this action to a mouse type", "Mouse")
            icon.name: "input-mouse-symbolic"

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = InputSequence.Mouse;
                refreshDialogData();
            }
        }
        QQC2.RadioButton {
            id: applicationRadio

            readonly property int index: 3

            icon.name: "applications-all-symbolic"
            text: i18nc("@option:radio Set this action to an application-defined type", "Application-defined")

            QQC2.ButtonGroup.group: radioGroup

            onToggled: {
                inputSequence.type = InputSequence.ApplicationDefined;
                refreshDialogData();
            }
        }

        QQC2.MenuSeparator {
            Layout.fillWidth: true
        }

        StackLayout {
            currentIndex: radioGroup.checkedButton.index

            ColumnLayout {
                id: disabledView

                QQC2.Label {
                    text: i18nc("@info", "This button is disabled.")

                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                }
            }
            ColumnLayout {
                id: keyboardView

                KeySequenceItem {
                    id: seq

                    showCancelButton: true
                    modifierlessAllowed: true
                    modifierOnlyAllowed: true
                    multiKeyShortcutsAllowed: false
                    checkForConflictsAgainst: ShortcutType.None

                    onCaptureFinished: actionDialog.inputSequence.setKeySequence(keySequence)
                }
            }
            ColumnLayout {
                id: mouseView

                QQC2.ComboBox {
                    id: clickCombo

                    textRole: "text"
                    valueRole: "value"
                    model: [
                        { value: Qt.LeftButton, text: i18nc("@action:inmenu Left mouse click", "Left Click") },
                        { value: Qt.MiddleButton, text: i18nc("@action:inmenu Middle mouse click", "Middle Click") },
                        { value: Qt.RightButton, text: i18nc("@action:inmenu Right mouse click", "Right Click") }
                    ]

                    onActivated: actionDialog.inputSequence.setMouseButton(currentValue)
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.CheckBox {
                        text: "Ctrl"
                    }

                    QQC2.CheckBox {
                        text: "Alt"
                    }

                    QQC2.CheckBox {
                        text: "Meta"
                    }

                    QQC2.CheckBox {
                        text: "Shift"
                    }
                }
            }
            ColumnLayout {
                id: applicationView

                QQC2.Label {
                    text: i18nc("@info", "This tablet button is directly sent to the focused application. You must configure the button inside of the application." )

                    wrapMode: Text.WordWrap

                    Layout.fillWidth: true
                }
            }
        }
    }
}