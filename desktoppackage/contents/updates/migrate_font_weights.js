// Find all depicted widgets and migrate their font weights from qt5 to qt6 style

function migrateFontWeight(oldWeight) {
    // Takes old weight (Qt5 weight) and returns the Qt6 equivalent
    // Qt5 font weights: https://doc.qt.io/qt-5/qfont.html#Weight-enum
    // Qt6 font weights: https://doc.qt.io/qt-6/qfont.html#Weight-enum
    switch(oldWeight) {
        case 0:
            return 100;
        case 12:
            return 200;
        case 25:
            return 300;
        case 50:
            return 400;
        case 57:
            return 500;
        case 63:
            return 600;
        case 75:
            return 700;
        case 81:
            return 800;
        case 87:
            return 900;
        default:
            return 400;
    }
}

var containments = desktops().concat(panels());
for (var c in containments) {
    const cont = containments[c];
    const widgets = cont.widgets();
    for (var w in widgets) {
        const widget = widgets[w];
        switch(widget.type) {
            case "org.kde.plasma.digitalclock":
                widget.currentConfigGroup = new Array('Appearance');
                widget.writeConfig("fontWeight", migrateFontWeight(widget.readConfig("fontWeight")));
                break;
        }
    }
}

