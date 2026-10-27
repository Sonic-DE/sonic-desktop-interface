/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KeyboardIpcServer.h"

#include <QIODevice>

#include <Sonic/Ipc/Connection>
#include <Sonic/Ipc/Server>
#include <Sonic/Logging/Log>

namespace {
// One message per frame: the message is the frame type, the payload only its fields.
template<typename... Fields>
QByteArray payloadOf(const Fields &...fields)
{
    QByteArray payload;
    QDataStream stream(&payload, QIODevice::WriteOnly);
    ((stream << fields), ...);
    return payload;
}

QVector<LayoutEntry> entriesOf(const QList<LayoutNames> &layouts)
{
    QVector<LayoutEntry> entries;
    entries.reserve(layouts.size());
    for (const auto &n : layouts) {
        entries.append({n.shortName, n.displayName, n.longName});
    }
    return entries;
}
}

KeyboardIpcServer::KeyboardIpcServer(QObject *parent,
                                     GetLayoutsListFn getLayoutsListFn,
                                     GetLayoutFn getLayoutFn,
                                     SwitchToNextLayoutFn switchToNextLayoutFn,
                                     SwitchToPreviousLayoutFn switchToPreviousLayoutFn,
                                     SetLayoutFn setLayoutFn,
                                     const QString &path)
    : QObject(parent)
    , m_server(new Sonic::Ipc::Server(this))
    , m_getLayoutsListFn(std::move(getLayoutsListFn))
    , m_getLayoutFn(std::move(getLayoutFn))
    , m_switchToNextLayoutFn(std::move(switchToNextLayoutFn))
    , m_switchToPreviousLayoutFn(std::move(switchToPreviousLayoutFn))
    , m_setLayoutFn(std::move(setLayoutFn))
{
    if (path.isEmpty()) {
        sonicWarning() << "KeyboardIpcServer: no safe location for the IPC socket; not listening";
        return;
    }

    // A stale socket from a crashed daemon is replaced; a live one never is.
    if (!m_server->listen(path, Sonic::Ipc::Server::Access::User, protocolVersion, maxFramePayload)) {
        sonicWarning() << "KeyboardIpcServer failed to listen on" << path << ":" << m_server->errorString();
        return;
    }
    sonicDebug() << "KeyboardIpcServer listening on" << path;

    connect(m_server, &Sonic::Ipc::Server::newConnection, this, &KeyboardIpcServer::onNewConnection);
}

QString KeyboardIpcServer::socketPath()
{
    return keyboardIpcSocketPath();
}

void KeyboardIpcServer::onNewConnection(Sonic::Ipc::Connection *connection)
{
    connect(connection, &Sonic::Ipc::Connection::frameReceived, this, [this, connection](quint32 type, const QByteArray &payload) {
        handleFrame(connection, type, payload);
    });
}

void KeyboardIpcServer::handleFrame(Sonic::Ipc::Connection *connection, quint32 type, const QByteArray &payload)
{
    QDataStream in(payload);

    switch (Msg(type)) {
    case HELLO:
        // The frame header already carried a matching protocol version.
        connection->send(HELLO_REPLY, payloadOf(true));
        // Initial state push so the client doesn't need to send
        // GET_LAYOUTS / GET_LAYOUT after the handshake.
        replyLayouts(connection);
        replyLayout(connection);
        break;
    case GET_LAYOUTS:
        replyLayouts(connection);
        break;
    case GET_LAYOUT:
        replyLayout(connection);
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
        sonicWarning() << "KeyboardIpcServer: unknown message" << type;
        break;
    }
}

void KeyboardIpcServer::replyLayouts(Sonic::Ipc::Connection *connection)
{
    const QVector<LayoutEntry> entries = m_getLayoutsListFn ? entriesOf(m_getLayoutsListFn()) : QVector<LayoutEntry>();
    connection->send(LAYOUTS_REPLY, payloadOf(entries));
}

void KeyboardIpcServer::replyLayout(Sonic::Ipc::Connection *connection)
{
    const quint32 index = m_getLayoutFn ? m_getLayoutFn() : 0;
    connection->send(LAYOUT_REPLY, payloadOf(index));
}

void KeyboardIpcServer::broadcastLayoutsChanged(const QList<LayoutNames> &layouts)
{
    m_server->broadcast(LAYOUTS_CHANGED, payloadOf(entriesOf(layouts)));
}

void KeyboardIpcServer::broadcastLayoutChanged(uint index)
{
    m_server->broadcast(LAYOUT_CHANGED, payloadOf(quint32(index)));
}
