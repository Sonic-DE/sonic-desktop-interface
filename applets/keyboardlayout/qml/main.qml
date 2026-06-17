/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.workspace.components
import org.kde.plasma.private.kcm_keyboard as KCMKeyboard
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    signal layoutSelected(int layoutIndex)

    preferredRepresentation: fullRepresentation
    toolTipMainText: Plasmoid.title
    toolTipSubText: "" // proper subtext is set by fullRepresentation

    readonly property bool inEmbeddedContainment: Plasmoid.containment.containmentType === PlasmaCore.Containment.CustomEmbedded

    fullRepresentation: KeyboardLayoutSwitcher {
        id: switcher

        hoverEnabled: true
        Plasmoid.status: hasMultipleKeyboardLayouts ? PlasmaCore.Types.ActiveStatus : root.inEmbeddedContainment ? PlasmaCore.Types.HiddenStatus : PlasmaCore.Types.PassiveStatus

        // Derive a country code for the flag/label. Prefers the layout's
        // shortName when usable, otherwise extracts from longName
        // (e.g. "English (US)" -> "US"), falling back to the daemon's
        // system default country code.
        function countryCode() {
            const sn = switcher.layoutNames.shortName;
            if (sn && sn.length >= 2) {
                return sn;
            }
            const ln = switcher.layoutNames.longName || "";
            const m = ln.match(/\(([A-Za-z]{2,3})\)/);
            if (m) {
                return m[1];
            }
            return KCMKeyboard.Flags.getDefaultCountryCode();
        }

        Binding {
            root.toolTipSubText: switcher.layoutNames.longName
        }

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: root.toolTipMainText
            subText: root.toolTipSubText
        }

        Instantiator {
            id: actionsInstantiator
            model: switcher.keyboardLayout.layoutsList
            delegate: PlasmaCore.Action {
                required property string longName
                required property string shortName
                required property int index

                text: longName
                icon.icon: KCMKeyboard.Flags.getIcon(shortName.length >= 2 ? shortName : switcher.countryCode())
                onTriggered: {
                    root.layoutSelected(index);
                }
            }
            onObjectAdded: (index, object) => {
                Plasmoid.contextualActions.push(object)
            }
            onObjectRemoved: (index, object) => {
                const i = Plasmoid.contextualActions.indexOf(object);
                if (i >= 0) {
                    Plasmoid.contextualActions.splice(i, 1);
                }
            }
        }

        Connections {
            target: root

            function onLayoutSelected(layoutIndex) {
               switcher.keyboardLayout.layout = layoutIndex;
            }
        }

        Connections {
            target: Plasmoid

            function onActivated() {
                switcher.keyboardLayout.switchToNextLayout()
            }
        }

        Text {
            id: flag

            anchors.fill: parent

            visible: Plasmoid.configuration.displayStyle === 1

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "Noto Color Emoji"
            font.pixelSize: Math.min(width, height) * 0.8
            text: {
                const cc = switcher.countryCode();
                if (cc.length < 2) return "";
                return String.fromCodePoint(0x1F1E6 + cc.toUpperCase().charCodeAt(0) - 65,
                                            0x1F1E6 + cc.toUpperCase().charCodeAt(1) - 65);
            }
        }

        PlasmaComponents3.Label {
            id: countryCode

            anchors.centerIn: parent
            width: Math.min(switcher.width, switcher.height)
            height: width

            visible: Plasmoid.configuration.displayStyle === 0

            font.pointSize: height || Kirigami.Theme.defaultFont.pointSize
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: switcher.countryCode().toUpperCase()
            textFormat: Text.PlainText
        }
    }

    function actionTriggered(actionName) {
        const layoutIndex = parseInt(actionName);
        if (!isNaN(layoutIndex)) {
            layoutSelected(layoutIndex);
        }
    }
}
