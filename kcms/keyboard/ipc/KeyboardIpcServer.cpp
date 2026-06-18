/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KeyboardIpcServer.h"

#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QLoggingCategory>

#include "debug.h"

KeyboardIpcServer::KeyboardIpcServer(QObject *parent,
                                     GetLayoutsListFn getLayoutsListFn,
                                     GetLayoutFn getLayoutFn,
                                     SwitchToNextLayoutFn switchToNextLayoutFn,
                                     SwitchToPreviousLayoutFn switchToPreviousLayoutFn,
                                     SetLayoutFn setLayoutFn)
    : QObject(parent)
    , m_server(new QLocalServer(this))
    , m_getLayoutsListFn(std::move(getLayoutsListFn))
    , m_getLayoutFn(std::move(getLayoutFn))
    , m_switchToNextLayoutFn(std::move(switchToNextLayoutFn))
    , m_switchToPreviousLayoutFn(std::move(switchToPreviousLayoutFn))
    , m_setLayoutFn(std::move(setLayoutFn))
{
    const QString path = socketPath();
    // If the canonical path lives under /tmp (XDG_RUNTIME_DIR was
    // unset), make sure the per-uid directory exists with mode 0700.
    if (path.startsWith(QStringLiteral("/tmp/"))) {
        QDir d(path);
        d.cdUp();
        if (!d.exists() && !d.mkpath(QStringLiteral("."))) {
            qCWarning(KCM_KEYBOARD) << "Failed to create IPC socket directory" << d.absolutePath();
            return;
        }
        // mkpath respects the process umask; we want 0700 for the
        // per-user fallback directory. Set it after creation.
        QFile::setPermissions(d.absolutePath(), QFile::Permissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner));
    }

    // Remove any stale socket file left behind by a previous (crashed)
    // daemon instance. On Linux, listen() would otherwise fail with
    // AddressInUseError.
    QLocalServer::removeServer(path);

    if (!m_server->listen(path)) {
        qCWarning(KCM_KEYBOARD) << "KeyboardIpcServer failed to listen on" << path << ":" << m_server->errorString();
        return;
    }
    qCDebug(KCM_KEYBOARD) << "KeyboardIpcServer listening on" << path;

    connect(m_server, &QLocalServer::newConnection, this, &KeyboardIpcServer::onNewConnection);
}

QString KeyboardIpcServer::socketPath()
{
    return keyboardIpcSocketPath();
}

void KeyboardIpcServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket *socket = m_server->nextPendingConnection();
        if (!socket) {
            continue;
        }
        socket->setParent(this);
        m_clients.insert(socket);

        connect(socket, &QLocalSocket::readyRead, this, &KeyboardIpcServer::onReadyRead);
        connect(socket, &QLocalSocket::disconnected, this, &KeyboardIpcServer::onDisconnected);
        connect(socket, &QLocalSocket::errorOccurred, this, &KeyboardIpcServer::onErrorOccurred);
    }
}

void KeyboardIpcServer::onReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket) {
        return;
    }

    while (socket->bytesAvailable() > 0) {
        QDataStream in(socket);
        in.startTransaction();
        qint64 payloadSize = 0;
        in >> payloadSize;
        if (!in.commitTransaction()) {
            return; // wait for more data
        }
        if (payloadSize < 0 || payloadSize > 64 * 1024 * 1024) {
            qCWarning(KCM_KEYBOARD) << "KeyboardIpcServer: refusing frame with implausible size" << payloadSize;
            socket->abort();
            return;
        }

        QByteArray payload(payloadSize, Qt::Uninitialized);
        in.startTransaction();
        if (in.readRawData(payload.data(), payloadSize) != payloadSize) {
            if (!in.commitTransaction()) {
                return; // wait for more data
            }
            // commitTransaction returned true but we didn't read the full payload
            qCWarning(KCM_KEYBOARD) << "KeyboardIpcServer: short read of payload";
            socket->abort();
            return;
        }
        if (!in.commitTransaction()) {
            return; // wait for more data
        }

        if (in.status() == QDataStream::ReadCorruptData) {
            qCWarning(KCM_KEYBOARD) << "KeyboardIpcServer: corrupt frame from client";
            socket->abort();
            return;
        }

        handleFrame(socket, payload);
    }
}

void KeyboardIpcServer::handleFrame(QLocalSocket *socket, QByteArray &payload)
{
    QDataStream in(&payload, QIODevice::ReadOnly);
    qint32 msgInt = MSG_UNKNOWN;
    in >> msgInt;
    const Msg msg = static_cast<Msg>(msgInt);

    switch (msg) {
    case HELLO: {
        qint32 version = 0;
        in >> version;
        const bool accepted = (version == protocolVersion);
        QByteArray reply;
        {
            QDataStream s(&reply, QIODevice::WriteOnly);
            s << qint32(HELLO_REPLY);
            s << qint32(protocolVersion);
            s << accepted;
        }
        sendToSocket(socket, reply);
        if (accepted) {
            // Initial state push so the client doesn't need to send
            // GET_LAYOUTS / GET_LAYOUT after the handshake.
            replyLayouts(socket);
            replyLayout(socket);
        }
        break;
    }
    case GET_LAYOUTS:
        replyLayouts(socket);
        break;
    case GET_LAYOUT:
        replyLayout(socket);
        break;
    case SET_LAYOUT: {
        quint32 index = 0;
        in >> index;
        if (m_setLayoutFn) {
            m_setLayoutFn(index);
        }
        break;
    }
    case SWITCH_NEXT:
        if (m_switchToNextLayoutFn) {
            m_switchToNextLayoutFn();
        }
        break;
    case SWITCH_PREVIOUS:
        if (m_switchToPreviousLayoutFn) {
            m_switchToPreviousLayoutFn();
        }
        break;
    default:
        qCWarning(KCM_KEYBOARD) << "KeyboardIpcServer: unknown message" << msgInt;
        break;
    }
}

void KeyboardIpcServer::replyHello(QLocalSocket *socket)
{
    Q_UNUSED(socket);
    // HELLO is special-cased in handleFrame because the reply depends on
    // the client's reported version; we don't use this helper for it.
}

void KeyboardIpcServer::replyLayouts(QLocalSocket *socket)
{
    QByteArray reply;
    QVector<LayoutEntry> entries;
    if (m_getLayoutsListFn) {
        const QList<LayoutNames> names = m_getLayoutsListFn();
        entries.reserve(names.size());
        for (const auto &n : names) {
            entries.append({n.shortName, n.displayName, n.longName});
        }
    }
    {
        QDataStream s(&reply, QIODevice::WriteOnly);
        s << qint32(LAYOUTS_REPLY);
        s << entries;
    }
    sendToSocket(socket, reply);
}

void KeyboardIpcServer::replyLayout(QLocalSocket *socket)
{
    QByteArray reply;
    quint32 index = 0;
    if (m_getLayoutFn) {
        index = m_getLayoutFn();
    }
    {
        QDataStream s(&reply, QIODevice::WriteOnly);
        s << qint32(LAYOUT_REPLY);
        s << index;
    }
    sendToSocket(socket, reply);
}

void KeyboardIpcServer::sendToSocket(QLocalSocket *socket, const QByteArray &payload)
{
    if (!socket || socket->state() != QLocalSocket::ConnectedState) {
        return;
    }
    QDataStream socketStream(socket);
    socketStream << qint64(payload.size());
    socket->write(payload.constData(), payload.size());
}

void KeyboardIpcServer::sendToAll(const QByteArray &payload)
{
    for (QLocalSocket *socket : std::as_const(m_clients)) {
        sendToSocket(socket, payload);
    }
}

void KeyboardIpcServer::broadcastLayoutsChanged(const QList<LayoutNames> &layouts)
{
    QVector<LayoutEntry> entries;
    entries.reserve(layouts.size());
    for (const auto &n : layouts) {
        entries.append({n.shortName, n.displayName, n.longName});
    }

    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << qint32(LAYOUTS_CHANGED);
        s << entries;
    }
    sendToAll(payload);
}

void KeyboardIpcServer::broadcastLayoutChanged(uint index)
{
    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << qint32(LAYOUT_CHANGED);
        s << quint32(index);
    }
    sendToAll(payload);
}

void KeyboardIpcServer::onDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    if (!socket) {
        return;
    }
    m_clients.remove(socket);
    socket->deleteLater();
}

void KeyboardIpcServer::onErrorOccurred(QLocalSocket::LocalSocketError socketError)
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());
    qCWarning(KCM_KEYBOARD) << "KeyboardIpcServer client error:" << socketError << (socket ? socket->errorString() : QString());
}
