/*
 * SPDX-FileCopyrightText: 2026 SonicDE Community
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "../cursortheme.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTest>

#include <cstdio>

int kapplymousetheme_main(int argc, char **argv);

static bool applied = false;
static bool receivedProbe = false;
static bool restoredBeforeAppDestruction = false;

static void previousHandler(QtMsgType, const QMessageLogContext &, const QString &message)
{
    if (message == QStringLiteral("after helper teardown")) {
        receivedProbe = true;
    }
}

namespace CursorTheme {
void applyCursorTheme(const QString &theme, int size)
{
    applied = theme == QStringLiteral("silver_cursors_dark") && size == 36;
    // Initialize the production handler's cached state before teardown.
    qWarning("cursor backend regression probe");
    auto *app = QCoreApplication::instance();
    QObject::connect(
        app,
        &QObject::destroyed,
        app,
        [] {
            const auto installed = qInstallMessageHandler(previousHandler);
            restoredBeforeAppDestruction = installed == previousHandler;
            qInstallMessageHandler(installed);
        },
        Qt::DirectConnection);
}
}

static int runHelper(bool validArguments)
{
    char program[] = "kapplymousetheme";
    char theme[] = "silver_cursors_dark";
    char size[] = "36";
    char *arguments[] = {program, theme, size, nullptr};
    qInstallMessageHandler(previousHandler);
    const int result = kapplymousetheme_main(validArguments ? 3 : 1, arguments);
    // Inspect the handler after the real QGuiApplication has been destroyed.
    const auto installed = qInstallMessageHandler(previousHandler);
    if (installed != previousHandler) {
        std::fputs("cursor helper did not restore the previous message handler\n", stderr);
        return 10;
    }
    qWarning("after helper teardown");
    if (!receivedProbe || result != (validArguments ? 0 : 1) || applied != validArguments || (validArguments && !restoredBeforeAppDestruction)) {
        return 11;
    }
    qInstallMessageHandler(nullptr);
    return 0;
}

class KApplyMouseThemeTest : public QObject {
    Q_OBJECT

private:
    void checkHelper(bool validArguments)
    {
        QTemporaryDir state;
        QVERIFY(state.isValid());
        auto environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
        environment.insert(QStringLiteral("QT_QPA_PLATFORMTHEME"), QStringLiteral("generic"));
        environment.insert(QStringLiteral("QT_DEBUG_PLUGINS"), QStringLiteral("1"));
        environment.insert(QStringLiteral("XDG_STATE_HOME"), state.path());

        QProcess helper;
        helper.setProcessEnvironment(environment);
        helper.start(QCoreApplication::applicationFilePath(), {validArguments ? QStringLiteral("--valid") : QStringLiteral("--invalid")});
        QVERIFY(helper.waitForStarted(5000));
        QVERIFY(helper.waitForFinished(20000));
        const auto output = helper.readAllStandardOutput();
        const auto errors = helper.readAllStandardError();
        const QByteArray diagnostics = output + errors;
        QCOMPARE(helper.exitStatus(), QProcess::NormalExit);
        QVERIFY2(helper.exitCode() == 0, diagnostics.constData());

        if (validArguments) {
            // Sonic Logging writes one file per executable, named after the process.
            const QString binary = QFileInfo(QCoreApplication::applicationFilePath()).fileName();
            QFile log(state.filePath(QStringLiteral("sonic/%1.log").arg(binary)));
            QVERIFY2(log.open(QIODevice::ReadOnly), qPrintable(log.errorString()));
            QVERIFY(log.readAll().contains(QByteArray("(WW) [" + binary.toUtf8() + "] cursor backend regression probe")));
        }
    }

private Q_SLOTS:
    void restoresHandlerAfterApplyingTheme()
    {
        checkHelper(true);
    }

    void restoresHandlerOnInvalidArguments()
    {
        checkHelper(false);
    }
};

int main(int argc, char **argv)
{
    if (argc == 2 && QByteArray(argv[1]) == "--valid") {
        return runHelper(true);
    }
    if (argc == 2 && QByteArray(argv[1]) == "--invalid") {
        return runHelper(false);
    }
    QCoreApplication app(argc, argv);
    KApplyMouseThemeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "kapplymousethemetest.moc"
