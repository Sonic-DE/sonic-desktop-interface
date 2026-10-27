/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QObject>
#include <QString>
#include <QVector>
#include <functional>

#include "../layoutnames.h"
#include "IpcProtocol.h"

namespace Sonic::Ipc {
class Connection;
class Server;
}

class KeyboardIpcServer : public QObject
{
    Q_OBJECT

public:
    using GetLayoutsListFn = std::function<QList<LayoutNames>()>;
    using GetLayoutFn = std::function<uint()>;
    using SwitchToNextLayoutFn = std::function<void()>;
    using SwitchToPreviousLayoutFn = std::function<void()>;
    using SetLayoutFn = std::function<void(uint)>;

    //! Listens on \a path; the default is the canonical per-user socket. Tests pass their own.
    KeyboardIpcServer(QObject *parent,
                      GetLayoutsListFn getLayoutsListFn,
                      GetLayoutFn getLayoutFn,
                      SwitchToNextLayoutFn switchToNextLayoutFn,
                      SwitchToPreviousLayoutFn switchToPreviousLayoutFn,
                      SetLayoutFn setLayoutFn,
                      const QString &path = socketPath());

    void broadcastLayoutsChanged(const QList<LayoutNames> &layouts);
    void broadcastLayoutChanged(uint index);

    static QString socketPath();

private Q_SLOTS:
    void onNewConnection(Sonic::Ipc::Connection *connection);

private:
    void handleFrame(Sonic::Ipc::Connection *connection, quint32 type, const QByteArray &payload);
    void replyLayouts(Sonic::Ipc::Connection *connection);
    void replyLayout(Sonic::Ipc::Connection *connection);

    Sonic::Ipc::Server *m_server;

    GetLayoutsListFn m_getLayoutsListFn;
    GetLayoutFn m_getLayoutFn;
    SwitchToNextLayoutFn m_switchToNextLayoutFn;
    SwitchToPreviousLayoutFn m_switchToPreviousLayoutFn;
    SetLayoutFn m_setLayoutFn;
};
