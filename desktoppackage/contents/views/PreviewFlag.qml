/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami 2.20 as Kirigami

Column {
    spacing: Kirigami.Units.smallSpacing
    opacity: 0.8

    Text {
        text: desktop.previewFlagTitle
        font.pixelSize: Kirigami.Theme.defaultFont.pixelSize
        style: Text.Outline
        styleColor: "white"
    }

    Text {
        text: desktop.previewFlagText
        font.pixelSize: Kirigami.Theme.defaultFont.pixelSize
        style: Text.Outline
        styleColor: "white"
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: Qt.openUrlExternally("https://bugs.kde.org/")
    }
}