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

    Rectangle {
        id: outline
        radius: Kirigami.Units.smallSpacing
        color: Kirigami.Theme.backgroundColor

        anchors.fill: parent

        border {
            color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            width: 1

            Behavior on color {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
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

            height: Kirigami.Units.largeSpacing
            color: isSelected ? Kirigami.Theme.highlightColor : Kirigami.Theme.disabledTextColor
            smooth: true

            Behavior on color {
                PropertyAnimation {
                    duration: Kirigami.Units.longDuration
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

