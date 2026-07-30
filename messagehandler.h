/***************************************************************************
 * SPDX-FileCopyrightText: 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 * SPDX-FileCopyrightText: 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 ***************************************************************************/

#ifndef SONICLOGIN_MESSAGEHANDLER_H
#define SONICLOGIN_MESSAGEHANDLER_H

#include <QDateTime>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QStandardPaths>

#include <cerrno>
#include <cstring>
#include <stdio.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

// In debug builds, enable all Qt logging categories at debug level
#ifndef QT_NO_DEBUG
static const bool s_debugLoggingEnabled = []() {
    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true"));
    return true;
}();
#endif

inline void rotateLogFile(const QString &s_logFilePath)
{
    // Throttle the size check to once per 60 seconds to avoid an extra
    // stat(2) syscall on every log call.
    static QElapsedTimer lastCheck;
    if (!lastCheck.isValid() || lastCheck.elapsed() >= 60000) {
        lastCheck.restart();
    } else {
        return;
    }

    QFileInfo logFileInfo(s_logFilePath);
    // Don't rotate if the file doesn't exist or is below the 2MB threshold.
    if (!logFileInfo.exists() || logFileInfo.size() <= 2097152) {
        return;
    }

    QFile logFile(s_logFilePath);

    QString s_logFilePathLast = s_logFilePath + QStringLiteral(".last");

    // Delete old backup file if it exists
    QFile backupFile(s_logFilePathLast);
    if (backupFile.exists()) {
        backupFile.remove();
    }

    // Rename current log file to backup
    logFile.rename(s_logFilePathLast);
}

inline void ensureLogFileExists(const QString &s_logFilePath)
{
    // Per-user state directories are created on demand.
    const QString logDirectory = QFileInfo(s_logFilePath).absolutePath();
    QDir().mkpath(logDirectory);
    // Directory traversal requires the execute bit; the log file itself is non-executable.
    QFile::setPermissions(logDirectory, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    QFile logFile(s_logFilePath);
    if (!logFile.exists()) {
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            logFile.close();
        }
        chmod(s_logFilePath.toUtf8().constData(), 0600);
    }
}

static void messageHandler(QtMsgType type, const QString &category, const QString &msg)
{
    static bool loggingCapabilityChecked = false;
    static const QString s_logFilePath =
        QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation) + QStringLiteral("/sonic/desktop-interface.log");

    if (!loggingCapabilityChecked) {
        loggingCapabilityChecked = true;
    }

    // Filter out noisy low-level X11 / XCB / XInput events that flood the
    // log when debug categories are enabled. These come from categories
    // like "libkwin" / "kwin_wayland_drm" / Qt internal xcb handlers and
    // are not useful for application debugging.
    if (type == QtDebugMsg) {
        if (msg.contains(QStringLiteral("XCB_")) || msg.contains(QStringLiteral("XInput Event("))) {
            return;
        }
    }

    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);
    QString systemdPrefix;
    QString logPriority;
    switch (type) {
    case QtInfoMsg:
        systemdPrefix = QStringLiteral("<6> ");
        logPriority = QStringLiteral("(II) ");
        break;
    case QtWarningMsg:
        systemdPrefix = QStringLiteral("<4> ");
        logPriority = QStringLiteral("(WW) ");
        break;
    case QtCriticalMsg:
        systemdPrefix = QStringLiteral("<2> ");
        logPriority = QStringLiteral("(EE) ");
        break;
    case QtFatalMsg:
        systemdPrefix = QStringLiteral("<1> ");
        logPriority = QStringLiteral("(EE) ");
        break;
    default:
        systemdPrefix = QStringLiteral("<7> ");
        logPriority = QStringLiteral("(DD) ");
        break;
    }

    // If stdout/stderr are unhandled, try to use manual file
    rotateLogFile(s_logFilePath);
    ensureLogFileExists(s_logFilePath);
    QFile logFile(s_logFilePath);
    if (logFile.open(QIODevice::Append | QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&logFile);

        out << timestamp << " " << logPriority << QStringLiteral("[%1] ").arg(category) << msg << "\n";
        out.flush();
        logFile.close();
    }

    QTextStream standardOut = QTextStream(stdout);

    standardOut << systemdPrefix << logPriority << ": " << QStringLiteral("[%1] ").arg(category) << msg << Qt::endl;
    standardOut.flush();
}

#endif // SONICLOGIN_MESSAGEHANDLER_H
