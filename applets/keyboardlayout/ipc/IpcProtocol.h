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
//
// Each message is one Sonic IPC frame: the message is the frame type and the
// payload holds only the fields listed here. The frame header carries
// protocolVersion; a peer speaking another version is rejected by the library.
enum Msg : qint32 {
    MSG_UNKNOWN = 0,
    HELLO = 1, // client → server, on every connect: no fields
    HELLO_REPLY, // server → client: bool accepted; followed by LAYOUTS_REPLY and LAYOUT_REPLY
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

// 2: Sonic IPC framing, with the message in the frame header.
constexpr quint16 protocolVersion = 2;
constexpr qint64 maxFramePayload = 64 * 1024 * 1024;

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
// KeyboardIpcServer and the applet's KeyboardLayoutSocket:
// <RuntimeLocation>/kded_keyboard.sock, or Sonic IPC's checked per-user
// /tmp fallback. Empty when no safe location exists.
QString keyboardIpcSocketPath();
