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
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Position:")
            }
            PanelRepresentation {
                text: "Bottom"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                alignment: Qt.AlignCenter
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Alignment:")
            }
            PanelRepresentation {
                text: "Left"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                alignment: Qt.AlignLeft
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                level: menuColumn.headingLevel
                Layout.alignment: Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Type:")
            }
            PanelRepresentation {
                text: "Dock"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                floatingGap: Kirigami.Units.smallSpacing
                alignment: Qt.AlignCenter
            }
        }

    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        Layout.fillWidth: true

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Visibility:")
            }
            PanelRepresentation {
                text: "Auto Hide"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                sunkenPanel: true
                alignment: Qt.AlignCenter
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                level: menuColumn.headingLevel
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Opacity:")
            }
            PanelRepresentation {
                text: "Adaptive"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                adaptivePanel: true
                alignment: Qt.AlignCenter
            }
        }

        ColumnLayout {
            Layout.preferredWidth: menuColumn.width / 3
            spacing: Kirigami.Units.smallSpacing
            Kirigami.Heading {
                level: menuColumn.headingLevel
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Floating:")
            }
            PanelRepresentation {
                text: "Yes"
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                floatingGap: Kirigami.Units.smallSpacing
                alignment: Qt.AlignCenter
            }
        }

    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter
        spacing: Kirigami.Units.largeSpacing

        PC3.Label {
            id: spinBoxLabel
            wrapMode: Text.Wrap

            text: panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge ? i18nd("plasma_shell_org.kde.plasma.desktop", "Panel width:") : i18nd("plasma_shell_org.kde.plasma.desktop", "Panel height:")
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

        PC3.Label {
            Layout.leftMargin: Kirigami.Units.gridUnit * 2
            wrapMode: Text.Wrap

            text: "Panel width:"
        }
        PC3.SpinBox {

            editable: true
            focus: !Kirigami.InputMethod.willShowOnActive
            from: 50
            to: panel.screenGeometry.width

            stepSize: 20

            value: panel.length
            onValueModified: {
                panel.length = value
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
