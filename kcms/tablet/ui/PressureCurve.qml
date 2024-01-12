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
import org.kde.plasma.tablet.kcm 1.0

Rectangle {
    id: root

    signal controlPointsUpdated(var points)

    implicitWidth: 220
    implicitHeight: 220

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    border {
        color: Kirigami.Theme.disabledTextColor
        width: 1
    }

    clip: true
    color: Kirigami.Theme.backgroundColor

    property CubicCurve curve: CubicCurve {}
    property bool fullyInitialized: false

    function updatePolyLine() {
        let newPoints = [];

        for (let i = 0; i < root.width; i++) {
            const normalizedX = i / root.width;
            const curY = root.height - curve.value(normalizedX) * root.height;

            newPoints.push(Qt.point(i, curY));
        }

        newPoints.push(Qt.point(root.width + 1, root.height + 1));
        newPoints.push(Qt.point(0, root.height + 1));
        polyLine.path = newPoints;
    }

    function updateControlPoints() {
        controlRepeater.model = root.curve.points;

        console.info("UPDATING!!! " + fullyInitialized);

        if (fullyInitialized) {
            root.controlPointsUpdated(root.curve.toString());
        }
    }

    Component.onCompleted: {
        updatePolyLine();
        updateControlPoints();
        fullyInitialized = true;
    }

    // Vertical lines
    Repeater {
        model: 4
        delegate: Shape {
            anchors.fill: parent

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window

            ShapePath {
                strokeWidth: 1
                strokeColor: Kirigami.Theme.backgroundColor
                fillColor: "transparent"

                startX: line.x
                startY: 2

                PathLine {
                    id: line

                    x: index * (root.width / 4)
                    y: root.height - 2
                }
            }
        }
    }

    // Vertical lines
    Repeater {
        model: 4
        delegate: Shape {
            anchors.fill: parent

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window

            ShapePath {
                strokeWidth: 1
                strokeColor: Kirigami.Theme.backgroundColor
                fillColor: "transparent"

                startX: 2
                startY: line.y

                PathLine {
                    id: line

                    x: root.width - 2
                    y: index * (root.height / 4)
                }
            }
        }
    }

    Shape {
        anchors.fill: parent

        ShapePath {
            strokeWidth: 2
            strokeColor: Kirigami.Theme.textColor
            fillColor: Qt.alpha(Kirigami.Theme.textColor, 0.2)
            simplify: true

            PathPolyline {
                id: polyLine
            }
        }
    }

    component ControlCircle: Rectangle {
        required property real controlX
        required property real controlY

        width: 15
        height: width

        border {
            width: 2
            color: Kirigami.Theme.textColor
        }

        radius: width
        color: "transparent"

        x: (root.width * controlX) - (width / 2)
        y: (root.height * controlY) - (height / 2)
    }

    Repeater {
        id: controlRepeater

        delegate: ControlCircle {
            id: controlCircle

            required property int index

            controlX: root.curve.points[index].x
            controlY: 1.0 - root.curve.points[index].y

            DragHandler {
                persistentTranslation.x: controlCircle.width / 2
                persistentTranslation.y: controlCircle.height / 2

                xAxis.minimum: {
                    if (controlCircle.index === 0) {
                        return 0.0;
                    } else {
                        return root.curve.points[index - 1].x * root.width;
                    }
                }
                xAxis.maximum: {
                    if (controlCircle.index + 1 === root.curve.points.length) {
                        return 1.0 * root.width;
                    } else {
                        return root.curve.points[index + 1].x * root.width;
                    }
                }
                xAxis.onActiveValueChanged: (delta)=> {
                    const oldPoint = root.curve.points[index];
                    const newDelta = delta / root.width;
                    if ((oldPoint.x * root.width + delta) > xAxis.minimum && (oldPoint.x * root.width + delta) < xAxis.maximum) {
                        oldPoint.x += newDelta;
                        root.curve.setPoint(index, oldPoint);
                        root.updatePolyLine();
                    }
                }

                // QtQuick's origin is at the top-left, while the curve's coordinate space is in the bottom-left hence all the flips.
                yAxis.enabled: index !== 0 && index + 1 !== root.curve.points.length
                yAxis.maximum: {
                    if (controlCircle.index === 0) {
                        return 0.0;//1.0 * root.height;
                    } else {
                        return (1.0 - root.curve.points[index - 1].y) * root.height;
                    }
                }
                yAxis.minimum: {
                    if (controlCircle.index + 1 === root.curve.points.length) {
                        return 1.0 * root.height;
                    } else {
                        return (1.0 - root.curve.points[index + 1].y) * root.height;
                    }
                }
                yAxis.onActiveValueChanged: (delta)=> {
                    const oldPoint = root.curve.points[index];
                    const newDelta = delta / root.height;
                    if (((1.0 - oldPoint.y) * root.height + delta) > yAxis.minimum && ((1.0 - oldPoint.y) * root.height + delta) < yAxis.maximum) {
                        oldPoint.y += -newDelta;
                        root.curve.setPoint(index, oldPoint);
                        root.updatePolyLine();
                    }
                }
                onActiveChanged: {
                    if (!active) {
                        root.updateControlPoints();
                    }
                }
            }
        }
    }

    TapHandler {
        onTapped: (eventPoint, button) => {
            // Only allow up to 4 points
            if (root.curve.points.length < 4) {
                const point = Qt.point(eventPoint.position.x / root.width, 1.0 - (eventPoint.position.y / root.height));

                root.curve.addPoint(point);
                updatePolyLine();
                updateControlPoints();
            }
        }
    }
}
