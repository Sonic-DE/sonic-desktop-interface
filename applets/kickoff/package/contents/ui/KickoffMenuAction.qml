
import QtQml.Models 2.15
import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import QtQml 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PC2
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.kicker 0.1 as Kicker

QtObject {
    id: action
    // PC2.MenuItem properties
    property QtObject parent: null
    property bool separator: false
    property bool section: false
    property string text: ""
    property var icon: null
    property bool checkable: false
    property bool checked: false

    

    // Relevant Item properties
    property bool enabled: true

    // So it can be used like an action
    function trigger(source) {
        triggered(source)
    }
    signal triggered(QtObject source)

    property string iconText: ""
    property string toolTip: ""
    property string statusTip: ""
    property string whatsThis: ""
    property var font: Qt.application.font
    property var shortcut: null
    property int shortcutContext: 0
    property bool autoRepeat: false
    property bool visible: true
    property int menuRole: 0
    property bool iconVisibleInMenu: true
    property bool shortcutVisibleInContextMenu: Qt.styleHints.showShortcutsInContextMenus
    property int priority: 0

    property var data: null

    
    property string description: ""
    
}
