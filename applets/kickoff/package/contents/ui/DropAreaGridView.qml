/*
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

KickoffGridView {
    id: root

    function moveRow(targetIndex) {
        if (targetIndex < 0 || targetIndex >= count) {
            return;
        }

        model.moveRow(currentIndex, targetIndex);
        currentIndex = targetIndex;
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Up", "Alt+Shift+Up"]
        onActivated: if (currentIndex >= view.columns) moveRow(currentIndex - view.columns)
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Down", "Alt+Shift+Down"]
        onActivated: if (currentIndex < count - view.columns) moveRow(currentIndex + view.columns)
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Left", "Alt+Shift+Left"]
        onActivated: if (currentIndex % view.columns > 0) moveRow(currentIndex - 1)
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Right", "Alt+Shift+Right"]
        onActivated: if (currentIndex % view.columns !== view.columns - 1) moveRow(currentIndex + 1)
    }

    KickoffDropArea {
        z: -1
        parent: root
        anchors.fill: parent
        targetView: root.view
        scrollUpMargin: root.header.height * 2
        scrollDownMargin: root.footer.height * 2
    }
}
