/*
    SPDX-FileCopyrightText: 2011 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// #include <kapplication.h>

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include "../flags.h"
#include "../keyboard_config.h"
#include "../keyboard_daemon.h"
#include "../xkb_rules.h"

class KeyboardDaemonTest : public QObject
{
    Q_OBJECT

    KeyboardDaemon *keyboardDaemon;
    QTemporaryDir tempDir;
    //    KApplication* kapplication;

private Q_SLOTS:
    void initTestCase()
    {
        QVERIFY(tempDir.isValid());
        qputenv("XDG_CONFIG_HOME", tempDir.path().toUtf8());
        qputenv("XDG_DATA_HOME", QDir(tempDir.path()).filePath(QStringLiteral("data")).toUtf8());

        const QString kxkbrc = QDir(tempDir.path()).filePath(QStringLiteral("kxkbrc"));
        QFile file(kxkbrc);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write(QByteArrayLiteral("[Layout]\nLayoutList=us\nUse=false\n"));
        file.close();

        //    	kapplication = new KApplication();
        //    	const KAboutData* kAboutData = new KAboutData(i18n("a").toLatin1(), i18n("a").toLatin1(), KLocalizedString(), i18n("a").toLatin1());
        //    	KCmdLineArgs::init(kAboutData);
        keyboardDaemon = new KeyboardDaemon(this, QList<QVariant>());
        // Startup configuration can make the X server report a layout map change,
        // whose 1000 ms debounce emits layoutListChanged; let it fire before the
        // tests start counting emissions.
        QTest::qWait(2000);
    }

    void cleanupTestCase()
    {
        delete keyboardDaemon;
        //    	delete kapplication;
    }

    void testDaemon()
    {
        QVERIFY(keyboardDaemon != nullptr);

        //        QVERIFY( ! flags->getTransparentPixmap().isNull() );
        //
        //        const QIcon iconUs(flags->getIcon("us"));
        //        QVERIFY( ! iconUs.isNull() );
        //        QVERIFY( flags->getIcon("--").isNull() );
        //
        //    	KeyboardConfig keyboardConfig;
        //        LayoutUnit layoutUnit("us");
        //        LayoutUnit layoutUnit1("us", "intl");
        //        layoutUnit1.setDisplayName("usi");
        //        LayoutUnit layoutUnit2("us", "other");
        //
        //        keyboardConfig.showFlag = true;
        //        const QIcon iconUsFlag = flags->getIconWithText(layoutUnit, keyboardConfig);
        //        QVERIFY( ! iconUsFlag.isNull() );
        //        QCOMPARE( image(iconUsFlag), image(iconUs) );
        //
        //        keyboardConfig.showFlag = false;
        //        const QIcon iconUsText = flags->getIconWithText(layoutUnit, keyboardConfig);
        //        QVERIFY( ! iconUsText.isNull() );
        //        QVERIFY( image(iconUsText) != image(iconUs) );
        //
        //        keyboardConfig.layouts.append(layoutUnit1);
        //        QCOMPARE( flags->getShortText(layoutUnit, keyboardConfig), QString("us") );
        //        QCOMPARE( flags->getShortText(layoutUnit1, keyboardConfig), QString("usi") );
        //        QCOMPARE( flags->getShortText(layoutUnit2, keyboardConfig), QString("us") );
        //
        //        const Rules* rules = Rules::readRules();
        //        QCOMPARE( flags->getLongText(layoutUnit, rules), QString("USA") );
        //        QVERIFY( flags->getLongText(layoutUnit1, rules).startsWith("USA - International") );
        //        QCOMPARE( flags->getLongText(layoutUnit2, rules), QString("USA - other") );
        //
        //        flags->clearCache();
    }

    //    void loadRulesBenchmark() {
    //    	QBENCHMARK {
    //    		Flags* flags = new Flags();
    //    		delete flags;
    //    	}
    //    }

    void testKxkbrcChangeEmitsLayoutListChanged()
    {
        QSignalSpy spy(keyboardDaemon, &KeyboardDaemon::layoutListChanged);
        QVERIFY(spy.isValid());
        QCOMPARE(spy.count(), 0);

        const QString kxkbrc = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kxkbrc");
        QFile file(kxkbrc);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        file.write(QByteArrayLiteral("[Layout]\nLayoutList=gb\nUse=false\n"));
        file.close();

        // Longer than the daemon's 1000 ms debounce.
        QVERIFY(spy.wait(3000));
        QCOMPARE(spy.count(), 1);
    }

    // A user who has never configured layouts has no kxkbrc when the daemon
    // starts; the first save must still be picked up, and so must later ones.
    void testKxkbrcCreatedLaterIsWatched()
    {
        QSignalSpy spy(keyboardDaemon, &KeyboardDaemon::layoutListChanged);
        QVERIFY(spy.isValid());

        const QString kxkbrc = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kxkbrc");
        QVERIFY(QFile::remove(kxkbrc));
        // Let the debounce started by the removal run out before creating the file again.
        QTest::qWait(2000);
        spy.clear();

        // Created by rename, the way KConfig saves through QSaveFile.
        const QString staged = kxkbrc + QStringLiteral(".new");
        QFile created(staged);
        QVERIFY(created.open(QIODevice::WriteOnly | QIODevice::Text));
        created.write(QByteArrayLiteral("[Layout]\nLayoutList=de\nUse=false\n"));
        created.close();
        QVERIFY(QFile::rename(staged, kxkbrc));
        QVERIFY(spy.wait(3000));
        spy.clear();

        // Rewritten in place: no directory entry changes, so only a watch
        // on the file itself sees this.
        QFile rewritten(kxkbrc);
        QVERIFY(rewritten.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text));
        rewritten.write(QByteArrayLiteral("[Layout]\nLayoutList=fr\nUse=false\n"));
        rewritten.close();
        QVERIFY(spy.wait(3000));
    }
};

// need GUI for xkb protocol in xkb_rules.cpp
QTEST_MAIN(KeyboardDaemonTest)

#include "keyboard_daemon_test.moc"
