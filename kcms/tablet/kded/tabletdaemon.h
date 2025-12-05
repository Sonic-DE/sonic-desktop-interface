/*
    SPDX-FileCopyrightText: 2025 KDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KDEDModule>
#include <QHash>

class InputDevice;
class QScreen;

/**
 * @brief KDED module providing global keyboard shortcuts for tablet screen mapping
 *
 * This daemon automatically loads on session start and registers global shortcuts
 * to cycle tablets between screens or map them to specific screens.
 *
 * Thread safety: All D-Bus signals are delivered on the main Qt event loop thread.
 * No additional synchronization is needed for m_tablets access.
 */
class TabletDaemon : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.TabletDaemon")

public:
    TabletDaemon(QObject *parent, const QVariantList &args);
    ~TabletDaemon() override;

    // Maximum number of "Map to Screen N" shortcuts to pre-register.
    // Users with more than 10 screens can still use the cycle shortcut.
    // This limit prevents excessive shortcuts in the settings UI.
    static constexpr int MAX_SUPPORTED_SCREENS = 10;

public Q_SLOTS:
    /**
     * @brief Cycle all connected tablets to the next available screen
     *
     * Determines the current screen from the first tablet and cycles all tablets
     * to the next screen in the system's screen list.
     */
    Q_SCRIPTABLE void cycleScreen();

    /**
     * @brief Map all connected tablets to a specific screen by index
     * @param screenIndex Zero-based screen index (0 = first screen, 1 = second, etc.)
     *
     * If the screen index is out of range, shows an error OSD and does nothing.
     */
    Q_SCRIPTABLE void mapToScreen(int screenIndex);

private Q_SLOTS:
    void loadDevices();
    void onDeviceAdded(const QString &sysName);
    void onDeviceRemoved(const QString &sysName);
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);

private:
    void setupShortcuts();
    void showOSD(const QString &message, const QString &icon = QStringLiteral("video-display"));

    /**
     * @brief Ensure tablets are loaded, showing error if none found
     * @return true if at least one tablet is available, false otherwise
     */
    bool ensureTabletsLoaded();

    /**
     * @brief Get display-friendly name for a screen
     * @param screen The QScreen to get the name from
     * @return The screen's model name if available, otherwise its system name
     */
    QString getScreenDisplayName(QScreen *screen) const;

    // Key: device sysName (e.g., "event7"), Value: InputDevice object
    QHash<QString, InputDevice *> m_tablets;
};
