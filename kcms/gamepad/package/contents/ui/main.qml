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

    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 35

    DeviceModel {
        id: deviceModel
    }

    Connections {
        target: deviceModel
        function onRowsRemoved() {
            selectedDevice = Math.min(selectedDevice, deviceModel.rowCount() - 1)
            selectGamepadType()
        }

        function onRowsInserted() {
            // Nothing selected before, so select the one existing item
            if (deviceModel.rowCount() === 1) {
                selectedDevice = 0
                selectGamepadType()
            }
        }
    }

    DeviceTypeModel {
        id: deviceTypeModel
    }

    property int gamepadCount: deviceList.count
    property int selectedDevice: deviceList.currentIndex
    property var currentDevice: deviceModel.device(selectedDevice)
    property var newlayout

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

    ColumnLayout {
        id: layout

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        implicitWidth: parent.width

        visible: gamepadCount > 0

        Kirigami.FormLayout {
            id: deviceSelectionlayout

            ListView {
                Kirigami.FormData.label: i18nd("kcm_gamepad", "Device:")
                id: deviceList
    //            width: parent.width
                implicitHeight: 40
                orientation: Qt.Horizontal
                model: deviceModel

                // Device selection row
                Layouts.Layout.fillWidth: true

                delegate:
                    QQC2.Button {
                        checkable: true
                        checked: deviceList.currentIndex === index
                        icon.name: connectionType === Gamepad.BluetoothType ? "network-bluetooth" : "input-gamepad"
                        hoverEnabled: true

                        QQC2.ToolTip.delay: 1000
                        QQC2.ToolTip.timeout: 5000
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: name
                        text: i18nc("Controller number name", "Gamepad #" + (index + 1))
                        onClicked: {
                            deviceList.currentIndex = index
                            selectGamepadType();
                        }
                    }

            }

//            Connections {
//                target: deviceSelector
//                function onActivated() {
//                    selectGamepadType()
//                }
//            }

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

        // Create a new gamepad of the given type and load it after control
        var currentIndex = deviceTypeSelector.currentIndex
        console.log("currentIndex: " + deviceTypeSelector.currentIndex)

        var typeName = deviceTypeModel.qmlName(currentIndex)
        console.log("typename: " + typeName)
        if (typeName != "") {
            // Remove the gamepad object if any
            if (typeof newlayout !== "undefined") {
                newlayout.destroy()
            }

            var str = "import './gamepadtypes' as GamepadTypes
                GamepadTypes." + typeName + "{ id: gamepadgui; device: currentDevice; visible: currentDevice != null; }"
            newlayout = Qt.createQmlObject(str, layout)
        }
    }
}
