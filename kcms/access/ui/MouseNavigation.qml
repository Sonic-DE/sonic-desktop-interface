/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

Kirigami.FormLayout {
    RowLayout {
        Kirigami.FormData.label:  i18n("Use number pad to move cursor:")
        QQC2.CheckBox {
            id: mouseKeys

            text: i18n("Enable")

            KCM.SettingStateBinding {
                configObject: kcm.mouseSettings
                settingName: "MouseKeys"
            }

            checked: kcm.mouseSettings.mouseKeys
            onToggled: kcm.mouseSettings.mouseKeys = checked
        }
        Kirigami.ContextualHelpButton {
            toolTipText: i18n("The numpad key 5 functions as a mouse click. The keys 2, 4, 6, and 8 allow for cardinal movement (down, left, right, and up). The keys 1, 3, 7, and 9 allow for diagonal movement.")
        }
    }

    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration delay:")

        from: 1
        to: 490

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationDelay"
        }

        value: kcm.mouseSettings.accelerationDelay
        onValueChanged: kcm.mouseSettings.accelerationDelay = value
        textFromValue: function(value) { return value + " ms" }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Repeat interval:")

        from: 1
        to: 130

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "RepetitionInterval"
        }

        value: kcm.mouseSettings.repetitionInterval
        onValueChanged: kcm.mouseSettings.repetitionInterval = value
        textFromValue: function(value) { return value + " ms" }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration time:")

        from: 1
        to: 100

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "AccelerationTime"
        }

        value: kcm.mouseSettings.accelerationTime
        onValueChanged: kcm.mouseSettings.accelerationTime = value
        textFromValue: function(value) { return value + " ms" }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label:  i18n("Maximum speed:")

        from: 1
        to: 100

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "MaxSpeed"
        }

        value: kcm.mouseSettings.maxSpeed
        onValueChanged: kcm.mouseSettings.maxSpeed = value
        textFromValue: function(value) { return value + " ms" }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Pointer acceleration:")

        from: -1000
        to: 5000

        KCM.SettingStateBinding {
            configObject: kcm.mouseSettings
            settingName: "ProfileCurve"
        }

        value: kcm.mouseSettings.profileCurve
        onValueChanged: kcm.mouseSettings.profileCurve = value
        textFromValue: function(value) { return value + " ms" }
    }
}
