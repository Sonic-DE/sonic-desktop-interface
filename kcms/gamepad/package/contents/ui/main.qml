/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kcm 1.4 as KCM
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Layouts 1.3 as Layouts
import org.kde.plasma.gamepad.kcm 1.0
import QtQuick.Shapes 1.15

KCM.SimpleKCM {
    id: root
    // KCM.ConfigModule.quickHelp: i18n("This module lets you test and configure game controllers.")

    DeviceModel {
        id: deviceModel
    }

    DeviceTypeModel {
        id: deviceTypeModel
    }

    property var gamepadCount: deviceSelector.count
    property int selectedDevice: deviceSelector.currentIndex
    property var currentDevice: deviceModel.device(selectedDevice)

    Kirigami.PlaceholderMessage {
        text: i18n("No gamepad found")
        anchors.centerIn: parent
        visible: gamepadCount === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    actions: [
        Kirigami.Action {
            id: addAction
            icon.name: "list-add"
            text: i18nc("@action:button", "Add bluetooth device...")
            onTriggered: { Qt.openUrlExternally("systemsettings://kcm_bluetooth"); }
        },

        Kirigami.Action {
            icon.name: "configure"
            text: i18nd("kcm_pulseaudio", "Advanced View…")
            onTriggered: kcm.push("AdvancedPage.qml", { "device": currentDevice })
            enabled: currentDevice !== null
        }
    ]

    Kirigami.FormLayout {
        id: formLayout

        anchors.fill: parent

        visible: gamepadCount > 0

        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_gamepad", "Device:")
            id: deviceSelector

            Layouts.Layout.fillWidth: true
            model: deviceModel
            textRole: "name"

            Connections {
                target: deviceModel
                function onRowsRemoved() {
                    deviceSelector.currentIndex = Math.min(deviceSelector.currentIndex, deviceModel.rowCount() - 1)
                    selectGamepadType()
                }

                function onRowsInserted() {
                    console.log("onRowsAdded called, currentindex is " + deviceSelector.currentIndex)
                    if (deviceSelector.currentIndex == -1) {
                        deviceSelector.currentIndex = 0
                        selectGamepadType()
                    }
                }
            }

            Connections {
                target: deviceSelector
                function onActivated() {
                    selectGamepadType()
                }
            }
        }

        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_gamepad", "Device type:")
            id: deviceTypeSelector

            Layouts.Layout.fillWidth: true
            model: deviceTypeModel
            textRole: "name"
            onCurrentIndexChanged: {
                makeGamepadObject();
            }
        }
    }

    Component.onCompleted: {
        // First select the gamepad type for the current gamepad if any
        selectGamepadType()

        // Then generate the gamepad object to show
        makeGamepadObject()
    }

    function selectGamepadType() {
        // Select the default gamepad type for the current gamepad
        var deviceType = currentDevice.type
        console.log("Current device type: " + deviceType)
        // NOTE: For this to work deviceType needs to match 1:1 to the combobox contents...
        deviceTypeSelector.currentIndex = currentDevice.deviceType
    }

    function makeGamepadObject() {

        // Create a new gamepad of the given type and load it after formLayout
        var currentIndex = deviceTypeSelector.currentIndex
        console.log("currentIndex: " + deviceTypeSelector.currentIndex)

        var typeName = deviceTypeModel.qmlName(currentIndex)
        console.log("typename: " + typeName)
        if (typeName != "") {
            // Remove the gamepad object if any
            if (typeof gamepadgui !== "undefined") {
                gamepadgui.destroy()
            }

            var str = "import './gamepadtypes' as GamepadTypes
                GamepadTypes." + typeName + "{ id: gamepadgui; device: currentDevice; visible: currentDevice != null }"
            var newlayout = Qt.createQmlObject(str, root)
        }
    }
}
