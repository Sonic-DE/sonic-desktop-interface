/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>
    SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
// qmllint disable unqualified
import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.plasma.private.kcm_keyboard as KCMKeyboard
import org.kde.plasma.keyboardlayout.ipc as KCMKeyboardIPC
import org.kde.kcmutils

SimpleKCM {
    id: root

    property int cfg_displayStyle

    readonly property string layoutShortName:
        KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList.length
            ? KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList[KCMKeyboardIPC.KeyboardLayoutSocket.layout].shortName
            : ""
    readonly property string displayName:
        KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList.length
            ? KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList[KCMKeyboardIPC.KeyboardLayoutSocket.layout].displayName
            : ""

    // Fall back to the system default layout's country code when the
    // user has not configured any layouts (layoutShortName is empty).
    function countryCode() {
        if (root.layoutShortName.length >= 2) {
            return root.layoutShortName;
        }
        return KCMKeyboard.Flags.getDefaultCountryCode();
    }

    Kirigami.FormLayout {
        RadioButton {
            id: showLabel
            Kirigami.FormData.label: i18nc("@title:group of radio buttons, options are language codes or images", "Display style:")
            text: root.countryCode().toUpperCase()
            checked: root.cfg_displayStyle === 0
            onToggled: root.cfg_displayStyle = 0;
        }

        RadioButton {
            id: showFlag
            checked: root.cfg_displayStyle === 1
            onToggled: root.cfg_displayStyle = 1;
            contentItem: Text {
                anchors.left: parent.left
                anchors.leftMargin: showFlag.indicator.width + showFlag.spacing
                anchors.verticalCenter: parent.verticalCenter
                font.family: "Noto Color Emoji"
                font.pixelSize: Kirigami.Units.iconSizes.medium
                text: {
                    const cc = root.countryCode().toUpperCase();
                    if (cc.length < 2) return "\u2328";
                    return String.fromCodePoint(0x1F1E6 + cc.charCodeAt(0) - 65,
                                                0x1F1E6 + cc.charCodeAt(1) - 65);
                }
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        Button {
            Kirigami.FormData.label: i18nc("@label prefixed to button, as in 'keyboard layouts'", "Layouts:")
            text: i18nc("@action:button opens kcm_keyboard", "Configure…")
            icon.name: "configure"
            onClicked: KCMLauncher.openSystemSettings("kcm_keyboard", "--tab=layouts")
        }
    }
}
