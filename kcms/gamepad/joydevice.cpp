/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joydevice.h"

#include "joybutton.h"

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
    // The various buttons we expect gamepads to possibly give us
    const QVector<int> buttonsToCheck = {
        // First normal dpad
        BTN_DPAD_UP,
        BTN_DPAD_DOWN,
        BTN_DPAD_LEFT,
        BTN_DPAD_RIGHT,

        // Then normal buttons
        BTN_SOUTH,
        BTN_EAST,
        BTN_C,
        BTN_NORTH,
        BTN_WEST,
        BTN_Z,
        BTN_TL,
        BTN_TR,
        BTN_TL2,
        BTN_TR2,
        BTN_SELECT,
        BTN_START,
        BTN_MODE,
        BTN_THUMBL,
        BTN_THUMBR,

        // Then weird buttons
        BTN_TRIGGER,
        BTN_THUMB,
        BTN_THUMB2,
        BTN_TOP,
        BTN_TOP2,
        BTN_PINKIE,
        BTN_BASE,
        BTN_BASE2,
        BTN_BASE3,
        BTN_BASE4,
        BTN_BASE5,
        BTN_BASE6,
    };

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

    for (int code : buttonsToCheck) {
        if (libevdev_has_event_code(m_device, EV_KEY, code)) {
            JoyButton *button = new JoyButton(m_vendor, code, this);
            m_buttons.insert(code, button);
            m_numButtons++;
        }
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_X) || libevdev_has_event_code(m_device, EV_ABS, ABS_Y)
        || libevdev_has_event_code(m_device, EV_ABS, ABS_Z)) {
        m_axisCodes.append(ABS_X);
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_RX) || libevdev_has_event_code(m_device, EV_ABS, ABS_RY)
        || libevdev_has_event_code(m_device, EV_ABS, ABS_RZ)) {
        m_axisCodes.append(ABS_RX);
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_HAT0X) || libevdev_has_event_code(m_device, EV_ABS, ABS_HAT0Y)) {
        m_axisCodes.append(ABS_HAT0X);
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_HAT1X) || libevdev_has_event_code(m_device, EV_ABS, ABS_HAT1Y)) {
        m_axisCodes.append(ABS_HAT1X);
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_HAT2X) || libevdev_has_event_code(m_device, EV_ABS, ABS_HAT2Y)) {
        m_axisCodes.append(ABS_HAT2X);
        m_numAxes++;
    }

    if (libevdev_has_event_code(m_device, EV_ABS, ABS_HAT3X) || libevdev_has_event_code(m_device, EV_ABS, ABS_HAT3Y)) {
        m_axisCodes.append(ABS_HAT3X);
        m_numAxes++;
    }

    m_hasRumble = libevdev_has_event_code(m_device, EV_FF, FF_RUMBLE);

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

QString JoyDevice::axisName(int code)
{
    QString name = i18n("Unknown axis %1", code);

    switch (code) {
    case ABS_X:
        name = i18nc("First axis", "First axis");
        break;
    case ABS_RX:
        name = i18nc("Second axis", "Second axis");
        break;
    case ABS_HAT0X:
        name = i18nc("First HAT device", "First HAT");
        break;
    case ABS_HAT1X:
        name = i18nc("Second HAT device", "Second HAT");
        break;
    case ABS_HAT2X:
        name = i18nc("Third HAT device", "Third HAT");
        break;
    case ABS_HAT3X:
        name = i18nc("Fourth HAT device", "Fourth HAT");
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
        if (m_buttons.contains(ev.code)) {
            qDebug() << "Setting state of button " << ev.code << " to value " << ev.value;
            JoyButton *button = m_buttons.value(ev.code);
            button->setState(ev.value);
        }
    } else if (ev.type == EV_ABS) {
        int axisIndex = 0;
        switch (ev.code) {
        case ABS_X:
        case ABS_Y:
        case ABS_Z:
            if (m_axisCodes.contains(ABS_X))
                axisIndex = m_axisCodes.indexOf(ABS_X);
            else
                axisIndex = -1;
            break;
        case ABS_RX:
        case ABS_RY:
        case ABS_RZ:
            if (m_axisCodes.contains(ABS_RX))
                axisIndex = m_axisCodes.indexOf(ABS_RX);
            else
                axisIndex = -1;
            break;
        case ABS_HAT0X:
        case ABS_HAT0Y:
            if (m_axisCodes.contains(ABS_HAT0X))
                axisIndex = m_axisCodes.indexOf(ABS_HAT0X);
            else
                axisIndex = -1;
            break;
        case ABS_HAT1X:
        case ABS_HAT1Y:
            if (m_axisCodes.contains(ABS_HAT1X))
                axisIndex = m_axisCodes.indexOf(ABS_HAT1X);
            else
                axisIndex = -1;
            break;
        case ABS_HAT2X:
        case ABS_HAT2Y:
            if (m_axisCodes.contains(ABS_HAT2X))
                axisIndex = m_axisCodes.indexOf(ABS_HAT2X);
            else
                axisIndex = -1;
            break;
        case ABS_HAT3X:
        case ABS_HAT3Y:
            if (m_axisCodes.contains(ABS_HAT3X))
                axisIndex = m_axisCodes.indexOf(ABS_HAT3X);
            else
                axisIndex = -1;
            break;
        }

        // At this point if axisIndex is -1 it means we got an unexpected event
        // so silently ignore
        if (axisIndex == -1)
            return;

        if (ev.code == ABS_Y || ev.code == ABS_RY || ev.code == ABS_HAT0Y || ev.code == ABS_HAT1Y || ev.code == ABS_HAT2Y || ev.code == ABS_HAT3Y) {
            m_axisState[axisIndex].setY(normalize(ev.code, ev.value));
        } else {
            m_axisState[axisIndex].setX(normalize(ev.code, ev.value));
        }

        emit axisStateChanged();
    }
}
