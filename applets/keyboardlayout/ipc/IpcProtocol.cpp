/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IpcProtocol.h"

#include <Sonic/Ipc/Server>

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
    return Sonic::Ipc::Server::userSocketPath(QStringLiteral("kded_keyboard.sock"));
}
