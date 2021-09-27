/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

KickoffGridView {
    id: root
    delegate: DraggableItemDelegate {
        icon.width: PlasmaCore.Units.iconSizes.large
        icon.height: PlasmaCore.Units.iconSizes.large
        display: PC3.AbstractButton.TextUnderIcon
        width: view.cellWidth
        mouseArea.drag.axis: Drag.XAndYAxis
    }
    KickoffDropArea {
        z: -1
        parent: root
        anchors.fill: parent
        targetView: root.view
        scrollUpMargin: root.header.height * 2
        scrollDownMargin: root.footer.height * 2
    }
}
