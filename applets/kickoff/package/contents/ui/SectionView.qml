/*
    SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.19 as Kirigami

KickoffGridView {
    id: root

    signal hideSectionViewRequested(int index)

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
}
