/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KDEDModule>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QTimer>
#include <optional>

#include "bindings.h"
#include "layout_memory.h"
#include "layoutnames.h"

class XInputEventNotifier;
class KeyboardConfig;
class KeyboardSettings;
class KeyboardIpcServer;

class Q_DECL_EXPORT KeyboardDaemon : public KDEDModule
{
    Q_OBJECT

    KeyboardSettings *keyboardSettings;
    KeyboardConfig *keyboardConfig;
    KeyboardLayoutActionCollection *actionCollection;
    XInputEventNotifier *xEventNotifier;
    LayoutMemory layoutMemory;
    std::optional<uint> lastUsedLayout;

    QFileSystemWatcher *keyboardFileWatcher = nullptr;
    QTimer *keyboardFileDebounce = nullptr;
    QTimer *layoutMapDebounce = nullptr;
    KeyboardIpcServer *m_ipcServer = nullptr;

    QString kxkbrcPath() const;

    void registerListeners();
    void registerShortcut();
    void unregisterListeners();
    void unregisterShortcut();
    void setLastUsedLayoutValue(uint newValue);

    void broadcastLayoutsChanged();
    void broadcastLayoutChanged(uint index);

private Q_SLOTS:
    void configureKeyboard();
    void configureInput();
    void layoutChangedSlot();
    bool setLayout(QAction *action);
    void kxkbrcChanged(const QString &path);
    void kxkbrcDebounceTimeout();

public Q_SLOTS:
    void switchToNextLayout();
    void switchToPreviousLayout();
    bool setLayout(uint index);
    uint getLayout() const;
    QList<LayoutNames> getLayoutsList() const;

Q_SIGNALS:
    void layoutChanged(uint index);
    void layoutListChanged();

public:
    KeyboardDaemon(QObject *parent, const QList<QVariant> &);
    ~KeyboardDaemon() override;
};
