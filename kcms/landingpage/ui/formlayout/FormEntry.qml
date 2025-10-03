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

    property real leftPadding: label.width

    implicitWidth: impl.implicitWidth
    implicitHeight: impl.implicitHeight

    Layout.fillWidth: true

    signal clicked

    QQC.Label {
        id: label
        anchors {
            top: parent.top
            right: parent.left
            topMargin: root.contentItem.Kirigami.FormData.buddyFor.height/2 - label.height/2 + impl.padding
        }
        visible: text.length > 0
        Kirigami.MnemonicData.enabled: {
                const buddy = root.contentItem?.Kirigami.FormData.buddyFor;
                if (buddy && buddy.enabled && buddy.visible && buddy.activeFocusOnTab) {
                    // Only set mnemonic if the buddy doesn't already have one.
                    const buddyMnemonic = buddy.Kirigami.MnemonicData;
                    return !buddyMnemonic.label || !buddyMnemonic.enabled;
                } else {
                    return false;
                }
            }
        Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.FormLabel
        Kirigami.MnemonicData.label: root.title
        text: Kirigami.MnemonicData.richTextLabel
        Accessible.name: Kirigami.MnemonicData.plainTextLabel
        Shortcut {
            sequence: label.Kirigami.MnemonicData.sequence
            onActivated: {
                const buddy = root.contentItem?.Kirigami.FormData.buddyFor;

                const buttonBuddy = buddy as T.AbstractButton;
                // animateClick is only in Qt 6.8,
                // it also takes into account focus policy.
                if (buttonBuddy && buttonBuddy.animateClick) {
                    buttonBuddy.animateClick();
                } else {
                    buddy.forceActiveFocus(Qt.ShortcutFocusReason);
                }
            }
        }
    }

    T.ItemDelegate {
        id: impl
        anchors.fill: parent
        implicitWidth: layout.implicitWidth + padding * 2
        implicitHeight: layout.implicitHeight + padding * 2
        padding: Kirigami.Units.mediumSpacing

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
        visible: false
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            leftMargin: Kirigami.Units.largeSpacing
            rightMargin: Kirigami.Units.largeSpacing
        }
    }
}
