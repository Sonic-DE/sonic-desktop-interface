/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

// Qt
#include <QAbstractItemModel>
#include <QHash>
#include <QJSValue>
#include <QKeySequence>
#include <QMimeData>
#include <QObject>
#include <QPixmap>
#include <QTimer>

#include <qqmlregistration.h>

// KDE
#include <PlasmaActivities/Controller>

// Local
#include "sortedactivitiesmodel.h"

class QAction;
class QRasterWindow;
class QQmlEngine;
class QJSEngine;

namespace KIO
{
class PreviewJob;
}

class SwitcherBackendModKeyTracker;

class SwitcherBackend : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_NAMED_ELEMENT(Backend)
    QML_SINGLETON

    Q_PROPERTY(bool shouldShowSwitcher READ shouldShowSwitcher WRITE setShouldShowSwitcher NOTIFY shouldShowSwitcherChanged)
    Q_PROPERTY(bool dropEnabled READ dropEnabled CONSTANT)

public:
    explicit SwitcherBackend(QObject *parent);
    ~SwitcherBackend() override;

    static SwitcherBackend *create(QQmlEngine *engine, QJSEngine *scriptEngine);

Q_SIGNALS:
    void showSwitchNotification(const QString &id, const QString &name, const QString &icon);
    void shouldShowSwitcherChanged(bool value);

public Q_SLOTS:
    void init();

    bool shouldShowSwitcher() const;
    void setShouldShowSwitcher(bool shouldShowSwitcher);

    QAbstractItemModel *runningActivitiesModel() const;
    QAbstractItemModel *stoppedActivitiesModel() const;

    void setCurrentActivity(const QString &activity);
    void stopActivity(const QString &activity);
    void removeActivity(const QString &activity);

    void setDropMode(bool value);
    void drop(QMimeData *mimeData, int modifiers, const QVariant &activityId);
    void dropCopy(QMimeData *mimeData, const QVariant &activityId);
    void dropMove(QMimeData *mimeData, const QVariant &activityId);
    bool dropEnabled() const;

    void toggleActivityManager();

private:
    template<typename Handler>
    inline void registerShortcut(const QString &actionName, const QString &name, const QKeySequence &shortcut, Handler &&handler);

    enum Direction {
        Next,
        Previous,
    };

    void switchToActivity(Direction i);

private Q_SLOTS:
    void keybdSwitchToNextActivity();
    void keybdSwitchToPreviousActivity();
    void keybdSwitchedToAnotherActivity();

    void updateShouldShowSwitcherWithModifiersPressed(bool modifiersPressed);

    void onCurrentActivityChanged(const QString &id);

private:
    friend class SwitcherBackendModKeyTracker;

    QHash<QString, QKeySequence> m_actionShortcut;
    SwitcherBackendModKeyTracker *m_currentModKeyTracker = nullptr;
    KActivities::Controller m_activities;
    QString m_previousActivity;
    bool m_shouldShowSwitcher;

    bool m_dropModeActive;
    QTimer m_dropModeHider;

    SortedActivitiesModel *m_runningActivitiesModel = nullptr;
    SortedActivitiesModel *m_stoppedActivitiesModel = nullptr;
};

class SwitcherBackendModKeyTracker : public QObject
{
    Q_OBJECT

public:
    static const int ActivitySwitcherShowDelay = 100;
    static const int ModKeyPollingInterval = 100;
    static const int CurrentActivityChangedTimeout = 2000;
    static const int InputWindowActivationTimeout = 1000;

public:
    SwitcherBackendModKeyTracker(SwitcherBackend *parent, Qt::KeyboardModifiers trackedActionModifiers);
    ~SwitcherBackendModKeyTracker() override;

    void destroyIfNotQueriedWithinMsec(int msec);
    void showActivitySwitcherIfNeededAfterMsec(int delayInMsec);

private Q_SLOTS:
    void showActivitySwitcherIfNeededAsap();

private:
    void queryModifiersAndUpdateShouldShowSwitcher();

private:
    SwitcherBackend *m_parent;
    Qt::KeyboardModifiers m_trackedActionModifiers;
    QTimer m_modKeyPollingTimer;
    QRasterWindow *m_inputWindow = nullptr;
    qint64 m_timeoutId = 0;
    int m_previousInputWindowSetupDelayMsec = 0;
};
