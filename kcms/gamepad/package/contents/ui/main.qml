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

KCM.SimpleKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you test and configure game controllers.")

    DeviceModel {
        id: deviceModel
    }

    property var joystickCount: deviceSelector.count
    property int selectedDevice: deviceSelector.currentIndex

    Kirigami.PlaceholderMessage {
        text: i18n("No gamepad found")
        anchors.centerIn: parent
        visible: joystickCount === 0
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }

    Kirigami.FormLayout {
        id: formLayout

        anchors.fill: parent

        visible: joystickCount > 0

        // Device
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcm_joystick", "Device:")
            id: deviceSelector

            Layouts.Layout.fillWidth: true
            model: deviceModel
            textRole: "name"
        }

        ColumnLayout {
            id: deviceProperties
            visible: root.selectedDevice > -1

            property var currentDevice: deviceModel.device(selectedDevice)

            QQC2.Label {
                text: i18n("Buttons: ") + deviceProperties.currentDevice.numButtons
            }

            QQC2.Label {
                text: i18n("Brand: ") + deviceProperties.currentDevice.brandName(deviceProperties.currentDevice.brand)
            }
        }
    }
}
