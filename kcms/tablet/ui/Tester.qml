/*
    SPDX-FileCopyrightText: Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls as QQC2
import org.kde.layershell as LayerShell

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm

QQC2.ApplicationWindow {
    id: root

    required property var tabletEvents
    property bool toolDown: false

    width: 730

    minimumWidth: layout.implicitWidth + layout.anchors.leftMargin + layout.anchors.rightMargin
    minimumHeight: layout.implicitHeight + layout.anchors.topMargin + layout.anchors.bottomMargin

    title: i18ndc("kcm_tablet", "@title", "Tablet Tester")

    function insideDrawingSquare(x: real, y: real): bool {
        return x >= drawingSquare.x &&
            y >= drawingSquare.y &&
            x <= drawingSquare.x + drawingSquare.width &&
            y <= drawingSquare.y + drawingSquare.height;
    }

    function scrollLogToBottom(): void {
        logScrollView.QQC2.ScrollBar.vertical.position = 1.0 - logScrollView.QQC2.ScrollBar.vertical.size;
    }

    Connections {
        target: tabletEvents

        function onToolDown(x: real, y: real): void {
            if (insideDrawingSquare(x, y)) {
                root.toolDown = true;
                penPath.path = [];

                penLogText.append(i18nd("kcm_tablet", "Stylus press X=%1 Y=%2", x, y));
                scrollLogToBottom();
            }
        }

        function onToolUp(): void {
            root.toolDown = false;

            penLogText.append(i18nd("kcm_tablet", "Stylus release"));
            scrollLogToBottom();
        }

        function onToolMotion(x: real, y: real): void {
            if (insideDrawingSquare(x, y) && root.toolDown) {
                penPath.path.push(Qt.point(x, y));

                penLogText.append(i18nd("kcm_tablet", "Stylus move X=%1 Y=%2", x, y));
                scrollLogToBottom();
            }
        }
    }

    ColumnLayout {
        id: layout

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.largeSpacing

            right: parent.right
            rightMargin: Kirigami.Units.largeSpacing

            top: parent.top
            topMargin: Kirigami.Units.largeSpacing

            bottom: parent.bottom
            bottomMargin: Kirigami.Units.largeSpacing
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Rectangle {
                id: drawingSquare

                implicitWidth: 220
                implicitHeight: 220

                border {
                    color: Kirigami.Theme.textColor
                    width: 1
                }

                color: Qt.rgba(0.78, 0.78, 0.78, 1.0)

                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                Layout.preferredWidth: root.width / 3

                // Vertical lines
                Repeater {
                    model: 8
                    delegate: Shape {
                        anchors.fill: parent

                        ShapePath {
                            strokeWidth: 1
                            strokeColor: Qt.rgba(0.5, 0.5, 0.5, 1.0)
                            fillColor: "transparent"

                            startX: line.x

                            PathLine {
                                id: line

                                x: index * (drawingSquare.width / 8)
                                y: drawingSquare.height
                            }
                        }
                    }
                }

                // Vertical lines
                Repeater {
                    model: 8
                    delegate: Shape {
                        anchors.fill: parent

                        ShapePath {
                            strokeWidth: 1
                            strokeColor: Qt.rgba(0.5, 0.5, 0.5, 1.0)
                            fillColor: "transparent"

                            startY: line.y

                            PathLine {
                                id: line

                                x: drawingSquare.width
                                y: index * (drawingSquare.height / 8)
                            }
                        }
                    }
                }

                // Pen Path
                Shape {
                    anchors.fill: parent

                    ShapePath {
                        strokeWidth: 2
                        strokeColor: "blue"
                        fillColor: "transparent"
                        PathPolyline {
                            id: penPath
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredHeight: drawingSquare.implicitHeight

                QQC2.ScrollView {
                    id: logScrollView

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    QQC2.TextArea {
                        id: penLogText

                        text: i18nd("kcm_tablet", "## Legend:\n# X, Y - event coordinate\n")
                        readOnly: true
                        wrapMode: TextEdit.Wrap
                    }
                }

                QQC2.Button {
                    text: i18ndc("kcm_tablet", "Clear the event log", "Clear")
                    icon.name: "edit-clear"

                    onClicked: {
                        penLogText.clear();
                        penPath.path = [];
                    }

                    Layout.fillWidth: true
                }
            }
        }

        QQC2.Button {
            icon.name: "dialog-close"
            text: i18ndc("kcm_tablet", "Close the tablet tester", "Close")

            onClicked: root.close();

            Layout.alignment: Qt.AlignRight
        }
    }
}