/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AutomounterSettings.h"

void AutomounterSettings::usrRead()
{
    qDeleteAll(m_devices);
    m_devices.clear();
    for (const QString &udi : knownDevices()) {
        m_devices[udi] = new DeviceSettings(sharedConfig(), udi, this);
    }
}

bool AutomounterSettings::usrSave()
{
    bool saveOk = false;
    for (const auto settings : m_devices.values()) {
        saveOk &= settings->save();
    }
    return saveOk;
}

bool AutomounterSettings::usrIsSaveNeeded()
{
    return isSaveNeeded() || std::any_of(m_devices.cbegin(), m_devices.cend(), [](const DeviceSettings *device) {
               return device->isSaveNeeded();
           });
}

QStringList AutomounterSettings::knownDevices() const
{
    return config()->group("Devices").groupList();
}

DeviceSettings *AutomounterSettings::deviceSettings(const QString &udi) const
{
    return m_devices.value(udi);
}

bool AutomounterSettings::shouldAutomountDevice(const QString &udi, AutomountType type) const
{
    // We auto-mount the device, if:
    // 1.) auto-mounting is forced, or
    // 2.) auto-mounting is enabled
    //     and auto-mounting on login/attach is configured
    //     and the device is known, has been seen mounted last, or is unknown to us

    const bool automountUnknown = automountUnknownDevices();
    const bool known = deviceSettings(udi)->isKnown();
    const bool lastSeenMounted = deviceSettings(udi)->lastSeenMounted();

    bool typeCondition = false;
    bool deviceAutomount = false;
    switch (type) {
    case Login:
        typeCondition = automountOnLogin();
        deviceAutomount = deviceSettings(udi)->mountOnLogin();
        break;
    case Attach:
        typeCondition = automountOnPlugin();
        deviceAutomount = deviceSettings(udi)->mountOnAttach();
        break;
    }

    return deviceAutomount || (typeCondition && (known || lastSeenMounted || automountUnknown));
}

void AutomounterSettings::setDeviceLastSeenMounted(const QString &udi, bool mounted)
{
    if (!m_devices.contains(udi)) {
        m_devices[udi] = new DeviceSettings(sharedConfig(), udi, this);
    }
    if (mounted) {
        deviceSettings(udi)->setIsKnown(true);
    }
    deviceSettings(udi)->setLastSeenMounted(mounted);
}

void AutomounterSettings::setDeviceInfo(const Solid::Device &dev)
{
    const QString udi = dev.udi();
    if (!m_devices.contains(udi)) {
        m_devices[udi] = new DeviceSettings(sharedConfig(), udi, this);
    }
    auto settings = deviceSettings(udi);
    settings->setName(dev.description());
    settings->setIcon(dev.icon());
}

void AutomounterSettings::removeDeviceGroup(const QString &udi)
{
    if (config()->group("Devices").hasGroup(udi)) {
        config()->group("Devices").group(udi).deleteGroup();
    }
}
