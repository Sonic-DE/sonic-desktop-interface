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

import org.kde.plasma.gamecontroller.kcm

KCM.SimpleKCM {
    id: root

    required property var device

    ColumnLayout {
        anchors.fill: parent

        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.alignment: Qt.AlignTop

                QQC2.Label {
                    text: i18nc("@label Visual representation of an axis position", "Position:")
                    textFormat: Text.PlainText
                }

                QQC2.Label {
                    text: i18nc("@label Visual representation of left stick position", "Left Stick:")
                    textFormat: Text.plainText
                    visible: leftPosWidget.visible
                }

                PositionWidget {
                    id: leftPosWidget
                    index: GamepadButton.SDL_CONTROLLER_AXIS_LEFTX
                    visible: device.hasAxis(GamepadButton.SDL_CONTROLLER_AXIS_LEFTX)

                    device: root.device
                }

                QQC2.Label {
                    text: i18nc("@label Visual representation of right stick position", "Right Stick:")
                    textFormat: Text.plainText
                    visible: rightPosWidget.visible
                }

                PositionWidget {
                    id: rightPosWidget
                    index:  GamepadButton.SDL_CONTROLLER_AXIS_RIGHTX
                    visible: device.hasAxis(GamepadButton.SDL_CONTROLLER_AXIS_RIGHTX)

                    device: root.device
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 50 // Same space for the two columns

                QQC2.Label {
                    text: i18nc("@label Gamepad buttons", "Buttons:")
                    textFormat: Text.PlainText
                }

                Table {
                    model: ButtonModel {
                        device: root.device
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }

            ColumnLayout {
                spacing: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 50 // Same space for the two columns

                QQC2.Label {
                    text: i18nc("@label Gamepad axes (sticks)", "Axes:")
                    textFormat: Text.PlainText
                }

                Table {
                    model: AxesModel {
                        device: root.device
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }
    }
}
