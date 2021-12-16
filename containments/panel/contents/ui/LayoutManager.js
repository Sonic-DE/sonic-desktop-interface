/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Niccolò Venerandi <niccolo@venerandi.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

var layout;
var root;
var plasmoid;
var marginHighlights;
var appletsModel;


function restore() {
    var configString = String(plasmoid.configuration.AppletOrder)

    //array, a cell for encoded item order
    var itemsArray = configString.split(";");

    //map applet id->order in panel
    var idsOrder = new Object();
    //map order in panel -> applet pointer
    var appletsOrder = new Object();

    for (var i = 0; i < itemsArray.length; i++) {
        //property name: applet id
        //property value: order
        idsOrder[itemsArray[i]] = i;
    }

    for (var i = 0; i < plasmoid.applets.length; ++i) {
        if (idsOrder[plasmoid.applets[i].id] !== undefined) {
            appletsOrder[idsOrder[plasmoid.applets[i].id]] = plasmoid.applets[i];
        //ones that weren't saved in AppletOrder go to the end
        } else {
            appletsOrder["unordered"+i] = plasmoid.applets[i];
        }
    }

    //finally, restore the applets in the correct order
    for (var i in appletsOrder) {
        root.addApplet(appletsOrder[i], -1, -1)
    }
    //rewrite, so if in the orders there were now invalid ids or if some were missing creates a correct list instead
    save();
}

function save() {
    var ids = new Array();
    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];

        if (child.applet) {
            ids.push(child.applet.id);
        }
    }
    plasmoid.configuration.AppletOrder = ids.join(';');
    updateMargins();
}

function indexAtCoordinates(x, y) {
    if (root.isHorizontal) {
        y = layout.height / 2;
    } else {
        x = layout.width / 2;
    }
    var child = undefined;
    while (!child) {
        if (root.isHorizontal) {
            // Only yields incorrect results for widgets smaller than the
            // row/column spacing, which is luckly fairly unrealistic
            x -= layout.rowSpacing
        } else {
            y -= layout.columnSpacing
        }
        child = layout.childAt(x, y);
    }

    if ((plasmoid.formFactor === 3 && y < child.y + child.height/2) ||
        (plasmoid.formFactor !== 3 && x < child.x + child.width/2)) {
        return child.i;
    } else {
        return child.i+1;
    }
}

function updateMargins() {
    for (var i = 0; i < marginHighlights.length; ++i) {
        marginHighlights[i].destroy();
    }
    marginHighlights = [];
    var inThickArea = false;
    var startApplet = undefined;
    for (var i = 0; i < layout.children.length; ++i) {
        var child = layout.children[i];
        if (child.dragging) {child = child.dragging}
        if (child.applet) {
            child.inThickArea = inThickArea;
            if ((child.applet.constraintHints & PlasmaCore.Types.MarginAreasSeparator) == PlasmaCore.Types.MarginAreasSeparator) {
                var marginRect = rectHighlightEl.createObject(root, {
                    startApplet: startApplet,
                    endApplet: child,
                    thickArea: inThickArea
                });
                marginHighlights.push(marginRect);
                var startApplet = child;
                inThickArea = !inThickArea;
            }
        }
    }
    if (marginHighlights.length == 0) return;
    var marginRect = rectHighlightEl.createObject(root, {
        startApplet: startApplet,
        endApplet: undefined,
        thickArea: inThickArea
    });
    marginHighlights.push(marginRect);
}
