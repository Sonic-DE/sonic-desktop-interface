/*
    SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QObject>
#include <qqmlregistration.h>

namespace Sonic::Ipc {
class Client;
class Connection;
}

// The applet's link to sonic-win's dwell clicker.
class DwellClickerSocket : public QObject {
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool enabled READ isEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int button READ button NOTIFY buttonChanged)

public:
    explicit DwellClickerSocket(QObject *parent = nullptr);

    bool isConnected() const;
    bool isEnabled() const;
    int button() const;

    Q_INVOKABLE void setButton(int button);
    Q_INVOKABLE void setEnabled(bool enabled);

Q_SIGNALS:
    void connectedChanged();
    void enabledChanged();
    void buttonChanged();

private:
    void onConnected(Sonic::Ipc::Connection *connection);
    void handleFrame(quint32 type, const QByteArray &payload);
    void setConnection(Sonic::Ipc::Connection *connection);

    Sonic::Ipc::Client *m_client;
    Sonic::Ipc::Connection *m_connection = nullptr;
    bool m_enabled = false;
    int m_button = 0;
};
