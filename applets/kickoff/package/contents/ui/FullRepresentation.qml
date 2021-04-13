/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013 2014 David Edmundson <davidedmundson@kde.org>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>
    Copyright (C) 2021 by Noah Davis <noahadvs@gmail.com>

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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.private.kicker 0.1 as Kicker

EmptyPage {
    id: root

    leftPadding: -KickoffSingleton.backgroundMetrics.margins.left
    rightPadding: -KickoffSingleton.backgroundMetrics.margins.right
    bottomPadding: -KickoffSingleton.backgroundMetrics.margins.bottom

    Layout.minimumWidth: implicitWidth
    Layout.minimumHeight: implicitHeight

    header: Header {
        id: header
        focus: true
        KeyNavigation.tab: contentItemStackView.currentItem.contentItem
    }

    contentItem: VerticalStackView {
        id: contentItemStackView
        // Not using a component to prevent it from being destroyed
        initialItem: NormalView {
            id: normalView
            objectName: "normalView"
        }

        Component {
            id: searchView
            KickoffListView {
                objectName: "searchView"
                model: KickoffSingleton.runnerModel.modelForRow(0)
            }
        }

        Connections {
            target: root.header
            function onSearchTextChanged() {
                if (root.header.searchText.length === 0 && contentItemStackView.currentItem.objectName != "normalView") {
                    contentItemStackView.reverseTransitions = true
                    contentItemStackView.replace(normalView)
                } else if (root.header.searchText.length > 0 && contentItemStackView.currentItem.objectName != "searchView") {
                    contentItemStackView.reverseTransitions = false
                    contentItemStackView.replace(searchView)
                }
            }
        }

        Component.onCompleted: {
            // Break bindings after completing to keep a consistent implicit size.
            // It's unlikely that any more or any less width or height will be needed.
            //implicitWidth = currentItem.implicitWidth + leftPadding + rightPadding
            //implicitHeight = currentItem.implicitHeight + leftPadding + rightPadding
            header.preferredNameAndIconWidth = currentItem.preferredSideBarWidth
        }
    }

    Keys.forwardTo: root.header.searchField

    Kicker.DragHelper {
        id: dragHelper

        dragIconSize: PlasmaCore.Units.iconSizes.medium
        onDropped: root.parent.dragSource = null
    }

    Kicker.RootModel {
        id: rootModel

        autoPopulate: false

        appletInterface: plasmoid

        flat: true // have categories, but no subcategories
        sorted: plasmoid.configuration.alphaSort
        showSeparators: false
        showTopLevelItems: true

        showAllApps: true
        showAllAppsCategorized: false
        showRecentApps: false
        showRecentDocs: false
        showRecentContacts: false
        showPowerSession: false
        showFavoritesPlaceholder: true

        // Triggered earlier than Component.onCompleted
        Component.onCompleted: {
            favoritesModel.initForClient("org.kde.plasma.kickoff.favorites.instance-" + plasmoid.id)

            if (!plasmoid.configuration.favoritesPortedToKAstats) {
                favoritesModel.portOldFavorites(plasmoid.configuration.favorites);
                plasmoid.configuration.favoritesPortedToKAstats = true;
            }

            rootModel.refresh();
            KickoffSingleton.rootModel = Qt.binding(() => { return rootModel })
        }
    }

    Kicker.RunnerModel {
        id: runnerModel
        query: root.header.searchText
        appletInterface: plasmoid
        mergeResults: true
        favoritesModel: rootModel.favoritesModel
        Component.onCompleted: KickoffSingleton.runnerModel = Qt.binding(() => { return runnerModel })
    }

    Kicker.ComputerModel {
        id: computerModel

        appletInterface: plasmoid

        favoritesModel: rootModel.favoritesModel

        Component.onCompleted: {
            systemApplications = plasmoid.configuration.systemApplications;
            KickoffSingleton.computerModel = Qt.binding(() => { return computerModel })
        }
    }

    Kicker.RecentUsageModel {
        id: recentUsageModel
        favoritesModel: rootModel.favoritesModel
        Component.onCompleted: KickoffSingleton.recentUsageModel = Qt.binding(() => { return recentUsageModel })
    }

    Kicker.RecentUsageModel {
        id: frequentUsageModel
        favoritesModel: rootModel.favoritesModel
        ordering: 1 // Popular / Frequently Used
        Component.onCompleted: KickoffSingleton.frequentUsageModel = Qt.binding(() => { return frequentUsageModel })
    }
}
