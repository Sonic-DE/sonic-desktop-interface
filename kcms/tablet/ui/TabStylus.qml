/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

Kirigami.FormLayout {
    id: root

    required property var device

    Repeater {
        model: [
            { value: 0x14b, text: i18nd("kcm_tablet", "Pen button 1:") },
            { value: 0x14c, text: i18nd("kcm_tablet", "Pen button 2:") },
            { value: 0x149, text: i18nd("kcm_tablet", "Pen button 3:") }
        ] // BTN_STYLUS, BTN_STYLUS2, BTN_STYLUS3

        delegate: KeySequenceItem {
            id: seq
            Kirigami.FormData.label: (pressed ? "<b>" : "") + modelData.text + (pressed ? "</b>" : "")
            property bool pressed: false

            Connections {
                target: tabletEvents
                function onToolButtonReceived(hardware_serial_hi, hardware_serial_lo, button, pressed) {
                    if (button !== modelData.value) {
                        return;
                    }
                    seq.pressed = pressed
                }
            }

            keySequence: kcm.toolButtonMapping(root.device.name, modelData.value)
            Connections {
                target: kcm
                function onSettingsRestored() {
                    seq.keySequence = kcm.toolButtonMapping(root.device.name, modelData.value)
                }
            }

            showCancelButton: true
            modifierlessAllowed: true
            modifierOnlyAllowed: true
            multiKeyShortcutsAllowed: false
            checkForConflictsAgainst: ShortcutType.None

            onCaptureFinished: {
                kcm.assignToolButtonMapping(root.device.name, modelData.value, keySequence)
            }
        }
    }

    TabletEvents {
        id: tabletEvents

        anchors.fill: parent

        onPadButtonsChanged: {
            if (!path.endsWith(form.padDevice.sysName)) {
                return;
            }
            buttonsRepeater.model = buttonCount
        }
    }
}