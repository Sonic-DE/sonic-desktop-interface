/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <QDebug>
#include <libudev.h>

#include "joydevice.h"

DeviceModel::DeviceModel()
{
    auto udev = udev_new();
    if (!udev) {
        qDebug() << "Failed to create udev context!";
    }

    auto enumerate = udev_enumerate_new(udev);

    udev_enumerate_scan_devices(enumerate);
    if (!enumerate) {
        qDebug() << "Cannot create enumerate context!";
    }

    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);

    auto devices = udev_enumerate_get_list_entry(enumerate);
    if (!devices) {
        qDebug() << "Failed to query udev devices!";
    }

    udev_list_entry *dev_list_entry = nullptr;

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path, *tmp;

        path = udev_list_entry_get_name(dev_list_entry);
        auto dev = udev_device_new_from_syspath(udev, path);

        auto val = udev_device_get_property_value(dev, "ID_INPUT_JOYSTICK");
        if (val != nullptr && strcmp(val, "1") == 0) {
            auto devnode = udev_device_get_devnode(dev);

            // we only care about eventX nodes, ignore jsX and inputX
            if (devnode != nullptr && strstr(devnode, "event") != nullptr) {
                auto joy = new JoyDevice(dev, this);

                m_devices.push_back(joy);
            }
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}

JoyDevice *DeviceModel::device(int index) const
{
    if (index < 0 || index > m_devices.count())
        return nullptr;

    return m_devices.at(index);
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case CustomRoles::NameRole:
        return m_devices[index.row()]->getName();
    case CustomRoles::DeviceRole:
        return QVariant::fromValue(m_devices[index.row()]);
    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{CustomRoles::NameRole, "name"}, {CustomRoles::DeviceRole, "device"}};
}
