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

        QVERIFY(spy.wait(1000));
        QCOMPARE(spy.count(), 1);
    }
};

// need GUI for xkb protocol in xkb_rules.cpp
QTEST_MAIN(KeyboardDaemonTest)

#include "keyboard_daemon_test.moc"
