/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QQC2
import org.kde.kirigami 2.6 as Kirigami
import org.kde.plasma.tablet.kcm 1.0
import org.kde.kcm 1.3

SimpleKCM {
    id: root

    ConfigModule.buttons: ConfigModule.Default | ConfigModule.Apply

    implicitWidth: Kirigami.Units.gridUnit * 38
    implicitHeight: Kirigami.Units.gridUnit * 35

    header: Kirigami.InlineMessage {
        Layout.fillWidth: true

        type: Kirigami.MessageType.Information
        visible: combo.count === 0
        text: i18n("No drawing tablets found.")
    }

    readonly property rect fittingSetting: Qt.rect(tabletItem.x/outputItem.width, tabletItem.y/outputItem.height, tabletItem.width/outputItem.width, tabletItem.height/outputItem.height)

    Kirigami.FormLayout {
        id: form
        enabled: combo.count > 0
        QQC2.ComboBox {
            id: combo
            Kirigami.FormData.label: i18nd("kcmtablet", "Device:")
            model: kcm.devicesModel

            onCurrentIndexChanged: {
                parent.device = kcm.devicesModel.deviceAt(combo.currentIndex)
            }
        }

        property QtObject device: null

        QQC2.ComboBox {
            id: outputsCombo
            Kirigami.FormData.label: i18nd("kcmtablet", "Target display:")
            model: OutputsModel {
                id: outputsModel
            }
            enabled: count > 2 //It's only interesting when there's more than 1 screen
            currentIndex: outputsModel.rowForOutputName(parent.device.outputName)
            textRole: "display"
            onActivated: {
                parent.device.outputName = outputsModel.outputNameAt(currentIndex)
            }
        }
        QQC2.ComboBox {
            Kirigami.FormData.label: i18nd("kcmtablet", "Orientation:")
            model: OrientationsModel {
                id: orientationsModel
            }
            enabled: parent.device && parent.device.supportsOrientation
            currentIndex: orientationsModel.rowForOrientation(parent.device.orientation)
            textRole: "display"
            onActivated: {
                parent.device.orientation = orientationsModel.orientationAt(currentIndex)
            }
        }
        QQC2.CheckBox {
            Kirigami.FormData.label: i18nd("kcmtablet", "Left-handed mode:")
            enabled: parent.device && parent.device.supportsLeftHanded
            checked: parent.device && parent.device.leftHanded
            onCheckedChanged: {
                parent.device.leftHanded = checked
            }
        }
        QQC2.ComboBox {
            id: mappingCombo
            Layout.fillWidth: true
            Kirigami.FormData.label: i18nd("kcmtablet", "Mapping:")
            model: OutputsFittingModel {}
        }
        QQC2.CheckBox {
            id: keepAspectRatio
            Kirigami.FormData.label: i18nd("kcmtablet", "Mapping keeps aspect ratio:")
            visible: mappingCombo.currentIndex >= 2
            checked: true
        }

        // Display fit demo
        Item {
            id: views
            Layout.fillWidth: true
            Layout.preferredHeight: width * 0.7
            Rectangle {
                id: outputItem
                color: Kirigami.Theme.positiveTextColor
                opacity: 0.7
                readonly property var outputPhysicalSize: outputsCombo.model.data(outputsCombo.model.index(outputsCombo.currentIndex, 0), Qt.UserRole + 1)
                readonly property var outputSize: outputsCombo.model.data(outputsCombo.model.index(outputsCombo.currentIndex, 0), Qt.UserRole + 2)
                readonly property real aspectRatio: outputPhysicalSize.width / outputPhysicalSize.height
                width: parent.width * 0.7
                height: width / aspectRatio
            }

            Rectangle {
                id: tabletItem
                color: Kirigami.Theme.negativeTextColor
                opacity: 0.5
                readonly property real aspectRatio: mappingCombo.currentIndex == 0 ? outputItem.aspectRatio : form.device.size.width / form.device.size.height
                width: tabletSizeHandle.x
                height: tabletSizeHandle.y

                QQC2.Label {
                    anchors.centerIn: parent
                    text: i18n("%1,%2 - %3x%4", Math.floor(fittingSetting.x * outputItem.outputSize.width)
                                              , Math.floor(fittingSetting.y * outputItem.outputSize.height)
                                              , Math.floor(fittingSetting.width * outputItem.outputSize.width)
                                              , Math.floor(fittingSetting.height * outputItem.outputSize.height))
                }

                DragHandler {
                    acceptedButtons: Qt.LeftButton
                    cursorShape: Qt.ClosedHandCursor
                    target: parent
                    enabled: mappingCombo.currentIndex >= 2

                    xAxis.minimum: 0
                    xAxis.maximum: outputItem.width - tabletItem.width

                    yAxis.minimum: 0
                    yAxis.maximum: outputItem.height - tabletItem.height

                }

                QQC2.Button {
                    id: tabletSizeHandle
                    x: outputItem.width
                    y: outputItem.width / parent.aspectRatio
                    visible: mappingCombo.currentIndex >= 2

                    DragHandler {
                        acceptedButtons: Qt.LeftButton
                        cursorShape: Qt.SizeFDiagCursor
                        target: parent

                        xAxis.minimum: 10
                        xAxis.maximum: outputItem.width - tabletItem.x

                        yAxis.minimum: keepAspectRatio.checked ? (tabletItem.width / tabletItem.aspectRatio) : 10
                        yAxis.maximum: keepAspectRatio.checked ? (tabletItem.width / tabletItem.aspectRatio) : outputItem.height - tabletItem.y
                    }
                }
            }
        }
    }
}
