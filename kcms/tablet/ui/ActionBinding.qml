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

QQC2.Button {
    id: root

    required property InputSequence inputSequence

    signal gotInputSequence(sequence: InputSequence)

    text: inputSequence.toString()

    onClicked: {
        actionDialog.open();

        refreshDialogData();
    }

    function refreshDialogData(): void {
        switch (inputSequence.type) {
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

    Kirigami.Dialog {
        id: actionDialog

        title: i18n("Select Button Action")
        modal: true

        implicitWidth: Kirigami.Units.gridUnit * 20
        implicitHeight: Kirigami.Units.gridUnit * 15

        topPadding: Kirigami.Units.largeSpacing
        leftPadding: Kirigami.Units.largeSpacing
        rightPadding: Kirigami.Units.largeSpacing
        bottomPadding: Kirigami.Units.largeSpacing

        standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
        showCloseButton: false

        onAccepted: root.gotInputSequence(inputSequence)

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            QQC2.ButtonGroup { id: radioGroup }

            QQC2.RadioButton {
                id: keyboardRadio

                readonly property int index: 0

                icon.name: "input-keyboard-symbolic"
                text: qsTr("Keyboard")

                QQC2.ButtonGroup.group: radioGroup

                onToggled: {
                    inputSequence.type = InputSequence.Keyboard;
                    refreshDialogData();
                }
            }
            QQC2.RadioButton {
                id: mouseRadio

                readonly property int index: 1

                text: qsTr("Mouse")
                icon.name: "input-mouse-symbolic"

                QQC2.ButtonGroup.group: radioGroup

                onToggled: {
                    inputSequence.type = InputSequence.Mouse;
                    refreshDialogData();
                }
            }
            QQC2.RadioButton {
                id: applicationRadio

                readonly property int index: 2

                text: qsTr("Application-defined")
                icon.name: "applications-all-symbolic"

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
                width: parent.width
                currentIndex: radioGroup.checkedButton.index
                ColumnLayout {
                    id: homeTab

                    KeySequenceItem {
                        id: seq

                        showCancelButton: true
                        modifierlessAllowed: true
                        modifierOnlyAllowed: true
                        multiKeyShortcutsAllowed: false
                        checkForConflictsAgainst: ShortcutType.None

                        onCaptureFinished: root.inputSequence.setKeySequence(keySequence)
                    }
                }
                ColumnLayout {
                    id: discoverTab

                    QQC2.ComboBox {

                        textRole: "text"
                        valueRole: "value"
                        model: [
                            { value: Qt.LeftButton, text: i18n("Left Click") },
                            { value: Qt.MiddleButton, text: i18n("Middle Click") },
                            { value: Qt.RightButton, text: i18n("Right Click") }
                        ]

                        onActivated: root.inputSequence.setMouseButton(currentValue)
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
                    id: activityTab

                    QQC2.Label {
                        text: i18n("This button is directly sent to the focused application. You must configure this button inside of the application." )

                        wrapMode: Text.WordWrap

                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}