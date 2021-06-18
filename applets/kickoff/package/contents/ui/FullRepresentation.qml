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
import QtQuick.Templates 2.15 as T
import QtQuick.Layouts 1.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.private.kicker 0.1 as Kicker

PlasmaExtras.Representation {
    id: root

    collapseMarginsHint: true

    Layout.minimumWidth: implicitWidth
    Layout.minimumHeight: implicitHeight

    /* NOTE: Important things to know about keyboard input handling:
     *
     * - Key events are passed up to parent items until the end is reached.
     * Be mindful of this when using `Keys.forwardTo`.
     *
     * - Keys defaults to BeforeItem while KeyNavigation defaults to AfterItem.
     * 
     * - When Keys and KeyNavigation are using the same priority, it seems like
     * the one declared first in the QML file gets priority over the other.
     *
     * - Except for Keys.onPressed, all Keys.on*Pressed signals automatically
     * set `event.accepted = true`.
     *
     * - If you do `item.forceActiveFocus()` and `item` is a focus scope, the
     * children of `item` won't necessarily get focus. It seems like Keys is
     * better for forcing a specific thing to be focused while KeyNavigation is
     * better at passing focus down to children of the thing you want to focus
     * when dealing with focus scopes.
     * This is why a mixture of Keys and KeyNavigation are used.
     *
     * - KeyNavigation uses BacktabFocusReason (TabFocusReason if mirrored) for left,
     * TabFocusReason (BacktabFocusReason if mirrored) for right,
     * BacktabFocusReason for up and TabFocusReason for down.
     */

    header: Header {
        id: header
        focus: true
        Binding {
            target: KickoffSingleton
            property: "header"
            value: header
            restoreMode: Binding.RestoreBinding
        }
    }

    contentItem: VerticalStackView {
        id: contentItemStackView
        movementTransitionsEnabled: true
        // Not using a component to prevent it from being destroyed
        initialItem: NormalPage {
            id: normalPage
            objectName: "normalPage"
        }

        Component {
            id: searchViewComponent
            KickoffListView {
                id: searchView
                objectName: "searchView"
                // Forces the function be re-run every time runnerModel.count changes.
                // This is absolutely necessary to make the search view work reliably.
                model: runnerModel.count ? KickoffSingleton.runnerModel.modelForRow(0) : null
                activeFocusOnTab: true
                // always focus the first item in the header focus chain
                KeyNavigation.tab: root.header.nextItemInFocusChain()
                T.StackView.onActivated: {
                    KickoffSingleton.sideBarView = null
                    KickoffSingleton.contentAreaView = searchView
                    KickoffSingleton.lastFocusedView = searchView
                }
            }
        }

        Keys.priority: Keys.AfterItem
        // This is here rather than root because events are implicitly forwarded
        // to parent items. Don't want to send multiple events to searchField.
        Keys.forwardTo: KickoffSingleton.searchField

        Connections {
            target: root.header
            function onSearchTextChanged() {
                if (root.header.searchText.length === 0 && contentItemStackView.currentItem.objectName != "normalPage") {
                    contentItemStackView.reverseTransitions = true
                    contentItemStackView.replace(normalPage)
                } else if (root.header.searchText.length > 0 && contentItemStackView.currentItem.objectName != "searchView") {
                    contentItemStackView.reverseTransitions = false
                    contentItemStackView.replace(searchViewComponent)
                }
            }
        }

        Component.onCompleted: {
            // Break bindings after completing to keep a consistent implicit size.
            // It's unlikely that any more or any less width or height will be needed.
            implicitWidth = currentItem.implicitWidth + leftPadding + rightPadding
            implicitHeight = currentItem.implicitHeight + leftPadding + rightPadding
            header.preferredNameAndIconWidth = normalPage.preferredSideBarWidth
        }
    }

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
        systemApplications: plasmoid.configuration.systemApplications
        Component.onCompleted: {
            //systemApplications = plasmoid.configuration.systemApplications;
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

    //Connections {
        //target: computerModel

        //function onSystemApplicationsChanged() {
            //plasmoid.configuration.systemApplications = target.systemApplications;
        //}
    //}

    //Connections {
        //target: plasmoid.configuration

        //function onSystemApplicationsChanged() {
            //computerModel.systemApplications = plasmoid.configuration.systemApplications;
        //}
    //}
}
