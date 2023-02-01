/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.15
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.13 as Kirigami
import org.kde.newstuff 1.91 as NewStuff
import org.kde.kcm 1.4 as KCM
import QtQuick.Controls 2.0 as QQC2
import QtQuick.Layouts 1.3 as Layouts
import org.kde.plasma.joystick.kcm 1.0

KCM.SimpleKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the splash screen theme.")

    DeviceModel {
        id: deviceModel
    }

    property var joystickCount: deviceSelector.count

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

        Repeater {
            model: deviceModel[deviceSelector.index].joydevice.numButtons

            QQC2.Label {
                required property int index

                text: index
            }
        }
    }
}
