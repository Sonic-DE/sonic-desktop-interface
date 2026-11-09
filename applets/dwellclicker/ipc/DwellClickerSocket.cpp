/*
    SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DwellClickerSocket.h"

#include <DwellClickerIpc.h>

#include <Sonic/Ipc/Client>
#include <Sonic/Ipc/Connection>
#include <Sonic/Logging/Log>

namespace Protocol = Sonic::DwellClickerIpc;

DwellClickerSocket::DwellClickerSocket(QObject *parent)
    : QObject(parent)
    , m_client(new Sonic::Ipc::Client(Protocol::ProtocolVersion, Protocol::MaxPayload, this))
{
    // Follows sonic-win across restarts; stops for good if sonic-win rejects this protocol version.
    m_client->setReconnectInterval(2000);
    connect(m_client, &Sonic::Ipc::Client::connected, this, &DwellClickerSocket::onConnected);
    connect(m_client, &Sonic::Ipc::Client::connectionFailed, this, [](const QString &reason) {
        sonicWarning() << "DwellClickerSocket: can't reach the window manager:" << reason;
    });
    connect(m_client, &Sonic::Ipc::Client::versionRejected, this, [](quint16 serverVersion) {
        sonicWarning() << "DwellClickerSocket: the window manager speaks protocol" << serverVersion << "; the applet can't control it";
    });

    const QString path = Protocol::socketPath();
    if (path.isEmpty()) {
        sonicWarning() << "DwellClickerSocket: no safe location for the IPC socket; not connecting";
        return;
    }
    m_client->connectToServer(path);
}

bool DwellClickerSocket::isConnected() const
{
    return m_connection;
}

bool DwellClickerSocket::isEnabled() const
{
    return m_enabled;
}

int DwellClickerSocket::button() const
{
    return m_button;
}

void DwellClickerSocket::setButton(int button)
{
    if (m_connection) {
        m_connection->send(Protocol::SetButton, Protocol::encodeButton(button));
    }
}

void DwellClickerSocket::setEnabled(bool enabled)
{
    if (m_connection) {
        m_connection->send(Protocol::SetEnabled, Protocol::encodeEnabled(enabled));
    }
}

void DwellClickerSocket::onConnected(Sonic::Ipc::Connection *connection)
{
    connect(connection, &Sonic::Ipc::Connection::frameReceived, this, &DwellClickerSocket::handleFrame);
    connect(connection, &Sonic::Ipc::Connection::disconnected, this, [this, connection] {
        if (m_connection == connection) {
            setConnection(nullptr);
        }
    });
    setConnection(connection);
}

void DwellClickerSocket::setConnection(Sonic::Ipc::Connection *connection)
{
    const bool wasConnected = m_connection;
    m_connection = connection;
    if (wasConnected != bool(m_connection)) {
        Q_EMIT connectedChanged();
    }
}

void DwellClickerSocket::handleFrame(quint32 type, const QByteArray &payload)
{
    if (type != Protocol::State) {
        sonicWarning() << "DwellClickerSocket: unknown message" << type;
        return;
    }
    bool enabled = false;
    qint32 button = 0;
    if (!Protocol::decodeState(payload, &enabled, &button)) {
        sonicWarning() << "DwellClickerSocket: malformed state message";
        return;
    }
    if (m_enabled != enabled) {
        m_enabled = enabled;
        Q_EMIT enabledChanged();
    }
    if (m_button != button) {
        m_button = button;
        Q_EMIT buttonChanged();
    }
}
