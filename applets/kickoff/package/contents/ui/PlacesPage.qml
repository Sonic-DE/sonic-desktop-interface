import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

BasePage {
    id: root
    sideBarComponent: KickoffListView {
        id: sideBarView
        focus: true // needed for Loaders
        model: placesCategoryModel
        delegate: KickoffItemDelegate {
            action: null
            width: view.contentItem.width
            indicator.visible: true
            onClicked: {
                view.currentIndex = index
                forceActiveFocus(Qt.MouseFocusReason)
            }
        }
    }
    contentAreaComponent: KickoffListView {
        id: contentAreaView
        focus: true
        // use same width as applicationsGridView
        implicitWidth: KickoffSingleton.gridCellSize * 4 + leftPadding + rightPadding
        objectName: "frequentlyUsedView"
        model: {
            switch (root.sideBarItem.currentIndex) {
                case 0: return KickoffSingleton.computerModel
                case 1: return KickoffSingleton.recentUsageModel
                case 2: return KickoffSingleton.frequentUsageModel
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
        value: root.contentAreaItem // NOT root.contentAreaItem.currentItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
    Binding {
        target: KickoffSingleton
        property: "lastFocusedView"
        value: root.contentAreaItem // NOT root.contentAreaItem.currentItem
        when: root.T.StackView.status === T.StackView.Active && root.visible
        restoreMode: Binding.RestoreBinding
    }
}
