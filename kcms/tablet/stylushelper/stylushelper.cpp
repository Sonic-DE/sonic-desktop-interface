/*
    localegenhelper.cpp
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "stylushelper.h"
#include "stylushelperadaptor.h"

#include <errno.h>
#include <fcntl.h>
#include <libinput.h>
#include <libudev.h>
#include <poll.h>
#include <unistd.h>

#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QTimer>

static int open_restricted(const char *path, int flags, void *user_data)
{
    int fd = open(path, flags);
    return fd < 0 ? -errno : fd;
}

static void close_restricted(int fd, void *user_data)
{
    close(fd);
}

class InputListener : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    void stop()
    {
        m_stop = true;
    }
public Q_SLOTS:
    void startListen()
    {
        struct libinput_interface interface = {
            open_restricted,
            close_restricted,
        };
        struct libinput *li;
        struct libinput_event *event;

        li = libinput_udev_create_context(&interface, NULL, udev_new());
        if (libinput_udev_assign_seat(li, "seat0")) {
            Q_EMIT error(QStringLiteral("failed to assign seat"));
            return;
        }

        // handle device add events
        if (libinput_dispatch(li)) {
            Q_EMIT error(QStringLiteral("failed to dispath"));
        }
        while ((event = libinput_get_event(li)) != NULL) {
            handleEvent(event);
        }

        struct pollfd fds;

        fds.fd = libinput_get_fd(li);
        fds.events = POLLIN;
        fds.revents = 0;
        if (poll(&fds, 1, -1) > -1) {
            do {
                // handle device add events
                if (libinput_dispatch(li)) {
                    Q_EMIT error(QStringLiteral("failed to dispath"));
                }
                while ((event = libinput_get_event(li)) != NULL) {
                    handleEvent(event);
                }
            } while (!m_stop && poll(&fds, 1, -1) > -1);
        } else {
            Q_EMIT error(QStringLiteral("can't poll"));
        }
    };
Q_SIGNALS:
    void error(const QString &reason);
    void stylusEvents(unsigned int id);

private:
    void handleEvent(struct libinput_event *event)
    {
        enum libinput_event_type type = libinput_event_get_type(event);
        switch (type) {
        case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
        case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
        case LIBINPUT_EVENT_TABLET_TOOL_TIP:
        case LIBINPUT_EVENT_TABLET_TOOL_BUTTON: {
            struct libinput_event_tablet_tool *tool_event = libinput_event_get_tablet_tool_event(event);
            struct libinput_tablet_tool *tool = libinput_event_tablet_tool_get_tool(tool_event);
            uint64_t id = libinput_tablet_tool_get_tool_id(tool);
            Q_EMIT stylusEvents(id);
            break;
        }
        default:
            break;
        }
    }

    bool m_stop = false;
};

StylusHelper::StylusHelper()
    : m_authority(PolkitQt1::Authority::instance())
    , m_listener(new InputListener(this))
{
    new StylusHelperAdaptor(this);
    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.stylushelper"))) {
        qWarning() << "another helper is already running";
        QCoreApplication::instance()->exit();
    }
    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/StylusHelper"), this)) {
        qWarning() << "unable to register service interface to dbus";
        QCoreApplication::instance()->exit();
    }
    connect(m_authority, &PolkitQt1::Authority::checkAuthorizationFinished, this, &StylusHelper::startListenPrivate);
    connect(this, &StylusHelper::startInputListener, m_listener, &InputListener::startListen);
    connect(m_listener, &InputListener::stylusEvents, this, &StylusHelper::stylusEvents);
    m_listener->moveToThread(&m_thread);
    m_thread.start();
}

void StylusHelper::startListen()
{
    m_authority->checkAuthorization(QStringLiteral("org.kde.stylushelper.startListen"),
                                    PolkitQt1::SystemBusNameSubject(message().service()),
                                    PolkitQt1::Authority::AllowUserInteraction);
}

void StylusHelper::startListenPrivate(PolkitQt1::Authority::Result result)
{
    if (result == PolkitQt1::Authority::Yes) {
        if (!m_alreadyStarted) {
            m_alreadyStarted = true;
            Q_EMIT startInputListener();
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    StylusHelper helper;
    return app.exec();
}
