/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2005 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cursortheme.h"

#include "messagehandler.h"
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QScopeGuard>

static void kapplymousethemeMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    messageHandler(type, QStringLiteral("KAPPLYMOUSETHEME"), msg);
}
int main(int argc, char *argv[])
{
    int ret = 0;
    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication app(argc, argv);
    const auto previousMessageHandler = qInstallMessageHandler(kapplymousethemeMessageHandler);
    // Restore the handler before app teardown and static plugin destruction:
    // those can log after messageHandler's cached QString has been destroyed.
    const auto restoreMessageHandler = qScopeGuard([previousMessageHandler] {
        qInstallMessageHandler(previousMessageHandler);
    });
    if (argc != 3)
        return 1;
    QString theme = QFile::decodeName(argv[1]);
    QString size = QFile::decodeName(argv[2]);

    // Note: If you update this code, update kcm.cpp as well.

    CursorTheme::applyCursorTheme(theme, size.toInt());

    return ret;
}
