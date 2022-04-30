/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.14 as Kirigami
import org.kde.kitemmodels 1.0 as KItemModels
import org.kde.plasma.configuration 2.0

Rectangle {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 40
    implicitHeight: Kirigami.Units.gridUnit * 30

    Layout.minimumWidth: Kirigami.Units.gridUnit * 30
    Layout.minimumHeight: Kirigami.Units.gridUnit * 21

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    color: Kirigami.Theme.backgroundColor

    property bool isContainment: false

    property ConfigModel globalConfigModel:  globalAppletConfigModel

    ConfigModel {
        id: globalAppletConfigModel
        ConfigCategory {
            name: i18nd("plasma_shell_org.kde.plasma.desktop", "Keyboard Shortcuts")
            icon: "preferences-desktop-keyboard"
            source: "ConfigurationShortcuts.qml"
        }
    }

    KItemModels.KSortFilterProxyModel {
        id: configDialogFilterModel
        sourceModel: configDialog.configModel
        filterRowCallback: (row, parent) => {
            return sourceModel.data(sourceModel.index(row, 0), ConfigModel.VisibleRole);
        }
    }

    function settingValueChanged() {
        applyButton.enabled = true;
    }

    function open(item) {
        appLoader.isAboutPage = false;
        if (item.source) {
            appLoader.isAboutPage = item.source === "AboutPlugin.qml";
            appLoader.setSource(Qt.resolvedUrl("ConfigurationAppletPage.qml"), {configItem: item});
        } else if (item.kcm) {
            appLoader.setSource(Qt.resolvedUrl("ConfigurationKcmPage.qml"), {kcm: item.kcm, internalPage: item.kcm.mainUi});
        } else {
            appLoader.source = "";
        }

        applyButton.enabled = false
    }

    Connections {
        target: appLoader.item

        function onSettingValueChanged() {
            applyButton.enabled = true;
        }
    }

    Component.onCompleted: {
        // if we are a containment then the first item will be ConfigurationContainmentAppearance
        // if the applet does not have own configs then the first item will be Shortcuts
        if (isContainment || !configDialog.configModel || configDialog.configModel.count === 0) {
            open(root.globalConfigModel.get(0))
        } else {
            open(configDialog.configModel.get(0))
        }
    }

    function applicationWindow() {
        return app;
    }


    QQC2.ScrollView {
        id: categoriesScroll
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
        width: Kirigami.Units.gridUnit * 7
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0
        focus: true
        Accessible.role: Accessible.MenuBar
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
        }

        ColumnLayout {
            id: categories

            spacing: 0
            width: categoriesScroll.width
            focus: true

            Keys.onUpPressed: {
                const buttons = categories.children

                let foundPrevious = false
                for (let i = buttons.length - 1; i >= 0; --i) {
                    const button = buttons[i];
                    if (!button.hasOwnProperty("highlighted")) {
                        // not a ConfigCategoryDelegate
                        continue;
                    }

                    if (foundPrevious) {
                        categories.openCategory(button.item)
                        return
                    } else if (button.highlighted) {
                        foundPrevious = true
                    }
                }
            }

            Keys.onDownPressed: {
                const buttons = categories.children

                let foundNext = false
                for (let i = 0, length = buttons.length; i < length; ++i) {
                    const button = buttons[i];
                    if (!button.hasOwnProperty("highlighted")) {
                        continue;
                    }

                    if (foundNext) {
                        categories.openCategory(button.item)
                        return
                    } else if (button.highlighted) {
                        foundNext = true
                    }
                }
            }

            function openCategory(item) {
                if (applyButton.enabled) {
                    messageDialog.item = item;
                    messageDialog.open();
                    return;
                }
                open(item)
            }

            Component {
                id: categoryDelegate
                ConfigCategoryDelegate {
                    id: delegate
                    onActivated: categories.openCategory(model);
                    highlighted: {
                        if ( appLoader.item ){
                            if (model.kcm && appLoader.item.kcm) {
                                return model.kcm == appLoader.item.kcm
                            } else if (appLoader.item.configItem) {
                                return model.source == appLoader.item.configItem.source
                            } else {
                                return appLoader.item.source == Qt.resolvedUrl(model.source)
                            }
                        }
                        return false
                    }
                    item: model
                }
            }

            Repeater {
                Layout.fillWidth: true
                model: root.isContainment ? globalConfigModel : undefined
                delegate: categoryDelegate
            }
            Repeater {
                Layout.fillWidth: true
                model: configDialogFilterModel
                delegate: categoryDelegate
            }
            Repeater {
                Layout.fillWidth: true
                model: !root.isContainment ? globalConfigModel : undefined
                delegate: categoryDelegate
            }
            Repeater {
                Layout.fillWidth: true
                model: ConfigModel {
                    ConfigCategory{
                        name: i18nd("plasma_shell_org.kde.plasma.desktop", "About")
                        icon: "help-about"
                        source: "AboutPlugin.qml"
                    }
                }
                delegate: categoryDelegate
            }
        }
    }

    Kirigami.Separator {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        z: 1
    }
    Kirigami.Separator {
        anchors {
            right: categoriesScroll.right
            top: parent.top
            bottom: parent.bottom
        }
        z: 1
    }

    QQC2.Page {
        id: page
        anchors {
            left: categoriesScroll.right
            top: parent.top
            right: parent.right
            bottom: parent.bottom
        }

        header: QQC2.Pane {
            padding: Kirigami.Units.largeSpacing
            contentItem: Kirigami.Heading {
                id: heading
                level: 1
                maximumLineCount: 1
                elide: Text.ElideRight
                text: page.title
                textFormat: Text.PlainText
            }
            background: Item {
                Kirigami.Separator {
                    id: topSeparator
                    visible: appLoader.item
                        && appLoader.item.flickable
                        && !(appLoader.item.flickable.atYBeginning
                        && appLoader.item.flickable.atYEnd)
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.bottom
                    }
                }
            }
        }

        title: appLoader.item ? appLoader.item.title : ""

        Loader {
            id: appLoader
            anchors.fill: parent

            property bool isAboutPage: false
        }

        MessageDialog {
            id: messageDialog
            icon: StandardIcon.Warning
            property var item
            title: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply Settings")
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "The settings of the current module have changed. Do you want to apply the changes or discard them?")
            standardButtons: StandardButton.Apply | StandardButton.Discard | StandardButton.Cancel
            onApply: {
                applyAction.trigger()
                root.open(item)
            }
            onDiscard: {
                root.open(item)
            }
        }

        footer: QQC2.Pane {

            padding: Kirigami.Units.largeSpacing

            contentItem: RowLayout {
                id: buttonsRow
                spacing: Kirigami.Units.smallSpacing

                Item {
                    Layout.fillWidth: true
                }

                QQC2.Button {
                    icon.name: "dialog-ok"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "OK")
                    onClicked: acceptAction.trigger()
                    KeyNavigation.tab: categories
                }
                QQC2.Button {
                    id: applyButton
                    enabled: false
                    icon.name: "dialog-ok-apply"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Apply")
                    visible: !appLoader.isAboutPage && appLoader.item && (!appLoader.item.kcm || appLoader.item.kcm.buttons & 4) // 4 = Apply button
                    onClicked: applyAction.trigger()
                }
                QQC2.Button {
                    icon.name: "dialog-cancel"
                    text: i18nd("plasma_shell_org.kde.plasma.desktop", "Cancel")
                    onClicked: cancelAction.trigger()
                    visible: !appLoader.isAboutPage
                }
            }
            background: Item {
                Kirigami.Separator {
                    id: bottomSeparator
                    visible: appLoader.item
                        && appLoader.item.flickable
                        && !(appLoader.item.flickable.atYBeginning
                        && appLoader.item.flickable.atYEnd)
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                    }
                }
            }
        }

        QQC2.Action {
            id: acceptAction
            onTriggered: {
                applyAction.trigger();
                configDialog.close();
            }
            shortcut: "Return"
        }

        QQC2.Action {
            id: applyAction
            onTriggered: {
                appLoader.item.saveConfig()

                applyButton.enabled = false;
            }
        }

        QQC2.Action {
            id: cancelAction
            onTriggered: configDialog.close();
            shortcut: "Escape"
        }
    }
}
