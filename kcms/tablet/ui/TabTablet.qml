
import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.tablet.kcm 1.1
import org.kde.kcmutils
import org.kde.kquickcontrols 2.0

Kirigami.FormLayout {
    required property var device

    property QtObject padDevice: null
    QQC2.ComboBox {
        Kirigami.FormData.label: i18ndc("kcm_tablet", "@label:listbox The pad we are configuring", "Pad:")
        model: kcm.padsModel

        onCurrentIndexChanged: {
            parent.padDevice = kcm.padsModel.deviceAt(currentIndex)
        }

        onCountChanged: if (count > 0 && currentIndex < 0) {
            currentIndex = 0;
        }
        enabled: count > 0
        displayText: enabled ? currentText : i18n("None")
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

    Repeater {
        id: buttonsRepeater
        model: tabletEvents.padButtons

        delegate: KeySequenceItem {
            id: seq
            Kirigami.FormData.label: (pressed ? "<b>" : "") + i18nd("kcm_tablet", "Pad button %1:", modelData + 1) + (pressed ? "</b>" : "")
            property bool pressed: false

            Connections {
                target: tabletEvents
                function onPadButtonReceived(path, button, pressed) {
                    if (button !== modelData || !path.endsWith(form.padDevice.sysName)) {
                        return;
                    }
                    seq.pressed = pressed
                }
            }

            keySequence: kcm.padButtonMapping(form.padDevice.name, modelData)
            Connections {
                target: kcm
                function onSettingsRestored() {
                    seq.keySequence = kcm.padButtonMapping(form.padDevice.name, modelData)
                }
            }

            showCancelButton: true
            modifierlessAllowed: true
            modifierOnlyAllowed: true
            multiKeyShortcutsAllowed: false
            checkForConflictsAgainst: ShortcutType.None

            onCaptureFinished: {
                kcm.assignPadButtonMapping(form.padDevice.name, modelData, keySequence)
            }
        }
    }
}