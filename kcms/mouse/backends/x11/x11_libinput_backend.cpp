/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_libinput_backend.h"
#include "x11_input_properties.h"

#include <config-X11.h>

#include <KConfigGroup>
#include <KSharedConfig>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>

#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <limits>
#if HAVE_EVDEV
#include <evdev-properties.h>
#endif

namespace
{
bool hasProperty(Display *display, int deviceId, Atom property)
{
    int count = 0;
    Atom *properties = XIListProperties(display, deviceId, &count);
    bool found = false;
    for (int i = 0; properties && i < count; ++i) {
        found |= properties[i] == property;
    }
    if (properties)
        XFree(properties);
    return found;
}
}

X11LibinputBackend::X11LibinputBackend()
    : InputBackend()
    , m_device(new X11LibinputDummyDevice(this, QX11Info::display()))
{
    connect(m_device.get(), &X11LibinputDummyDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);

    Display *display = QX11Info::display();
    const Atom libinputAtom = XInternAtom(display, LIBINPUT_PROP_ACCEL, True);
    const Atom touchpadAtom = XInternAtom(display, XI_TOUCHPAD, True);
    int count = 0;
    XDeviceInfo *devices = XListInputDevices(display, &count);
    for (int i = 0; devices && i < count; ++i) {
        const bool pointer = devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer;
        m_hasPointerDevices |= pointer && devices[i].type != touchpadAtom;
        if (pointer && devices[i].type != touchpadAtom && (!libinputAtom || !hasProperty(display, devices[i].id, libinputAtom))) {
            m_hasLegacyDevices = true;
            break;
        }
    }
    if (devices)
        XFreeDeviceList(devices);
    m_device->setLegacySupport(m_hasLegacyDevices);

    auto *rescanTimer = new QTimer(this);
    rescanTimer->setInterval(1000);
    connect(rescanTimer, &QTimer::timeout, this, [this, libinputAtom, touchpadAtom] {
        bool hasPointerDevices = false;
        bool hasLegacyDevices = false;
        Display *display = QX11Info::display();
        int count = 0;
        XDeviceInfo *devices = XListInputDevices(display, &count);
        for (int i = 0; devices && i < count; ++i) {
            const bool pointer = devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer;
            hasPointerDevices |= pointer && devices[i].type != touchpadAtom;
            hasLegacyDevices |= pointer && devices[i].type != touchpadAtom && (!libinputAtom || !hasProperty(display, devices[i].id, libinputAtom));
        }
        if (devices)
            XFreeDeviceList(devices);
        if (hasPointerDevices != m_hasPointerDevices || hasLegacyDevices != m_hasLegacyDevices) {
            m_hasPointerDevices = hasPointerDevices;
            m_hasLegacyDevices = hasLegacyDevices;
            m_device->setLegacySupport(m_hasLegacyDevices);
            Q_EMIT inputDevicesChanged();
            Q_EMIT legacySettingsChanged();
        }
    });
    rescanTimer->start();
}

bool X11LibinputBackend::save()
{
    bool ok = m_device->save();
    Display *display = QX11Info::display();
    if (m_hasLegacyDevices && (m_legacyAcceleration != m_savedLegacyAcceleration || m_legacyThreshold != m_savedLegacyThreshold)) {
        const int numerator = qMax(1, qRound(m_legacyAcceleration * 10.0));
        XChangePointerControl(display, True, True, numerator, 10, m_legacyThreshold);
        XSync(display, False);
    }
    if (m_hasLegacyDevices) {
        const Atom libinputAtom = XInternAtom(display, LIBINPUT_PROP_ACCEL, True);
        const Atom touchpadAtom = XInternAtom(display, XI_TOUCHPAD, True);
        int count = 0;
        XDeviceInfo *devices = XListInputDevices(display, &count);
        for (int i = 0; devices && i < count; ++i) {
            const bool pointer = devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer;
            if (!pointer || devices[i].type == touchpadAtom || hasProperty(display, devices[i].id, libinputAtom))
                continue;
            XDevice *device = XOpenDevice(display, devices[i].id);
            if (!device)
                continue;
            unsigned char mapping[256] = {};
            const int buttons = XGetDeviceButtonMapping(display, device, mapping, 256);
            if (buttons >= 3) {
                mapping[0] = m_device->isLeftHanded() ? 3 : 1;
                mapping[2] = m_device->isLeftHanded() ? 1 : 3;
                if (XSetDeviceButtonMapping(display, device, mapping, buttons) != Success)
                    ok = false;
            }
            XCloseDevice(display, device);
        }
        if (devices)
            XFreeDeviceList(devices);
    }
#if HAVE_EVDEV
    if (m_hasLegacyDevices) {
        const Atom scrollAtom = XInternAtom(display, EVDEV_PROP_SCROLL_DISTANCE, True);
        const Atom libinputAtom = XInternAtom(display, LIBINPUT_PROP_ACCEL, True);
        const Atom touchpadAtom = XInternAtom(display, XI_TOUCHPAD, True);
        int count = 0;
        XDeviceInfo *devices = XListInputDevices(display, &count);
        for (int i = 0; devices && i < count; ++i) {
            const bool pointer = devices[i].use == IsXPointer || devices[i].use == IsXExtensionPointer;
            if (!pointer || devices[i].type == touchpadAtom || hasProperty(display, devices[i].id, libinputAtom)
                || !hasProperty(display, devices[i].id, scrollAtom))
                continue;
            Atom type = None;
            int format = 0;
            unsigned long items = 0, remaining = 0;
            unsigned char *data = nullptr;
            if (XIGetProperty(display, devices[i].id, scrollAtom, 0, 3, False, XA_INTEGER, &type, &format, &items, &remaining, &data) == Success && data
                && type == XA_INTEGER && format == 32 && items == 3) {
                auto *values = reinterpret_cast<qint32 *>(data);
                for (int j = 0; j < 3; ++j) {
                    if (values[j] != std::numeric_limits<qint32>::min())
                        values[j] = m_device->isNaturalScroll() ? -qAbs(values[j]) : qAbs(values[j]);
                }
                XIChangeProperty(display, devices[i].id, scrollAtom, XA_INTEGER, 32, XIPropModeReplace, data, 3);
            }
            if (data)
                XFree(data);
        }
        if (devices)
            XFreeDeviceList(devices);
    }
#endif

    auto mouseConfig = KSharedConfig::openConfig(QStringLiteral("kcminputrc"), KConfig::NoGlobals);
    KConfigGroup mouse(mouseConfig, QStringLiteral("Mouse"));
    mouse.writeEntry("Acceleration", m_legacyAcceleration);
    mouse.writeEntry("Threshold", m_legacyThreshold);
    mouse.writeEntry("handed", m_device->isLeftHanded() ? 1 : 0);
    mouse.writeEntry("reverseScrollPolarity", m_device->isNaturalScroll());
    auto globals = KSharedConfig::openConfig(QStringLiteral("kdeglobals"), KConfig::NoGlobals);
    KConfigGroup kde(globals, QStringLiteral("KDE"));
    kde.writeEntry("DoubleClickInterval", m_doubleClickInterval);
    kde.writeEntry("StartDragTime", m_dragStartTime);
    kde.writeEntry("StartDragDist", m_dragStartDistance);
    kde.writeEntry("WheelScrollLines", m_wheelScrollLines);
    ok &= mouse.sync() && kde.sync();
    if (ok) {
        m_device->commitLegacySettings();
        m_savedLegacyAcceleration = m_legacyAcceleration;
        m_savedLegacyThreshold = m_legacyThreshold;
        m_savedDoubleClickInterval = m_doubleClickInterval;
        m_savedDragStartTime = m_dragStartTime;
        m_savedDragStartDistance = m_dragStartDistance;
        m_savedWheelScrollLines = m_wheelScrollLines;
        QDBusMessage message =
            QDBusMessage::createSignal(QStringLiteral("/KGlobalSettings"), QStringLiteral("org.kde.KGlobalSettings"), QStringLiteral("notifyChange"));
        message.setArguments({3, 0});
        QDBusConnection::sessionBus().send(message);
    }
    return ok;
}

bool X11LibinputBackend::load()
{
    bool ok = m_device->load();
    Display *display = QX11Info::display();
    int numerator = 1, denominator = 1;
    XGetPointerControl(display, &numerator, &denominator, &m_legacyThreshold);
    m_legacyAcceleration = denominator ? qreal(numerator) / denominator : 1.0;
    KConfigGroup mouse(KSharedConfig::openConfig(QStringLiteral("kcminputrc"), KConfig::NoGlobals), QStringLiteral("Mouse"));
    m_legacyAcceleration = mouse.readEntry("Acceleration", m_legacyAcceleration);
    m_legacyThreshold = mouse.readEntry("Threshold", m_legacyThreshold);
    m_device->loadLegacySettings(mouse.readEntry("handed", 0) != 0, mouse.readEntry("reverseScrollPolarity", false));
    KConfigGroup kde(KSharedConfig::openConfig(QStringLiteral("kdeglobals"), KConfig::NoGlobals), QStringLiteral("KDE"));
    m_doubleClickInterval = kde.readEntry("DoubleClickInterval", 400);
    m_dragStartTime = kde.readEntry("StartDragTime", 500);
    m_dragStartDistance = kde.readEntry("StartDragDist", 10);
    m_wheelScrollLines = kde.readEntry("WheelScrollLines", 3);
    m_savedLegacyAcceleration = m_legacyAcceleration;
    m_savedLegacyThreshold = m_legacyThreshold;
    m_savedDoubleClickInterval = m_doubleClickInterval;
    m_savedDragStartTime = m_dragStartTime;
    m_savedDragStartDistance = m_dragStartDistance;
    m_savedWheelScrollLines = m_wheelScrollLines;
    Q_EMIT legacySettingsChanged();
    return ok;
}

bool X11LibinputBackend::defaults()
{
    const bool ok = m_device->defaults();
    m_legacyAcceleration = 2.0;
    m_legacyThreshold = 2;
    m_doubleClickInterval = 400;
    m_dragStartTime = 500;
    m_dragStartDistance = 10;
    m_wheelScrollLines = 3;
    Q_EMIT legacySettingsChanged();
    Q_EMIT needsSaveChanged();
    return ok;
}

bool X11LibinputBackend::isSaveNeeded() const
{
    return m_device->isSaveNeeded() || m_legacyAcceleration != m_savedLegacyAcceleration || m_legacyThreshold != m_savedLegacyThreshold
        || m_doubleClickInterval != m_savedDoubleClickInterval || m_dragStartTime != m_savedDragStartTime || m_dragStartDistance != m_savedDragStartDistance
        || m_wheelScrollLines != m_savedWheelScrollLines;
}

void X11LibinputBackend::kcmInit()
{
    m_device->defaultsFromX();

    load();
    save();
}

QString X11LibinputBackend::errorString() const
{
    return m_errorString;
}

int X11LibinputBackend::deviceCount() const
{
    return m_hasPointerDevices ? 1 : 0;
}

bool X11LibinputBackend::isAnonymousInputDevice() const
{
    return true;
}

QList<InputDevice *> X11LibinputBackend::inputDevices() const
{
    return m_hasPointerDevices ? QList<InputDevice *>{m_device.get()} : QList<InputDevice *>{};
}

bool X11LibinputBackend::legacySettingsAvailable() const
{
    return m_hasLegacyDevices;
}
qreal X11LibinputBackend::legacyAcceleration() const
{
    return m_legacyAcceleration;
}
void X11LibinputBackend::setLegacyAcceleration(qreal value)
{
    value = qBound(0.1, value, 20.0);
    if (!qFuzzyCompare(m_legacyAcceleration, value)) {
        m_legacyAcceleration = value;
        Q_EMIT legacySettingsChanged();
        Q_EMIT needsSaveChanged();
    }
}
int X11LibinputBackend::legacyThreshold() const
{
    return m_legacyThreshold;
}
void X11LibinputBackend::setLegacyThreshold(int value)
{
    value = qBound(0, value, 20);
    if (m_legacyThreshold != value) {
        m_legacyThreshold = value;
        Q_EMIT legacySettingsChanged();
        Q_EMIT needsSaveChanged();
    }
}
int X11LibinputBackend::doubleClickInterval() const
{
    return m_doubleClickInterval;
}
void X11LibinputBackend::setDoubleClickInterval(int value)
{
    value = qBound(100, value, 2000);
    if (m_doubleClickInterval != value) {
        m_doubleClickInterval = value;
        Q_EMIT legacySettingsChanged();
        Q_EMIT needsSaveChanged();
    }
}
int X11LibinputBackend::dragStartTime() const
{
    return m_dragStartTime;
}
void X11LibinputBackend::setDragStartTime(int value)
{
    value = qBound(100, value, 2000);
    if (m_dragStartTime != value) {
        m_dragStartTime = value;
        Q_EMIT legacySettingsChanged();
        Q_EMIT needsSaveChanged();
    }
}
int X11LibinputBackend::dragStartDistance() const
{
    return m_dragStartDistance;
}
void X11LibinputBackend::setDragStartDistance(int value)
{
    value = qBound(1, value, 20);
    if (m_dragStartDistance != value) {
        m_dragStartDistance = value;
        Q_EMIT legacySettingsChanged();
        Q_EMIT needsSaveChanged();
    }
}
int X11LibinputBackend::wheelScrollLines() const
{
    return m_wheelScrollLines;
}
void X11LibinputBackend::setWheelScrollLines(int value)
{
    value = qBound(1, value, 12);
    if (m_wheelScrollLines != value) {
        m_wheelScrollLines = value;
        Q_EMIT legacySettingsChanged();
        Q_EMIT needsSaveChanged();
    }
}
