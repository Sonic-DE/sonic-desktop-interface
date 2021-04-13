/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma Singleton

import QtQml.Models 2.15
import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: root
    // These are set in FullRepresentation.qml because the `plasmoid` context property
    // doesn't work here and using `Plasmoid` from org.kde.plasma.plasmoid doesn't work either.
    property Kicker.RootModel rootModel
    property Kicker.RunnerModel runnerModel
    property Kicker.ComputerModel computerModel
    property Kicker.RecentUsageModel recentUsageModel
    property Kicker.RecentUsageModel frequentUsageModel

    property DelegateModel appsModel: DelegateModel {
        model: rootModel && rootModel.hasChildren ? root.rootModel.modelForRow(0) : null
    }

    property PlasmaCore.DataSource powerManagement: PlasmaCore.DataSource {
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    property PlasmaCore.FrameSvgItem backgroundMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: "dialogs/background"
    }

    property PlasmaCore.FrameSvgItem listItemMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    // We don't need to make more than one of these
    property PlasmaCore.Svg lineSvg: PlasmaCore.Svg {
        imagePath: "widgets/line"
    }
    property PlasmaCore.Svg arrowsSvg: PlasmaCore.Svg {
        imagePath: "widgets/arrows"
    }

    // Set in Kickoff.qml
    property bool reverseVerticalLayout: false
    property bool inPanel: false
    property bool vertical: false

    property real gridCellSize: gridDelegate.implicitHeight
    property real listDelegateHeight: listDelegate.implicitHeight

    KickoffItemDelegate {
        id: gridDelegate
        visible: false
        enabled: false
        icon.width: PlasmaCore.Units.iconSizes.large
        icon.height: PlasmaCore.Units.iconSizes.large
        text: "asdf"
        decoration: "start-here-kde"
        description: "asdf"
        display: PC3.AbstractButton.TextUnderIcon
        width: implicitHeight
        action: null
        indicator: null
    }
    KickoffItemDelegate {
        id: listDelegate
        visible: false
        enabled: false
        icon.width: PlasmaCore.Units.iconSizes.smallMedium
        icon.height: PlasmaCore.Units.iconSizes.smallMedium
        text: "asdf"
        decoration: "start-here-kde"
        description: "asdf"
        action: null
        indicator: null
    }
}
