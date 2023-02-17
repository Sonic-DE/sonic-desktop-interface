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
            m_buttonCodes.append(code);
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
    case BTN_DPAD_UP:
        name = i18nc("Up button", "Up");
        break;
    case BTN_DPAD_DOWN:
        name = i18nc("Down button", "Down");
        break;
    case BTN_DPAD_LEFT:
        name = i18nc("Left button", "Left");
        break;
    case BTN_DPAD_RIGHT:
        name = i18nc("Right button", "Right");
        break;
    case BTN_TRIGGER:
        name = i18nc("Trigger button", "Trigger");
        break;
    case BTN_THUMB:
        name = i18nc("Thumb button", "Thumb");
        break;
    case BTN_THUMB2:
        name = i18nc("Second thumb button", "Thumb 2");
        break;
    case BTN_TOP:
        name = i18nc("Top button", "Top");
        break;
    case BTN_TOP2:
        name = i18nc("Second top button", "Top 2");
        break;
    case BTN_PINKIE:
        name = i18nc("Pinkie (smallest finger) button", "Pinkie");
        break;
    case BTN_BASE:
        name = i18nc("First base button", "Base 1");
        break;
    case BTN_BASE2:
        name = i18nc("Second base button", "Base 2");
        break;
    case BTN_BASE3:
        name = i18nc("Third base button", "Base 3");
        break;
    case BTN_BASE4:
        name = i18nc("Fourth base button", "Base 4");
        break;
    case BTN_BASE5:
        name = i18nc("Fifth base button", "Base 5");
        break;
    case BTN_BASE6:
        name = i18nc("Sixfth base button", "Base 6");
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
        if (m_buttonCodes.contains(ev.code)) {
            const int index = m_buttonCodes.indexOf(ev.code);
            m_buttonState[index] = ev.value;
            emit buttonStateChanged();
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
