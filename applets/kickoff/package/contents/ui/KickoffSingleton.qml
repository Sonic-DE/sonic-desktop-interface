/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

pragma Singleton

import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PC2
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

// Using Item because it has a default property.
// Trying to create a default property for a QtObject seems to cause segfaults.
Item {
    id: root
    visible: false
    // Workaround for `plasmoid` context property not working in singletons
    property var plasmoid: null
    //BEGIN Models and Data Sources
    // These are set in FullRepresentation.qml because the `plasmoid` context property
    // doesn't work here and using `Plasmoid` from org.kde.plasma.plasmoid doesn't work either.
    // Even the `plasmoid` property defined above doesn't quite work,
    // but it works better than nothing for things like `plasmoid.expanded`
    property Kicker.RootModel rootModel
    property Kicker.RunnerModel runnerModel
    property Kicker.ComputerModel computerModel
    property Kicker.RecentUsageModel recentUsageModel
    property Kicker.RecentUsageModel frequentUsageModel

    property DelegateModel actionMenuDelegateModel: DelegateModel {
        id: actionMenuDelegateModel
        // Not a QQC1 MenuItem. It's actually a custom QQuickItem.
        // It uses a QAction for its action, but that's not available in QML,
        // so don't try to use the action property.
        delegate: PC2.MenuItem {
            required property var model
            required property int index
            text: model.display
            icon: model.decoration
        }
    }

    readonly property PlasmaCore.DataSource powerManagement: PlasmaCore.DataSource {
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
        // For some reason, these signal handlers need to be here for `data` to actually contain data.
        onSourceAdded: {
            disconnectSource(source);
            connectSource(source);
        }
        onSourceRemoved: {
            disconnectSource(source);
        }
    }
    //END

    //BEGIN UI elements
    // Set in FullRepresentation.qml
    property Item header: null

    // Set in Header.qml
    property PC3.TextField searchField: null

    // Set in FullRepresentation.qml, ApplicationPage.qml, PlacesPage.qml
    property Item sideBarView: null // is null when searching
    property Item contentAreaView: null // is searchView when searching

    // Set in FullRepresentation.qml, ApplicationPage.qml, PlacesPage.qml, KickoffListView.qml, KickoffGridView.qml
    property Item lastFocusedView: null

    // Set in NormalPage.qml
    property Item footer: null
    //END

    //BEGIN Reusable Objects
    readonly property PlasmaCore.Svg lineSvg: PlasmaCore.Svg {
        imagePath: "widgets/line"
    }
    readonly property PlasmaCore.Svg arrowsSvg: PlasmaCore.Svg {
        imagePath: "widgets/arrows"
    }
    //END

    //BEGIN State Properties
    // Set in Kickoff.qml
    property bool reverseVerticalLayout: false
    property bool inPanel: false
    property bool vertical: false
    //END

    //BEGIN Metrics
    readonly property PlasmaCore.FrameSvgItem backgroundMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: "dialogs/background"
    }

    readonly property PlasmaCore.FrameSvgItem listItemMetrics: PlasmaCore.FrameSvgItem {
        visible: false
        imagePath: "widgets/listitem"
        prefix: "normal"
    }

    readonly property real gridCellSize: gridDelegate.implicitHeight
    readonly property real listDelegateHeight: listDelegate.implicitHeight
    readonly property real listDelegateContentHeight: listDelegate.implicitContentHeight
    //END

    //BEGIN Private
    KickoffItemDelegate {
        id: gridDelegate
        visible: false
        enabled: false
        icon.width: PlasmaCore.Units.iconSizes.large
        icon.height: PlasmaCore.Units.iconSizes.large
        model: null
        index: -1
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
        model: null
        index: -1
        text: "asdf"
        decoration: "start-here-kde"
        description: "asdf"
        action: null
        indicator: null
    }
    //END
}
