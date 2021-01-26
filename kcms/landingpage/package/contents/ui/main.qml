/*
 * Copyright 2021 Marco Martin <mart@kde.org>
 * Copyright 2018 Furkan Tokac <furkantokac34@gmail.com>
 * Copyright (C) 2019 Nate Graham <nate@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
import QtQuick 2.7
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.3
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kcm 1.3 as KCM

import org.kde.plasma.core 2.0 as PlasmaCore

KCM.SimpleKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 40

    Kirigami.FormLayout {
        id: formLayout

        QQC2.ButtonGroup { id: themeGroup }

        QQC2.RadioButton {
            id: lightTheme
            Kirigami.FormData.label: i18n("Appearance:")
            text: i18n("Light Theme")
            checked: kcm.globalsSettings.colorScheme === "BreezeLight"
            onToggled: kcm.globalsSettings.colorScheme = "BreezeLight"
            QQC2.ButtonGroup.group: themeGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "colorScheme"
            }
        }
        QQC2.RadioButton {
            id: darkTheme
            text: i18n("Dark Theme")
            checked: kcm.globalsSettings.colorScheme === "BreezeDark"
            onToggled: kcm.globalsSettings.colorScheme = "BreezeDark"
            QQC2.ButtonGroup.group: themeGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "colorScheme"
            }
        }

        // We want to show the slider in a logarithmic way. ie
        // move from 4x, 3x, 2x, 1x, 0.5x, 0.25x, 0.125x
        // 0 is a special case
        ColumnLayout {
            Kirigami.FormData.label: i18n("Animation speed:")
            Kirigami.FormData.buddyFor: slider

            QQC2.Slider {
                id: slider
                Layout.fillWidth: true
                from: -4
                to: 4
                stepSize: 0.5
                snapMode: QQC2.Slider.SnapAlways
                onMoved: {
                    if(value === to) {
                        kcm.globalsSettings.animationDurationFactor = 0;
                    } else {
                        kcm.globalsSettings.animationDurationFactor = 1.0 / Math.pow(2, value);
                    }
                }
                value: if (kcm.globalsSettings.animationDurationFactor === 0) {
                    return slider.to;
                } else {
                    return -(Math.log(kcm.globalsSettings.animationDurationFactor) / Math.log(2));
                }

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "animationDurationFactor"
                }
            }
            RowLayout {
                QQC2.Label {
                    text: i18nc("Animation speed", "Slow")
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("Animation speed", "Instant")
                }
            }
        }
        RowLayout {
            QQC2.Button {
                text: i18n("More Appearance Settings...")
                onClicked: kcm.openKCM("kcm_lookandfeel")
            }
            QQC2.Button {
                text: i18n("Change Wallpaper...")
                onClicked: kcm.openWallpaperDialog()
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        // Click behavior settings

        QQC2.ButtonGroup { id: singleClickGroup }

        QQC2.RadioButton {
            id: singleClick
            Kirigami.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
            text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
            checked: kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = true
            QQC2.ButtonGroup.group: singleClickGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "singleClick"
            }
        }

        QQC2.RadioButton {
            id: doubleClick
            text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
            checked: !kcm.globalsSettings.singleClick
            onToggled: kcm.globalsSettings.singleClick = false
            QQC2.ButtonGroup.group: singleClickGroup

            KCM.SettingStateBinding {
                configObject: kcm.globalsSettings
                settingName: "singleClick"
                extraEnabledConditions: singleClick.enabled
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            text: singleClick.checked ? i18n("Select by clicking on item's selection marker") : i18n("Open by double-clicking instead")
            elide: Text.ElideRight
            font: theme.smallestFont
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("File Indexing:")
            text: i18n("Enabled")
            checked: kcm.balooSettings.indexingEnabled
            onCheckStateChanged: {
                kcm.balooSettings.indexingEnabled = checked
            }
        }
        QQC2.Button {
            text: i18n("More Behavior Settings...")
            onClicked: kcm.openKCM("kcm_workspace")
        }
    }
}
