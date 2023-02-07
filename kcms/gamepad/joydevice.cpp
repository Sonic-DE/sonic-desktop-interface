/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

#include <KLocalizedString>
#include <QDebug>
#include <fcntl.h>

JoyDevice::JoyDevice(udev_device *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_brand(UnknownBrand)
    , m_model(i18n("Unknown Model"))
{
    m_name = udev_device_get_sysattr_value(m_device, "name");

    // if the event node is split up (which is most likely) from the device node, the name
    // actually exists on the parent node, so let's travel our way until we (hopefully) find it.
    auto parent_device = udev_device_get_parent(m_device);
    while (m_name.isEmpty() && parent_device != nullptr) {
        m_name = udev_device_get_sysattr_value(parent_device, "name");
        parent_device = udev_device_get_parent(parent_device);
    }

    auto devname = udev_device_get_property_value(m_device, "DEVNAME");

    qDebug() << "Opening evdev input stream " << devname;

    auto fd = open(devname, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qDebug() << "Failed to open fd!";
        return;
    }

    libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
        qDebug() << "Failed to open evdev stream!";
        return;
    }

    for (int code = BTN_A; code < BTN_A + 6; code++) {
        if (libevdev_has_event_code(dev, EV_KEY, code))
            m_numButtons++;
    }

    for (int code = BTN_LEFT; code < BTN_LEFT + 7; code++) {
        if (libevdev_has_event_code(dev, EV_KEY, code))
            m_numButtons++;
    }

    // TODO: Calculate numSticks by checking how many axes the device has
}

JoyDevice::JoyDevice()
{
}
