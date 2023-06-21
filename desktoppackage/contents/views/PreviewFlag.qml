/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

Column {
    spacing: Kirigami.Units.smallSpacing
    opacity: 0.5

    Text {
        text: "Report Bugs"
        font.pixelSize: 24
        style: Text.Outline
        styleColor: "white"
    }

    Text {
        text: `Go to https://bugs.kde.org to report Plasma 6 bugs`
        font.pixelSize: 16
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