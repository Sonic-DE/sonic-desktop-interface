/*
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {

    QQC2.ButtonGroup { id: effectGroup }

    QQC2.RadioButton {
        QQC2.ButtonGroup.group: effectGroup
        Kirigami.FormData.label: i18nc("@label", "Zoom:")

        text: i18nc("@option check, enable zoom effect, following 'Zoom:'", "Full screen") // TODO: Better name?
        checked: kcm.zoomMagnifierSettings.zoom
        onToggled: { kcm.zoomMagnifierSettings.zoom = true; kcm.zoomMagnifierSettings.magnifier = false; }

        KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
    }

    ColumnLayout {
        // TODO
    }

    QQC2.RadioButton {
        QQC2.ButtonGroup.group: effectGroup

        text: i18nc("@option check, enable magnify effect, following 'Zoom:'", "Magnifier") // TODO: Better name?
        checked: kcm.zoomMagnifierSettings.magnifier
        onToggled: { kcm.zoomMagnifierSettings.zoom = false; kcm.zoomMagnifierSettings.magnifier = true; }

        KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
    }

    ColumnLayout {
        // TODO
    }

    QQC2.RadioButton {
        QQC2.ButtonGroup.group: effectGroup

        text: i18nc("@option check, disable zoom/magnify effect", "Disabled")
        checked: !(kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)
        onToggled: { kcm.zoomMagnifierSettings.zoom = false; kcm.zoomMagnifierSettings.magnifier = false; }

        KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
    }
}
