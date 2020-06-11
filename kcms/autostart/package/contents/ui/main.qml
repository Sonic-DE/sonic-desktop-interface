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
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Dialogs 1.3
import org.kde.kcm 1.2 as KCM
import org.kde.plasma.kcm.autostart 1.0

KCM.ScrollViewKCM {

    header: Kirigami.InlineMessage {
        id: errorMessage
        type: Kirigami.MessageType.Error
        showCloseButton: true

        Connections {
            target: kcm.model
            function onError(message) {
                errorMessage.visible = true
                errorMessage.text = message
            }
        }
    }

    view: ListView {
        clip: true
        model: kcm.model

        delegate: Kirigami.SwipeListItem {
            Row {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    id: appIcon
                    source: model.iconName
                    width: Kirigami.Units.iconSizes.medium
                    height: Kirigami.Units.iconSizes.medium
                }

                Label {
                    height: appIcon.height
                    text: model.source === AutostartModel.XdgAutoStart ? model.name : model.command
                }
            }

            actions: [
                Kirigami.Action {
                    text: i18n("Properties")
                    icon.name: "document-properties"
                    onTriggered: kcm.model.editApplication(model.index)
                    visible: model.source === AutostartModel.XdgAutoStart
                },
                Kirigami.Action {
                    text: i18n("Remove")
                    icon.name: "list-remove"
                    onTriggered: kcm.model.removeEntry(model.index)
                }
            ]
        }

        section.property: "source"
        section.delegate: Kirigami.ListSectionHeader {
            text: {
                if (section == AutostartModel.XdgAutoStart) {
                    return i18n("Applications")
                }
                if (section == AutostartModel.XdgScripts || section == AutostartModel.PlasmaStart) {
                    return i18n("Login Scripts")
                }
                if (section == AutostartModel.PlasmaShutdown) {
                    return i18n("Logout Scripts")
                }
            }
        }
    }

    footer: Row {
        spacing: Kirigami.Units.largeSpacing

        FileDialog {
            id: startupFileDialog
            title: i18n("Choose Startup Script")
            folder: shortcuts.home
            selectMultiple: false
            onAccepted: {
                kcm.model.addScript(startupFileDialog.fileUrl, AutostartModel.XdgScripts)
            }
        }

        FileDialog {
            id: logoutFileDialog
            title: i18n("Choose Logout Script")
            folder: shortcuts.home
            selectMultiple: false
            onAccepted: {
                kcm.model.addScript(logoutFileDialog.fileUrl, AutostartModel.PlasmaShutdown)
            }
        }

        Button {
            icon.name: "list-add"
            text: i18n("Add Application")
            onClicked: kcm.model.showApplicationDialog()
        }
        Button {
            icon.name: "list-add"
            text: i18n("Add Login Script")
            onClicked: startupFileDialog.open()
        }
        Button {
            icon.name: "list-add"
            text: i18n("Add Logout Script")
            onClicked: logoutFileDialog.open()
        }
    }
}
