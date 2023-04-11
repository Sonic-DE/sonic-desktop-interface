/*
    SPDX-FileCopyrightText: 2019 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick.Controls 2.15 as QQC2

QQC2.Action {
    property string category
    checked: window.pageStack.get(0).title === text
    text: i18ndc("org.kde.plasma.emojier", "Emoji Category", category)

    onTriggered: {
        window.pageStack.replace("qrc:/ui/CategoryPage.qml", {title: text, category: category, model: emoji })
    }
}
