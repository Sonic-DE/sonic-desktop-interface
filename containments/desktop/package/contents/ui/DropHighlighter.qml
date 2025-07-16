/*
 *  SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import QtQml

import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami

Item {
    id: highlighter

    property QtObject folderModel
    property QtObject positioner

    property Item gridView

    function reset(): void {
        highlighterModel.clear()
    }

    function handleDragMove(event, pos): void {
        let urls = Array(...event.mimeData.urls);
        urls.sort((first, second) => {
            return positioner.indexForUrl(first) - positioner.indexForUrl(second);
        });

        urls.forEach((url) => {
            const index = __internal.find(url);

            if (index < 0) {
                __internal.addItem(url, pos);
            } else {
                __internal.updateItem(index, pos);
            }
        });
    }

    Repeater {
        model: highlighterModel
        delegate: _cDropDelegate
    }

    ListModel {
        id: highlighterModel
    }

    Component {
        id: _cDropDelegate

        KSvg.FrameSvgItem {
            x: model.x + __internal.padding + (__internal.isRightDirection ? -gridView.contentX : 0)
            y: model.y + __internal.padding

            width: gridView.cellWidth - __internal.padding * 2
            height: gridView.cellHeight - __internal.padding * 2

            prefix: "hover"
            imagePath: "widgets/viewitem"

            opacity: index > __internal.maximumHighlights ? 0 : 1 - (index / __internal.maximumHighlights)
        }
    }

    QtObject {
        id: __internal

        readonly property int maximumHighlights: 10

        readonly property bool isRightDirection: gridView.effectiveLayoutDirection === Qt.RightToLeft

        readonly property real padding: Kirigami.Units.smallSpacing

        function calculatePosition(url, pos) {
            const offset = getOffset(url)

            const itemX = pos.x + offset.x + (gridView.cellWidth / 2);
            const itemY = pos.y + offset.y + gridView.verticalDropHitscanOffset;

            const column = Math.floor(itemX / gridView.cellWidth);
            const row = Math.floor(itemY / gridView.cellHeight);

            return {
                x: gridView.cellWidth * column,
                y: gridView.cellHeight * row,
            };
        }

        function getOffset(url): real {
            const index = positioner.indexForUrl(url);
            const offset = folderModel.dragCursorOffset(positioner.map(index));

            return offset;
        }

        function find(url): int {
            for (let index = 0; index < highlighterModel.count; index++) {
                if (highlighterModel.get(index).url === url) {
                    return index;
                }
            }

            return -1;
        }

        function addItem(url, pos): void {
            if (highlighterModel.count < maximumHighlights) {
                const position = calculatePosition(url, pos);
                highlighterModel.append({
                    "url": url,
                    "x": position.x,
                    "y": position.y,
                });
            }
        }

        function updateItem(index, pos): void {
            const currentItem = highlighterModel.get(index)
            const position = calculatePosition(currentItem.url, pos);

            if (position.x !== currentItem.x) {
                highlighterModel.setProperty(index, "x", position.x);
            }

            if (position.y !== currentItem.y) {
                highlighterModel.setProperty(index, "y", position.y);
            }
        }
    }
}
