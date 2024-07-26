import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

Item {
    id: outputAreaView
    
    required property var device

    function resetOutputArea(mode): void {
        var outputArea;
        if (mode === 0) {
            outputArea = Qt.rect(0,0, 1,1);
        } else {
            outputArea = Qt.rect(0, 0, 1, outputItem.aspectRatio / outputAreaItem.aspectRatio);
        }

        if (mode === 0) {
            // If fit to screen, use the whole size of the display
            outputAreaItem.x = 0
            outputAreaItem.y = 0
            outputAreaItem.width = Qt.binding(() => outputItem.outputWidth);
            outputAreaItem.height = Qt.binding(() => outputItem.outputHeight);
        } else {
            outputAreaItem.x = Qt.binding(() => outputArea.x * outputItem.outputWidth);
            outputAreaItem.y = Qt.binding(() => outputArea.y * outputItem.outputHeight);
            outputAreaItem.width = Qt.binding(() => tabletSizeHandle.x);
            outputAreaItem.height = Qt.binding(() => tabletSizeHandle.y);
            tabletSizeHandle.x = Qt.binding(() => outputArea.width * outputItem.outputWidth);
            tabletSizeHandle.y = Qt.binding(() => outputArea.height * outputItem.outputHeight);
        }
    }

    readonly property rect outputAreaSetting: Qt.rect(outputAreaItem.x/outputItem.width, outputAreaItem.y/outputItem.height,
        outputAreaItem.width/outputItem.width, outputAreaItem.height/outputItem.height)

    property bool changed: false

    onOutputAreaSettingChanged: {
        if (outputAreaView.device && changed) {
            outputAreaView.device.outputArea = outputAreaSetting
        }
    }

    enabled: outputAreaView.device
    implicitHeight: outputItem.height + tabletItem.height + Kirigami.Units.largeSpacing

    Output {
        id: outputItem

        // Resolution of the selected display
        screenSize: outputsModel.data(outputsModel.index(outputsCombo.currentIndex, 0), Qt.UserRole + 2)

        width: parent.width
        height: implicitHeight

        Rectangle {
            id: outputAreaItem

            color: Kirigami.Theme.activeBackgroundColor
            opacity: 0.8
            readonly property real aspectRatio: outputAreaCombo.currentIndex === 0 ? outputItem.aspectRatio : outputAreaView.device.size.width / outputAreaView.device.size.height
            width: tabletSizeHandle.x
            height: tabletSizeHandle.y

            ColumnLayout {
                anchors.centerIn: parent
                visible: outputAreaCombo.currentIndex >= 2

                Kirigami.Icon {
                    source: "transform-move-symbolic"

                    Layout.alignment: Qt.AlignHCenter
                }

                QQC2.Label {
                    text: i18ndc("kcm_tablet", "tablet area position - size", "%1,%2 - %3×%4", String(Math.floor(outputAreaView.outputAreaSetting.x * outputItem.screenSize.width))
                        , String(Math.floor(outputAreaView.outputAreaSetting.y * outputItem.screenSize.height))
                        , String(Math.floor(outputAreaView.outputAreaSetting.width * outputItem.screenSize.width))
                        , String(Math.floor(outputAreaView.outputAreaSetting.height * outputItem.screenSize.height)))
                    textFormat: Text.PlainText

                    Layout.fillWidth: true
                }
            }

            border {
                width: 1
                color: Kirigami.Theme.highlightColor
            }

            DragHandler {
                cursorShape: Qt.ClosedHandCursor
                target: parent
                enabled: outputAreaCombo.currentIndex >= 2
                onActiveChanged: { outputAreaView.changed = true }

                xAxis.minimum: 0
                xAxis.maximum: outputItem.outputWidth - outputAreaItem.width

                yAxis.minimum: 0
                yAxis.maximum: outputItem.outputHeight - outputAreaItem.height
            }

            TapHandler {
                gesturePolicy: TapHandler.WithinBounds
            }

            QQC2.Button {
                id: tabletSizeHandle
                x: outputItem.width
                y: outputItem.width / parent.aspectRatio
                visible: outputAreaCombo.currentIndex >= 2
                icon.name: "transform-scale-symbolic"
                display: QQC2.AbstractButton.IconOnly
                text: i18nd("kcm_tablet", "Resize the tablet area")
                QQC2.ToolTip {
                    text: tabletSizeHandle.text
                    visible: parent.hovered
                    delay: Kirigami.Units.toolTipDelay
                }

                DragHandler {
                    cursorShape: Qt.SizeFDiagCursor
                    target: parent
                    onActiveChanged: { outputAreaView.changed = true }

                    xAxis.minimum: 10
                    xAxis.maximum: outputItem.outputWidth

                    yAxis.minimum: keepAspectRatio.checked ? (outputAreaItem.outputWidth / outputAreaItem.aspectRatio) : 10
                    yAxis.maximum: keepAspectRatio.checked ? (outputAreaItem.outputWidth / outputAreaItem.aspectRatio) : outputItem.outputHeight
                }
            }
        }
    }

    Tablet {
        id: tabletItem

        anchors {
            top: outputItem.bottom
            topMargin: Kirigami.Units.largeSpacing
            horizontalCenter: parent.horizontalCenter
        }

        readonly property size outputSize: outputAreaView.device.size
        readonly property real aspectRatio: outputSize.width / outputSize.height

        property var mapped: mapToItem(outputAreaView, tabletItem.internalRect.x, tabletItem.internalRect.y)
        property var mappedSize: mapToItem(outputAreaView, tabletItem.internalRect.width, tabletItem.internalRect.height)

        outputWidth: parent.width * 0.7
        outputHeight: width / aspectRatio
    }

    Shape {
        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.5)

            startX: outputAreaItem.x + outputAreaItem.parent.x
            startY: outputAreaItem.y + outputAreaItem.parent.y

            PathLine { x: tabletItem.x + tabletItem.mapped.x; y: tabletItem.mapped.y + tabletItem.y }
        }

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.5)

            startX: outputAreaItem.x + outputAreaItem.parent.x + outputAreaItem.width
            startY: outputAreaItem.y + outputAreaItem.parent.y

            PathLine { x: tabletItem.mapped.x + tabletItem.mappedSize.x; y: tabletItem.mapped.y + tabletItem.y }
        }

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.5)

            startX: outputAreaItem.x + outputAreaItem.parent.x
            startY: outputAreaItem.y + outputAreaItem.parent.y + outputAreaItem.height

            PathLine { x: tabletItem.x + tabletItem.mapped.x; y: tabletItem.mapped.y + tabletItem.mappedSize.y}
        }

        ShapePath {
            strokeWidth: 1
            strokeColor: Qt.alpha(Kirigami.Theme.highlightColor, 0.5)

            startX: outputAreaItem.x + outputAreaItem.parent.x + outputAreaItem.width
            startY: outputAreaItem.y + outputAreaItem.parent.y + outputAreaItem.height

            PathLine { x: tabletItem.mapped.x + tabletItem.mappedSize.x; y: tabletItem.mapped.y  + tabletItem.mappedSize.y}
        }
    }
}
