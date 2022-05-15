/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.components 3.0 as PC3

KickoffGridView {
    id: root

    signal hideSectionViewRequested(int index)

    property string currentSection

    delegate: PC3.AbstractButton {
        id: sectionButton

        width: view.cellWidth
        height: view.cellHeight

        hoverEnabled: true
        onHoveredChanged: if (hovered) {
            root.view.currentIndex = index;
        }

        readonly property var dataArray: modelData.split(':')
        readonly property string sectionName: dataArray[0]
        readonly property int firstItemIndex: parseInt(dataArray[1])

        contentItem: PC3.Label {
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            maximumLineCount: 1
            elide: Text.ElideRight
            font.pixelSize: KickoffSingleton.listDelegateContentHeight
            text: sectionName
        }

        onClicked: root.hideSectionViewRequested(firstItemIndex)
    }

    Component.onCompleted: {
        for (let i = 0; i < model.length; i++) {
            if (model[i].split(':')[0] === root.currentSection) {
                view.currentIndex = i;
                return;
            }
        }
    }
}
