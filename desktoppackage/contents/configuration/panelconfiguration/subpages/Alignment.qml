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
import ".."

ColumnLayout {
    id: menuColumn
    height: implicitHeight
    width: parent.width
    required property var stackView
    spacing: Kirigami.Units.largeSpacing

    readonly property int headingLevel: 2


    PlasmaExtras.PlasmoidHeading {
        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            PC3.ToolButton {
                Layout.leftMargin: Kirigami.Units.smallSpacing
                icon.name: "arrow-left"
                onClicked: stackView.pop()
            }

            Kirigami.Heading {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Panel Alignment")
            }
        }
    }

    GridLayout {

        rowSpacing: Kirigami.Units.largeSpacing
        columns: 3

        Item {
            Layout.preferredWidth: menuColumn.width / 3 - 4
            PanelRepresentation {
                anchors.horizontalCenter: parent.horizontalCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Left")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                alignment: Qt.AlignBottom | Qt.AlignLeft
                mainIconSource: "align-horizontal-left"
                checked: panel.alignment === Qt.AlignLeft
                onClicked: panel.alignment = Qt.AlignLeft
            }
        }

        Item {
            Layout.preferredWidth: menuColumn.width / 3 - 4
            PanelRepresentation {
                anchors.horizontalCenter: parent.horizontalCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Center")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                alignment: Qt.AlignBottom | Qt.AlignHCenter
                mainIconSource: "align-horizontal-center"
                checked: panel.alignment === Qt.AlignCenter
                onClicked: panel.alignment = Qt.AlignCenter
            }
        }

        Item {
            Layout.preferredWidth: menuColumn.width / 3 - 4
            PanelRepresentation {
                anchors.horizontalCenter: parent.horizontalCenter
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Right")
                tooltip: panel.formFactor === PlasmaCore.Types.Vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the top; no effect when panel is maximized") : i18nd("plasma_shell_org.kde.plasma.desktop", "Aligns a non-maximized panel to the left; no effect when panel is maximized")
                alignment: Qt.AlignBottom | Qt.AlignRight
                mainIconSource: "align-horizontal-right"
                checked: panel.alignment === Qt.AlignRight
                onClicked: panel.alignment = Qt.AlignRight
            }
        }
    }
}
