/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>
    SPDX-FileCopyrightText: 2024 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQuick.Controls 2.5

ShapeCanvas {
    id: root

    property QtObject key
    property variant keyColor: key.pressed ? activePalette.highlight : activePalette.button
    property variant labelColor: key.pressed ? activePalette.highlightedText : activePalette.buttonText

    shape: key ? key.shape : null
    strokeSyle: activePalette.shadow
    fillStyle: keyColor

    onKeyColorChanged: requestPaint()

    KeyCap {
        key: parent.key

        anchors.fill: parent
        anchors.margins: 22 // arbitrary spacing to key outlines
    }

    Component.onCompleted: {
        if (!parent || !parent.row) {
            // There's implicit layout logic below when used inside a row.
            // Key may also be used standalone, so skip the layout bits.
            return;
        }

        const siblings = parent.children.filter(item => item !== this);
        if (parent.row.orientation === Qt.Horizontal) {
            const previousX = Math.max(...siblings.map(item => item.x + item.width),  0);
            x = previousX + key.gap;
            y = shape.bounds.y;
        } else {
            const previousY = Math.max(...siblings.map(item => item.y + item.height), 0);
            y = previousY + key.gap;
            x = shape.bounds.y;
        }
    }
}
