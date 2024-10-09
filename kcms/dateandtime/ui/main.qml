/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Controls

import QtPositioning
import QtLocation
import QtCore

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3.0 as PC3

KCM.SimpleKCM {
    id: win

    property string hoveredTimeZone: ""
    property string selectedTimeZoneId: ""
    property string timezoneFileUrl: ""

    GeoJsonData {
        id: geoDatabase
        sourceUrl: win.timezoneFileUrl
    }

    ColumnLayout {

        spacing: Kirigami.Units.largeSpacing

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }

        MapView {
            id: view

            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 25

            map.plugin: Plugin {
                name: "osm"
                PluginParameter {
                    name: 'osm.mapping.offline.directory'
                    value: ':/kcm/kcm_timezone/offline_tiles'
                }
            }
            map.zoomLevel: 0
            map.minimumZoomLevel: 0
            map.maximumZoomLevel: 5
            map.center: QtPositioning.coordinate(3, 8)
            map.activeMapType: map.supportedMapTypes[0]

            property variant referenceSurface: QtLocation.ReferenceSurface.Map

            MapItemView {
                id: miv
                parent: view.map
                model: geoDatabase.model
                delegate: GeoJsonDelegate {}
            }
        }

        PC3.Label {
            text: "You have currently selected the <b>" + win.selectedTimezoneId + "</b> timezone."
            visible: win.selectedTimezoneId
            Layout.alignment: Qt.AlignHCenter
        }

        PC3.Label {
            text: "You are currently hovering the <b>" + win.hoveredTimeZone + "</b> timezone."
            visible: win.hoveredTimeZone
            Layout.alignment: Qt.AlignHCenter
        }

    }

    // Labels and buttons will go here

}
