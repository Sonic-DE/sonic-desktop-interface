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
        id: enableScreenReader
        text: i18n("Enable Screen Reader")

        KCM.SettingStateBinding {
            configObject: kcm.screenReaderSettings
            settingName: "Enabled"
        }

        visible: screenReaderInstalled
        checked: kcm.screenReaderSettings.enabled
        onToggled: kcm.screenReaderSettings.enabled = checked
    }
    QQC2.Button {
        text: i18n("Launch Orca Screen Reader Configuration…")

        visible: screenReaderInstalled
        enabled: !kcm.screenReaderSettings.isImmutable("Enabled") && screenReaderInstalled

        onClicked: kcm.launchOrcaConfiguration()
    }
    QQC2.Label {
        text: kcm.orcaLaunchFeedback
        textFormat: Text.PlainText
    }

    onVisibleChanged: {
        if (visible === true && screenReaderInstalled === null) {
            screenReaderInstalled = kcm.orcaInstalled()
        }
    }
}
