/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.12
import Qt.labs.platform 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.components 2.0

KeyboardLayoutSwitcher {
    id: root

    signal layoutSelected(int layout)

    activeFocusOnTab: true
    hoverEnabled: true

    Plasmoid.onActivated: keyboardLayout.switchToNextLayout();
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus

    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            Plasmoid.activated();
            break;
        }
    }
    Accessible.name: Plasmoid.title
    Accessible.description: i18nc("@info:tooltip", "Current keyboard layout is %1", layoutNames.longName)
    Accessible.role: Accessible.Button

    function iconURL(name) {
        return StandardPaths.locate(StandardPaths.GenericDataLocation,
                        "kf5/locale/countries/" + name + "/flag.png")
    }

    function actionTriggered(selectedLayout) {
        layoutSelected(selectedLayout)
    }

    onLayoutSelected: (layout) => {
        keyboardLayout.layout = layout;
    }

    Connections {
        target: keyboardLayout

        function onLayoutsListChanged() {
            plasmoid.clearActions()

            keyboardLayout.layoutsList.forEach(
                        function(layout, index) {
                            plasmoid.setAction(
                                        index,
                                        layout.longName,
                                        iconURL(layout.shortName).toString().substring(7) // remove file:// scheme
                                        )
                        }
                        )
        }
    }

    Component {
        id: iconComponent

        PlasmaCore.IconItem {
            active: containsMouse
            source: iconURL(layoutNames.shortName)
        }
    }

    Component {
        id: labelComponent

        PlasmaComponents3.Label {
            font.pointSize: height
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            text: layoutNames.displayName || layoutNames.shortName
        }
    }

    Loader {
        anchors.fill: parent
        sourceComponent: plasmoid.configuration.showFlag && source ? iconComponent : labelComponent
    }

    PlasmaCore.ToolTipArea {
        id: toolTip
        anchors.fill: parent
        mainText: Plasmoid.title
        subText: layoutNames.longName
    }
}
