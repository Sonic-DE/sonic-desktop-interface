/*
    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2012 Dan Vratil <dvratil@redhat.com>

    SPDX-License-Identifier: GPL-2.0-or-later

    Adapted from KScreen
*/
import QtQuick 2.12
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as Controls
import QtGraphicalEffects 1.0
import org.kde.kirigami 2.4 as Kirigami

Item {
    id: output

    property bool isSelected: true

    onIsSelectedChanged: {
        if (isSelected) {
            z = 89;
        } else {
            z = 0;
        }
    }

    SystemPalette {
        id: palette
    }

    Rectangle {
        id: outline
        radius: 4
        color: palette.window

        anchors.fill: parent

        border {
            color: isSelected ? palette.highlight : palette.shadow
            width: 1

            Behavior on color {
                PropertyAnimation {
                    duration: 150
                }
            }
        }
    }

    Item {
        id: orientationPanelContainer

        anchors.fill: output
        visible: false

        Rectangle {
            id: orientationPanel
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }

            height: 10
            color: isSelected ? palette.highlight : palette.shadow
            smooth: true

            Behavior on color {
                PropertyAnimation {
                    duration: 150
                }
            }
        }
    }

    OpacityMask {
        anchors.fill: orientationPanelContainer
        source: orientationPanelContainer
        maskSource: outline
    }
}

