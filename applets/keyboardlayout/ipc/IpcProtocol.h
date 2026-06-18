/*
    SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDataStream>
#include <QMetaType>
#include <QString>
#include <QVector>

// Protocol types are at global scope so they can be referenced from both
// the daemon (KeyboardIpcServer) and the applet's wrapper
// (KeyboardLayoutSocket). Keeping them global matches the folder plugin's
// pattern for QML types.
enum Msg : qint32 {
    MSG_UNKNOWN = 0,
    HELLO = 1, // client → server: qint32 version
    HELLO_REPLY, // server → client: qint32 version, bool accepted
    GET_LAYOUTS, // client → server
    LAYOUTS_REPLY, // server → client: QVector<LayoutEntry>
    GET_LAYOUT, // client → server
    LAYOUT_REPLY, // server → client: quint32
    SET_LAYOUT, // client → server: quint32
    SWITCH_NEXT, // client → server
    SWITCH_PREVIOUS, // client → server
    LAYOUTS_CHANGED, // server → client (push)
    LAYOUT_CHANGED, // server → client (push): quint32
    MSG_LAST,
};

constexpr qint32 protocolVersion = 1;

// Wire-format copy of LayoutNames (kcms/keyboard/layoutnames.h). The daemon
// constructs LayoutNames as
// {layoutUnit.layout(), displayName, Flags::getLongText(layoutUnit)}
// in KeyboardDaemon::getLayoutsList().
struct LayoutEntry {
    QString shortName;
    QString displayName;
    QString longName;
};
Q_DECLARE_METATYPE(LayoutEntry)

// QDataStream operator overloads for LayoutEntry (QVector<LayoutEntry>
// uses Qt's built-in container operators).
QDataStream &operator<<(QDataStream &out, const LayoutEntry &entry);
QDataStream &operator>>(QDataStream &in, LayoutEntry &entry);

// Returns the canonical socket path used by both the daemon's
// KeyboardIpcServer and the applet's KeyboardLayoutSocket.
// XDG_RUNTIME_DIR/kded_keyboard.sock, falling back to
// /tmp/sonic-desktop-keyboard-<uid>/kded_keyboard.sock.
QString keyboardIpcSocketPath();
