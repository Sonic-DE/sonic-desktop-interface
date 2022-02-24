/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0 as DragDrop

import "LayoutManager.js" as LayoutManager

DragDrop.DropArea {
    id: root
    width: 640
    height: 48

//BEGIN properties
    Layout.preferredWidth: fixedWidth || currentLayout.implicitWidth
    Layout.preferredHeight: fixedHeight || currentLayout.implicitHeight

    property Item toolBox
    property var layoutManager: LayoutManager

    property Item dragOverlay

    property bool isHorizontal: plasmoid.formFactor !== PlasmaCore.Types.Vertical
    property int fixedWidth: 0
    property int fixedHeight: 0
    property bool hasSpacer
    // True when a widget is being drag and dropped within the panel.
    property bool dragAndDropping: false

    // These are invisible and only used to read panel margins
    // Both will fallback to "standard" panel margins if the theme does not
    // define a normal or a thick margin.
    PlasmaCore.FrameSvgItem {
        id: panelSvg
        visible: false
        prefix: 'normal'
        imagePath: "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: 'thick'
        imagePath: "widgets/panel-background"
    }
    property bool marginAreasEnabled: panelSvg.margins != thickPanelSvg.margins
    property var marginHighlightSvg: PlasmaCore.Svg{imagePath: "widgets/margins-highlight"}
    //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.
    //the size a margin should be to force a panel to be the required size above
    readonly property real spacingAtMinSize: Math.round(Math.max(1, (currentLayout.isLayoutHorizontal ? root.height : root.width) - units.iconSizes.smallMedium)/2)

//END properties

//BEGIN functions
function addApplet(applet, x, y) {
    // don't show applet if it chooses to be hidden but still make it
    // accessible in the panelcontroller
    // Due to the nature of how "visible" propagates in QML, we need to
    // explicitly set it on the container (so the Layout ignores it)
    // as well as the applet (so it reliably knows about), otherwise it can
    // happen that an applet erroneously thinks it's visible, or suddenly
    // starts thinking that way on teardown (virtual desktop pager)
    // leading to crashes
    var new_element = {applet: applet}

    applet.visible = Qt.binding(function() {
        return applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && plasmoid.userConfiguring);
    });

    if (x >= 0 && y >= 0) {
        appletsModel.insert(LayoutManager.indexAtCoordinates(x, y), new_element)
        //var index = LayoutManager.insertAtCoordinates(container, x , y);

    // Insert icons to the left of whatever is at the center (usually a Task Manager),
    // if it exists.
    // FIXME TODO: This is a real-world fix to produce a sensible initial position for
    // launcher icons added by launcher menu applets. The basic approach has been used
    // since Plasma 1. However, "add launcher to X" is a generic-enough concept and
    // frequent-enough occurrence that we'd like to abstract it further in the future
    // and get rid of the ugliness of parties external to the containment adding applets
    // of a specific type, and the containment caring about the applet type. In a better
    // system the containment would be informed of requested launchers, and determine by
    // itself what it wants to do with that information.
    } else if (applet.pluginName === "org.kde.plasma.icon" &&
            (middle = currentLayout.childAt(root.width / 2, root.height / 2))) {
        appletsModel.insert(middle.index, new_element);
    // Fall through to determining an appropriate insert position.
    } else {
        appletsModel.append(new_element);
    }
    LayoutManager.updateMargins();
}


function checkLastSpacer() {
    var flexibleFound = false;
    for (var i = 0; i < currentLayout.children.length; ++i) {
        var applet = currentLayout.children[i].applet;
        if (!applet || !applet.visible || !applet.Layout) {
            continue;
        }
        if ((root.isHorizontal && applet.Layout.fillWidth) ||
            (!root.isHorizontal && applet.Layout.fillHeight)) {
                hasSpacer = true;
            return
        }
    }
    hasSpacer = false;
}
//END functions

//BEGIN connections
    Component.onCompleted: {
        LayoutManager.plasmoid = plasmoid;
        LayoutManager.root = root;
        LayoutManager.layout = currentLayout;
        LayoutManager.marginHighlights = [];
        LayoutManager.appletsModel = appletsModel;
        LayoutManager.restore();

        plasmoid.action("configure").visible = Qt.binding(function() {
            return !plasmoid.immutable;
        });
        plasmoid.action("configure").enabled = Qt.binding(function() {
            return !plasmoid.immutable;
        });
    }

    onDragEnter: {
        if (plasmoid.immutable) {
            event.ignore();
            return;
        }
        //during drag operations we disable panel auto resize
        root.fixedWidth = root.Layout.preferredWidth
        root.fixedHeight = root.Layout.preferredHeight
        appletsModel.insert(LayoutManager.indexAtCoordinates(event.x, event.y), {applet: dndSpacer})
    }

    onDragMove: {
        LayoutManager.move(dndSpacer.parent.index, LayoutManager.indexAtCoordinates(event.x, event.y));
    }

    onDragLeave: {
        appletsModel.remove(dndSpacer.parent.index);
        root.fixedWidth = root.fixedHeight = 0;
    }

    onDrop: {
        appletsModel.remove(dndSpacer.parent.index);
        plasmoid.processMimeData(event.mimeData, event.x, event.y);
        event.accept(event.proposedAction);
        root.fixedWidth = root.fixedHeight = 0;
    }

    Containment.onAppletAdded: {
        addApplet(applet, x, y);
        checkLastSpacer();
        LayoutManager.save();
    }

    Containment.onAppletRemoved: {
        appletsModel.remove(applet.parent.index);
        checkLastSpacer();
        LayoutManager.save();
    }

    Plasmoid.onUserConfiguringChanged: {
        if (plasmoid.immutable) {
            if (dragOverlay) {
                dragOverlay.destroy();
            }
            return;
        }

        if (plasmoid.userConfiguring) {
            for (var i = 0; i < plasmoid.applets.length; ++i) {
                plasmoid.applets[i].expanded = false;
            }

            if (!dragOverlay) {
                var component = Qt.createComponent("ConfigOverlay.qml");
                if (component.status === Component.Ready) {
                    dragOverlay = component.createObject(root);
                } else {
                    console.log("Could not create ConfigOverlay:", component.errorString());
                }
                component.destroy();
            } else {
                dragOverlay.visible = true;
            }
        } else {
            dragOverlay.destroy();
        }
    }
//END connections

//BEGIN components
    Component {
        id: appletContainerComponent
        // This loader conditionally manages the BusyIndicator, it's not
        // loading the applet. The applet becomes a regular child item.
        Loader {
            id: container
            property bool isAppletContainer: true
            visible: applet.status !== PlasmaCore.Types.HiddenStatus || (!plasmoid.immutable && plasmoid.userConfiguring);

            //when the applet moves caused by its resize, don't animate.
            //this is completely heuristic, but looks way less "jumpy"
            property bool movingForResize: false

            Layout.fillWidth: applet && applet.Layout.fillWidth
            Layout.onFillWidthChanged: {
                if (plasmoid.formFactor !== PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }
            Layout.fillHeight: applet && applet.Layout.fillHeight
            Layout.onFillHeightChanged: {
                if (plasmoid.formFactor === PlasmaCore.Types.Vertical) {
                    checkLastSpacer();
                }
            }

            function getMargins(side, returnAllMargins = false, ignoreFillArea = false, forceThickArea = false) {
                //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon + smallSpace) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.
                var layout = {
                    top: currentLayout.isLayoutHorizontal, bottom: currentLayout.isLayoutHorizontal,
                    right: !currentLayout.isLayoutHorizontal, left: !currentLayout.isLayoutHorizontal
                };
                var fillArea = applet && (applet.constraintHints & PlasmaCore.Types.CanFillArea);
                return ((layout[side] || returnAllMargins) && !fillArea || ignoreFillArea) ? Math.round(Math.min(spacingAtMinSize, ((inThickArea || forceThickArea) ? thickPanelSvg.fixedMargins[side] : panelSvg.fixedMargins[side]))) : 0;
            }

            function getLayout(layoutType) {
                let suffixes = layoutType.endsWith('Width') ? ['height', 'width', 'fillWidth'] : ['width', 'height', 'fillHeight']
                if (Layout[suffixes[2]]) {suffixes[0] = suffixes[1]};
                if (currentLayout.isLayoutHorizontal == layoutType.endsWith('Width')) {
                    return Math.round(applet && applet.Layout[layoutType] > 0 ? applet.Layout[layoutType] : root[suffixes[0]])
                } else {
                    return Math.round(root[suffixes[1]])
                }
            }

            Layout.topMargin: getMargins('top')
            Layout.bottomMargin: getMargins('bottom')
            Layout.leftMargin: getMargins('left')
            Layout.rightMargin: getMargins('right')
            Layout.minimumWidth: getLayout('minimumWidth') - Layout.leftMargin - Layout.rightMargin
            Layout.minimumHeight: getLayout('minimumHeight') - Layout.bottomMargin - Layout.topMargin
            Layout.preferredWidth: getLayout('preferredWidth') - Layout.leftMargin - Layout.rightMargin
            Layout.preferredHeight: getLayout('preferredHeight') - Layout.bottomMargin - Layout.topMargin
            Layout.maximumWidth: getLayout('maximumWidth') - Layout.leftMargin - Layout.rightMargin
            Layout.maximumHeight: getLayout('maximumHeight') - Layout.bottomMargin - Layout.topMargin

            Item {
                // index -1 is for floating applets, which do not need a margin highlight
                opacity: plasmoid.editMode && marginAreasEnabled && !root.dragAndDropping && index != -1 ? 1 : 0
                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
                id: marginHighlightElements
                anchors.fill: parent
                PlasmaCore.SvgItem {
                    svg: marginHighlightSvg
                    elementId: 'fill'
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        leftMargin: -currentLayout.rowSpacing/2
                        rightMargin: -currentLayout.rowSpacing/2
                        topMargin: -getMargins('top')
                    }
                    height: getMargins('top', false, true)
                }
                PlasmaCore.SvgItem {
                    svg: marginHighlightSvg
                    visible: ((applet.constraintHints & PlasmaCore.Types.MarginAreasSeparator) == PlasmaCore.Types.MarginAreasSeparator)
                    elementId: 'topright'
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                        leftMargin: -currentLayout.rowSpacing/2
                        rightMargin: -currentLayout.rowSpacing/2
                        topMargin: getMargins('top', false, true)
                    }
                    height: getMargins('top', false, true, true) - anchors.topMargin
                }
                PlasmaCore.SvgItem {
                    svg: marginHighlightSvg
                    elementId: 'fill'
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        leftMargin: -currentLayout.rowSpacing/2
                        rightMargin: -currentLayout.rowSpacing/2
                        bottomMargin: -getMargins('bottom')
                    }
                    height: getMargins('bottom', false, true)
                }
                PlasmaCore.SvgItem {
                    svg: marginHighlightSvg
                    visible: ((applet.constraintHints & PlasmaCore.Types.MarginAreasSeparator) == PlasmaCore.Types.MarginAreasSeparator)
                    elementId: 'bottomright'
                    anchors {
                        bottom: parent.bottom
                        left: parent.left
                        right: parent.right
                        leftMargin: -currentLayout.rowSpacing/2
                        rightMargin: -currentLayout.rowSpacing/2
                        bottomMargin: getMargins('bottom', false, true)
                    }
                    height: getMargins('bottom', false, true, true) - anchors.bottomMargin
                }
            }

            required property Item applet
            required property int index
            property bool inThickArea: false
            property Item dragging

            onAppletChanged: {
                console.log('B')
                applet.parent = container
                applet.anchors.fill = container
            }

            active: applet && applet.busy
            sourceComponent: PlasmaComponents.BusyIndicator {}

            Layout.onMinimumWidthChanged: movingForResize = true;
            Layout.onMinimumHeightChanged: movingForResize = true;
            Layout.onMaximumWidthChanged: movingForResize = true;
            Layout.onMaximumHeightChanged: movingForResize = true;
            property int oldX: x
            property int oldY: y
            function animateFrom(xa, ya) {
                if (currentLayout.isLayoutHorizontal) translation.x = xa - x;
                else translation.y = ya - y;
                translAnim.running = true
            }
            onXChanged: {
                if (movingForResize) {
                    movingForResize = false;
                    return;
                }
                if (!oldX) return;
                animateFrom(oldX, y)
                oldX = x
            }
            onYChanged: {
                if (movingForResize) {
                    movingForResize = false;
                    return;
                }
                if (!oldY) return;
                animateFrom(x, oldY)
                oldY = y
            }
            transform: Translate {
                id: translation
            }
            NumberAnimation {
                id: translAnim
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.OutCubic
                target: translation
                properties: "x,y"
                to: 0
            }
        }
    }
//END components

//BEGIN UI elements

    anchors {
        leftMargin: currentLayout.isLayoutHorizontal ? Math.min(spacingAtMinSize, panelSvg.fixedMargins.left) : 0
        rightMargin: currentLayout.isLayoutHorizontal ? Math.min(spacingAtMinSize, panelSvg.fixedMargins.right) : 0
        topMargin: currentLayout.isLayoutHorizontal ? 0 : Math.min(spacingAtMinSize, panelSvg.fixedMargins.top)
        bottomMargin: currentLayout.isLayoutHorizontal ? 0 : Math.min(spacingAtMinSize, panelSvg.fixedMargins.bottom)
    }

    Item {
        id: dndSpacer
        property bool busy: false
        Layout.preferredWidth: width
        Layout.preferredHeight: height
        width: isHorizontal ? PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 5 : currentLayout.width
        height: isHorizontal ? currentLayout.height : PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 5
    }

    ListModel {
        id: appletsModel
    }

    GridLayout {
        id: currentLayout

        Repeater {
            model: appletsModel
            delegate: appletContainerComponent
        }

        readonly property bool isLayoutHorizontal: root.isHorizontal
        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.smallSpacing

        width: root.hasSpacer || !isLayoutHorizontal ? root.width : implicitWidth
        height: root.hasSpacer || isLayoutHorizontal ? root.height: implicitHeight

        rows: isHorizontal ? 1 : currentLayout.children.length
        columns: isHorizontal ? currentLayout.children.length : 1
        flow: isHorizontal ? GridLayout.LeftToRight : GridLayout.TopToBottom
        layoutDirection: Qt.application.layoutDirection
    }
//END UI elements
}
