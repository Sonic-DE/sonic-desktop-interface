/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kcm 1.3 as KCM
import org.kde.kirigami 2.19 as Kirigami
import org.kde.kquickcontrolsaddons 2.0  // For KCMShell

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 25

    Kirigami.FormLayout {
        width: parent.width
        anchors.top: parent.top

        QQC2.ButtonGroup {
            id: positionGroup
        }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("KRunner position:")
            checked: !kcm.krunnerSettings.freeFloating
            onToggled: kcm.krunnerSettings.freeFloating = false
            QQC2.ButtonGroup.group: positionGroup
            text: i18n("Top")
        }

        QQC2.RadioButton {
            checked: kcm.krunnerSettings.freeFloating
            onToggled: kcm.krunnerSettings.freeFloating = true
            QQC2.ButtonGroup.group: positionGroup
            text: i18n("Center")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "freeFloating"
            }
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18nc("@label part of a sentence", "Activate when:")
            checked: kcm.krunnerSettings.activateWhenTypingOnDesktop
            onCheckedChanged: kcm.krunnerSettings.activateWhenTypingOnDesktop = checked
            text: i18nc("@option:check", "Activate when pressing any key on the desktop")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "activateWhenTypingOnDesktop"
            }
        }

        Item {
            Layout.fillWidth: true
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("KRunner history:")
            checked: kcm.krunnerSettings.historyEnabled
            onCheckedChanged: kcm.krunnerSettings.historyEnabled = checked
            text: i18n("Enable")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "historyEnabled"
            }
        }

        QQC2.CheckBox {
            checked: kcm.krunnerSettings.retainPriorSearch
            onCheckedChanged: kcm.krunnerSettings.retainPriorSearch = checked
            text: i18n("Retain previous search")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "retainPriorSearch"
            }
        }

        QQC2.CheckBox {
            checked: kcm.krunnerSettings.activityAware
            onCheckedChanged: kcm.krunnerSettings.activityAware = checked
            text: i18n("Activity aware (previous search and history)")

            KCM.SettingStateBinding {
                configObject: kcm.krunnerSettings
                settingName: "activityAware"
            }
        }

        QQC2.Button {
            id: clearHistoryButton
            readonly property bool isSingleHistory: !kcm.krunnerSettings.activityAware || kcm.historyKeys.length < 2
            enabled: kcm.krunnerSettings.historyEnabled && kcm.historyKeys.length > 0

            icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
            text: isSingleHistory ? i18n("Clear History") : i18n("Clear History…")

            checkable: !isSingleHistory
            checked: activityMenu.visible

            // NOTE: Use onReleased to avoid race condition
            onReleased: {
                if (isSingleHistory) {
                    kcm.deleteAllHistory();
                    return;
                }

                if (!activityList.model) {
                    activityList.model = Qt.createQmlObject('import org.kde.activities 0.1; ActivityModel {}', root);
                }

                if (activityMenu.visible) {
                    activityMenu.close();
                } else {
                    activityMenu.popup(x, y + implicitHeight);
                }
            }
        }

        QQC2.Menu {
            id: activityMenu

            QQC2.MenuItem {
                icon.name: clearHistoryButton.icon.name
                text: i18nc("@item:inmenu delete krunner history for all activities", "For all activities")

                onTriggered: kcm.deleteAllHistory()
            }

            Repeater {
                id: activityList

                QQC2.MenuItem {
                    enabled: kcm.historyKeys.includes(model.id)

                    icon.source: model.iconSource || clearHistoryButton.icon.source
                    text: i18nc("@item:inmenu delete krunner history for this activity", "For activity \"%1\"", model.name)

                    onTriggered: kcm.deleteHistoryGroup(model.id)
                }
            }
        }

        Item {
            visible: pluginButton.visible
            Layout.fillWidth: true
        }

        Loader {
            id: pluginButton
            active: kcm.doesShowPluginButton
            visible: active

            Kirigami.FormData.label: i18nc("@label", "Plugins:")

            sourceComponent: QQC2.Button {
                text: i18nc("@action:button", "Enable or Disable Plugins…")
                icon.name: "plasma-search"

                onClicked: KCMShell.openSystemSettings("kcm_plasmasearch")
            }
        }
    }
}
