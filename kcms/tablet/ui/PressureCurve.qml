/*
    SPDX-FileCopyrightText: Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm 1.0

Rectangle {
    id: root

    property CubicCurve curve: CubicCurve {}
    property bool fullyInitialized: false
    property int selectedControlPoint: 0

    signal controlPointsUpdated(string points)

    implicitWidth: 220
    implicitHeight: 220

    onWidthChanged: update()
    onHeightChanged: update()

    color: Kirigami.Theme.backgroundColor
    clip: true

    Keys.onDeletePressed: removePoint(selectedControlPoint)

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    function update(): void {
        updateLine();
        updateControlPoints();
    }

    function updateLine(): void {
        let newPoints = [];

        for (let i = 0; i < root.width; i++) {
            const x = i / root.width;
            const y = root.height - curve.value(x) * root.height;

            newPoints.push(Qt.point(i, y));
        }

        // Add two points at the bottom for the fill
        newPoints.push(Qt.point(root.width, root.height));
        newPoints.push(Qt.point(0, root.height));

        polyLine.path = newPoints;
    }

    function updateControlPoints(): void {
        controlRepeater.model = root.curve.points;

        if (fullyInitialized) {
            root.controlPointsUpdated(root.curve.toString());
        }
    }

    function removePoint(index: int): void {
        // Removing the first or last points is not allowed
        if (!isFirstPoint(index) && !isLastPoint(index)) {
            curve.removePoint(index);
            update();
        }
    }

    function isFirstPoint(index: int): bool {
        return index === 0;
    }

    function isLastPoint(index: int): bool {
        return index + 1 >= curve.points.length;
    }

    Component.onCompleted: {
        update();
        fullyInitialized = true;
    }

    // Vertical grid lines
    Repeater {
        id: verticalRepeater

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
                startY: 0

                PathLine {
                    id: line

                    x: index * (root.width / verticalRepeater.count)
                    y: root.height
                }
            }
        }
    }

    // Horizontal grid lines
    Repeater {
        id: horizontalRepeater

        model: 4

        delegate: Shape {
            anchors.fill: parent

            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Window

            ShapePath {
                strokeWidth: 1
                strokeColor: Kirigami.Theme.backgroundColor
                fillColor: "transparent"

                startX: 0
                startY: line.y

                PathLine {
                    id: line

                    x: root.width
                    y: index * (root.height / horizontalRepeater.count)
                }
            }
        }
    }

    Shape {
        anchors.fill: parent

        ShapePath {
            strokeWidth: 2
            strokeColor: root.activeFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            fillColor: Qt.alpha(strokeColor, 0.2)
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
            color: root.activeFocus && root.selectedControlPoint === index ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
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
                xAxis {
                    minimum: {
                        if (isFirstPoint(controlCircle.index)) {
                            return 0.0;
                        } else {
                            return root.curve.points[index - 1].x * root.width;
                        }
                    }

                    maximum: {
                        if (isLastPoint(controlCircle.index)) {
                            return root.width;
                        } else {
                            return root.curve.points[index + 1].x * root.width;
                        }
                    }

                    onActiveValueChanged: (delta)=> {
                        const point = root.curve.points[index];
                        const newDelta = delta / root.width;
                        const newPosX = point.x * root.width + delta;

                        if (newPosX > xAxis.minimum && newPosX < xAxis.maximum) {
                            point.x += newDelta;
                            root.curve.setPoint(index, point);
                            root.updateLine();
                        }
                    }
                }

                // QtQuick's origin is at the top-left, while the curve's coordinate space is in the bottom-left hence all the flips.
                yAxis {
                    // Don't allow changing the y-value of the first and last points, you should only be able to change the threshold
                    enabled: !isFirstPoint(index) && !isLastPoint(index)

                    maximum: {
                        if (isFirstPoint(controlCircle.index)) {
                            return 0.0;
                        } else {
                            return (1.0 - root.curve.points[index - 1].y) * root.height;
                        }
                    }

                    minimum: {
                        if (isLastPoint(controlCircle.index)) {
                            return 1.0 * root.height;
                        } else {
                            return (1.0 - root.curve.points[index + 1].y) * root.height;
                        }
                    }

                    onActiveValueChanged: (delta)=> {
                        const point = root.curve.points[index];
                        const newDelta = delta / root.height;
                        const newPosY = (1.0 - point.y) * root.height + delta;
                        
                        if (newPosY > yAxis.minimum && newPosY < yAxis.maximum) {
                            point.y += -newDelta; // we're flipping here because of the aforementioned coordinate space difference
                            root.curve.setPoint(index, point);
                            root.updateLine();
                        }
                    }
                }

                onActiveChanged: {
                    if (active) {
                        root.forceActiveFocus(Qt.MouseFocusReason);
                        root.selectedControlPoint = index;
                    } else {
                        // We want to only update the control points once they are done dragging,
                        // if we do it during a drag then QtQuick will keep updating the binding, and it cancels the existing drag.
                        root.updateControlPoints();
                    }
                }
            }

            TapHandler {
                enabled: root.activeFocus

                onTapped: root.selectedControlPoint = index
                onLongPressed: root.removePoint(index)
            }
        }
    }

    TapHandler {
        onTapped: (eventPoint, button) => {
            if (root.activeFocus) {
                // Only allow up to 4 points
                if (root.curve.points.length < 4) {
                    const point = Qt.point(eventPoint.position.x / root.width, 1.0 - (eventPoint.position.y / root.height));

                    root.curve.addPoint(point);
                    root.update();
                }
            } else {
                root.forceActiveFocus(Qt.MouseFocusReason);
            }
        }
    }
}
