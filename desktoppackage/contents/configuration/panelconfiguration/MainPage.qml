/*
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo.venerandi@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.4 as QQC2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.configuration 2.0
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.shell.panel 0.1 as Panel
import org.kde.kquickcontrols 2.0

ColumnLayout {
    id: menuColumn
    height: implicitHeight
    spacing: Kirigami.Units.largeSpacing * 2

    property bool vertical: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge)

    readonly property int headingLevel: 2

    PlasmaExtras.PlasmoidHeading {
        RowLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                Layout.leftMargin: Kirigami.Units.smallSpacing
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Settings")
            }

            Item { Layout.fillWidth: true }

            PC3.ToolButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Spacer")
                icon.name: "distribute-horizontal-x"

                PC3.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove this panel; this action is undo-able")
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: hovered
            }

            PC3.ToolButton {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Widgets…")
                icon.name: "list-add"

                PC3.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove this panel; this action is undo-able")
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: hovered
            }

        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Position")
            }
            PanelRepresentation {
                id: positionRepresentation
                text: (panel.location === PlasmaCore.Types.TopEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") :
                       panel.location === PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Right") :
                       panel.location === PlasmaCore.Types.LeftEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Left") :
                       i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom"))
                Layout.alignment: Qt.AlignHCenter
                alignment: (panel.location === PlasmaCore.Types.TopEdge ? Qt.AlignHCenter | Qt.AlignTop :
                            panel.location === PlasmaCore.Types.RightEdge ? Qt.AlignVCenter | Qt.AlignRight :
                            panel.location === PlasmaCore.Types.LeftEdge ? Qt.AlignVCenter | Qt.AlignLeft :
                            Qt.AlignHCenter | Qt.AlignBottom)
                isVertical: menuColumn.vertical
                mainIconSource: (panel.location === PlasmaCore.Types.TopEdge ? "arrow-top" :
                                 panel.location === PlasmaCore.Types.RightEdge ? "arrow-right" :
                                 panel.location === PlasmaCore.Types.LeftEdge ? "arrow-left": "arrow-down")
                onClicked: {
                    console.log('ALL GOOD HERE')
                    setPositionButton.clicked()
                }
            }
            PC3.Button {
                id: setPositionButton
                property bool hintsShown: false
                Layout.alignment: Qt.AlignHCenter
                text: hintsShown ? i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel") : i18nd("plasma_shell_org.kde.plasma.desktop", "Set Position...")
                onClicked: hintsShown = !hintsShown
            }
            Repeater {
                model: Application.screens
                Item {
                    required property var modelData
                    PlasmaCore.Dialog {
                        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint
                        location: PlasmaCore.Types.Floating
                        visible: setPositionButton.hintsShown

                        x: modelData.virtualX + Kirigami.Units.largeSpacing
                        y: modelData.virtualY + modelData.height / 2 - mainItem.height / 2 - margins.top

                        mainItem: MouseArea {
                            cursorShape: Qt.PointingHandCursor
                            width: Kirigami.Units.iconSizes.large
                            height: Kirigami.Units.iconSizes.large
                            Kirigami.Icon {
                                anchors.fill: parent
                                Layout.alignment: Qt.AlignHCenter
                                source: "arrow-left"
                            }
                            onClicked: {
                                setPositionButton.hintsShown = false
                                panel.location = PlasmaCore.Types.LeftEdge
                            }
                        }
                    }
                    PlasmaCore.Dialog {
                        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint
                        location: PlasmaCore.Types.Floating
                        visible: setPositionButton.hintsShown

                        x: modelData.virtualX + modelData.width - Kirigami.Units.largeSpacing - margins.left - margins.right - mainItem.width
                        y: modelData.virtualY + modelData.height / 2 - mainItem.height / 2 - margins.top

                        mainItem: MouseArea {
                            cursorShape: Qt.PointingHandCursor
                            width: Kirigami.Units.iconSizes.large
                            height: Kirigami.Units.iconSizes.large
                            Kirigami.Icon {
                                anchors.fill: parent
                                Layout.alignment: Qt.AlignHCenter
                                source: "arrow-right"
                            }
                            onClicked: {
                                setPositionButton.hintsShown = false
                                panel.location = PlasmaCore.Types.RightEdge
                            }
                        }
                    }
                    PlasmaCore.Dialog {
                        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint
                        location: PlasmaCore.Types.Floating
                        visible: setPositionButton.hintsShown

                        x: modelData.virtualX + modelData.width / 2 - mainItem.width / 2 - margins.left
                        y: modelData.virtualY + Kirigami.Units.largeSpacing

                        mainItem: MouseArea {
                            cursorShape: Qt.PointingHandCursor
                            width: Kirigami.Units.iconSizes.large
                            height: Kirigami.Units.iconSizes.large
                            Kirigami.Icon {
                                anchors.fill: parent
                                Layout.alignment: Qt.AlignHCenter
                                source: "arrow-up"
                            }
                            onClicked: {
                                setPositionButton.hintsShown = false
                                panel.location = PlasmaCore.Types.TopEdge
                            }
                        }
                    }
                    PlasmaCore.Dialog {
                        flags: Qt.WindowStaysOnTopHint | Qt.WindowDoesNotAcceptFocus | Qt.BypassWindowManagerHint
                        location: PlasmaCore.Types.Floating
                        visible: setPositionButton.hintsShown

                        x: modelData.virtualX + modelData.width / 2 - mainItem.width / 2 - margins.left
                        y: modelData.virtualY + modelData.height - mainItem.height - margins.top - margins.bottom - Kirigami.Units.largeSpacing

                        mainItem: MouseArea {
                            cursorShape: Qt.PointingHandCursor
                            width: Kirigami.Units.iconSizes.large
                            height: Kirigami.Units.iconSizes.large
                            Kirigami.Icon {
                                anchors.fill: parent
                                Layout.alignment: Qt.AlignHCenter
                                source: "arrow-down"
                            }
                            onClicked: {
                                setPositionButton.hintsShown = false
                                panel.location = PlasmaCore.Types.BottomEdge
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alignment")
            }
            PanelRepresentation {
                Layout.alignment: Qt.AlignHCenter
                mainIconSource: {
                    if (menuColumn.vertical) {
                        if (alignmentBox.previewIndex === 0) {
                            return "align-vertical-top"
                        } else if (alignmentBox.previewIndex === 1) {
                            return "align-vertical-center"
                        } else {
                            return "align-vertical-bottom"
                        }
                    } else {
                        if (alignmentBox.previewIndex === 0) {
                            return "align-horizontal-left"
                        } else if (alignmentBox.previewIndex === 1) {
                            return "align-horizontal-center"
                        } else {
                            return "align-horizontal-right"
                        }
                    }
                }
                alignment: {
                    let first, second;
                    if (menuColumn.vertical) {
                        if (alignmentBox.previewIndex === 0) {
                            first = Qt.AlignTop
                        } else if (alignmentBox.previewIndex === 1) {
                            first = Qt.AlignVCenter
                        } else {
                            first = Qt.AlignBottom
                        }
                        if (panel.location === PlasmaCore.Types.LeftEdge) {
                            second = Qt.AlignLeft
                        } else {
                            second = Qt.AlignRight
                        }
                    } else {
                        if (alignmentBox.previewIndex === 0) {
                            first = Qt.AlignLeft
                        } else if (alignmentBox.previewIndex === 1) {
                            first = Qt.AlignHCenter
                        } else {
                            first = Qt.AlignRight
                        }
                        if (panel.location === PlasmaCore.Types.TopEdge) {
                            second = Qt.AlignTop
                        } else {
                            second = Qt.AlignBottom
                        }
                    }
                    return first | second;
                }
                onClicked: stackView.push("subpages/Alignment.qml", {stackView: stackView})
                isVertical: menuColumn.vertical
            }
            PC3.ComboBox {
                id: alignmentBox
                Layout.alignment: Qt.AlignHCenter
                property int previewIndex: highlightedIndex > -1 ? highlightedIndex : currentIndex
                model: [
                    menuColumn.vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") : i18nd("plasma_shell_org.kde.plasma.desktop", "Left"),
                    i18nd("plasma_shell_org.kde.plasma.desktop", "Center"),
                    menuColumn.vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom") : i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                ]
                currentIndex: (panel.alignment === Qt.AlignLeft ? 0 :
                                panel.alignment === Qt.AlignCenter ? 1 : 2)
                onActivated: (index) => {
                    if (index === 0) {
                        panel.alignment = Qt.AlignLeft
                    } else if (index === 1) {
                        panel.alignment = Qt.AlignCenter
                    } else {
                        panel.alignment = Qt.AlignRight
                    }
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                level: menuColumn.headingLevel
                Layout.alignment: Qt.AlignHCenter
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@label", "Width")
            }
            PanelRepresentation {
                Layout.alignment: Qt.AlignHCenter
                mainIconSource: (widthBox.previewIndex === 1 ? "gnumeric-ungroup" :
                                 widthBox.previewIndex === 0 ? (menuColumn.vertical ? "panel-fit-height" : "panel-fit-width") : "kdenlive-custom-effect")
                isVertical: menuColumn.vertical
                alignment: positionRepresentation.alignment
                fillAvailable: widthBox.previewIndex === 0
            }
            PC3.ComboBox {
                id: widthBox

                property int previewIndex: highlightedIndex > -1 ? highlightedIndex : currentIndex

                Layout.alignment: Qt.AlignHCenter
                model: [
                    menuColumn.vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Fill height") : i18nd("plasma_shell_org.kde.plasma.desktop", "Fill width"),
                    i18nd("plasma_shell_org.kde.plasma.desktop", "Fit content"),
                    i18nd("plasma_shell_org.kde.plasma.desktop", "Custom")
                ]
                currentIndex: (panel.lengthMode === Panel.Global.FillAvailable ? 0 :
                                panel.lengthMode === Panel.Global.FitContent ? 1 : 2)
                onActivated: (index) => {
                    if (index === 0) {
                        panel.lengthMode = Panel.Global.FillAvailable
                    } else if (index === 1) {
                        panel.lengthMode = Panel.Global.FitContent
                    } else {
                        panel.lengthMode = Panel.Global.Custom
                    }
                }
            }
        }

    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@label", "Visibility")
            }
            PanelRepresentation {
                Layout.alignment: Qt.AlignHCenter
                sunkenPanel: autoHideSwitch.checked
            }
            PC3.Switch {
                id: autoHideSwitch
                Layout.alignment: Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Auto hide")
                Component.onCompleted: checked = configDialog.visibilityMode === Panel.Global.AutoHide
                onCheckedChanged: {
                    if (checked) {
                        configDialog.visibilityMode = Panel.Global.AutoHide
                    } else {
                        configDialog.visibilityMode = Panel.Global.NormalPanel
                    }
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opacity")
            }
            PanelRepresentation {
                Layout.alignment: Qt.AlignHCenter
                adaptivePanel: transparencyBox.previewIndex === 0
                translucentPanel: transparencyBox.previewIndex === 2
            }
            PC3.ComboBox {
                id: transparencyBox
                Layout.alignment: Qt.AlignHCenter
                property int previewIndex: popup.visible ? highlightedIndex : currentIndex
                model: [
                    i18nd("plasma_shell_org.kde.plasma.desktop", "Adaptive"),
                    i18nd("plasma_shell_org.kde.plasma.desktop", "Opaque"),
                    i18nd("plasma_shell_org.kde.plasma.desktop", "Translucent")
                ]
                currentIndex: (panel.opacityMode === Panel.Global.Adaptive ? 0 :
                                panel.opacityMode === Panel.Global.Opaque ? 1 : 2)
                onActivated: (index) => {
                    if (index === 0) {
                        panel.opacityMode = Panel.Global.Adaptive
                    } else if (index === 1) {
                        panel.opacityMode = Panel.Global.Opaque
                    } else {
                        panel.opacityMode = Panel.Global.Translucent
                    }
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                level: menuColumn.headingLevel
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Style")
            }
            PanelRepresentation {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Yes")
                Layout.alignment: Qt.AlignHCenter
                floatingGap: Kirigami.Units.smallSpacing * floatingSwitch.checked
            }
            PC3.Switch {
                id: floatingSwitch
                Layout.alignment: Qt.AlignHCenter
                text: "Floating"
                Component.onCompleted: checked = panel.floating
                onCheckedChanged: panel.floating = checked
            }
        }

    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: Kirigami.Units.largeSpacing

        PC3.Label {
            id: spinBoxLabel
            wrapMode: Text.Wrap

            text: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Width:") : i18nd("plasma_shell_org.kde.plasma.desktop", "Height:")
        }
        PC3.SpinBox {
            id: spinBox

            editable: true
            focus: !Kirigami.InputMethod.willShowOnActive
            from: Math.max(20, panel.minThickness) // below this size, the panel is mostly unusable
            to: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? panel.screenToFollow.geometry.width / 2 : panel.screenToFollow.geometry.height / 2

            stepSize: 2

            value: panel.thickness
            onValueModified: {
                panel.thickness = value
                // Adjust the position of the config bar too TODO
            }
        }
    }

    PlasmaExtras.PlasmoidHeading {
        location: PlasmaExtras.PlasmoidHeading.Footer
        Layout.topMargin: Kirigami.Units.smallSpacing
        topPadding: Kirigami.Units.smallSpacing * 2
        leftPadding: Kirigami.Units.smallSpacing
        bottomPadding: Kirigami.Units.smallSpacing

        Layout.fillWidth: true
        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: Kirigami.Units.largeSpacing

            PC3.Label {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Focus shortcut:")
                visible: panel.adaptiveOpacityEnabled

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                }

                PC3.ToolTip {
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Press this keyboard shortcut to move focus to the Panel")
                    visible: mouseArea.containsMouse
                }
            }

            KeySequenceItem {
                id: button
                keySequence: Plasmoid.globalShortcut
                onCaptureFinished: {
                    Plasmoid.globalShortcut = button.keySequence
                }
            }

            Item {Layout.fillWidth: true}

            PC3.ToolButton {
                text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@action:button Delete the panel", "Delete")
                icon.name: "delete"

                PC3.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove this panel; this action is undo-able")
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: hovered

                onClicked: Plasmoid.internalAction("remove").trigger();
            }
        }
    }
}
