/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QLocalSocket>
#include <QMetaType>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QVariantList>
#include <QVariantMap>
#include <qqmlregistration.h>

#include "IpcProtocol.h"

class KeyboardLayoutSocket : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(uint layout READ layout NOTIFY layoutChanged)
    Q_PROPERTY(QVariantList layoutsList READ layoutsList NOTIFY layoutsListChanged)
    Q_PROPERTY(bool hasMultipleLayouts READ hasMultipleLayouts NOTIFY layoutsListChanged)

public:
    explicit KeyboardLayoutSocket(QObject *parent = nullptr);

    uint layout() const;
    QVariantList layoutsList() const;
    bool hasMultipleLayouts() const;

    Q_INVOKABLE void switchToNextLayout();
    Q_INVOKABLE void switchToPreviousLayout();
    Q_INVOKABLE void setLayout(uint index);

    static QString serverPath();

Q_SIGNALS:
    void layoutChanged();
    void layoutsListChanged();

private Q_SLOTS:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onErrorOccurred(QLocalSocket::LocalSocketError socketError);
    void reconnect();

private:
    void sendFrame(const QByteArray &payload);
    void handleFrame(QByteArray &payload);
    void sendHello();

    QLocalSocket *m_socket;
    QTimer *m_reconnectTimer;
    uint m_layout;
    QVariantList m_layoutsList;
    bool m_helloAccepted;
};
