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
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.16 as Kirigami

//import "code/tools.js" as Tools

PC3.ItemDelegate {
    id: root

    readonly property Item view: ListView.view || GridView.view
    readonly property bool textUnderIcon: display === PC3.AbstractButton.TextUnderIcon
    property var decoration: model.decoration
    property string description: model.description ? model.description : ""
    property bool viewScrolling: false
    property bool delayHover: false

    leftPadding: KickoffSingleton.listItemMetrics.margins.left
    rightPadding: KickoffSingleton.listItemMetrics.margins.right
    topPadding: KickoffSingleton.listItemMetrics.margins.top
    bottomPadding: KickoffSingleton.listItemMetrics.margins.bottom

    enabled: !model.disabled

    icon.width: PlasmaCore.Units.iconSizes.smallMedium
    icon.height: PlasmaCore.Units.iconSizes.smallMedium

    text: model.name ? model.name : model.display

    // Using an action so that it can be replaced.
    // using `model` () instead of `root.model` leads to errors about
    // `model` not having the trigger() function
    action: T.Action {
        onTriggered: {
            // if successfully triggered, close popup
            if(root.view.model.trigger(index, "", null)) {
                plasmoid.expanded = false
            }
        }
    }

    background: null
    contentItem: GridLayout {
        baselineOffset: label.y + label.baselineOffset
        columnSpacing: parent.spacing
        rowSpacing: parent.spacing
        flow: root.textUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
        PlasmaCore.IconItem {
            id: iconItem

            Layout.alignment: root.textUnderIcon ? Qt.AlignHCenter | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter
            implicitWidth: root.icon.width
            implicitHeight: root.icon.height

            animated: false
            usesPlasmaTheme: false

            source: root.decoration || root.icon.name || root.icon.source
        }
        PC3.Label {
            id: label

            Layout.alignment: root.textUnderIcon ? Qt.AlignHCenter | Qt.AlignTop : Qt.AlignLeft | Qt.AlignVCenter
            Layout.fillWidth: true
            Layout.preferredHeight: root.textUnderIcon && lineCount === 1 ? implicitHeight * 2 : implicitHeight
            Layout.rightMargin: root.indicator && root.indicator.visible ? root.spacing + root.indicator.width : 0
            text: root.text
            elide: Text.ElideRight
            horizontalAlignment: root.textUnderIcon ? Text.AlignHCenter : Text.AlignLeft
            verticalAlignment: root.textUnderIcon ? Text.AlignTop : Text.AlignVCenter
            maximumLineCount: 2
            wrapMode: Text.Wrap
        }
    }

    indicator: PlasmaCore.SvgItem {
        anchors.right: parent.contentItem.right
        anchors.verticalCenter: parent.contentItem.verticalCenter
        implicitWidth: naturalSize.width
        implicitHeight: naturalSize.height
        // using only model.hasChildren leads to "Unable to assign [undefined] to bool" errors
        visible: model.hasChildren == true

        svg: KickoffSingleton.arrowsSvg
        elementId: parent.mirrored ? "left-arrow" : "right-arrow"
    }

    PC3.ToolTip.text: {
        if (label.truncated && descriptionLabel.truncated) {
            return `${text} (${description})`
        } else if (descriptionLabel.truncated) {
            return description
        } else {
            return text
        }
    }
    PC3.ToolTip.visible: hovered && (label.truncated || descriptionLabel.truncated)
    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay

    PC3.Label {
        id: descriptionLabel
        parent: root
        anchors {
            left: root.contentItem.left
            right: root.contentItem.right
            baseline: root.contentItem.baseline
            leftMargin: root.textUnderIcon ? 0 : root.implicitContentWidth + root.spacing
            rightMargin: root.indicator && root.indicator.visible ? root.spacing + root.indicator.width : 0
            baselineOffset: root.textUnderIcon ? implicitHeight : 0
        }
        visible: text.length > 0 && text !== root.text && label.lineCount === 1
        enabled: false
        text: root.description
        elide: Text.ElideRight
        horizontalAlignment: root.textUnderIcon ? Text.AlignHCenter : Text.AlignRight
        verticalAlignment: root.textUnderIcon ? Text.AlignTop : Text.AlignVCenter
        maximumLineCount: 1
    }

    Timer {
        id: scrollHoverDelayTimer
        interval: 100
        onTriggered: {
            root.delayHover = false
            if (hovered) {
                view.currentIndex = index
            }
        }
    }

    onViewScrollingChanged: {
        root.delayHover = true
        scrollHoverDelayTimer.restart()
    }

    onHoveredChanged: {
        if (hovered && !root.delayHover) {
            view.currentIndex = index
        }
    }
    onClicked: {
        view.currentIndex = index
    }
}
