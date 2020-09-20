/*
    Copyright 2020  Devin Lin <espidev@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.6
import QtQuick.Dialogs 1.1
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5 as QQC2

import org.kde.kirigami 2.8 as Kirigami

Kirigami.OverlaySheet {
    id: fingerprintRoot

    property var account
    property var fingerprintModel: kcm.fingerprintModel
    
    function openAndClear() {
        fingerprintModel.switchUser(account.username);
        this.open();
    }

    header: Kirigami.Heading {
        text: i18n("Fingerprint")
    }

    ColumnLayout {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 16

        QQC2.Label {
            visible: fingerprintModel.currentError !== ""
            text: fingerprintModel.currentError
        }
        
        ColumnLayout {
            id: fingerprints
            spacing: Kirigami.Units.smallSpacing
            visible: !fingerprintModel.currentlyEnrolling
            Layout.alignment: Qt.AlignHCenter
            
            ListView {
                model: fingerprintModel.fingerprints
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                Layout.rightMargin: Kirigami.Units.smallSpacing

                delegate: Kirigami.SwipeListItem {
                    contentItem: RowLayout {
                        Kirigami.Icon {
                            source: "fingerprint-gui"
                            height: Kirigami.Units.iconSizes.medium
                            width: Kirigami.Units.iconSizes.medium
                        }
                        QQC2.Label {
                            Layout.fillWidth: true
                            text: modelData
                        }
                    }
                }
            }
            
            QQC2.Button {
                text: i18n("Clear Fingerprints")
                enabled: fingerprintModel.fingerprints.size() !== 0
                Layout.alignment: Qt.AlignLeft
                onClicked: fingerprintModel.clearFingerprints();
            }
            QQC2.Button {
                text: i18n("Add Fingerprint")
                Layout.alignment: Qt.AlignRight
                onClicked: fingerprintModel.startEnrolling("left-index-finger");
            }
        }
        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            visible: fingerprintModel.currentlyEnrolling
            Layout.alignment: Qt.AlignHCenter  
            
            QQC2.Label {
                text: fingerprintModel.enrollFeedback
            }
        }
    }
}

