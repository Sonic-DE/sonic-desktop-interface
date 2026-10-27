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

// Remembers which configured layout the user selected. X merges layouts whose
// keymaps are identical (us and au, for example) into one group and wraps a
// locked group past the last real one back into range, so the X group alone
// can't tell such layouts apart.
class LayoutSelection {
public:
    void select(uint layout, uint xGroup)
    {
        m_layout = layout;
        m_xGroup = xGroup;
    }
    void reset()
    {
        m_layout.reset();
    }
    // The selected layout while X still sits on the group it landed on;
    // otherwise something else moved X, and X's group is the truth.
    uint current(uint xGroup) const
    {
        return m_layout && xGroup == m_xGroup ? *m_layout : xGroup;
    }

private:
    std::optional<uint> m_layout;
    uint m_xGroup = 0;
};

class Q_DECL_EXPORT KeyboardDaemon : public KDEDModule
{
    Q_OBJECT

    KeyboardSettings *keyboardSettings;
    KeyboardConfig *keyboardConfig;
    KeyboardLayoutActionCollection *actionCollection;
    XInputEventNotifier *xEventNotifier;
    LayoutMemory layoutMemory;
    std::optional<uint> lastUsedLayout;
    LayoutSelection layoutSelection;

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
    bool selectLayout(uint index);
    void scrollLayouts(int delta);
    LayoutUnit currentLayoutUnit() const;

    void broadcastLayoutsChanged();
    void broadcastLayoutChanged(uint index);

private Q_SLOTS:
    void configureKeyboard();
    void configureInput();
    void layoutChangedSlot();
    bool setLayout(QAction *action);
    void kxkbrcChanged(const QString &path);
    void kxkbrcDirectoryChanged(const QString &path);
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
