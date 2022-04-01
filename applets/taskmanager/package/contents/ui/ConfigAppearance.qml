/*
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0

Kirigami.FormLayout {
    anchors.left: parent.left
    anchors.right: parent.right

    readonly property bool plasmaPaAvailable: Qt.createComponent("PulseAudio.qml").status === Component.Ready
    readonly property bool plasmoidVertical: Plasmoid.formFactor === PlasmaCore.Types.Vertical
    readonly property bool iconOnly: Plasmoid.pluginName === "org.kde.plasma.icontasks"

    property alias cfg_showToolTips: showToolTips.checked
    property alias cfg_highlightWindows: highlightWindows.checked
    property bool cfg_indicateAudioStreams
    property alias cfg_maxStripes: maxStripes.value
    property alias cfg_forceStripes: forceStripes.checked
    property int cfg_iconSpacing: 0

    CheckBox {
        id: showToolTips
        Kirigami.FormData.label: i18n("General:")
        text: i18n("Show tooltips")
    }

    RowLayout {
        // HACK: Workaround for Kirigami bug 434625
        // due to which a simple Layout.leftMargin on CheckBox doesn't work
        Item { implicitWidth: Kirigami.Units.gridUnit }
        CheckBox {
            id: highlightWindows
            text: i18n("Highlight windows when hovering over task tooltips")
            enabled: showToolTips.checked
        }
    }

    CheckBox {
        id: indicateAudioStreams
        text: i18n("Mark applications that play audio")
        checked: cfg_indicateAudioStreams && plasmaPaAvailable
        onCheckedChanged: cfg_indicateAudioStreams = checked
        enabled: plasmaPaAvailable
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    SpinBox {
        id: maxStripes
        Kirigami.FormData.label: plasmoidVertical ? i18n("Maximum columns:") : i18n("Maximum rows:")
        from: 1
    }

    CheckBox {
        id: forceStripes
        text: plasmoidVertical ? i18n("Always arrange tasks in rows of as many columns") : i18n("Always arrange tasks in columns of as many rows")
        enabled: maxStripes.value > 1
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    Slider {
        id: iconSpacingSlider

        visible: iconOnly
        Kirigami.FormData.label: i18n("Spacing between icons:")

        Layout.preferredWidth: Kirigami.Units.gridUnit * 15
        enabled: !Kirigami.Settings.tabletMode
        from: 0
        to: 2
        stepSize: 1
        snapMode: Slider.SnapAlways

        value: enabled ? cfg_iconSpacing : to
        onMoved: cfg_iconSpacing = value
    }

    Item {
        id: sliderLabelRow
        Layout.preferredWidth: iconSpacingSlider.Layout.preferredWidth
        Layout.preferredHeight: childrenRect.height
        opacity: iconSpacingSlider.enabled ? 1 : 0.5

        Label {
            anchors.left: parent.left
            text: i18n("Small")
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Normal")
        }

        Label {
            anchors.right: parent.right
            text: i18n("Large")
        }
    }

    Label {
        visible: Kirigami.Settings.tabletMode
        text: i18nc("@info:usagetip under a set of radio buttons when tablet mode is on", "Automatically set to Large when in tablet mode")
        font: Kirigami.Theme.smallFont
    }
}
