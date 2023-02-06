/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

#include <QDebug>
#include <QFile>
#include <QSocketNotifier>
#include <Solid/Block>
#include <Solid/GenericInterface>
#include <csignal>
#include <fcntl.h>

JoyDevice::JoyDevice(const Solid::Device &device, QObject *parent)
    : QObject(parent)
    , m_model(i18n("Unknown Model"))
{
    auto inputDevice = device.as<Solid::Block>();

    m_name = QStringLiteral("%1 %2").arg(device.vendor()).arg(device.product());

    m_vendor = device.vendor();

    auto fd = open(QFile::encodeName(inputDevice->device()), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        qDebug() << "Failed to open fd!";
        return;
    }

    int rc = libevdev_new_from_fd(fd, &m_device);
    if (rc < 0) {
        qDebug() << "Failed to open evdev stream!";
        return;
    }

    if (m_name == " ") {
        m_name = libevdev_get_name(m_device);
    }

    for (int code = BTN_A; code < BTN_A + 18; code++) {
        if (libevdev_has_event_code(m_device, EV_KEY, code)) {
            m_buttonCodes.append(code);
            m_numButtons++;
        }
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_X) || libevdev_has_event_code(m_device, EV_ABS, ABS_Y)
        || libevdev_has_event_code(m_device, EV_ABS, ABS_Z)) {
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_RX) || libevdev_has_event_code(m_device, EV_ABS, ABS_RY)
        || libevdev_has_event_code(m_device, EV_ABS, ABS_RZ)) {
        m_numAxes++;
    }

    m_hasRumble = libevdev_has_event_code(m_device, EV_FF, FF_RUMBLE);

    m_buttonState.resize(m_numButtons);
    m_axisState.resize(m_numAxes);

    auto notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &JoyDevice::poll);
}

JoyDevice::JoyDevice()
    : m_model(i18n("Unknown Model"))
{
}

void JoyDevice::poll()
{
    const int fd = libevdev_get_fd(m_device);
    struct input_event ev;
    int ret = read(fd, &ev, sizeof(ev));
    if (ret == 0) {
        qDebug() << "nothing to read";
    } else if (ret < 0) {
        qWarning() << "Error while reading" << strerror(errno);
    } else {
        processEvent(ev);
    }

    uint bytes;
    ret = ::ioctl(fd, FIONREAD, &bytes);
    if (ret == 0 && bytes >= sizeof(ev))
        poll();
}

QString JoyDevice::buttonName(int code)
{
    QString name = i18n("Unknown button %1", code);
    switch (code) {
    case BTN_WEST:
        name = i18nc("Western direction button", "West");
        break;
    case BTN_NORTH:
        name = i18nc("Northern direction button", "North");
        break;
    case BTN_EAST:
        name = i18nc("Eastern direction button", "East");
        break;
    case BTN_SOUTH:
        name = i18nc("Southern direction button", "South");
        break;
    case BTN_START:
        name = i18nc("Start button", "Start");
        break;
    case BTN_SELECT:
        name = i18nc("Select button", "Select");
        break;
    case BTN_C:
        name = i18nc("C button", "C");
        break;
    case BTN_Z:
        name = i18nc("Z button", "Z");
        break;
    case BTN_TR:
        name = i18nc("R1 button", "R1");
        break;
    case BTN_TR2:
        name = i18nc("R2 button", "R2");
        break;
    case BTN_TL:
        name = i18nc("L1 button", "L1");
        break;
    case BTN_TL2:
        name = i18nc("L2 button", "L2");
        break;
    case BTN_MODE:
        name = i18nc("Mode button", "Mode");
        break;
    case BTN_THUMBL:
        name = i18nc("Left thumb stick button", "Left Thumb");
        break;
    case BTN_THUMBR:
        name = i18nc("Right thumb stick button", "Right Thumb");
        break;
    }
    return name;
}

float JoyDevice::normalize(int code, __s32 value)
{
    // TODO: cache this
    auto info = libevdev_get_abs_info(m_device, code);

    __s32 adjustedValue = value - info->value;

    return adjustedValue < 0 ? -static_cast<float>(adjustedValue) / info->minimum : static_cast<float>(adjustedValue) / info->maximum;
}

void JoyDevice::processEvent(struct input_event &ev)
{
    if (ev.type == EV_KEY) {
        const int index = ev.code - BTN_SOUTH;
        if (index >= 0 && index < m_buttonState.size()) {
            m_buttonState[ev.code - BTN_SOUTH] = ev.value;

            emit buttonStateChanged();
        }
    } else if (ev.type == EV_ABS) {
        int axisIndex = ev.code != ABS_X && ev.code != ABS_Y && ev.code != ABS_Z;

        if (ev.code == ABS_Y || ev.code == ABS_RY) {
            m_axisState[axisIndex].setY(normalize(ev.code, ev.value));
        } else {
            m_axisState[axisIndex].setX(normalize(ev.code, ev.value));
        }

        emit axisStateChanged();
    }
}
