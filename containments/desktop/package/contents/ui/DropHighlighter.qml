
import QtQuick
import QtQml 2.15

import org.kde.plasma.core as PlasmaCore

Item {
    id: highlighter

    property bool dropHighlight: true

    property QtObject folderModel
    property QtObject positioner

    property Item gridView

    function reset() {
        highlighterModel.clear()
    }

    function handleDragMove(event, pos) {
        if (folderModel.dragging) {
            if (!main.dropHighlight) {
                return;
            }
        }

        for (let i in event.mimeData.urls) {
            let url = event.mimeData.urls[i];
            let index = __internal.find(url)

            if (index < 0) {
                __internal.addItem(url, pos);
            } else {
                __internal.updateItem(index, pos);
            }
        }
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

        PlasmaCore.FrameSvgItem {
            x: model.x + (__internal.isRightDirection ? 3 * __internal.padding - 1 : __internal.padding)
            y: model.y + __internal.padding

            width: gridView.cellWidth - __internal.padding * 2
            height: gridView.cellHeight - __internal.padding * 2

            prefix: "hover"
            imagePath: "widgets/viewitem"

            opacity: 0.5
        }
    }

    QtObject {
        id: __internal

        readonly property bool isRightDirection: gridView.effectiveLayoutDirection === Qt.RightToLeft

        readonly property real padding: PlasmaCore.Units.smallSpacing

        function calculatePosition(url, pos) {
            let offset = getOffset(url)

            let itemX = pos.x + offset.x + (gridView.cellWidth / 2);
            let itemY = pos.y + offset.y + gridView.verticalDropHitscanOffset;

            let column = Math.floor(itemX / gridView.cellWidth);
            let row = Math.floor(itemY / gridView.cellHeight);

            return {
                x: gridView.cellWidth * column,
                y: gridView.cellHeight * row,
            };
        }

        function getOffset(url) {
            let index = positioner.indexForUrl(url);
            let offset = folderModel.dragCursorOffset(positioner.map(index));

            return offset;
        }

        function find(url) {
            for (let index = 0; index < highlighterModel.count; index++) {
                if (highlighterModel.get(index).url === url) {
                    return index;
                }
            }

            return -1;
        }

        function addItem(url, pos) {
            let position = calculatePosition(url, pos);
            highlighterModel.append({
                "url": url,
                "x": position.x,
                "y": position.y,
            });
        }

        function updateItem(index, pos) {
            let currentItem = highlighterModel.get(index)
            let position = calculatePosition(currentItem.url, pos);

            if (position.x !== currentItem.x) {
                highlighterModel.setProperty(index, "x", position.x);
            }

            if (position.y !== currentItem.y) {
                highlighterModel.setProperty(index, "y", position.y);
            }
        }
    }
}
