/*
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QFileInfo>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTest>

class PluginInstallerRcTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
        qputenv("PLUGININSTALLER_TEST_MODE", "true");
    }
    void testInstall()
    {
        const QString executable = QFINDTESTDATA("krunner-plugininstaller");
        QProcess p;
        p.start(executable, {"install", QFINDTESTDATA("testplugin"), "--no-confirm"});

        p.waitForFinished();
        qDebug() << p.readAllStandardError();
        QCOMPARE(p.exitCode(), 0);

        QString dataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
        QVERIFY(QFileInfo::exists(dataLocation + "/krunner/dbusplugins/org.kde.testplugin.desktop"));
        QVERIFY(QFileInfo::exists(dataLocation + "/dbus-1/services/org.kde.testplugin.service"));
    }
};

QTEST_MAIN(PluginInstallerRcTest)

#include "plugininstallerrctest.moc"
