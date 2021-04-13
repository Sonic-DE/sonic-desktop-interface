import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

BasePage {
    id: root
    sideBarComponent: KickoffListView {
        model: placesCategoryModel
        delegate: KickoffItemDelegate {
            width: view.contentItem.width
            indicator.visible: true
        }
    }
    contentAreaComponent: VerticalStackView {
        id: stackView
        initialItem: computerView
        Component {
            id: computerView
            KickoffListView {
                // use same width as applicationsGridView
                implicitWidth: KickoffSingleton.gridCellSize * 4 + leftPadding + rightPadding
                objectName: "computerView"
                model: KickoffSingleton.computerModel
            }
        }
        Component {
            id: recentlyUsedView
            KickoffListView {
                // use same width as applicationsGridView
                implicitWidth: KickoffSingleton.gridCellSize * 4 + leftPadding + rightPadding
                objectName: "recentlyUsedView"
                model: KickoffSingleton.recentUsageModel
            }
        }
        Component {
            id: frequentlyUsedView
            KickoffListView {
                // use same width as applicationsGridView
                implicitWidth: KickoffSingleton.gridCellSize * 4 + leftPadding + rightPadding
                objectName: "frequentlyUsedView"
                model: KickoffSingleton.frequentUsageModel
            }
        }

        Connections {
            target: root.sideBarItem
            function onCurrentIndexChanged() {
                switch (root.sideBarItem.currentIndex) {
                    case 0: stackView.reverseTransitions = false
                        stackView.replace(computerView)
                        break
                    case 1: stackView.reverseTransitions = stackView.currentItem.objectName === "computerView"
                        stackView.replace(recentlyUsedView)
                        break
                    case 2: stackView.reverseTransitions = true
                        stackView.replace(frequentlyUsedView)
                        break
                }
            }
        }
    }

    // we make our model ourselves
    ListModel {
        id: placesCategoryModel
        ListElement { display: "Computer"; decoration: "computer" }
        ListElement { display: "History"; decoration: "view-history" }
        ListElement { display: "Frequently Used"; decoration: "clock" }
        Component.onCompleted: {
            // Can't use a function in a QML ListElement declaration
            placesCategoryModel.setProperty(0, "display", i18n("Computer"))
            placesCategoryModel.setProperty(1, "display", i18n("History"))
            placesCategoryModel.setProperty(2, "display", i18n("Frequently Used"))

            if (KickoffSingleton.powerManagement.data["PowerDevil"]
                && KickoffSingleton.powerManagement.data["PowerDevil"]["Is Lid Present"]) {
                placesCategoryModel.setProperty(0, "decoration", "computer-laptop")
            }
        }
    }
}
