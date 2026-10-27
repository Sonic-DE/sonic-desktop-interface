/*
    SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "IpcProtocol.h"
#include "KeyboardIpcServer.h"

#include <Sonic/Ipc/Client>
#include <Sonic/Ipc/Connection>

#include <QDataStream>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include <memory>

namespace {
// What a client saw, decoded as KeyboardLayoutSocket decodes it.
struct Received {
    QList<quint32> types;
    bool accepted = false;
    QVector<LayoutEntry> layouts;
    quint32 layout = 0;
};

// A KeyboardLayoutSocket-equivalent client on a test socket.
class TestClient {
public:
    explicit TestClient(const QString &path, quint16 version = protocolVersion, int reconnectMs = 0)
        : client(version, maxFramePayload)
    {
        client.setReconnectInterval(reconnectMs);
        QObject::connect(&client, &Sonic::Ipc::Client::connected, &client, [this](Sonic::Ipc::Connection *c) {
            connection = c;
            QObject::connect(c, &Sonic::Ipc::Connection::frameReceived, c, [this](quint32 type, const QByteArray &payload) {
                QDataStream in(payload);
                received.types << type;
                if (type == HELLO_REPLY) {
                    in >> received.accepted;
                } else if (type == LAYOUTS_REPLY || type == LAYOUTS_CHANGED) {
                    in >> received.layouts;
                } else if (type == LAYOUT_REPLY || type == LAYOUT_CHANGED) {
                    in >> received.layout;
                }
            });
            c->send(HELLO, QByteArray());
        });
        client.connectToServer(path);
    }

    Sonic::Ipc::Client client;
    Sonic::Ipc::Connection *connection = nullptr;
    Received received;
};
}

class KeyboardIpcTest : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<KeyboardIpcServer> makeServer()
    {
        return std::make_unique<KeyboardIpcServer>(
            nullptr,
            [] {
                return QList<LayoutNames>{{QStringLiteral("us"), QStringLiteral("en"), QStringLiteral("English (US)")},
                                          {QStringLiteral("de"), QStringLiteral("de"), QStringLiteral("German")}};
            },
            [] {
                return 1u;
            },
            [this] {
                ++m_next;
            },
            [this] {
                ++m_previous;
            },
            [this](uint index) {
                m_setLayout = int(index);
            },
            m_path);
    }

    QTemporaryDir m_dir;
    QString m_path;
    int m_counter = 0;
    int m_next = 0;
    int m_previous = 0;
    int m_setLayout = -1;

private Q_SLOTS:
    void initTestCase()
    {
        qRegisterMetaType<Sonic::Ipc::Connection *>();
    }

    void init()
    {
        QVERIFY(m_dir.isValid());
        m_path = m_dir.filePath(QStringLiteral("keyboard-%1.sock").arg(m_counter++));
        m_next = m_previous = 0;
        m_setLayout = -1;
    }

    void handshakePushesInitialState()
    {
        const auto server = makeServer();
        TestClient client(m_path);
        QTRY_COMPARE(client.received.types.size(), 3);
        QCOMPARE(client.received.types, (QList<quint32>{HELLO_REPLY, LAYOUTS_REPLY, LAYOUT_REPLY}));
        QVERIFY(client.received.accepted);
        QCOMPARE(client.received.layouts.size(), 2);
        QCOMPARE(client.received.layouts.at(1).longName, QStringLiteral("German"));
        QCOMPARE(client.received.layout, 1u);
    }

    void commandsReachCallbacks()
    {
        const auto server = makeServer();
        TestClient client(m_path);
        QTRY_VERIFY(client.connection);
        QByteArray payload;
        QDataStream(&payload, QIODevice::WriteOnly) << quint32(2);
        client.connection->send(SET_LAYOUT, payload);
        client.connection->send(SWITCH_NEXT, QByteArray());
        client.connection->send(SWITCH_PREVIOUS, QByteArray());
        QTRY_COMPARE(m_setLayout, 2);
        QTRY_COMPARE(m_next, 1);
        QTRY_COMPARE(m_previous, 1);
    }

    void broadcastReachesEveryClient()
    {
        const auto server = makeServer();
        TestClient first(m_path);
        TestClient second(m_path);
        QTRY_COMPARE(first.received.types.size(), 3);
        QTRY_COMPARE(second.received.types.size(), 3);
        server->broadcastLayoutChanged(0);
        QTRY_VERIFY(first.received.types.contains(LAYOUT_CHANGED));
        QTRY_VERIFY(second.received.types.contains(LAYOUT_CHANGED));
        QCOMPARE(first.received.layout, 0u);
        QCOMPARE(second.received.layout, 0u);
    }

    void versionMismatchIsRejectedForGood()
    {
        const auto server = makeServer();
        TestClient client(m_path, 1, 50);
        QSignalSpy rejected(&client.client, &Sonic::Ipc::Client::versionRejected);
        QSignalSpy reconnected(&client.client, &Sonic::Ipc::Client::connected);
        QTRY_COMPARE(rejected.size(), 1);
        QCOMPARE(rejected.first().first().value<quint16>(), protocolVersion);
        QTest::qWait(500);
        QCOMPARE(reconnected.size(), 0);
        QVERIFY(client.received.types.isEmpty());
    }

    void reconnectsAfterServerRestart()
    {
        auto server = makeServer();
        TestClient client(m_path, protocolVersion, 50);
        QTRY_COMPARE(client.received.types.size(), 3);
        server.reset();
        server = makeServer();
        // The new server pushes the initial state again after the client's new HELLO.
        QTRY_COMPARE_WITH_TIMEOUT(client.received.types.size(), 6, 5000);
        QCOMPARE(client.received.types.mid(3), (QList<quint32>{HELLO_REPLY, LAYOUTS_REPLY, LAYOUT_REPLY}));
    }
};

QTEST_GUILESS_MAIN(KeyboardIpcTest)

#include "keyboard_ipc_test.moc"
