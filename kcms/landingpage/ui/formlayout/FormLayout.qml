import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import org.kde.kirigami as Kirigami

Item {
    id: root
    default property alias entries: layout.data
    Accessible.role: Accessible.Form

    implicitWidth: layout.implicitWidth + Kirigami.Units.largeSpacing * 2
    implicitHeight: layout.implicitHeight + Kirigami.Units.largeSpacing * 2

    ColumnLayout {
        id: layout
        property real labelWidth: 0
        onImplicitWidthChanged: {
            let w = 0;
            for (let entry of children) {
                w = Math.max(w, entry?.leftPadding ?? 0);
            }
            labelWidth = w;
        }
        anchors {
            centerIn: parent
            horizontalCenterOffset: labelWidth/2
        }
        width: Math.min(implicitWidth, parent.width) - labelWidth
        spacing: Kirigami.Units.largeSpacing
    }
}
