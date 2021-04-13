/*
 *    Copyright 2014  Sebastian Kügler <sebas@kde.org>
 *    SPDX-FileCopyrightText: (C) 2020 Carl Schwan <carl@carlschwan.eu>
 *    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtGraphicalEffects 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.kquickcontrolsaddons 2.0 as KQuickAddons

PlasmaExtras.PlasmoidHeading {
    id: root

    contentHeight: Math.max(searchField.implicitHeight, configureButton.implicitHeight)
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    property alias searchText: searchField.text
    property PC3.TextField searchField: searchField
    property Item configureButton: configureButton
    property Item avatar: avatar
    property real preferredNameAndIconWidth: 0

    KCoreAddons.KUser {
        id: kuser
    }

    spacing: KickoffSingleton.backgroundMetrics.margins.left

    RowLayout {
        id: nameAndIcon
        spacing: root.spacing
        anchors.left: parent.left
        height: parent.height
        width: root.preferredNameAndIconWidth

        PC3.RoundButton {
            id: avatar
            visible: KQuickAddons.KCMShell.authorize("kcm_users.desktop").length > 0
            hoverEnabled: true
            Layout.fillHeight: true
            Layout.minimumWidth: height
            Layout.maximumWidth: height
            // FIXME: Not using text with display because of RoundButton bugs in plasma-framework
            Accessible.name: i18n("Go to user settings")
            leftPadding: PlasmaCore.Units.devicePixelRatio
            rightPadding: PlasmaCore.Units.devicePixelRatio
            topPadding: PlasmaCore.Units.devicePixelRatio
            bottomPadding: PlasmaCore.Units.devicePixelRatio
            contentItem: Loader {
                sourceComponent: kuser.faceIconUrl ? imageComponent : icon
                Component {
                    id: imageComponent
                    Image {
                        id: imageItem
                        anchors.fill: avatar.contentItem
                        source: kuser.faceIconUrl
                        smooth: true
                        sourceSize.width: avatar.contentItem.width
                        sourceSize.height: avatar.contentItem.height
                        fillMode: Image.PreserveAspectCrop
                    }
                }
                Component {
                    id: iconComponent
                    PlasmaCore.IconItem {
                        id: iconItem
                        anchors.fill: avatar.contentItem
                        source: "user"
                    }
                }
                layer.enabled: kuser.faceIconUrl
                layer.effect: OpacityMask {
                    anchors.fill: avatar.contentItem
                    source: avatar.contentItem
                    maskSource: Rectangle {
                        visible: false
                        radius: height/2
                        width: avatar.contentItem.width
                        height: avatar.contentItem.height
                    }
                }
            }
            Rectangle {
                parent: avatar.background
                anchors.fill: avatar.background
                anchors.margins: -PlasmaCore.Units.devicePixelRatio
                z: 1
                radius: height/2
                color: "transparent"
                border.width: avatar.visualFocus ? PlasmaCore.Units.devicePixelRatio * 2 : 0
                border.color: PlasmaCore.Theme.buttonFocusColor
            }
            // Only used to keep the exact circular shape consistent with the image.
            // Without this, it looks significantly worse.
            background.layer.enabled: kuser.faceIconUrl
            background.layer.effect: OpacityMask {
                anchors.fill: avatar.background
                source: avatar.background
                maskSource: Rectangle {
                    visible: false
                    radius: height/2
                    width: avatar.background.width
                    height: avatar.background.height
                }
            }
            HoverHandler {
                id: hoverHandler
                cursorShape: Qt.PointingHandCursor
            }
            PC3.ToolTip.text: Accessible.name
            PC3.ToolTip.visible: hovered
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            onClicked: KQuickAddons.KCMShell.openSystemSettings("kcm_users")
        }

        MouseArea {
            id: nameAndInfoMouseArea
            hoverEnabled: true

            Layout.fillHeight: true
            Layout.fillWidth: true

            PlasmaExtras.Heading {
                id: nameLabel
                anchors.fill: parent
                opacity: parent.containsMouse ? 0 : 1

                level: 2
                text: kuser.fullName
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            PlasmaExtras.Heading {
                id: infoLabel
                anchors.fill: parent
                level: 5
                opacity: parent.containsMouse ? 1 : 0
                text: kuser.os !== "" ? `${kuser.loginName}@${kuser.host} (${kuser.os})` : `${kuser.loginName}@${kuser.host}`
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignLeft
                verticalAlignment: Text.AlignVCenter

                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            PC3.ToolTip.text: infoLabel.text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: infoLabel.truncated && containsMouse
        }
    }
    RowLayout {
        id: rowLayout
        spacing: root.spacing
        height: parent.height
        anchors {
            left: nameAndIcon.right
            right: parent.right
        }
        PlasmaCore.SvgItem {
            id: separator
            Layout.fillHeight: true
            implicitWidth: naturalSize.width
            implicitHeight: 0
            elementId: "vertical-line"
            svg: KickoffSingleton.lineSvg
        }

        PC3.TextField {
            id: searchField

            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            focus: true

            placeholderText: i18n("Search...")
            clearButtonShown: true

            Accessible.editable: true
            Accessible.searchEdit: true

            Connections {
                target: plasmoid
                function onExpandedChanged() {
                    if(!plasmoid.expanded) {
                        searchField.clear()
                    }
                }
            }
        }

        PC3.ToolButton {
            id: configureButton
            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            visible: plasmoid.action("configure").enabled
            icon.name: "configure"
            text: plasmoid.action("configure").text
            display: PC3.ToolButton.IconOnly

            PC3.ToolTip.text: text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: hovered

            onClicked: plasmoid.action("configure").trigger()
            /*Keys.onPressed: {
                // On tab focus on left pane (or search when searching)
                if (event.key == Qt.Key_Tab) {
                    navigationMethod.state = "keyboard"
                    // There's no left panel when we search
                    if (root.state == "Search") {
                        keyboardNavigation.state = "RightColumn"
                        root.currentContentView.forceActiveFocus()
                    } else if (mainTabGroup.state == "top") {
                        applicationButton.forceActiveFocus(Qt.TabFocusReason)
                    } else {
                        keyboardNavigation.state = "LeftColumn"
                        root.currentView.forceActiveFocus()
                    }
                    event.accepted = true;
                    return;
                }
            }*/
        }
        PC3.ToolButton {
            checkable: true
            checked: plasmoid.configuration.pin
            icon.name: "window-pin"
            text: i18n("Keep Open")
            display: PC3.ToolButton.IconOnly
            PC3.ToolTip.text: text
            PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
            PC3.ToolTip.visible: hovered
            onToggled: plasmoid.configuration.pin = checked
            Binding {
                target: plasmoid
                property: "hideOnWindowDeactivate"
                value: !plasmoid.configuration.pin
            }
        }
    }
}
