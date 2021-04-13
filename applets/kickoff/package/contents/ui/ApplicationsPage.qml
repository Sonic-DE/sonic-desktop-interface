import QtQuick 2.0
import QtQuick.Layouts 1.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

BasePage {
    id: root
    sideBarComponent: KickoffListView {
        model: KickoffSingleton.rootModel
    }
    contentAreaComponent: VerticalStackView {
        id: stackView
        readonly property string preferredAppsViewObjectName: plasmoid.configuration.applicationsDisplay == 0 ? "applicationsGridView" : "applicationsListView"
        readonly property Component preferredAppsViewComponent: plasmoid.configuration.applicationsDisplay == 0 ? applicationsGridView : applicationsListView
        // Prevents creating and destroying the model a lot
        readonly property Kicker.AppsModel appsModel: KickoffSingleton.rootModel.modelForRow(root.sideBarItem.currentIndex)

        initialItem: favoritesLoader

        Component {
            id: favoritesLoader
            Loader {
                objectName: "favoritesLoader"
                sourceComponent: plasmoid.configuration.favoritesDisplay == 0 ? favoritesGridView : favoritesListView
                Component {
                    id: favoritesListView
                    KickoffListView {
                        model: KickoffSingleton.rootModel.favoritesModel
                    }
                }
                Component {
                    id: favoritesGridView
                    KickoffGridView {
                        model: KickoffSingleton.rootModel.favoritesModel
                    }
                }
            }
        }

        Component {
            id: applicationsListView
            KickoffListView {
                objectName: "applicationsListView"
                // use same width as applicationsGridView
                implicitWidth: KickoffSingleton.gridCellSize * 4 + leftPadding + rightPadding
                model: stackView.appsModel
                section.property: model && model.description == "KICKER_ALL_MODEL" ? "display" : ""
                section.criteria: ViewSection.FirstCharacter
            }
        }

        Component {
            id: applicationsGridView
            KickoffGridView {
                objectName: "applicationsGridView"
                model: stackView.appsModel
            }
        }

        Connections {
            target: root.sideBarItem
            function onCurrentIndexChanged() {
                if (root.sideBarItem.currentIndex === 0) {
                    stackView.replace(favoritesLoader)
                } else if (root.sideBarItem.currentIndex === 1 && stackView.currentItem.objectName !== "applicationsListView") {
                    // Always use list view for alphabetical apps view since grid view doesn't have sections
                    // TODO: maybe find a way to have a list view with grids in each section?
                    stackView.replace(applicationsListView)
                } else if (root.sideBarItem.currentIndex > 1 && stackView.currentItem.objectName !== stackView.preferredAppsViewObjectName) {
                    stackView.replace(stackView.preferredAppsViewComponent)
                }
            }
        }
    }
}
