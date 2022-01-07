/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

MouseArea {
    id: configurationArea

    z: 1000

    anchors.fill: currentLayout

    hoverEnabled: true

    property Item currentApplet
    property int lastX
    property int lastY

    readonly property int spacerHandleSize: PlasmaCore.Units.smallSpacing
    property bool isHorizontal: plasmoid.formFactor === PlasmaCore.Types.Horizontal //TODO read this from main.qml

    onHeightChanged: tooltip.visible = false;
    onWidthChanged: tooltip.visible = false;

    onPositionChanged: {
        if (pressed) {

            // If the object has been dragged outside of the panel and there's
            // a different containment there, we remove it from the panel
            // containment and add it to the new one.
            var padding = PlasmaCore.Units.gridUnit * 5;
            if (currentApplet && (mouse.x < -padding || mouse.y < -padding ||
                mouse.x > width + padding || mouse.y > height + padding)) {
                var newCont = plasmoid.containmentAt(mouse.x, mouse.y);

                if (newCont && newCont !== plasmoid) {
                    var newPos = newCont.mapFromApplet(plasmoid, mouse.x, mouse.y);
                    var applet = currentApplet.applet;
                    currentApplet.destroy();
                    applet.anchors.fill = undefined
                    newCont.addApplet(applet, newPos.x, newPos.y);
                    return;
                }
            }

            if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                currentApplet.y += (mouse.y - lastY);
            } else {
                currentApplet.x += (mouse.x - lastX);
            }
            lastX = mouse.x;
            lastY = mouse.y;

            var item = currentLayout.childAt(mouse.x, mouse.y);

            if (item && item !== placeHolder) {
                var posInItem = mapToItem(item, mouse.x, mouse.y);
                let i = 0;

                if ((!isHorizontal && posInItem.y < item.height/3) ||
                    (isHorizontal && posInItem.x < item.width/3)) {
                    root.layoutManager.move(placeHolder.parent.i, item.i);
                    root.layoutManager.updateMargins();
                } else if ((!isHorizontal && posInItem.y > 2*item.height/3) ||
                          (isHorizontal && posInItem.x > 2*item.width/3)) {
                    root.layoutManager.move(placeHolder.parent.i, item.i+1);
                    root.layoutManager.updateMargins();
                }
            }

        } else {
            var item = currentLayout.childAt(mouse.x, mouse.y);
            if (root.dragOverlay && item) {
                root.dragOverlay.currentApplet = item;
            } else {
                root.dragOverlay.currentApplet = null;
            }
        }

        if (root.dragOverlay.currentApplet) {
            hideTimer.stop();
            tooltip.visible = true;
            tooltip.raise();
        }
    }

    onEntered: hideTimer.stop();

    onCurrentAppletChanged: {
        if (!currentApplet || !root.dragOverlay.currentApplet) {
            hideTimer.start();
            return;
        }
    }

    onPressed: {
        // Need to set currentApplet here too, to make touch selection + drag
        // with with a touchscreen, because there are no entered events in that
        // case
        let item = currentLayout.childAt(mouse.x, mouse.y);
        if (!item) {return}
        tooltip.visible = true;
        tooltip.raise();
        hideTimer.stop();

        lastX = mouse.x;
        lastY = mouse.y;
        // We set the current applet being dragged as a property of placeHolder
        // to be able to read its properties from the LayoutManager
        appletsModel.insert(item.i, {context_applet: placeHolder});
        currentApplet = appletContainerComponent.createObject(root, {applet: item.applet})
        item.applet.parent = currentApplet
        item.applet.anchors.fill = currentApplet
        root.dragOverlay.currentApplet = currentApplet
        placeHolder.dragging = currentApplet;
        currentApplet.x = item.x
        currentApplet.y = item.y
        currentApplet.width = item.width
        currentApplet.height = item.height
        //root.layoutManager.insertBefore(currentApplet, placeHolder);
        appletsModel.remove(item.i)
        currentApplet.z = 900;
    }

    onReleased: finishDragOperation()

    onCanceled: finishDragOperation()

    function finishDragOperation() {
        if (!currentApplet) {
            return;
        }

        appletsModel.insert(placeHolder.parent.i, {context_applet: currentApplet.applet})
        appletsModel.remove(placeHolder.parent.i)
        //root.layoutManager.insertBefore(placeHolder, currentApplet);
        placeHolder.parent = configurationArea;
        currentApplet.z = 1;

        root.layoutManager.save();
    }

    Item {
        id: placeHolder
        property Item dragging
        property bool busy: false
        Layout.preferredWidth: currentApplet && currentApplet.width
        Layout.preferredHeight: currentApplet && currentApplet.height
        visible: configurationArea.containsMouse
        Layout.fillWidth: currentApplet ? currentApplet.Layout.fillWidth : false
        Layout.fillHeight: currentApplet ? currentApplet.Layout.fillHeight : false
    }

    Timer {
        id: hideTimer
        interval: PlasmaCore.Units.longDuration
        onTriggered: tooltip.visible = false;
    }

    Rectangle {
        id: handle
        x: currentApplet && currentApplet.x
        y: currentApplet && currentApplet.y
        width: currentApplet && currentApplet.width
        height: currentApplet && currentApplet.height
        visible: configurationArea.containsMouse
        color: PlasmaCore.Theme.backgroundColor
        radius: 3
        opacity: currentApplet ? 0.5 : 0
        PlasmaCore.IconItem {
            source: "transform-move"
            width: Math.min(parent.width, parent.height)
            height: width
            anchors.centerIn: parent
        }
        Behavior on x {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on y {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on width {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on height {
            enabled: !configurationArea.pressed
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    PlasmaCore.Dialog {
        id: tooltip
        visualParent: currentApplet

        type: PlasmaCore.Dialog.Dock
        flags: Qt.WindowStaysOnTopHint|Qt.WindowDoesNotAcceptFocus|Qt.BypassWindowManagerHint
        location: plasmoid.location

        onVisualParentChanged: {
            if (visualParent) {
                currentApplet.applet.prepareContextualActions();
                alternativesButton.visible = currentApplet.applet.action("alternatives") && currentApplet.applet.action("alternatives").enabled;
                configureButton.visible = currentApplet.applet.action("configure") && currentApplet.applet.action("configure").enabled;
                label.text = currentApplet.applet.title;
            }
        }

        mainItem: MouseArea {
            enabled: currentApplet
            width: handleButtons.width
            height: handleButtons.height
            hoverEnabled: true
            onEntered: hideTimer.stop();
            onExited:  hideTimer.restart();

            ColumnLayout {
                id: handleButtons
                spacing: PlasmaCore.Units.smallSpacing

                PlasmaExtras.PlasmoidHeading {
                    leftPadding: PlasmaCore.Units.smallSpacing * 2
                    rightPadding: PlasmaCore.Units.smallSpacing * 2

                    contentItem: PlasmaExtras.Heading {
                        id: label
                        level: 3
                        horizontalAlignment: Text.AlignHCenter
                    }
                }

                PlasmaComponents.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the top unless it's on
                    // a top panel
                    visible: tooltip.location !== PlasmaCore.Types.TopEdge
                             && currentApplet
                             && currentApplet.applet.action("remove")
                             && currentApplet.applet.action("remove").enabled
                    iconSource: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("remove").trigger();
                    }
                }
                PlasmaComponents.ToolButton {
                    id: configureButton
                    Layout.fillWidth: true
                    iconSource: "configure"
                    text: i18n("Configure…")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("configure").trigger()
                    }
                }
                PlasmaComponents.ToolButton {
                    id: alternativesButton
                    Layout.fillWidth: true
                    iconSource: "widget-alternatives"
                    text: i18n("Show Alternatives…")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("alternatives").trigger()
                    }
                }
                PlasmaComponents.ToolButton {
                    Layout.fillWidth: true
                    // we want destructive actions to be far from the initial
                    // cursor position, so show this on the bottom for top panels
                    visible: tooltip.location === PlasmaCore.Types.TopEdge
                             && currentApplet
                             && currentApplet.applet.action("remove")
                             && currentApplet.applet.action("remove").enabled
                    iconSource: "delete"
                    text: i18n("Remove")
                    onClicked: {
                        tooltip.visible = false;
                        currentApplet.applet.action("remove").trigger();
                    }
                }
            }
        }
    }
}
