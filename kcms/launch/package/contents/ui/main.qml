/*
   Copyright (c) 2017 Eike Hein <hein@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0 as QtControls
import QtQuick.Dialogs 1.2 as QtDialogs
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.0

Kirigami.ScrollablePage {
    id: root

    ConfigModule.quickHelp: i18n("Launch Feedback")

    Kirigami.FormLayout {
        id: formLayout

        QtControls.ComboBox {
            id: busyCursor

            Kirigami.FormData.label: i18n("Busy Cursor")

            model: [i18n("No Busy Cursor"), i18n("Passive Busy Cursor"), i18n("Blinking Cursor"), i18n("Bouncing Cursor")]

            currentIndex: kcm.busyCursorCurrentIndex
            onCurrentIndexChanged: kcm.busyCursorCurrentIndex = currentIndex
        }

        RowLayout {
            QtControls.Label {
                enabled: busyCursor.currentIndex > 0

                text: i18n("Startup indication timeout:")
            }

            QtControls.SpinBox {
                stepSize: 1

                enabled: busyCursor.currentIndex > 0

                value: kcm.busyCursorTimeout
                onValueChanged: kcm.busyCursorTimeout = value

                textFromValue: function(value, locale) { return i18np("%1 sec", "%1 secs", value)}
            }
        }

        QtControls.CheckBox {
            id: taskManagerNotification

            Kirigami.FormData.label: i18n("Task Manager Notification")

            text: i18n("Enable Task Manager notification")

            checked: kcm.taskManagerNotification
            onCheckedChanged: kcm.taskManagerNotification = checked
        }

        RowLayout {
            QtControls.Label {
                enabled: taskManagerNotification.checked

                text: i18n("Startup indication timeout:")
            }

            QtControls.SpinBox {
                stepSize: 1

                enabled: taskManagerNotification.checked

                value: kcm.taskManagerNotificationTimeout
                onValueChanged: kcm.taskManagerNotificationTimeout = value

                textFromValue: function(value, locale) { return i18np("%1 sec", "%1 secs", value)}
            }
        }
    }
}

