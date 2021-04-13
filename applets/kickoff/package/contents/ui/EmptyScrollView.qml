/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.16 as Kirigami

/**
 * This is meant to be a very basic ScrollView that behaves like most ScrollViews do,
 * but inherits no externally defined content or behavior, except for the PC3 ScrollBars.
 */
T.ScrollView {
    id: root

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    leftPadding: {
        if (mirrored && PC3.ScrollBar.vertical.visible) {
            return PC3.ScrollBar.vertical.width// + horizontalPadding
        } else {
            return horizontalPadding
        }
    }
    rightPadding: {
        if (!mirrored && PC3.ScrollBar.vertical.visible) {
            return PC3.ScrollBar.vertical.width// + horizontalPadding
        } else {
            return horizontalPadding
        }
    }

    PC3.ScrollBar.vertical: PC3.ScrollBar {
        visible: size < 1 && policy !== PC3.ScrollBar.AlwaysOff
        parent: root
        x: root.mirrored ? 0 : root.width - width
        y: root.topPadding
        height: root.availableHeight
    }

    Kirigami.WheelHandler {
        target: root.contentItem
    }
}
