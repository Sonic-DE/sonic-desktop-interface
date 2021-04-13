/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
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
import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

FocusScope {
    id: root
    property real preferredSideBarWidth: 0

    property alias sideBarComponent: sideBarLoader.sourceComponent
    property alias sideBarItem: sideBarLoader.item
    property alias contentAreaComponent: contentAreaLoader.sourceComponent
    property alias contentAreaItem: contentAreaLoader.item

    readonly property real implicitSideBarWidth: sideBarLoader.implicitWidth

    implicitWidth: preferredSideBarWidth + separator.implicitWidth + contentAreaLoader.implicitWidth
    implicitHeight: Math.max(sideBarLoader.implicitHeight, contentAreaLoader.implicitHeight)

    Loader {
        id: sideBarLoader
        width: root.preferredSideBarWidth || implicitWidth
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        // Loaders act as FocusScopes, so this should work fine
        //KeyNavigation.left: !mirrored ? null : contentAreaLoader
        //KeyNavigation.right: mirrored ? null : contentAreaLoader
    }
    PlasmaCore.SvgItem {
        id: separator
        anchors {
            left: sideBarLoader.right
            top: parent.top
            bottom: parent.bottom
        }
        implicitWidth: naturalSize.width
        implicitHeight: implicitWidth
        elementId: "vertical-line"
        svg: KickoffSingleton.lineSvg
    }
    Loader {
        id: contentAreaLoader
        anchors {
            left: separator.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        focus: true
//         KeyNavigation.left: !mirrored ? sideBarLoader : null
        //KeyNavigation.right: mirrored ? sideBarLoader : null
    }
}
