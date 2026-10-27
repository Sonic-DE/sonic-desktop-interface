/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KeyboardLayoutSocket.h"

#include <QDataStream>
#include <QIODevice>

#include <Sonic/Ipc/Client>
#include <Sonic/Ipc/Connection>
#include <Sonic/Logging/Log>

KeyboardLayoutSocket::KeyboardLayoutSocket(QObject *parent)
    : QObject(parent)
    , m_client(new Sonic::Ipc::Client(protocolVersion, maxFramePayload, this))
    , m_layout(0)
    , m_helloAccepted(false)
{
    // Required at runtime for QVariant round-trips and queued
    // connections. Q_DECLARE_METATYPE alone is not enough.
    qRegisterMetaType<LayoutEntry>("LayoutEntry");

    // Follows kded across restarts; stops for good if kded rejects this protocol version.
    m_client->setReconnectInterval(2000);
    connect(m_client, &Sonic::Ipc::Client::connected, this, &KeyboardLayoutSocket::onConnected);
    connect(m_client, &Sonic::Ipc::Client::versionRejected, this, [this](quint16 serverVersion) {
        sonicWarning() << "KeyboardLayoutSocket: the keyboard daemon speaks protocol" << serverVersion << "; layout switching is unavailable";
        m_layoutsList.clear();
        Q_EMIT layoutsListChanged();
    });

    const QString path = serverPath();
    if (path.isEmpty()) {
        sonicWarning() << "KeyboardLayoutSocket: no safe location for the IPC socket; not connecting";
        return;
    }
    m_client->connectToServer(path);
}

QString KeyboardLayoutSocket::serverPath()
{
    return keyboardIpcSocketPath();
}

uint KeyboardLayoutSocket::layout() const
{
    return m_layout;
}

QVariantList KeyboardLayoutSocket::layoutsList() const
{
    return m_layoutsList;
}

bool KeyboardLayoutSocket::hasMultipleLayouts() const
{
    return m_layoutsList.size() > 1;
}

void KeyboardLayoutSocket::switchToNextLayout()
{
    send(SWITCH_NEXT);
}

void KeyboardLayoutSocket::switchToPreviousLayout()
{
    send(SWITCH_PREVIOUS);
}

void KeyboardLayoutSocket::setLayout(uint index)
{
    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << quint32(index);
    }
    send(SET_LAYOUT, payload);
}

void KeyboardLayoutSocket::send(Msg message, const QByteArray &payload)
{
    if (m_connection) {
        m_connection->send(message, payload);
    }
}

void KeyboardLayoutSocket::onConnected(Sonic::Ipc::Connection *connection)
{
    m_connection = connection;
    m_helloAccepted = false;
    connect(connection, &Sonic::Ipc::Connection::frameReceived, this, &KeyboardLayoutSocket::handleFrame);
    connect(connection, &Sonic::Ipc::Connection::disconnected, this, [this, connection] {
        if (m_connection == connection) {
            m_connection = nullptr;
        }
    });
    send(HELLO);
}

void KeyboardLayoutSocket::handleFrame(quint32 type, const QByteArray &payload)
{
    QDataStream in(payload);

    switch (Msg(type)) {
    case HELLO_REPLY: {
        bool accepted = false;
        in >> accepted;
        m_helloAccepted = accepted;
        if (!accepted) {
            sonicWarning() << "KeyboardLayoutSocket: server rejected HELLO";
        }
        break;
    }
    case LAYOUTS_REPLY:
    case LAYOUTS_CHANGED: {
        QVector<LayoutEntry> entries;
        in >> entries;
        QVariantList list;
        list.reserve(entries.size());
        for (const auto &e : entries) {
            QVariantMap m;
            m.insert(QStringLiteral("shortName"), e.shortName);
            m.insert(QStringLiteral("displayName"), e.displayName);
            m.insert(QStringLiteral("longName"), e.longName);
            list.append(m);
        }
        m_layoutsList = list;
        Q_EMIT layoutsListChanged();
        break;
    }
    case LAYOUT_REPLY:
    case LAYOUT_CHANGED: {
        quint32 index = 0;
        in >> index;
        if (m_layout != index) {
            m_layout = index;
            Q_EMIT layoutChanged();
        }
        break;
    }
    default:
        sonicWarning() << "KeyboardLayoutSocket: unknown message" << type;
        break;
    }
}
