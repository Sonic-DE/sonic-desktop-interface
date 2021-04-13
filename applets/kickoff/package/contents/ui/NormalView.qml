import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3

EmptyPage {
    id: root
    property real preferredSideBarWidth: Math.max(footer.implicitTabBarWidth, stackView.currentItem ? stackView.currentItem.implicitSideBarWidth : 0)
//     implicitWidth: Math.max(footer.implicitWidth, KickoffSingleton.backgroundMetrics.margins.left+root.preferredSideBarWidth+3+412)

    contentItem: HorizontalStackView {
        id: stackView
        focus: true
        reverseTransitions: footer.tabBarCurrentIndex === 1
        initialItem: applicationsPage
        Component {
            id: applicationsPage
            ApplicationsPage {
                preferredSideBarWidth: root.preferredSideBarWidth + KickoffSingleton.backgroundMetrics.margins.left
            }
        }
        Component {
            id: placesPage
            PlacesPage {
                preferredSideBarWidth: root.preferredSideBarWidth + KickoffSingleton.backgroundMetrics.margins.left
            }
        }
        Connections {
            target: footer
            function onTabBarCurrentIndexChanged() {
                if (footer.tabBarCurrentIndex === 0) {
                    stackView.replace(applicationsPage)
                } else if (footer.tabBarCurrentIndex === 1) {
                    stackView.replace(placesPage)
                }
            }
        }
    }

    footer: Footer {
        id: footer
        preferredTabBarWidth: root.preferredSideBarWidth
    }

    Item {
        id: mouseFilterArea
        property bool filtered: false
        parent: root
        z: 1
        anchors {
            left: parent.left
            top: filtered ? parent.top : footer.top
            bottom: parent.bottom
        }
        width: preferredSideBarWidth + KickoffSingleton.backgroundMetrics.margins.left
        // NOTE: This is rather delicate, so be careful about changing it.
        HoverHandler {
            property real enterY: -1
            property real enterX: -1
            signal entered()
            signal exited()
            // Give the user some visual feedback
            cursorShape: mouseFilterArea.filtered ? Qt.BusyCursor : Qt.ArrowCursor
            onHoveredChanged: hovered ? entered() : exited()
            onEntered: {
                enterX = point.position.x
                enterY = point.position.y
            }
            onExited: {
                // For LTR: If moved right and up, filter
                // For RTL: If moved left and up, filter
                // I think moving in the opposite direction cancels the filter with a slight delay.
                // This is desireable, but I'm not sure why it works.
                if ((root.mirrored ? enterX > point.position.x : enterX < point.position.x) && enterY > point.position.y) {
                    mouseFilterArea.filtered = true
                    mouseFilterTimer.restart()
                } else {
                    mouseFilterArea.filtered = false
                    mouseFilterTimer.stop()
                }
                enterX = -1
                enterY = -1
            }
        }
    }
    Timer {
        id: mouseFilterTimer
        interval: root.height // Not sure, but maybe 1ms per pixel is a good way to pick a timer
        //triggeredOnStart: true
        onTriggered: mouseFilterArea.filtered = false
    }
}
