// SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "messagehandler.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QLatin1StringView>
#include <QtGlobal>

// Project-wide Qt message handler installer. Link this translation unit
// into every plugin .so that ends up loaded by a host process which itself
// does not install a handler (e.g. plasmashell loading containment and
// applet .so's from this project).
//
// qInstallMessageHandler is process-global: every plugin .so that links
// this TU calls it once at static init. The first installer wins; the
// rest are no-ops because qInstallMessageHandler replaces unconditionally,
// and the function pointer being installed is identical across all
// translation units, so behavior is the same regardless of which "wins".
//
// The real per-line QLoggingCategory is read from QMessageLogContext. That
// requires QT_MESSAGELOGCONTEXT to be defined when the emitter's TU is
// compiled; the top-level CMakeLists.txt enables it project-wide.

namespace
{

// Returns true if the given category belongs to a noisy source that should
// be dropped from the log. Only applies to debug-level messages; warnings,
// criticals, and fatals are always forwarded regardless of category.
bool isNoisyCategory(QLatin1StringView category)
{
    // Qt internal categories: qt.text, qt.qpa.*, qt.gui.*, qt.core.*, ...
    if (category.startsWith(QLatin1StringView("qt."))) {
        return true;
    }

    // KDE Frameworks internal categories: kf.kio.*, kf.config.*, ...
    if (category.startsWith(QLatin1StringView("kf."))) {
        return true;
    }

    // KWin internal categories: libkwin.*, kwin_wayland_*, kwin_x11_*, ...
    if (category.startsWith(QLatin1StringView("libkwin.")) || category.startsWith(QLatin1StringView("kwin_")) || category == QLatin1StringView("kwin")) {
        return true;
    }

    // Qt heap tracking — one line per allocation in debug builds.
    if (category == QLatin1StringView("heap")) {
        return true;
    }

    // Empty category: qDebug() / qWarning() called without a Q_LOGGING_CATEGORY.
    // These dominate the flood in any mixed codebase.
    if (category.isEmpty()) {
        return true;
    }

    return false;
}

} // namespace

static void desktopInterfaceMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // QMessageLogContext::category is a const char * owned by the static
    // category object; it is always non-null but may be "" for callers that
    // used qDebug() / qWarning() directly without a Q_LOGGING_CATEGORY.
    QLatin1StringView category(context.category ? context.category : "");

    // Drop noisy categories and verbose message bodies at debug level only.
    if (type == QtDebugMsg
        && (isNoisyCategory(category) || msg.contains(QLatin1StringView("[rub]"), Qt::CaseInsensitive)
            || msg.contains(QLatin1StringView("QRhiBufferData"), Qt::CaseInsensitive) || msg.contains(QLatin1StringView("QRhiTextureData"), Qt::CaseInsensitive)
            || msg.contains(QLatin1StringView("release to pool"), Qt::CaseInsensitive)
            || msg.contains(QLatin1StringView("now has the following listeners"), Qt::CaseInsensitive)
            || msg.contains(QLatin1StringView("ChangeListener listener="), Qt::CaseInsensitive)
            || msg.contains(QLatin1StringView("Generated BaselineJIT code"), Qt::CaseInsensitive)
            || msg.contains(QLatin1StringView("Code at [0x"), Qt::CaseInsensitive))) {
        return;
    }

    messageHandler(type, category.toString(), msg);
}

static void installDesktopInterfaceMessageHandler()
{
    qInstallMessageHandler(desktopInterfaceMessageHandler);
}

Q_COREAPP_STARTUP_FUNCTION(installDesktopInterfaceMessageHandler)
