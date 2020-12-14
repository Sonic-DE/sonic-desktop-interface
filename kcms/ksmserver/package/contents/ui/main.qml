/***************************************************************************
 *   Copyright (C) 2020 by Nicolas Fella <nicolas.fella@gmx.de             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

import QtQuick 2.10
import QtQuick.Controls 2.10
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Dialogs 1.3
import org.kde.desktopsession.private 1.0 
import org.kde.kcm 1.3 as KCM

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    Kirigami.FormLayout {
        CheckBox {
            text: i18n("Confirm logout")
            checked: Settings.confirmLogout
            onCheckStateChanged: Settings.confirmLogout = checked
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "confirmLogout"
            }
        }
        CheckBox {
            text: i18n("Offer shutdown options")
            checked: Settings.offerShutdown
            onCheckStateChanged: Settings.offerShutdown = checked
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "offerShutdown"
            }
        }
        ButtonGroup {
            buttons: [leaveEnd, leaveRestart, leaveOff]
        }
        RadioButton {
            id: leaveEnd
            Kirigami.FormData.label: i18n("Default leave option")
            text: i18n("End current session")
            checked: Settings.shutdownType === 0
            onCheckedChanged: Settings.shutdownType = 0
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "shutdownType"
            }
        }
        RadioButton {
            id: leaveRestart
            text: i18n("Restart computer")
            checked: Settings.shutdownType === 1
            onCheckedChanged: Settings.shutdownType = 1
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "shutdownType"
            }
        }
        RadioButton {
            id: leaveOff
            text: i18n("Turn off computer")
            checked: Settings.shutdownType === 2
            onCheckedChanged: Settings.shutdownType = 2
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "shutdownType"
            }
        }
        ButtonGroup {
            buttons: [loginRestore, loginManual, loginEmpty]
        }
        RadioButton {
            id: loginRestore
            Kirigami.FormData.label: i18n("On login")
            text: i18n("Restore previous saved session")
            checked: Settings.loginMode === 0
            onCheckedChanged: Settings.loginMode = 0
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "loginMode"
            }
        }
        RadioButton {
            id: loginManual
            text: i18n("Restore manually saved session")
            checked: Settings.loginMode === 1
            onCheckedChanged: Settings.loginMode = 1
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "loginMode"
            }
        }
        RadioButton {
            id: loginEmpty
            text: i18n("Start with an empty session")
            checked: Settings.loginMode === 2
            onCheckedChanged: Settings.loginMode = 2
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "loginMode"
            }
        }
        TextField {
            Kirigami.FormData.label: i18n("Applications to be excluded from sessions")
            text: Settings.excludeApps
            onTextChanged: Settings.excludeApps = text
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "excludeApps"
            }
        }
        CheckBox {
            text: i18n("Enter UEFI setup on next restart")
            checked: kcm.restartInSetupScreen
            onCheckStateChanged: kcm.restartInSetupScreen = checked
            ToolTip.text: i18n("When the computer is restarted the next time, enter firmware setup screen (e.g. UEFI or BIOS setup)")
        }
        Kirigami.InlineMessage {
            height: 40
            width: 30
            Kirigami.FormData.isSection: true
            type: kcm.error.length > 0 ? Kirigami.MessageType.Error : Kirigami.MessageType.Information
            visible: kcm.restartInSetupScreen || kcm.error
            text: kcm.error.length > 0
                ? i18n("Failed to request restart to firmware setup: %1", kcm.error)
                : kcm.isUEFI ? i18n("Next time the computer is restarted, it will enter the UEFI setup screen.")
                             : i18n("Next time the computer is restarted, it will enter the firmware setup screen.")
            showCloseButton: true
            actions: Kirigami.Action {
                visible: kcm.error.length === 0
                icon.name: "view-refresh"
                onTriggered: kcm.reboot();
            }
        }
    }
}
