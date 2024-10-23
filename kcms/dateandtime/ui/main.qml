/*
 *   SPDX-FileCopyrightText: 2024 Niccolò Venerandi <niccolo@venerandi.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import QtQuick.Dialogs
import QtQuick.Controls

import QtPositioning
import QtLocation
import QtCore

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigamiaddons.components as KirigamiComponents

KirigamiComponents.TimezoneSelector {
    selectedTimeZone: DTime.selectedTimeZone

    onSelectedTimeZoneChanged: {
        DTime.selectedTimeZone = selectedTimeZone
    }
}
