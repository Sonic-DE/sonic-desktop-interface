/*
 * SPDX-FileCopyrightText: 2026 Sebastian Sauer <dipesh@gmx.de>
 * SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.kirigami as Kirigami
import org.sonicde.dwellclicker.ipc
PlasmoidItem {
    id: root
    DwellClickerSocket {
        id: dwellClicker
    }
    Plasmoid.status: {
        if (!dwellClicker.connected) {
            return PlasmaCore.Types.HiddenStatus
        }
        return dwellClicker.enabled ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus
    }
    Plasmoid.icon: "input-mouse-click-left"
    toolTipMainText: Plasmoid.title
    toolTipSubText: dwellClicker.enabled ? i18nc("@info:tooltip", "Dwell clicking is on") : i18nc("@info:tooltip", "Dwell clicking is off")
    fullRepresentation: PlasmaExtras.Representation {
        id: dialogItem
        Layout.minimumWidth: Kirigami.Units.gridUnit * 12
        Layout.minimumHeight: Kirigami.Units.gridUnit * 10
        collapseMarginsHint: true
        header: PlasmaExtras.PlasmoidHeading {
            contentItem: ColumnLayout {
                spacing: 0
                PlasmaComponents3.Switch {
                    Layout.fillWidth: true
                    padding: Kirigami.Units.smallSpacing
                    text: i18nc("@option:check", "Dwell clicking")
                    enabled: dwellClicker.connected
                    checked: dwellClicker.enabled
                    onToggled: dwellClicker.setEnabled(checked)
                }
                PlasmaComponents3.Label {
                    Layout.fillWidth: true
                    text: i18nc("@info:usagetip", "Choose the next type of click:")
                    wrapMode: Text.Wrap
                    padding: Kirigami.Units.smallSpacing
                }
            }
        }
        contentItem: ColumnLayout {
            spacing: 0
            PlasmaComponents3.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                ListView {
                    id: listView
                    width: parent.width
                    enabled: dwellClicker.connected && dwellClicker.enabled
                    currentIndex: getIndexForActionId(dwellClicker.button)
                    function getIndexForActionId(id: int): int {
                        return id >= 10 ? id - 10 : -1;
                    }
                    readonly property var mouseActionModel: [
                        { name: i18nc("mouse button action", "Left-click"), actionIcon: "input-mouse-click-left", actionId: 10 },
                        { name: i18nc("mouse button action", "Double-click"), actionIcon: "input-mouse-click-left-double", actionId: 11 },
                        { name: i18nc("mouse button action", "Click-and-drag"), actionIcon: "input-mouse", actionId: 12 },
                        { name: i18nc("mouse button action", "Middle-click"), actionIcon: "input-mouse-click-middle", actionId: 13 },
                        { name: i18nc("mouse button action", "Right-click"), actionIcon: "input-mouse-click-right", actionId: 14 }
                    ]
                    model: listView.mouseActionModel
                    delegate: PlasmaComponents3.ItemDelegate {
                        required property int index
                        required property var modelData
                        icon.name: modelData.actionIcon
                        text: modelData.name
                        width: ListView.view.width
                        height: Kirigami.Units.gridUnit * 2
                        highlighted: ListView.isCurrentItem
                        onClicked: {
                            // If the item is already selected, unselect it by mapping to actionId 0 what
                            // equals NoButton. Otherwise select the clicked item and use its actionId.
                            const targetId = (listView.currentIndex === index) ? 0 : modelData.actionId
                            if (root.hideOnWindowDeactivate) {
                                root.expanded = false
                            }
                            dwellClicker.setButton(targetId)
                        }
                    }
                }
            }
        }
    }
}
