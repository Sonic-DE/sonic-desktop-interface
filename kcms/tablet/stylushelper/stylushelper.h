/*
    localegenhelper.h
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once
#include <PolkitQt1/Authority>
#include <QCoreApplication>
#include <QDBusContext>
#include <QObject>
#include <QThread>
#include <QTimer>

class InputListener;
class StylusHelper : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.stylushelper.StylusHelper")
public:
    StylusHelper();
    Q_SCRIPTABLE void startListen();
Q_SIGNALS:
    Q_SCRIPTABLE void stylusEvents(unsigned int id);
    void startInputListener();
private Q_SLOTS:
    void startListenPrivate(PolkitQt1::Authority::Result result);

private:
    bool m_alreadyStarted = false;
    QThread m_thread;
    PolkitQt1::Authority *m_authority = nullptr;
    QTimer m_timer;
    InputListener *m_listener;
};
