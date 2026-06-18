/*
    SPDX-FileCopyrightText: 2020 Andrey Butirsky <butirsky@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.private.kcm_keyboard as KCMKeyboard
import org.kde.plasma.keyboardlayout.ipc as KCMKeyboardIPC
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    preferredRepresentation: fullRepresentation
    toolTipMainText: Plasmoid.title

    readonly property bool inEmbeddedContainment: Plasmoid.containment.containmentType === PlasmaCore.Containment.CustomEmbedded

    // The list of right-click menu actions, one per layout.
    property var layoutActions: []

    Plasmoid.contextualActions: root.layoutActions

    Plasmoid.onActivated: KCMKeyboardIPC.KeyboardLayoutSocket.switchToNextLayout()

    Plasmoid.status:
        KCMKeyboardIPC.KeyboardLayoutSocket.hasMultipleLayouts
            ? PlasmaCore.Types.ActiveStatus
            : root.inEmbeddedContainment
                ? PlasmaCore.Types.HiddenStatus
                : PlasmaCore.Types.PassiveStatus

    toolTipSubText:
        KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList.length > KCMKeyboardIPC.KeyboardLayoutSocket.layout
            ? KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList[KCMKeyboardIPC.KeyboardLayoutSocket.layout].longName
            : ""

    // Derive a country code for the flag/label
    function countryCode() {
        const list = KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList;
        const idx = KCMKeyboardIPC.KeyboardLayoutSocket.layout;
        if (idx < 0 || idx >= list.length) {
            return KCMKeyboard.Flags.getDefaultCountryCode();
        }
        const sn = list[idx].shortName;
        if (sn && sn.length >= 2) {
            return sn;
        }
        const ln = list[idx].longName || "";
        const m = ln.match(/\(([A-Za-z]{2,3})\)/);
        if (m) {
            return m[1];
        }
        return KCMKeyboard.Flags.getDefaultCountryCode();
    }

    fullRepresentation: Item {
        id: fullRepresentation

        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: root.toolTipMainText
            subText: root.toolTipSubText
        }

        Instantiator {
            id: actionsInstantiator
            model: KCMKeyboardIPC.KeyboardLayoutSocket.layoutsList
            delegate: PlasmaCore.Action {
                required property var modelData
                required property int index

                text: modelData.longName
                icon.icon: KCMKeyboard.Flags.getIcon(
                    (modelData.shortName && modelData.shortName.length >= 2)
                        ? modelData.shortName
                        : root.countryCode())
                onTriggered: KCMKeyboardIPC.KeyboardLayoutSocket.setLayout(index)
            }
            onObjectAdded: (index, object) => {
                const actions = [...root.layoutActions];
                actions.splice(index, 0, object);
                root.layoutActions = actions;
            }
            onObjectRemoved: (index, object) => {
                const actions = [...root.layoutActions];
                actions.splice(index, 1);
                root.layoutActions = actions;
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
                const cc = root.countryCode();
                if (cc.length < 2) return "";
                return String.fromCodePoint(0x1F1E6 + cc.toUpperCase().charCodeAt(0) - 65,
                                            0x1F1E6 + cc.toUpperCase().charCodeAt(1) - 65);
            }
        }

        PlasmaComponents3.Label {
            id: countryCode

            anchors.centerIn: parent
            width: Math.min(fullRepresentation.width, fullRepresentation.height)
            height: width

            visible: Plasmoid.configuration.displayStyle === 0

            font.pointSize: height || Kirigami.Theme.defaultFont.pointSize
            fontSizeMode: Text.Fit
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            text: root.countryCode().toUpperCase()
            textFormat: Text.PlainText
        }
    }
}
