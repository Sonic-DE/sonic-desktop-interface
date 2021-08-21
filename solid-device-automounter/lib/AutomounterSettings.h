/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AUTOMOUNTERSETTINGS_H
#define AUTOMOUNTERSETTINGS_H

#include "AutomounterSettingsBase.h"
#include "DeviceSettings.h"

#include <KConfigGroup>

#include <Solid/Device>

class AutomounterSettings : public AutomounterSettingsBase
{
public:
    using AutomounterSettingsBase::AutomounterSettingsBase;
    enum AutomountType {
        Login,
        Attach,
    };
    DeviceSettings *deviceSettings(const QString &udi) const;
    QStringList knownDevices() const;
    bool shouldAutomountDevice(const QString &udi, AutomountType type) const;
    void setDeviceLastSeenMounted(const QString &udi, bool mounted);

    void saveDevice(const Solid::Device &dev);
    void removeDeviceGroup(const QString &udi);

private:
    bool deviceIsKnown(const QString &udi);
    bool deviceAutomountIsForced(const QString &udi, AutomountType type);
    QString getDeviceName(const QString &udi);
    bool getDeviceForcedAutomount(const QString &udi);
    QString getDeviceIcon(const QString &udi);

private:
    void usrRead() override;
    bool usrSave() override;
    KConfigGroup deviceSettingsGroup(const QString &udi);

private:
    QHash<QString, DeviceSettings *> m_devices;
};

#endif
