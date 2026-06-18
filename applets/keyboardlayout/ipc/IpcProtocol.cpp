/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IpcProtocol.h"

#include <QDir>
#include <QStandardPaths>

#include <unistd.h>

QDataStream &operator<<(QDataStream &out, const LayoutEntry &entry)
{
    out << entry.shortName << entry.displayName << entry.longName;
    return out;
}

QDataStream &operator>>(QDataStream &in, LayoutEntry &entry)
{
    in >> entry.shortName >> entry.displayName >> entry.longName;
    return in;
}

QString keyboardIpcSocketPath()
{
    const QString runtimeDir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (!runtimeDir.isEmpty()) {
        return QDir(runtimeDir).filePath(QStringLiteral("kded_keyboard.sock"));
    }
    return QStringLiteral("/tmp/sonic-desktop-keyboard-%1/kded_keyboard.sock").arg(::getuid());
}
