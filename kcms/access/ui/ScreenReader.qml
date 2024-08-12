/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    property var screenReaderInstalled : null

    QQC2.CheckBox {
        text: i18n("Enable Screen Reader")

        KCM.SettingStateBinding {
            configObject: kcm.screenReaderSettings
            settingName: "Enabled"
        }

        checked: kcm.screenReaderSettings.enabled
        onToggled: kcm.screenReaderSettings.enabled = checked
    }
    QQC2.Button {
        text: i18n("Launch Orca Screen Reader Configuration…")

        enabled: !kcm.screenReaderSettings.isImmutable("Enabled") && screenReaderInstalled

        onClicked: kcm.launchOrcaConfiguration()
    }
    QQC2.Label {
        text: kcm.orcaLaunchFeedback
        textFormat: Text.PlainText
    }
    QQC2.Label {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: screenReaderInstalled
            ? i18n("Please note that you may have to log out or reboot once to allow the screen reader to work properly.")
            : i18n("It appears that the Orca Screen Reader is not installed. Please install it before trying to use this feature, and then log out or reboot")
        textFormat: Text.PlainText
    }

    onVisibleChanged: {
        if (visible === true && screenReaderInstalled === null) {
            screenReaderInstalled = kcm.orcaInstalled()
        }
    }
}
