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
    title: i18n("Advanced Page")

    property var device

    ColumnLayout {
        anchors.fill: parent

        QQC2.Label {
            text: i18n("Rumble: ") + (root.device.hasRumble ? i18n("Yes") : i18n("No"))
        }

        QQC2.Label {
            text: i18n("Axes: ") + root.device.numAxes
        }

        QQC2.Label {
            text: i18n("Buttons: ") + root.device.numButtons
        }

        TableView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columnSpacing: 1
            rowSpacing: 1
            clip: true

            model: ButtonModel {
                device: root.device
            }

            delegate: Rectangle {
                color: "transparent"
                implicitWidth: label.implicitWidth + 8
                implicitHeight: label.implicitHeight + 4

                QQC2.Label {
                    id: label
                    text: display
                }
            }
        }

        TableView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            columnSpacing: 1
            rowSpacing: 1
            clip: true

            model: AxesModel {
                device: root.device
            }

            delegate: Rectangle {
                color: "transparent"
                implicitWidth: label.implicitWidth + 8
                implicitHeight: label.implicitHeight + 4

                QQC2.Label {
                    id: label
                    text: display
                }
            }
        }
    }
}