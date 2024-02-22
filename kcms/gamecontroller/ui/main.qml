/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo@venerandi.com>
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
// import org.kde.config // KAuthorized

import org.kde.plasma.gamecontroller.kcm

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 35

    Kirigami.PlaceholderMessage {
        icon.name: "input-gamepad"
        text: i18n("No game controllers found")
        explanation: i18n("Connect a wired or wireless controller")
        anchors.centerIn: parent
        visible: deviceModel.count === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: false

    DeviceModel {
        id: deviceModel
    }

    DeviceTypeModel {
        id: deviceTypeModel
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            maximumColumns: 4

            Repeater {
                id: repeater

                model: deviceModel

                Kirigami.Card {
                    required property int index
                    required property string name

                    Kirigami.Theme.colorSet: Kirigami.Theme.Window
                    Kirigami.Theme.inherit: false

                    property var deviceTypeRow: deviceTypeModel.rowFromType(deviceModel.device(index).gamepadType)

                    banner.title: name

                    topPadding: Kirigami.Units.gridUnit * 2

                    contentItem: Item {
                        implicitWidth: gamepadgui.width
                        implicitHeight: gamepadgui.height

                        GamepadRoot {
                            id: gamepadgui

                            anchors.centerIn: parent

                            width: Kirigami.Units.gridUnit * 20
                            height: Kirigami.Units.gridUnit * 20

                            device: deviceModel.device(index)
                            svgPath: deviceTypeModel.pathFromRow(deviceTypeRow)
                        }
                    }
                    actions: [
                        Kirigami.Action {
                            id: typesToPopulate

                            text: i18nc("@action:button Change type of gamepad preview", "Preview Type")
                            icon.name: "view-preview"
                        },
                        Kirigami.Action {
                            text: i18nc("@action:button", "Controller Information")
                            icon.name: "input-gamepad-symbolic"
                            onTriggered: kcm.push("AdvancedPage.qml", { device: gamepadgui.device })
                        }
                    ]

                    // HACK: Delegates of the Repeater must be Items, so I add an Action inside an Item,
                    // and I cannot use a Repeater inside another Action so I instantiate them here
                    // and re-parent them manually.
                    Repeater {
                        model: deviceTypeModel

                        Item {
                            Kirigami.Action {
                                id: deviceTypeAction

                                text: name
                                onTriggered: {

                                    deviceTypeRow = index;
                                    console.log("new deviceTypeRow is " + deviceTypeRow);
                                }

                                Component.onCompleted: typesToPopulate.children.push(deviceTypeAction)
                            }
                        }
                    }
                }
            }
        }
    }

}
