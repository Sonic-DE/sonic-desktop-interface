/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2015  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: kickoff

    Plasmoid.switchWidth: plasmoid.fullRepresentationItem ? plasmoid.fullRepresentationItem.Layout.minimumWidth : -1
    Plasmoid.switchHeight: plasmoid.fullRepresentationItem ? plasmoid.fullRepresentationItem.Layout.minimumHeight : -1

    Plasmoid.preferredRepresentation: plasmoid.compactRepresentation

    Plasmoid.fullRepresentation: FullRepresentation { id: fullRepresentation }

    Plasmoid.icon: plasmoid.configuration.icon

    Plasmoid.compactRepresentation: MouseArea {
        id: compactRoot

        implicitWidth: PlasmaCore.Units.iconSizeHints.panel
        implicitHeight: PlasmaCore.Units.iconSizeHints.panel

        Layout.minimumWidth: {
            if (!KickoffSingleton.inPanel) {
                return PlasmaCore.Units.iconSizes.small
            }

            if (KickoffSingleton.vertical) {
                return -1;
            } else {
                return Math.min(PlasmaCore.Units.iconSizeHints.panel, parent.height) * buttonIcon.aspectRatio;
            }
        }

        Layout.minimumHeight: {
            if (!KickoffSingleton.inPanel) {
                return PlasmaCore.Units.iconSizes.small
            }

            if (KickoffSingleton.vertical) {
                return Math.min(PlasmaCore.Units.iconSizeHints.panel, parent.width) * buttonIcon.aspectRatio;
            } else {
                return -1;
            }
        }

        Layout.maximumWidth: {
            if (!KickoffSingleton.inPanel) {
                return -1;
            }

            if (KickoffSingleton.vertical) {
                return PlasmaCore.Units.iconSizeHints.panel;
            } else {
                return Math.min(PlasmaCore.Units.iconSizeHints.panel, parent.height) * buttonIcon.aspectRatio;
            }
        }

        Layout.maximumHeight: {
            if (!KickoffSingleton.inPanel) {
                return -1;
            }

            if (KickoffSingleton.vertical) {
                return Math.min(PlasmaCore.Units.iconSizeHints.panel, parent.width) * buttonIcon.aspectRatio;
            } else {
                return PlasmaCore.Units.iconSizeHints.panel;
            }
        }

        hoverEnabled: true
        // For some reason, onClicked can cause the plasmoid to expand after
        // releasing sometimes in plasmoidviewer.
        // plasmashell doesn't seem to have this issue.
        onClicked: plasmoid.expanded = !plasmoid.expanded

        DropArea {
            id: compactDragArea
            anchors.fill: parent
        }

        Timer {
            id: expandOnDragTimer
            // this is an interaction and not an animation, so we want it as a constant
            interval: 250
            running: compactDragArea.containsDrag
            onTriggered: plasmoid.expanded = true
        }

        PlasmaCore.IconItem {
            id: buttonIcon

            readonly property double aspectRatio: (KickoffSingleton.vertical ? implicitHeight / implicitWidth
                : implicitWidth / implicitHeight)

            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse || compactDragArea.containsDrag
            smooth: true
            roundToIconSize: aspectRatio === 1
        }
    }

    property Item dragSource: null

    Kicker.ProcessRunner {
        id: processRunner;
    }

    Binding {
        target: KickoffSingleton
        property: "reverseVerticalLayout"
        value: false
    }
    Binding {
        target: KickoffSingleton
        property: "inPanel"
        value: plasmoid.location === PlasmaCore.Types.TopEdge
            || plasmoid.location === PlasmaCore.Types.RightEdge
            || plasmoid.location === PlasmaCore.Types.BottomEdge
            || plasmoid.location === PlasmaCore.Types.LeftEdge
    }
    Binding {
        target: KickoffSingleton
        property: "vertical"
        value: plasmoid.formFactor === PlasmaCore.Types.Vertical
    }

    function action_menuedit() {
        processRunner.runMenuEditor();
    }

    Component.onCompleted: {
        if (plasmoid.hasOwnProperty("activationTogglesExpanded")) {
            plasmoid.activationTogglesExpanded = true
        }
        if (plasmoid.immutability !== PlasmaCore.Types.SystemImmutable) {
            plasmoid.setAction("menuedit", i18n("Edit Applications..."), "kmenuedit");
        }
    }
} // root
