import QtQuick 2.12
import QtQuick.Layouts 1.2
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1
import org.kde.kirigami 2.5 as Kirigami
import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.workspace.keyboardlayout 1.0

Kirigami.FormLayout {
    property alias cfg_showFlag: showFlag.checked
    readonly property var layoutShortName: keyboardLayout.layoutsList.length ? keyboardLayout.layoutsList[keyboardLayout.layout].shortName
                                                                             : ""
    KeyboardLayout {
        id: keyboardLayout
    }


    GridLayout {
        Kirigami.FormData.label: i18n("Display style:")
        columns: 2

        RadioButton {
            checked: true
        }

        Text {
            text: layoutShortName.toUpperCase()
        }

        RadioButton {
            id: showFlag
        }

        Image {
            source: StandardPaths.locate(StandardPaths.GenericDataLocation,
                         "kf5/locale/countries/" + layoutShortName + "/flag.png")
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    Button {
        Kirigami.FormData.label: i18n("Layouts:")
        text: i18n("Configure…")
        icon.name: "configure"
        onClicked: KCMShell.openSystemSettings("kcm_keyboard", "--tab=layouts")
    }
}
