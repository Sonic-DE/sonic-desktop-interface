/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Shapes

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm as KCM
import org.kde.kcmutils
import org.kde.kquickcontrols

Item {
    id: root

    implicitHeight: parent.height

    Kirigami.PlaceholderMessage {
        text: i18nd("kcm_tablet", "No pad inputs found")
        explanation: i18n("There's no buttons or other inputs detected. This could mean you're missing a driver for this device.")
        anchors.centerIn: parent
        width: parent.width - (Kirigami.Units.largeSpacing * 4)
    }
}
