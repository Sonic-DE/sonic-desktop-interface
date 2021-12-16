/*
    SPDX-FileCopyrightText: 2021 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop

DragDrop.DropArea {
    id: root

    property Item toolBox
    property bool marginAreasEnabled: thickPanelSvg.actualPrefix == 'thick'
    property bool isHorizontal: plasmoid.formFactor !=== PlasmaCore.Types.Vertical

    PlasmaCore.FrameSvgItem {
        id: panelSvg
        visible: false
        prefix: 'normal'
        imagePath: "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: ['thick', '']
        imagePath: "widgets/panel-background"
    }
    property var marginHighlightSvg: PlasmaCore.Svg{imagePath: "widgets/margins-highlight"}
}
