/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami 2 as Kirigami

import org.kde.plasma.private.kicker 0.1 as Kicker

import "code/tools.js" as Tools

Window {
    id: dashboardRepresentation

    color: Qt.rgba(0, 0, 0, 0.737)
    flags: Qt.FramelessWindowHint

    Keys.onEscapePressed: {
        if (searching) {
            searchField.clear();
        } else {
            Plasmoid.expanded = false;
        }
    }

    HorizontalStackView {
        id: stackView
        anchors.fill: parent
        reverseTransitions: footer.tabBar.currentIndex === 1
        initialItem: ApplicationsPage {
            id: applicationsPage
            preferredSideBarWidth: root.preferredSideBarWidth + kickoff.backgroundMetrics.leftPadding
        }
        Component {
            id: placesPage
            PlacesPage {
                preferredSideBarWidth: root.preferredSideBarWidth + kickoff.backgroundMetrics.leftPadding
                preferredSideBarHeight: applicationsPage.implicitSideBarHeight
            }
        }
        Connections {
            target: footer.tabBar
            function onCurrentIndexChanged() {
                if (footer.tabBar.currentIndex === 0) {
                    stackView.replace(applicationsPage)
                } else if (footer.tabBar.currentIndex === 1) {
                    stackView.replace(placesPage)
                }
            }
        }
    }

    Component.onCompleted: {
        rootModel.refresh();
    }
}
