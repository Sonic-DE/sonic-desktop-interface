import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import org.kde.kirigami as Kirigami

Item {
    id: root

    property string title
    default property alias entries: innerLayout.data
    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    property real leftPadding: innerLayout.labelWidth
    Layout.fillWidth: true

    Kirigami.Separator {
        id: separator
        visible: root.y > 0 && title.length === 0
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            leftMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
        }
    }

    Kirigami.HeaderFooterLayout {
        id: layout
        anchors.fill: parent
        spacing: Kirigami.Units.smallSpacing

        header: Kirigami.Heading {
            level: 5
            horizontalAlignment: Text.AlignHCenter
            font.weight: Font.DemiBold
            text: root.title
        }
        contentItem: Item {
            implicitWidth: innerLayout.implicitWidth + innerLayout.labelWidth
            implicitHeight: innerLayout.implicitHeight
            ColumnLayout {
                id: innerLayout
                anchors {
                    fill: parent
                   // leftMargin: labelWidth
                }
                property real labelWidth: 0
                onImplicitWidthChanged: {
                    let w = 0;
                    for (let entry of children) {
                        w = Math.max(w, entry?.leftPadding ?? 0);
                    }
                    labelWidth = w;
                }
                spacing: 0
            }
        }
    }
}
