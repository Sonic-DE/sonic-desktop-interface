/*
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {

    QQC2.CheckBox {
        Kirigami.FormData.label: i18nc("@label", "Invert:")
        text: i18nc("@option check, Enable invert effect", "Enable")

        KCM.SettingStateBinding {
            configObject: kcm.invertSettings
            settingName: "Invert"
        }

        checked: kcm.invertSettings.invert
        onToggled: kcm.invertSettings.invert = checked
    }

    QQC2.Button {
        text: i18n("Configure Shortcuts…")
        icon.name: "preferences-desktop-keyboard-shortcut"

        onClicked: kcm.configureInvert()
    }
}
