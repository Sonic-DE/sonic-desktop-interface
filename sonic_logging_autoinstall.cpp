// SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Compiled into every plugin that ends up in a host process which installs no
// handler itself (sonicshell loading applets and containments). Each target
// passes its own SONIC_LOGGING_COMPONENT_ID; messages go to the host's file.

#include <Sonic/Logging/Logging>

#include <QFile>
#include <QStandardPaths>

#include <sys/stat.h>
#include <unistd.h>

SONIC_LOGGING_COMPONENT(SONIC_LOGGING_COMPONENT_ID)

namespace {
// The single shared log every plugin wrote before; nothing writes it any more,
// so this only removes anything the first time.
void removeOldSharedLog()
{
    struct stat out{};
    struct stat err{};
    const bool haveOut = fstat(STDOUT_FILENO, &out) == 0;
    const bool haveErr = fstat(STDERR_FILENO, &err) == 0;
    const QString log = QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation) + QStringLiteral("/sonic/desktop-interface.log");
    for (const QString &path : {log, QString(log + QStringLiteral(".last"))}) {
        struct stat st{};
        if (lstat(QFile::encodeName(path).constData(), &st) != 0) {
            continue;
        }
        if ((haveOut && st.st_dev == out.st_dev && st.st_ino == out.st_ino) || (haveErr && st.st_dev == err.st_dev && st.st_ino == err.st_ino)) {
            sonicInfo() << "keeping" << path << "because it is this process's output";
            continue;
        }
        if (QFile::remove(path)) {
            sonicInfo() << "removed the old log file" << path;
        } else {
            sonicWarning() << "cannot remove the old log file" << path;
        }
    }
}
}

SONIC_LOGGING_AUTO_INSTALL(removeOldSharedLog)
