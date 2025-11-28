/*
 *  SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.0

import org.kde.activities 0.1 as Activities
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.19 as Kirigami


KCM.ScrollViewKCM {

    implicitWidth: Kirigami.Units.gridUnit * 18
    implicitHeight: Kirigami.Units.gridUnit * 22

    KCM.ConfigModule.buttons: KCM.ConfigModule.Help

    Activities.ActivityModel {
        id: kactivities
    }

    view: ListView {
        id: activitiesList

        clip: true

        model: Activities.ActivitySortProxyModel {
            id: kactivitiesProxy

            sourceModel: kactivities
        }

        delegate: QQC2.ItemDelegate {
            width: ListView.view.width

            onClicked: kcm.configureActivity(model.id);

            contentItem: RowLayout {
                id: row

                Kirigami.Icon {
                    id: icon
                    height: Kirigami.Units.iconSizes.medium
                    width: height
                    source: model.icon
                }

                QQC2.Label {
                    Layout.fillWidth: true
                    text: model.name
                    textFormat: Text.PlainText
                }

                QQC2.ToolButton {
                    visible: kcm.isNewActivityAuthorized
                    enabled: activitiesList.count > 1 && model.index != 0
                    icon.name: "arrow-up"
                    text: i18nc("@info:tooltip", "Promote %1 activity", model.name)
                    display: QQC2.AbstractButton.IconOnly
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                    onClicked: {
                        kcm.setActivityUserSortOrder(model.id, model.index - 1);
                    }
                }

                QQC2.ToolButton {
                    visible: kcm.isNewActivityAuthorized
                    enabled: activitiesList.count > 1 && model.index < activitiesList.count - 1
                    icon.name: "arrow-down"
                    text: i18nc("@info:tooltip", "Demote %1 activity", model.name)
                    display: QQC2.AbstractButton.IconOnly
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                    onClicked: {
                        kcm.setActivityUserSortOrder(model.id, model.index + 1);
                    }
                }

                QQC2.ToolButton {
                    visible: kcm.isNewActivityAuthorized
                    enabled:  activitiesList.count > 1
                    icon.name: "edit-delete"
                    text: i18nc("@info:tooltip", "Delete %1 activity", model.name)
                    display: QQC2.AbstractButton.IconOnly
                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                    onClicked: {
                        removePrompt.activityId = model.id;
                        removePrompt.activityName = model.name;
                        removePrompt.open();
                    }
                }
            }
        }
    }

    actions: [
        Kirigami.Action {
            visible: kcm.isNewActivityAuthorized
            text: i18nc("@action:button", "Create New…")
            icon.name: "list-add"
            onTriggered: kcm.newActivity();
        },
        Kirigami.Action {
            visible: kcm.isNewActivityAuthorized && activitiesList.count > 1
            text: i18n("Default Sort")
            icon.name: "sort-name-symbolic"
            onTriggered: {
                kcm.setActivitySortMethodDefault();
            }
        }
    ]

    Kirigami.PromptDialog {
        id: removePrompt

        property string activityId: ""
        property string activityName: ""

        title: i18nc("@title:window", "Delete Activity")
        subtitle: i18nc("%1 is an activity name",
                         "Do you want to delete activity '%1'?", activityName)

        dialogType: Kirigami.PromptDialog.Warning

        standardButtons: Kirigami.Dialog.Cancel
        customFooterActions: [
            Kirigami.Action {
                text: i18n("Delete Activity")
                icon.name: "edit-delete"
                onTriggered: {
                    kcm.deleteActivity(removePrompt.activityId)
                    removePrompt.close()
                }
            }
        ]
    }
}
