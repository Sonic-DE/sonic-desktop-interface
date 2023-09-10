/*
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo.venerandi@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.configuration 2.0
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.shell.panel 0.1 as Panel
import org.kde.kquickcontrols 2.0
import "subpages"

ColumnLayout {
    id: menuColumn
    height: implicitHeight
    spacing: Kirigami.Units.largeSpacing * 2

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
                text: "Add spacer"
                icon.name: "distribute-horizontal-x"

                PC3.ToolTip.text: i18nd("plasma_shell_org.kde.plasma.desktop", "Remove this panel; this action is undo-able")
                PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
                PC3.ToolTip.visible: hovered
            }

            PC3.ToolButton {
                text: "Add widgets…"
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
                text: (panel.location === PlasmaCore.Types.TopEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Top") :
                       panel.location === PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Right") :
                       panel.location === PlasmaCore.Types.LeftEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Left") :
                       i18nd("plasma_shell_org.kde.plasma.desktop", "Bottom"))
                tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Select the side of the screen for the panel and the monitor it is in")
                Layout.alignment: Qt.AlignHCenter
                mainIconSource: (panel.location === PlasmaCore.Types.TopEdge ? "arrow-top" :
                                 panel.location === PlasmaCore.Types.RightEdge ? "arrow-right" :
                                 panel.location === PlasmaCore.Types.LeftEdge ? "arrow-left": "arrow-down")
                onClicked: stackView.push("subpages/Location.qml", {stackView: stackView})
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
                text: "Left"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignHCenter
                mainIconSource: "align-horizontal-left"
                onClicked: stackView.push("subpages/Alignment.qml", {stackView: stackView})
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                level: menuColumn.headingLevel
                Layout.alignment: Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Width")
            }
            PanelRepresentation {
                text: "Fit Content"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignHCenter
                floatingGap: Kirigami.Units.smallSpacing
                mainIconSource: "gnumeric-ungroup"
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
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility")
            }
            PanelRepresentation {
                text: "Auto Hide"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignHCenter
                sunkenPanel: true
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
                text: "Adaptive"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignHCenter
                adaptivePanel: true
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.mediumSpacing
            Kirigami.Heading {
                level: menuColumn.headingLevel
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating")
            }
            PanelRepresentation {
                text: "Yes"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignHCenter
                floatingGap: Kirigami.Units.smallSpacing
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

            Kirigami.Heading {
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Focus Shortcut:")
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
