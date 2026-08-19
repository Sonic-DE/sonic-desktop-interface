/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "foldermodeltest.h"
#include "foldermodel.h"
#include "positioner.h"
#include "screenmapper.h"

#include <QEvent>
#include <QMimeData>
#include <QQuickItem>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

QTEST_MAIN(FolderModelTest)

static const QLatin1String desktop(QLatin1String("Desktop"));

static QUrl stringToUrl(const QString &path)
{
    return QUrl::fromUserInput(path, {}, QUrl::AssumeLocalFile);
}

void FolderModelTest::createTestFolder(const QString &path)
{
    QDir dir(m_folderDir->path());
    dir.mkdir(path);
    dir.cd(path);
    dir.mkdir(QStringLiteral("firstDir"));
    QFile f;
    for (int i = 1; i < 15; i++) {
        if (i < 10) {
            f.setFileName(QStringLiteral("%1/file%2.txt").arg(dir.path(), QString::number(i)));
        } else {
            f.setFileName(QStringLiteral("%1/.file%2.txt").arg(dir.path(), QString::number(i - 9)));
        }

        if (!f.open(QFile::WriteOnly)) {
            continue;
        }
        f.close();
    }
}

void FolderModelTest::init()
{
    m_currentActivity = QStringLiteral("00000000-0000-0000-0000-000000000000");
    m_folderDir = new QTemporaryDir();
    createTestFolder(desktop);
    m_folderModel = new FolderModel(this);
    m_folderModel->classBegin();
    m_folderModel->setUrl(m_folderDir->path() + QDir::separator() + desktop);
    m_folderModel->componentComplete();
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    s.wait(1000);
}

void FolderModelTest::cleanup()
{
    delete m_folderDir;
    m_folderDir = nullptr;
    delete m_folderModel;
    m_folderModel = nullptr;
}

void FolderModelTest::tst_listing()
{
    QCOMPARE(m_folderModel->url(), m_folderDir->path() + QDir::separator() + desktop);

    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 10);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(i));
    }
}

void FolderModelTest::tst_listingDescending()
{
    m_folderModel->setSortDesc(true);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    const auto count = m_folderModel->rowCount();
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(count - i));
    }
}

void FolderModelTest::tst_listingFolderNotFirst()
{
    const auto count = m_folderModel->rowCount();
    m_folderModel->setSortDirsFirst(false);
    QCOMPARE(count, 10);
    QCOMPARE(m_folderModel->index(9, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    for (int i = 0; i < count - 1; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(i + 1));
    }
}

void FolderModelTest::tst_listingHidden()
{
    m_folderModel->setShowHiddenFiles(true);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 15);
    QCOMPARE(m_folderModel->index(0, 0).data(FolderModel::FileNameRole).toString(), QLatin1String("firstDir"));
    for (int i = 1; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QString fileName = index.data(FolderModel::FileNameRole).toString();
        // hidden files are listed first
        if (i <= 5) {
            QCOMPARE(fileName, QStringLiteral(".file%1.txt").arg(i));
        } else {
            QCOMPARE(fileName, QStringLiteral("file%1.txt").arg(i - 5));
        }
    }
}

void FolderModelTest::tst_filterListing()
{
    // a little bit weird API, as both pattern and mimetype needs to be set
    m_folderModel->setFilterPattern(QStringLiteral("*.txt"));
    m_folderModel->setFilterMimeTypes(QStringList{QStringLiteral("all/all")});
    m_folderModel->setFilterMode(FolderModel::FilterShowMatches);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 9);
    for (int i = 0; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        QCOMPARE(index.data(FolderModel::FileNameRole).toString(), QStringLiteral("file%1.txt").arg(i + 1));
    }
}

void FolderModelTest::tst_cd()
{
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);

    // go into firstDir subfolder
    const auto url = m_folderModel->resolvedUrl();
    m_folderModel->cd(0);
    QVERIFY(s.wait(500));
    const auto url2 = m_folderModel->resolvedUrl();
    QVERIFY(url.isParentOf(url2));

    // go back to Desktop
    m_folderModel->up();
    // The first signal might come from KCoreDirListerCache::emitItemsFromCache
    QVERIFY(s.wait(500));
    // The second signal comes from KCoreDirListerCache::slotUpdateResult
    s.wait(500);
    QCOMPARE(m_folderModel->resolvedUrl(), url);

    // try to cd to an invalid entry (a file)
    m_folderModel->cd(1);
    // Signal is not emitted here as it's invalided
    QVERIFY(!s.wait(500));
    QCOMPARE(m_folderModel->resolvedUrl(), url);
}

void FolderModelTest::tst_rename_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("name");
    QTest::newRow("Folder rename") << 0 << "firstDirRenamed";
    QTest::newRow("File rename") << 1 << "file1.pdf";
    QTest::newRow("Invalid rename") << 11 << "foo";
}

void FolderModelTest::tst_rename()
{
    QFETCH(int, row);
    QFETCH(QString, name);
    m_folderModel->rename(row, name);
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    const auto index = m_folderModel->index(row, 0);
    s.wait(500);
    QEXPECT_FAIL("Invalid rename", "This is expected to fail", Continue);
    QCOMPARE(index.data(FolderModel::FileNameRole).toString(), name);
}

void FolderModelTest::tst_selection()
{
    m_folderModel->setSelected(1);
    QVERIFY(m_folderModel->hasSelection());
    QVERIFY(m_folderModel->isSelected(1));

    m_folderModel->clearSelection();
    QVERIFY(!m_folderModel->hasSelection());

    m_folderModel->toggleSelected(1);
    QVERIFY(m_folderModel->isSelected(1));
    m_folderModel->toggleSelected(1);
    QVERIFY(!m_folderModel->isSelected(1));

    m_folderModel->setRangeSelected(1, 4);
    QVERIFY(m_folderModel->hasSelection());
    for (int i = 1; i <= 4; i++) {
        QVERIFY(m_folderModel->isSelected(i));
    }

    m_folderModel->updateSelection({5, 6}, false);
    for (int i = 1; i <= 4; i++) {
        QVERIFY(!m_folderModel->isSelected(i));
    }
    QVERIFY(m_folderModel->isSelected(5));
    QVERIFY(m_folderModel->isSelected(6));

    m_folderModel->setRangeSelected(1, 4);
    m_folderModel->pinSelection();
    m_folderModel->updateSelection({5, 6}, true);
    for (int i = 1; i <= 6; i++) {
        QVERIFY(m_folderModel->isSelected(i));
    }

    m_folderModel->unpinSelection();
    m_folderModel->updateSelection({5, 6}, true);
    for (int i = 1; i <= 6; i++) {
        if (i < 5) {
            QVERIFY(!m_folderModel->isSelected(i));
        } else {
            QVERIFY(m_folderModel->isSelected(i));
        }
    }
}

void FolderModelTest::tst_defaultValues()
{
    FolderModel folderModel;
    QCOMPARE(folderModel.status(), FolderModel::Status::None);
    QVERIFY(folderModel.locked());
    QVERIFY(!folderModel.sortDesc());
    QVERIFY(folderModel.sortDirsFirst());
    QVERIFY(!folderModel.parseDesktopFiles());
    QVERIFY(!folderModel.previews());
    QVERIFY(!folderModel.usedByContainment());
    QCOMPARE(folderModel.sortMode(), 0);
    QCOMPARE(folderModel.filterMode(), (int)FolderModel::NoFilter);
    QVERIFY(folderModel.newMenu());
    QCOMPARE(folderModel.filterCaseSensitivity(), Qt::CaseInsensitive);
    QVERIFY(folderModel.dynamicSortFilter());
    QVERIFY(folderModel.isSortLocaleAware());
}

void FolderModelTest::tst_actionMenu()
{
    const QStringList lst{QStringLiteral("open"),
                          QStringLiteral("cut"),
                          QStringLiteral("open"),
                          QStringLiteral("cut"),
                          QStringLiteral("undo"),
                          QStringLiteral("copy"),
                          QStringLiteral("paste"),
                          QStringLiteral("pasteto"),
                          QStringLiteral("rename"),
                          QStringLiteral("trash"),
                          QStringLiteral("del"),
                          QStringLiteral("restoreFromTrash"),
                          QStringLiteral("emptyTrash")};
    for (const QString &str : lst) {
        QVERIFY(m_folderModel->action(str));
    }
}

void FolderModelTest::tst_lockedChanged()
{
    QSignalSpy s(m_folderModel, &FolderModel::lockedChanged);
    m_folderModel->setLocked(false);
    QCOMPARE(s.count(), 1);
    m_folderModel->setLocked(false);
    QCOMPARE(s.count(), 1);
    m_folderModel->setLocked(true);
    QCOMPARE(s.count(), 2);
}

void FolderModelTest::tst_multiScreen()
{
    delete m_folderModel;
    // Custom instance for this test to set used by containment before marking component
    // as complete.
    m_folderModel = new FolderModel(this);
    m_folderModel->classBegin();
    m_folderModel->setUrl(m_folderDir->path() + QDir::separator() + desktop);
    m_folderModel->setUsedByContainment(true);
    m_folderModel->setScreen(0);
    m_folderModel->componentComplete();

    auto *screenMapper = ScreenMapper::instance();

    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    QVERIFY(s.wait(1000));
    const auto count = m_folderModel->rowCount();
    for (int i = 0; i < count; i++) {
        const auto index = m_folderModel->index(i, 0);
        const auto name = index.data(FolderModel::UrlRole).toUrl();
        // all items are on the first screen by default
        QCOMPARE(screenMapper->screenForItem(name, m_currentActivity), 0);
    }

    // move one file to a new screen
    const auto movedItem = m_folderModel->index(0, 0).data(FolderModel::UrlRole).toUrl();
    FolderModel secondFolderModel;
    secondFolderModel.classBegin();
    secondFolderModel.setUrl(m_folderDir->path() + QDir::separator() + desktop);
    secondFolderModel.setUsedByContainment(true);
    secondFolderModel.setScreen(1);
    secondFolderModel.componentComplete();
    QSignalSpy s2(&secondFolderModel, &FolderModel::listingCompleted);
    QVERIFY(s2.wait(1000));
    const auto count2 = secondFolderModel.rowCount();
    QCOMPARE(count2, 0);

    screenMapper->addMapping(movedItem, 1, m_currentActivity);
    m_folderModel->invalidate();
    secondFolderModel.invalidate();
    s.wait(1000);
    s2.wait(1000);
    // we have one less item
    QCOMPARE(m_folderModel->rowCount(), count - 1);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(secondFolderModel.index(0, 0).data(FolderModel::UrlRole).toUrl(), movedItem);
    QCOMPARE(screenMapper->screenForItem(movedItem, m_currentActivity), 1);

    // remove extra screen, we have all items back
    screenMapper->removeScreen(1, m_currentActivity, stringToUrl(m_folderModel->url()));
    s.wait(500);
    QCOMPARE(m_folderModel->rowCount(), count);
    QCOMPARE(secondFolderModel.rowCount(), 0);
    QCOMPARE(screenMapper->screenForItem(movedItem, m_currentActivity), 0);

    // add back extra screen, the item is moved there
    screenMapper->addScreen(1, m_currentActivity, stringToUrl(m_folderModel->url()));
    s.wait(500);
    s2.wait(500);
    QCOMPARE(m_folderModel->rowCount(), count - 1);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(secondFolderModel.index(0, 0).data(FolderModel::UrlRole).toUrl(), movedItem);
    QCOMPARE(screenMapper->screenForItem(movedItem, m_currentActivity), 1);

    // create a new item, it appears on the first screen
    QDir dir(m_folderDir->path());
    dir.cd(desktop);
    dir.mkdir(QStringLiteral("secondDir"));
    dir.cd(QStringLiteral("secondDir"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count);
    QCOMPARE(secondFolderModel.rowCount(), 1);
    QCOMPARE(screenMapper->screenForItem(stringToUrl(QLatin1String("file://") + dir.path()), m_currentActivity), 0);
}

void FolderModelTest::tst_multiScreenDifferenPath()
{
    m_folderModel->setUsedByContainment(true);
    m_folderModel->setScreen(0);
    QSignalSpy s(m_folderModel, &FolderModel::listingCompleted);
    s.wait(1000);
    const auto count = m_folderModel->rowCount();
    QCOMPARE(count, 10);

    const QLatin1String desktop2(QLatin1String("Desktop2"));
    createTestFolder(desktop2);
    FolderModel secondFolderModel;
    secondFolderModel.setUsedByContainment(true);
    secondFolderModel.setUrl(m_folderDir->path() + QDir::separator() + desktop2);
    secondFolderModel.setScreen(1);
    QSignalSpy s2(&secondFolderModel, &FolderModel::listingCompleted);
    s2.wait(1000);
    const auto count2 = secondFolderModel.rowCount();
    QCOMPARE(count2, 10);

    // create a new item, it appears on the first screen
    QDir dir(m_folderDir->path());
    dir.cd(desktop);
    dir.mkdir(QStringLiteral("secondDir"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count + 1);
    QCOMPARE(secondFolderModel.rowCount(), count2);

    // create a new item, it appears on the second screen
    dir.cd(m_folderDir->path() + QDir::separator() + desktop2);
    dir.mkdir(QStringLiteral("secondDir2"));
    s.wait(1000);
    QCOMPARE(m_folderModel->rowCount(), count + 1);
    QCOMPARE(secondFolderModel.rowCount(), count2 + 1);
}

void FolderModelTest::tst_initializeOnCorrectScreens()
{
    createTestFolder(desktop);
    const QString path = m_folderDir->path() + QDir::separator() + desktop;

    // Create screen_0
    auto folderFirstScreen = new FolderModel(this);
    folderFirstScreen->classBegin();
    folderFirstScreen->setUrl(path);
    folderFirstScreen->setUsedByContainment(true);
    folderFirstScreen->setScreen(0);
    folderFirstScreen->componentComplete();
    QSignalSpy s1(folderFirstScreen, &FolderModel::listingCompleted);
    QVERIFY(s1.wait(1000));

    // Create a mapping where one file is already on disabled screen_1 for testing purposes
    auto *screenMapper = ScreenMapper::instance();
    auto newMap = screenMapper->screenMapping();
    newMap[1] = QString::number(1);

    // The item on disabled screen_1 should move to enabled screen_0
    QSignalSpy screenMapSpy(screenMapper, &ScreenMapper::screenMappingChanged);
    screenMapper->setScreenMapping(newMap);
    QCOMPARE(screenMapSpy.count(), 1);
    QCOMPARE(screenMapper->screenMapping()[1], QStringLiteral("0"));

    // Create screen_1
    auto folderSecondScreen = new FolderModel(this);
    folderSecondScreen->classBegin();
    folderSecondScreen->setUrl(path);
    folderSecondScreen->setUsedByContainment(true);
    folderSecondScreen->setScreen(1);
    folderSecondScreen->componentComplete();
    QSignalSpy s2(folderSecondScreen, &FolderModel::listingCompleted);
    QVERIFY(s2.wait(1000));

    // Now that the screen_1 is enabled again, the item should be back on the screen_1
    QCOMPARE(screenMapper->screenMapping(), newMap);
}

void FolderModelTest::tst_userCrossScreenDrag()
{
    auto *applet = new Plasma::Applet(this, KPluginMetaData(), QVariantList{});
    applet->config().deleteGroup(QStringLiteral("General"));

    const QString path = m_folderDir->path() + QDir::separator() + desktop;
    const QUrl desktopUrl = QUrl::fromLocalFile(path);

    auto *screenMapper = ScreenMapper::instance();
    const QStringList originalMapping = screenMapper->screenMapping();

    auto *model0 = new FolderModel(this);
    model0->classBegin();
    model0->setUrl(path);
    model0->setUsedByContainment(true);
    model0->setScreen(0);
    model0->componentComplete();
    QVERIFY(QSignalSpy(model0, &FolderModel::listingCompleted).wait(1000));

    auto *model1 = new FolderModel(this);
    model1->classBegin();
    model1->setUrl(path);
    model1->setUsedByContainment(true);
    model1->setScreen(1);
    model1->componentComplete();
    QVERIFY(QSignalSpy(model1, &FolderModel::listingCompleted).wait(1000));

    auto *positioner1 = new Positioner(this);
    positioner1->setApplet(applet);
    positioner1->setEnabled(true);
    positioner1->setFolderModel(model1);
    positioner1->setPerStripe(3);

    QCOMPARE(model0->rowCount(), 10);
    QCOMPARE(model1->rowCount(), 0);

    const QUrl file1Url = model0->index(0, 0).data(FolderModel::UrlRole).toUrl();

    auto *mimeData = new QMimeData();
    mimeData->setUrls({file1Url});

    auto *dropEventObj = new QObject(this);
    dropEventObj->setProperty("mimeData", QVariant::fromValue<QObject *>(mimeData));
    dropEventObj->setProperty("x", 480);
    dropEventObj->setProperty("y", 270);
    dropEventObj->setProperty("proposedAction", (int)Qt::MoveAction);
    dropEventObj->setProperty("possibleActions", (int)(Qt::MoveAction | Qt::CopyAction));
    dropEventObj->setProperty("buttons", (int)Qt::LeftButton);
    dropEventObj->setProperty("modifiers", (int)Qt::NoModifier);

    auto *target = new QQuickItem();

    model1->drop(target, dropEventObj, -1);

    QTest::qWait(2000);

    QCOMPARE(model1->rowCount(), 1);
    QCOMPARE(model1->index(0, 0).data(FolderModel::UrlRole).toUrl(), file1Url);
    QCOMPARE(model0->rowCount(), 9);

    const int proxyIndex1 = positioner1->indexForUrl(file1Url);
    QVERIFY(proxyIndex1 >= 0);
    QVERIFY(!positioner1->isBlank(proxyIndex1));
    QCOMPARE(positioner1->data(positioner1->index(proxyIndex1, 0), FolderModel::UrlRole).toUrl(), file1Url);

    delete model0;
    delete model1;
    delete positioner1;
    delete applet;
    screenMapper->removeScreen(0, m_currentActivity, desktopUrl);
    screenMapper->removeScreen(1, m_currentActivity, desktopUrl);
    screenMapper->setScreenMapping(originalMapping);
}

void FolderModelTest::tst_userCrossScreenDragMultiple()
{
    auto *applet = new Plasma::Applet(this, KPluginMetaData(), QVariantList{});
    applet->config().deleteGroup(QStringLiteral("General"));

    const QString path = m_folderDir->path() + QDir::separator() + desktop;
    const QUrl desktopUrl = QUrl::fromLocalFile(path);

    auto *screenMapper = ScreenMapper::instance();
    const QStringList originalMapping = screenMapper->screenMapping();

    auto *model0 = new FolderModel(this);
    model0->classBegin();
    model0->setUrl(path);
    model0->setUsedByContainment(true);
    model0->setScreen(0);
    model0->componentComplete();
    QVERIFY(QSignalSpy(model0, &FolderModel::listingCompleted).wait(1000));

    auto *model1 = new FolderModel(this);
    model1->classBegin();
    model1->setUrl(path);
    model1->setUsedByContainment(true);
    model1->setScreen(1);
    model1->componentComplete();
    QVERIFY(QSignalSpy(model1, &FolderModel::listingCompleted).wait(1000));

    auto *positioner1 = new Positioner(this);
    positioner1->setApplet(applet);
    positioner1->setEnabled(true);
    positioner1->setFolderModel(model1);
    positioner1->setPerStripe(3);

    QCOMPARE(model0->rowCount(), 10);
    QCOMPARE(model1->rowCount(), 0);

    auto dropUrls = [&](const QList<QUrl> &urls) {
        auto *mimeData = new QMimeData();
        mimeData->setUrls(urls);
        auto *dropEventObj = new QObject(this);
        dropEventObj->setProperty("mimeData", QVariant::fromValue<QObject *>(mimeData));
        dropEventObj->setProperty("x", 480);
        dropEventObj->setProperty("y", 270);
        dropEventObj->setProperty("proposedAction", (int)Qt::MoveAction);
        dropEventObj->setProperty("possibleActions", (int)(Qt::MoveAction | Qt::CopyAction));
        dropEventObj->setProperty("buttons", (int)Qt::LeftButton);
        dropEventObj->setProperty("modifiers", (int)Qt::NoModifier);
        auto *target = new QQuickItem();
        model1->drop(target, dropEventObj, -1);
        QTest::qWait(2000);
        delete target;
        delete dropEventObj;
        delete mimeData;
    };

    auto assertMapIntegrity = [&]() {
        const auto p2s = positioner1->proxyToSourceMapping();
        const auto s2p = positioner1->sourceToProxyMapping();
        QCOMPARE(p2s.size(), model1->rowCount());
        QCOMPARE(s2p.size(), model1->rowCount());
        QList<int> sourceValues = p2s.values();
        std::sort(sourceValues.begin(), sourceValues.end());
        QList<int> uniqueSources = sourceValues;
        uniqueSources.erase(std::unique(uniqueSources.begin(), uniqueSources.end()), uniqueSources.end());
        QCOMPARE(sourceValues, uniqueSources);
        for (auto it = p2s.cbegin(); it != p2s.cend(); ++it) {
            QCOMPARE(s2p.value(it.value()), it.key());
        }
    };

    const QList<QUrl> firstBatch = {
        model0->index(0, 0).data(FolderModel::UrlRole).toUrl(),
        model0->index(1, 0).data(FolderModel::UrlRole).toUrl(),
        model0->index(2, 0).data(FolderModel::UrlRole).toUrl(),
    };
    dropUrls(firstBatch);

    QCOMPARE(model1->rowCount(), 3);
    QCOMPARE(model0->rowCount(), 7);
    assertMapIntegrity();

    positioner1->move({0, 10});
    QVERIFY(positioner1->isBlank(0));
    assertMapIntegrity();

    const QList<QUrl> secondBatch = {
        model0->index(0, 0).data(FolderModel::UrlRole).toUrl(),
        model0->index(1, 0).data(FolderModel::UrlRole).toUrl(),
        model0->index(2, 0).data(FolderModel::UrlRole).toUrl(),
    };
    dropUrls(secondBatch);

    QCOMPARE(model1->rowCount(), 6);
    QCOMPARE(model0->rowCount(), 4);
    assertMapIntegrity();

    QSet<int> proxyCells;
    for (const QUrl &url : firstBatch + secondBatch) {
        const int proxy = positioner1->indexForUrl(url);
        QVERIFY2(proxy >= 0, qPrintable(QStringLiteral("no proxy for %1").arg(url.toString())));
        QVERIFY(!positioner1->isBlank(proxy));
        QCOMPARE(positioner1->data(positioner1->index(proxy, 0), FolderModel::UrlRole).toUrl(), url);
        QVERIFY2(!proxyCells.contains(proxy), qPrintable(QStringLiteral("duplicate proxy cell %1").arg(proxy)));
        proxyCells.insert(proxy);
    }

    delete model0;
    delete model1;
    delete positioner1;
    delete applet;
    screenMapper->removeScreen(0, m_currentActivity, desktopUrl);
    screenMapper->removeScreen(1, m_currentActivity, desktopUrl);
    screenMapper->setScreenMapping(originalMapping);
}

void FolderModelTest::tst_userCrossScreenRoundTrip()
{
    auto *applet = new Plasma::Applet(this, KPluginMetaData(), QVariantList{});
    applet->config().deleteGroup(QStringLiteral("General"));

    const QString path = m_folderDir->path() + QDir::separator() + desktop;
    const QUrl desktopUrl = QUrl::fromLocalFile(path);

    auto *screenMapper = ScreenMapper::instance();
    const QStringList originalMapping = screenMapper->screenMapping();

    auto *model0 = new FolderModel(this);
    model0->classBegin();
    model0->setUrl(path);
    model0->setUsedByContainment(true);
    model0->setScreen(0);
    model0->componentComplete();
    QVERIFY(QSignalSpy(model0, &FolderModel::listingCompleted).wait(1000));

    auto *model1 = new FolderModel(this);
    model1->classBegin();
    model1->setUrl(path);
    model1->setUsedByContainment(true);
    model1->setScreen(1);
    model1->componentComplete();
    QVERIFY(QSignalSpy(model1, &FolderModel::listingCompleted).wait(1000));

    auto *positioner0 = new Positioner(this);
    positioner0->setApplet(applet);
    positioner0->setEnabled(true);
    positioner0->setFolderModel(model0);
    positioner0->setPerStripe(3);

    auto *positioner1 = new Positioner(this);
    positioner1->setApplet(applet);
    positioner1->setEnabled(true);
    positioner1->setFolderModel(model1);
    positioner1->setPerStripe(3);

    QCOMPARE(model0->rowCount(), 10);
    QCOMPARE(model1->rowCount(), 0);

    auto dropUrls = [](FolderModel *target, const QList<QUrl> &urls) {
        auto *mimeData = new QMimeData();
        mimeData->setUrls(urls);
        auto *dropEventObj = new QObject();
        dropEventObj->setProperty("mimeData", QVariant::fromValue<QObject *>(mimeData));
        dropEventObj->setProperty("x", 480);
        dropEventObj->setProperty("y", 270);
        dropEventObj->setProperty("proposedAction", (int)Qt::MoveAction);
        dropEventObj->setProperty("possibleActions", (int)(Qt::MoveAction | Qt::CopyAction));
        dropEventObj->setProperty("buttons", (int)Qt::LeftButton);
        dropEventObj->setProperty("modifiers", (int)Qt::NoModifier);
        auto *targetItem = new QQuickItem();
        target->drop(targetItem, dropEventObj, -1);
        QTest::qWait(2000);
        delete targetItem;
        delete dropEventObj;
        delete mimeData;
    };

    auto assertIntegrity = [](Positioner *p, FolderModel *m) {
        const auto p2s = p->proxyToSourceMapping();
        const auto s2p = p->sourceToProxyMapping();
        QCOMPARE(p2s.size(), m->rowCount());
        QCOMPARE(s2p.size(), m->rowCount());
        QList<int> sourceValues = p2s.values();
        std::sort(sourceValues.begin(), sourceValues.end());
        QList<int> uniqueSources = sourceValues;
        uniqueSources.erase(std::unique(uniqueSources.begin(), uniqueSources.end()), uniqueSources.end());
        QCOMPARE(sourceValues, uniqueSources);
        for (auto it = p2s.cbegin(); it != p2s.cend(); ++it) {
            QCOMPARE(s2p.value(it.value()), it.key());
        }
    };

    const QList<QUrl> moved = {
        model0->index(0, 0).data(FolderModel::UrlRole).toUrl(),
        model0->index(1, 0).data(FolderModel::UrlRole).toUrl(),
    };

    dropUrls(model1, moved);
    QCOMPARE(model1->rowCount(), 2);
    QCOMPARE(model0->rowCount(), 8);
    assertIntegrity(positioner0, model0);
    assertIntegrity(positioner1, model1);

    const QList<QUrl> returning = {
        model1->index(0, 0).data(FolderModel::UrlRole).toUrl(),
        model1->index(1, 0).data(FolderModel::UrlRole).toUrl(),
    };

    dropUrls(model0, returning);
    QCOMPARE(model0->rowCount(), 10);
    QCOMPARE(model1->rowCount(), 0);
    assertIntegrity(positioner0, model0);
    assertIntegrity(positioner1, model1);

    QSet<QUrl> screen0Urls;
    for (int i = 0; i < model0->rowCount(); ++i) {
        const QUrl url = model0->index(i, 0).data(FolderModel::UrlRole).toUrl();
        QVERIFY2(!screen0Urls.contains(url), qPrintable(QStringLiteral("duplicate url on screen0: %1").arg(url.toString())));
        screen0Urls.insert(url);
    }
    for (const QUrl &url : moved) {
        QVERIFY2(screen0Urls.contains(url), qPrintable(QStringLiteral("lost url on return: %1").arg(url.toString())));
    }

    delete model0;
    delete model1;
    delete positioner0;
    delete positioner1;
    delete applet;
    screenMapper->removeScreen(0, m_currentActivity, desktopUrl);
    screenMapper->removeScreen(1, m_currentActivity, desktopUrl);
    screenMapper->setScreenMapping(originalMapping);
}

void FolderModelTest::tst_crossScreenSelectionClears()
{
    const QString path = m_folderDir->path() + QDir::separator() + desktop;
    const QUrl desktopUrl = QUrl::fromLocalFile(path);

    auto *screenMapper = ScreenMapper::instance();
    const QStringList originalMapping = screenMapper->screenMapping();

    auto *model0 = new FolderModel(this);
    model0->classBegin();
    model0->setUrl(path);
    model0->setUsedByContainment(true);
    model0->setScreen(0);
    model0->componentComplete();
    QVERIFY(QSignalSpy(model0, &FolderModel::listingCompleted).wait(1000));

    auto *model1 = new FolderModel(this);
    model1->classBegin();
    model1->setUrl(path);
    model1->setUsedByContainment(true);
    model1->setScreen(1);
    model1->componentComplete();
    QVERIFY(QSignalSpy(model1, &FolderModel::listingCompleted).wait(1000));

    screenMapper->addMapping(model0->index(1, 0).data(FolderModel::UrlRole).toUrl(), 1, m_currentActivity);
    screenMapper->addMapping(model0->index(2, 0).data(FolderModel::UrlRole).toUrl(), 1, m_currentActivity);
    QTest::qWait(200);
    QVERIFY(model1->rowCount() >= 1);

    model0->setSelected(0);
    QVERIFY(model0->hasSelection());
    QVERIFY(model0->isSelected(0));
    QVERIFY(!model1->hasSelection());

    model1->setSelected(0);
    QVERIFY(model1->hasSelection());
    QVERIFY(model1->isSelected(0));
    QVERIFY2(!model0->hasSelection(), "selecting on screen 1 should clear screen 0 selection");

    model0->setSelected(0);
    QVERIFY(model0->hasSelection());
    QVERIFY(model0->isSelected(0));
    QVERIFY2(!model1->hasSelection(), "selecting on screen 0 should clear screen 1 selection");

    delete model0;
    delete model1;
    screenMapper->removeScreen(0, m_currentActivity, desktopUrl);
    screenMapper->removeScreen(1, m_currentActivity, desktopUrl);
    screenMapper->setScreenMapping(originalMapping);
}
