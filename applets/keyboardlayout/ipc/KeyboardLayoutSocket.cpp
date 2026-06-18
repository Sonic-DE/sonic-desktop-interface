/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "KeyboardLayoutSocket.h"

#include <QDataStream>
#include <QLocalSocket>
#include <QLoggingCategory>

#include "debug.h"

KeyboardLayoutSocket::KeyboardLayoutSocket(QObject *parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_reconnectTimer(nullptr)
    , m_layout(0)
    , m_helloAccepted(false)
{
    // Required at runtime for QVariant round-trips and queued
    // connections. Q_DECLARE_METATYPE alone is not enough.
    qRegisterMetaType<LayoutEntry>("LayoutEntry");

    m_socket = new QLocalSocket(this);
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(2000);

    connect(m_socket, &QLocalSocket::connected, this, &KeyboardLayoutSocket::onConnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &KeyboardLayoutSocket::onReadyRead);
    connect(m_socket, &QLocalSocket::disconnected, this, &KeyboardLayoutSocket::onDisconnected);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &KeyboardLayoutSocket::onErrorOccurred);
    connect(m_reconnectTimer, &QTimer::timeout, this, &KeyboardLayoutSocket::reconnect);

    m_socket->connectToServer(serverPath());
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
    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << qint32(SWITCH_NEXT);
    }
    sendFrame(payload);
}

void KeyboardLayoutSocket::switchToPreviousLayout()
{
    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << qint32(SWITCH_PREVIOUS);
    }
    sendFrame(payload);
}

void KeyboardLayoutSocket::setLayout(uint index)
{
    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << qint32(SET_LAYOUT);
        s << quint32(index);
    }
    sendFrame(payload);
}

void KeyboardLayoutSocket::sendFrame(const QByteArray &payload)
{
    if (!m_socket || m_socket->state() != QLocalSocket::ConnectedState) {
        return;
    }
    QDataStream socketStream(m_socket);
    socketStream << qint64(payload.size());
    m_socket->write(payload.constData(), payload.size());
}

void KeyboardLayoutSocket::sendHello()
{
    QByteArray payload;
    {
        QDataStream s(&payload, QIODevice::WriteOnly);
        s << qint32(HELLO);
        s << qint32(protocolVersion);
    }
    sendFrame(payload);
}

void KeyboardLayoutSocket::onConnected()
{
    m_reconnectTimer->stop();
    m_helloAccepted = false;
    sendHello();
}

void KeyboardLayoutSocket::reconnect()
{
    if (m_socket) {
        m_socket->connectToServer(serverPath());
    }
}

void KeyboardLayoutSocket::onReadyRead()
{
    while (m_socket && m_socket->bytesAvailable() > 0) {
        QDataStream in(m_socket);
        in.startTransaction();
        qint64 payloadSize = 0;
        in >> payloadSize;
        if (!in.commitTransaction()) {
            return; // wait for more data
        }
        if (payloadSize < 0 || payloadSize > 64 * 1024 * 1024) {
            qCWarning(KCM_KEYBOARD) << "KeyboardLayoutSocket: refusing frame with implausible size" << payloadSize;
            m_socket->abort();
            return;
        }

        QByteArray payload(payloadSize, Qt::Uninitialized);
        in.startTransaction();
        if (in.readRawData(payload.data(), payloadSize) != payloadSize) {
            if (!in.commitTransaction()) {
                return; // wait for more data
            }
            qCWarning(KCM_KEYBOARD) << "KeyboardLayoutSocket: short read of payload";
            m_socket->abort();
            return;
        }
        if (!in.commitTransaction()) {
            return; // wait for more data
        }

        if (in.status() == QDataStream::ReadCorruptData) {
            qCWarning(KCM_KEYBOARD) << "KeyboardLayoutSocket: corrupt frame from server";
            m_socket->abort();
            return;
        }

        handleFrame(payload);
    }
}

void KeyboardLayoutSocket::handleFrame(QByteArray &payload)
{
    QDataStream in(&payload, QIODevice::ReadOnly);
    qint32 msgInt = MSG_UNKNOWN;
    in >> msgInt;
    const Msg msg = static_cast<Msg>(msgInt);

    switch (msg) {
    case HELLO_REPLY: {
        qint32 version = 0;
        bool accepted = false;
        in >> version >> accepted;
        m_helloAccepted = accepted;
        if (!accepted) {
            qCWarning(KCM_KEYBOARD) << "KeyboardLayoutSocket: server rejected HELLO version" << version;
            m_socket->abort();
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
        qCWarning(KCM_KEYBOARD) << "KeyboardLayoutSocket: unknown message" << msgInt;
        break;
    }
}

void KeyboardLayoutSocket::onDisconnected()
{
    m_reconnectTimer->start();
}

void KeyboardLayoutSocket::onErrorOccurred(QLocalSocket::LocalSocketError socketError)
{
    Q_UNUSED(socketError);
    qCWarning(KCM_KEYBOARD) << "KeyboardLayoutSocket error:" << (m_socket ? m_socket->errorString() : QString());
    if (m_socket) {
        m_socket->abort();
    }
    m_reconnectTimer->start();
}
