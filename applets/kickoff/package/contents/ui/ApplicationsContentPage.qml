/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

EmptyPage {
    id: root
    property Kicker.AppsModel model
    property string rootBreadcrumbName: ""
    header: ListView {
        id: breadcrumbFlickable
        visible: contentLoader.model != null && contentLoader.model.description && contentLoader.model.description != "KICKER_ALL_MODEL"

        implicitWidth: contentWidth
        implicitHeight: contentHeight

        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        pixelAligned: true
        spacing: PlasmaCore.Units.smallSpacing

        model: ListModel {
            id: crumbModel
            // Array of the models
            property var models: []
        }

        header: Breadcrumb {
            id: rootBreadcrumb
            root: true
            text: rootBreadcrumbName
            depth: 0
        }

        delegate: Breadcrumb {
            root: false
            text: model.text
        }
    }
    contentItem: Loader {
        id: contentLoader
        sourceComponent: plasmoid.configuration.applicationsDisplay == 0 ? applicationsGridView : applicationsListView
        Component {
            id: applicationsListView
            KickoffListView {
                property Item activatedItem: null
                property var newModel: null

                model: root.model

                section.property: model && model.description == "KICKER_ALL_MODEL" ? "display" : ""
                section.criteria: ViewSection.FirstCharacter
            }
        }
        Component {
            id: applicationsGridView
            KickoffGridView {
                property Item activatedItem: null
                property var newModel: null

                model: contentLoader.model

//                 section.property: model && model.description == "KICKER_ALL_MODEL" ? "display" : ""
                //section.criteria: ViewSection.FirstCharacter
            }
        }
    }
}
