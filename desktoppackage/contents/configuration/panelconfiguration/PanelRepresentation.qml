import QtQuick
import QtQuick.Layouts

import org.kde.plasma.components as PC3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.ksvg as KSvg
import org.kde.plasma.shell.panel as Panel
import org.kde.kirigami as Kirigami

Item {
    id: root

    property string text
    property /*Qt::Alignment*/int alignment: Qt.AlignHCenter | Qt.AlignBottom
    property string tooltip

    property bool isVertical: false
    property bool checked: false
    property var mainIconSource: null
    property int screenHeight: Math.round(screenRect.height / 2)

    property bool panelVisible: true
    property bool translucentPanel: false
    property bool sunkenPanel: false
    property bool adaptivePanel: false
    property bool fillAvailable: false
    property int floatingGap: 0

    property bool windowVisible: false
    property int windowZ: 0
    property bool windowOverPanel: true

    onSunkenPanelChanged: sunkenAnimation.restart()
    onWindowVisibleChanged: sunkenAnimation.restart()

    readonly property bool iconAndLabelsShouldlookSelected: checked || mouseArea.pressed

    signal clicked()

    implicitHeight: mainItem.height
    implicitWidth: mainItem.width

    PC3.ToolTip {
        text: root.tooltip
        visible: mouseArea.containsMouse && text.length > 0
    }

    PlasmaExtras.Highlight {
        anchors.fill: parent
        anchors.margins: -Kirigami.Units.smallSpacing
        hovered: mouseArea.containsMouse
        pressed: root.iconAndLabelsShouldlookSelected
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }

    ColumnLayout {
        id: mainItem
        spacing: Kirigami.Units.smallSpacing
        Rectangle {
            id: screenRect

            Layout.alignment: Qt.AlignHCenter
            implicitWidth: Math.round(Math.min(Kirigami.Units.gridUnit * 6, Screen.width * 0.1))
            implicitHeight: Math.round(Math.min(Kirigami.Units.gridUnit * 4, Screen.width * 0.1))
            color: Qt.tint(Kirigami.Theme.backgroundColor, Qt.rgba(1, 1, 1, 0.3))
            border.color: Kirigami.Theme.highlightColor
            radius: Kirigami.Units.cornerRadius
            clip: root.sunkenPanel

            RowLayout {
                anchors.fill: parent
                z: 1
                Rectangle {
                    id: panelImage

                    implicitWidth: root.isVertical ? Math.round(parent.width / 6) : Math.round(parent.width * (root.fillAvailable ? 1 : 0.8))
                    implicitHeight: root.isVertical ? Math.round(parent.height * (root.fillAvailable ? 1 : 0.8)) : Math.round(parent.height / 4)
                    Layout.alignment: root.alignment
                    Layout.bottomMargin: root.sunkenPanel * -Math.round(height * sunkenValue) + root.floatingGap

                    property real sunkenValue
                    property bool shouldWindowCoverPanel: false

                    SequentialAnimation on sunkenValue {
                        id: sunkenAnimation
                        running: panel.sunkenPanel
                        loops: Animation.Infinite
                        ScriptAction {
                            script: panelImage.shouldWindowCoverPanel = false
                        }
                        SmoothedAnimation {
                            duration: Kirigami.Units.veryLongDuration * 2
                            velocity: -1
                            from: 0.9
                            to: 0
                        }
                        PauseAnimation {
                            duration: Kirigami.Units.veryLongDuration
                        }
                        ScriptAction {
                            script: panelImage.shouldWindowCoverPanel = true
                        }
                        PauseAnimation {
                            duration: Kirigami.Units.veryLongDuration
                        }
                        SmoothedAnimation {
                            duration: Kirigami.Units.veryLongDuration * 2
                            velocity: -1
                            from: 0
                            to: 0.9
                        }
                        PauseAnimation {
                            duration: Kirigami.Units.veryLongDuration * 3
                        }
                        ScriptAction {
                            script: panelImage.shouldWindowCoverPanel = false
                        }
                        PauseAnimation {
                            duration: Kirigami.Units.veryLongDuration
                        }
                    }

                    color: root.translucentPanel ? screenRect.color : Kirigami.Theme.backgroundColor
                    opacity: root.translucentPanel ? 0.8 : 1.0
                    border.color: "transparent"
                    visible: root.panelVisible
                    clip: root.adaptivePanel
                    radius: Kirigami.Units.cornerRadius


                    Loader {
                        id: horizontalAdaptivePanelLoader
                        active: root.adaptivePanel && !root.isVertical
                        sourceComponent: Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.left: parent.left
                            anchors.leftMargin: Math.round(panelImage.width / 3)
                            color: Qt.lighter(screenRect.color)
                            border.color: Kirigami.Theme.highlightColor
                            width: panelImage.width
                            height: Math.round(panelImage.height * 4)
                            radius: Math.round(height / 2)
                            rotation: 45
                        }
                    }

                    Loader {
                        id: verticalAdaptivePanelLoader
                        active: root.adaptivePanel && root.isVertical
                        sourceComponent: Rectangle {
                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.top: parent.top
                            anchors.topMargin: Math.round(panelImage.height / 4)
                            color: Qt.lighter(screenRect.color)
                            border.color: Kirigami.Theme.highlightColor
                            width: Math.round(panelImage.width * 2)
                            height: panelImage.height
                            radius: Math.round(height / 2)
                            rotation: 45
                        }
                    }

                    Rectangle {
                        id: panelBorder
                        anchors.fill: parent
                        color: "transparent"
                        border.color: Kirigami.Theme.highlightColor
                        radius: panelImage.radius
                    }
                }
            }

            Rectangle {
                id: window
                width: Math.round(parent.width / 2)
                height: Math.round(parent.height / 2)
                visible: root.windowVisible
                radius: 5
                color: Kirigami.Theme.highlightColor
                border.color: "transparent"

                x: Math.round(screenRect.width / 2 - width / 2) + Kirigami.Units.gridUnit

                y: screenRect.height - height - (windowOverPanel || panelImage.shouldWindowCoverPanel ? 1/2 : 3/2) * panelImage.height

                Behavior on y {
                    NumberAnimation {
                        duration: Kirigami.Units.veryLongDuration * 2
                    }
                }

                z: root.windowZ

                Row {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: Kirigami.Units.smallSpacing
                    spacing: Kirigami.Units.smallSpacing
                    Repeater {
                        model: 3
                        delegate: Rectangle {
                            width: Math.round(Kirigami.Units.gridUnit / 6)
                            height: width
                            radius: Math.round(height / 2)
                            color: Kirigami.Theme.textColor
                        }
                    }
                }
            }

            Kirigami.Icon {
                id: mainIcon
                visible: valid
                anchors.centerIn: parent
                transform: Translate {
                    y: root.isVertical ? 0 : Math.round((mainIcon.y - panelImage.y) / 4)
                    x: root.isVertical ? Math.round((mainIcon.x - panelImage.x) / 4) : 0
                }
                height: parent.height / 2
                source: root.mainIconSource
            }
        }
    }
}
