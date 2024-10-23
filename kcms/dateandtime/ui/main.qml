/*
 *   SPDX-FileCopyrightText: 2024 Niccolò Venerandi <niccolo@venerandi.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigamiaddons.components as KirigamiComponents

KirigamiComponents.TimezoneSelector {
    selectedTimeZone: DTime.selectedTimeZone

    onSelectedTimeZoneChanged: {
        DTime.selectedTimeZone = selectedTimeZone
    }
}
