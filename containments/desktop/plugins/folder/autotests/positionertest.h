/*
    SPDX-FileCopyrightText: 2017 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>
    SPDX-FileCopyrightText: Andras Mantia <andras.mantia@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Plasma/Applet>
#include <QJsonDocument>
#include <QObject>

class QTemporaryDir;
class FolderModel;
class Positioner;

class PositionerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void tst_positions_data();
    void tst_positions();
    void tst_map();
    void tst_move_data();
    void tst_move();
    void tst_nearestitem_data();
    void tst_nearestitem();
    void tst_isBlank();
    void tst_reset();
    void tst_defaultValues();
    void tst_changeEnabledStatus();
    void tst_changePerStripe();
    void tst_proxyMapping();
    void tst_config();

private:
    void checkPositions(int perStripe);
    void ensureFolderModelReady();
    QJsonDocument getCurrentConfig();

    QString m_currentActivity;
    Positioner *m_positioner;
    FolderModel *m_folderModel;
    QTemporaryDir *m_folderDir;
    Plasma::Applet *m_applet = nullptr;
};
