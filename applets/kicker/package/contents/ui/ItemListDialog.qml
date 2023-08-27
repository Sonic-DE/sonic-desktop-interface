/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami

import org.kde.plasma.private.kicker 0.1 as Kicker

Kicker.SubMenu {
    id: itemDialog

    property alias focusParent: itemListView.focusParent

    visible: true
    visualParent: focusParent.currentItem
    hideOnWindowDeactivate: true
    location: PlasmaCore.Types.Floating
    offset: Kirigami.Units.smallSpacing

    onWindowDeactivated: {
        kicker.expanded = false;
    }

    mainItem: ItemListView {
        id: itemListView

        width: itemListView.focusParent.minimumWidth
        height: {
            const m = itemListView.model.sourceModel;

            if (m === null || m === undefined) {
                // TODO: setting height to 0 triggers a warning in PlasmaQuick::Dialog
                return 0;
            }

            return Math.min(
                // either fit in screen boundaries (cut to the nearest item/separator boundary), ...
                __subFloorMultipleOf(
                    itemDialog.availableScreenRectForItem(itemListView).height
                    - itemDialog.margins.top
                    - itemDialog.margins.bottom,
                    itemHeight
                ) + m.separatorCount * (separatorHeight - itemHeight)
                ,
                // ...or fit the content itself -- whichever is shorter.
                ((m.count - m.separatorCount) * itemHeight)
                + (m.separatorCount * separatorHeight)
            );
        }

        // get x rounded down to the multiple of y, minus extra y.
        function __subFloorMultipleOf(x : real, y : real) : real {
            return (Math.floor(x / y) - 1) * y;
        }

        focus: true
        iconsEnabled: true
        dialog: itemDialog

        model: Kicker.FunnelModel {
            id: funnelModel
            sourceModel: itemListView.focusParent.model.modelForRow(itemListView.focusParent.currentIndex)
        }
    }
}
