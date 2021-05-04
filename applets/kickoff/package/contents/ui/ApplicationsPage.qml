/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

BasePage {
    id: root
    sideBarComponent: KickoffListView {
        id: sideBarView
        focus: true // needed for Loaders
        model: KickoffSingleton.rootModel
    }
    contentAreaComponent: VerticalStackView {
        id: stackView
        readonly property string preferredFavoritesViewObjectName: plasmoid.configuration.favoritesDisplay == 0 ? "favoritesGridViewComponent" : "favoritesListViewComponent"
        readonly property Component preferredFavoritesViewComponent: plasmoid.configuration.favoritesDisplay == 0 ? favoritesGridViewComponent : favoritesListViewComponent
        readonly property string preferredAppsViewObjectName: plasmoid.configuration.applicationsDisplay == 0 ? "applicationsGridView" : "applicationsListView"
        readonly property Component preferredAppsViewComponent: plasmoid.configuration.applicationsDisplay == 0 ? applicationsGridViewComponent : applicationsListViewComponent

        // TODO: find a better way to detect when a new model is null and keep the old model
        // instead of using 2 model properties
        property var testModel: KickoffSingleton.rootModel.modelForRow(root.sideBarItem.currentIndex)
        property var appsModel
        focus: true
        initialItem: preferredFavoritesViewComponent

        Component {
            id: favoritesListViewComponent
            KickoffListView {
                id: favoritesListView
                objectName: "favoritesListView"
                focus: true
                model: KickoffSingleton.rootModel.favoritesModel
            }
        }

        Component {
            id: favoritesGridViewComponent
            KickoffGridView {
                id: favoritesGridView
                objectName: "favoritesGridView"
                focus: true
                model: KickoffSingleton.rootModel.favoritesModel
            }
        }

        Component {
            id: applicationsListViewComponent
            KickoffListView {
                id: applicationsListView
                objectName: "applicationsListView"
                // use same width as applicationsGridView
                implicitWidth: KickoffSingleton.gridCellSize * 4 + leftPadding + rightPadding
                model: stackView.appsModel
                section.property: model && model.description == "KICKER_ALL_MODEL" ? "display" : ""
                section.criteria: ViewSection.FirstCharacter
            }
        }

        Component {
            id: applicationsGridViewComponent
            KickoffGridView {
                id: applicationsGridView
                objectName: "applicationsGridView"
                model: stackView.appsModel
            }
        }

        onPreferredFavoritesViewComponentChanged: {
            if (root.sideBarItem != null && root.sideBarItem.currentIndex === 0) {
                stackView.replace(stackView.preferredFavoritesViewComponent)
            }
        }
        onPreferredAppsViewComponentChanged: {
            if (root.sideBarItem != null && root.sideBarItem.currentIndex > 1) {
                stackView.replace(stackView.preferredAppsViewComponent)
            }
        }

        onTestModelChanged: {
            if (testModel != null) {
                appsModel = testModel
            }
        }

        Connections {
            target: root.sideBarItem
            function onCurrentIndexChanged() {
                if (root.sideBarItem.currentIndex === 0) {
                    stackView.replace(stackView.preferredFavoritesViewComponent)
                } else if (root.sideBarItem.currentIndex === 1 && stackView.currentItem.objectName !== "applicationsListView") {
                    // Always use list view for alphabetical apps view since grid view doesn't have sections
                    // TODO: maybe find a way to have a list view with grids in each section?
                    stackView.replace(applicationsListViewComponent)
                } else if (root.sideBarItem.currentIndex > 1 && stackView.currentItem.objectName !== stackView.preferredAppsViewObjectName) {
                    stackView.replace(stackView.preferredAppsViewComponent)
                }
            }
        }
    }
    // NormalPage doesn't get destroyed when deactivated, so the binding uses
    // StackView.status and visible. This way the bindings are reset when
    // NormalPage is Activated again.
    Binding {
        target: KickoffSingleton
        property: "sideBarView"
        value: root.sideBarItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
    Binding {
        target: KickoffSingleton
        property: "contentAreaView"
        value: root.contentAreaItem.currentItem // NOT just root.contentAreaItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
    Binding {
        target: KickoffSingleton
        property: "lastFocusedView"
        value: root.contentAreaItem.currentItem // NOT just root.contentAreaItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
}
