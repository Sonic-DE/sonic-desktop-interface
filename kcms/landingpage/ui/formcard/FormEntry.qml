import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FC

Item {
    id: root

    property string title
    property string subtitle
    property alias separatorVisible: separator.visible

    property alias contentItem: layout.contentItem
    property alias background: impl.background
    property alias footer: layout.footer

    implicitWidth: impl.implicitWidth
    implicitHeight: impl.implicitHeight

    Layout.fillWidth: true

    signal clicked

    T.ItemDelegate {
        id: impl
        anchors.fill: parent
        implicitWidth: layout.implicitWidth + padding * 2
        implicitHeight: layout.implicitHeight + padding * 2
        padding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

        onClicked: {
            print(root.parent.children[root.parent.children.indexOf(root) + 1])
            root.clicked
            root.contentItem.forceActiveFocus()
            if (root.contentItem instanceof T.AbstractButton) {
                root.contentItem.click()
            } else if (root.contentItem instanceof T.ComboBox) {
                root.contentItem.popup.open()
            }
        }

        contentItem: Kirigami.HeaderFooterLayout {
            id: layout
            spacing: Kirigami.Units.smallSpacing

            header: QQC.Label {
                visible: text.length > 0
                text: root.title
            }
            footer: QQC.Label {
                visible: text.length > 0
                text: root.subtitle
                opacity: 0.6
            }
        }
    }

    // TODO: this should be a property in the template
    Kirigami.Separator {
        id: separator
        opacity: 0.5
        visible: !(root.contentItem instanceof QQC.RadioButton) || !(root.parent.children[root.parent.children.indexOf(root) + 1].contentItem instanceof QQC.RadioButton)
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
        }
    }
}
