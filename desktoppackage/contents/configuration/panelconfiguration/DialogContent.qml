/*
    SPDX-FileCopyrightText: 2023 Niccolò Venerandi <niccolo.venerandi@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.15 as QQC2

StackView {
    id: stackView
    height: initialItem.implicitHeight

    initialItem: MainPage {}
}
