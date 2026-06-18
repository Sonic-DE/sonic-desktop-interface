/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QList>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>
#include <functional>

#include "../layoutnames.h"
#include "IpcProtocol.h"

class KeyboardIpcServer : public QObject
{
    Q_OBJECT

public:
    using GetLayoutsListFn = std::function<QList<LayoutNames>()>;
    using GetLayoutFn = std::function<uint()>;
    using SwitchToNextLayoutFn = std::function<void()>;
    using SwitchToPreviousLayoutFn = std::function<void()>;
    using SetLayoutFn = std::function<void(uint)>;

    KeyboardIpcServer(QObject *parent,
                      GetLayoutsListFn getLayoutsListFn,
                      GetLayoutFn getLayoutFn,
                      SwitchToNextLayoutFn switchToNextLayoutFn,
                      SwitchToPreviousLayoutFn switchToPreviousLayoutFn,
                      SetLayoutFn setLayoutFn);

    void broadcastLayoutsChanged(const QList<LayoutNames> &layouts);
    void broadcastLayoutChanged(uint index);

    static QString socketPath();

private Q_SLOTS:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QLocalSocket::LocalSocketError socketError);

private:
    void sendToAll(const QByteArray &payload);
    void sendToSocket(QLocalSocket *socket, const QByteArray &payload);
    void handleFrame(QLocalSocket *socket, QByteArray &payload);
    void replyHello(QLocalSocket *socket);
    void replyLayouts(QLocalSocket *socket);
    void replyLayout(QLocalSocket *socket);

    QLocalServer *m_server;
    QSet<QLocalSocket *> m_clients;

    GetLayoutsListFn m_getLayoutsListFn;
    GetLayoutFn m_getLayoutFn;
    SwitchToNextLayoutFn m_switchToNextLayoutFn;
    SwitchToPreviousLayoutFn m_switchToPreviousLayoutFn;
    SetLayoutFn m_setLayoutFn;
};
