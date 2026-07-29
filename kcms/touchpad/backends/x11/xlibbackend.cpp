/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cmath>
#include <cstring>

#include <QtAlgorithms>

#include <KLocalizedString>
#include <QDebug>

#include <config-X11.h>

// Includes are ordered this way because of #defines in Xorg's headers
#include "libinputtouchpad.h"
#if HAVE_SYNAPTICS
#include "synapticstouchpad.h"
#include <synaptics-properties.h>
#endif
#include "xlibbackend.h" // krazy:exclude=includes
#include "xlibnotifications.h" // krazy:exclude=includes
#include "xrecordkeyboardmonitor.h"

#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <xserver-properties.h>

struct DeviceListDeleter {
    void operator()(XDeviceInfo *p)
    {
        if (p) {
            XFreeDeviceList(p);
        }
    }
};

void XlibBackend::XDisplayCleanup::operator()(Display *p)
{
    if (p) {
        XCloseDisplay(p);
    }
}

XlibBackend *XlibBackend::initialize(QObject *parent)
{
    XlibBackend *backend = new XlibBackend(parent);
    if (!backend->m_display) {
        delete backend;
        return nullptr;
    }
    return backend;
}

XlibBackend::~XlibBackend()
{
}

XlibBackend::XlibBackend(QObject *parent)
    : TouchpadBackend(parent)
    , m_display(XOpenDisplay(nullptr))
    , m_connection(nullptr)
{
    if (m_display) {
        m_connection = XGetXCBConnection(m_display.get());
    }

    if (!m_connection) {
        m_errorString = i18n("Cannot connect to X server");
        return;
    }

    m_touchpadAtom.intern(m_connection, XI_TOUCHPAD);
    m_mouseAtom.intern(m_connection, XI_MOUSE);
    m_enabledAtom.intern(m_connection, XI_PROP_ENABLED);

    m_libinputIdentifierAtom.intern(m_connection, "libinput Send Events Modes Available");
#if HAVE_SYNAPTICS
    m_synapticsIdentifierAtom.intern(m_connection, SYNAPTICS_PROP_CAPABILITIES);
#endif

    m_device.reset(findTouchpad());
    if (!m_device) {
        m_errorString = i18n("No touchpad found");
    }
    watchForEvents();
}

LibinputTouchpad *XlibBackend::findTouchpad()
{
    int nDevices = 0;
    std::unique_ptr<XDeviceInfo, DeviceListDeleter> deviceInfo(XListInputDevices(m_display.get(), &nDevices));

    for (XDeviceInfo *info = deviceInfo.get(); info < deviceInfo.get() + nDevices; info++) {
        // Make sure device is touchpad
        if (info->type != m_touchpadAtom.atom()) {
            continue;
        }
        int nProperties = 0;
        std::shared_ptr<Atom> properties(XIListProperties(m_display.get(), info->id, &nProperties), XDeleter);

        Atom *atom = properties.get();
        Atom *atomEnd = properties.get() + nProperties;
        for (; atom != atomEnd; atom++) {
#if HAVE_XORGLIBINPUT
            if (*atom == m_libinputIdentifierAtom.atom()) {
                setMode(TouchpadInputBackendMode::XLibinput);
                return new LibinputTouchpad(m_display.get(), info->id);
            }
#endif
#if HAVE_SYNAPTICS
            if (*atom == m_synapticsIdentifierAtom.atom()) {
                setMode(TouchpadInputBackendMode::XSynaptics);
                return new SynapticsTouchpad(m_display.get(), info->id, QString::fromLocal8Bit(info->name));
            }
#endif
        }
    }

    return nullptr;
}

bool XlibBackend::save()
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->save();
    if (!success) {
        m_errorString = i18n("Cannot apply touchpad configuration");
    }

    return success;
}

bool XlibBackend::load()
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->load();
    if (!success) {
        m_errorString = i18n("Cannot read touchpad configuration");
    }
    return success;
}

bool XlibBackend::defaults()
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->defaults();
    if (!success) {
        m_errorString = i18n("Cannot read default touchpad configuration");
    }
    return success;
}

bool XlibBackend::isSaveNeeded() const
{
    if (!m_device) {
        return false;
    }

    return m_device->isSaveNeeded();
}

void XlibBackend::setTouchpadSuspended(bool suspend)
{
    if (!m_device) {
        return;
    }

    m_device->setSuspended(suspend);

    // FIXME? This should not be needed, m_notifications should trigger
    // a propertyChanged signal when we enable/disable the touchpad,
    // that will Q_EMIT touchpadStateChanged, but for some reason
    // XlibNotifications is not getting the property change events
    // so we just Q_EMIT touchpadStateChanged from here as a workaround
    Q_EMIT touchpadStateChanged();
}

bool XlibBackend::isTouchpadAvailable() const
{
    return m_device != nullptr;
}

bool XlibBackend::isTouchpadSuspended() const
{
    if (!m_device) {
        return true;
    }

    return m_device->isSuspended();
}

void XlibBackend::touchpadDetached()
{
    qWarning() << "Touchpad detached";
    m_device.reset();
    Q_EMIT touchpadReset();
    Q_EMIT inputDevicesChanged();
}

void XlibBackend::devicePlugged(int /*device*/)
{
    if (!m_device) {
        m_device.reset(findTouchpad());
        if (m_device) {
            qWarning() << "Touchpad reset";
            // We get called by m_notifications, need to use deleteLater
            m_notifications.release()->deleteLater();
            watchForEvents();
            Q_EMIT touchpadReset();
            Q_EMIT inputDevicesChanged();
        }
    }
}

void XlibBackend::propertyChanged(xcb_atom_t prop)
{
    if ((m_device && prop == m_device->touchpadOffAtom().atom()) || prop == m_enabledAtom.atom()) {
        Q_EMIT touchpadStateChanged();
    }
}

QList<LibinputCommon *> XlibBackend::inputDevices() const
{
    QList<LibinputCommon *> touchpads;

    if (m_device) {
        touchpads.push_back(m_device.get());
    }

    return touchpads;
}

bool XlibBackend::externalMousePresent() const
{
    int count = 0;
    std::unique_ptr<XDeviceInfo, DeviceListDeleter> devices(XListInputDevices(m_display.get(), &count));
    for (int i = 0; devices && i < count; ++i) {
        const XDeviceInfo &device = devices.get()[i];
        if (device.use != IsXExtensionPointer || device.type != const_cast<XcbAtom &>(m_mouseAtom).atom()) {
            continue;
        }
        PropertyInfo enabled(m_display.get(), device.id, const_cast<XcbAtom &>(m_enabledAtom).atom(), 0);
        if (enabled.value(0).toBool()) {
            return true;
        }
    }
    return false;
}

void XlibBackend::watchForEvents()
{
    if (!m_notifications) {
        m_notifications.reset(new XlibNotifications(m_display.get(), m_device ? m_device->deviceId() : XIAllDevices));
        connect(m_notifications.get(), &XlibNotifications::devicePlugged, this, &XlibBackend::devicePlugged);
        connect(m_notifications.get(), &XlibNotifications::touchpadDetached, this, &XlibBackend::touchpadDetached);
        connect(m_notifications.get(), &XlibNotifications::propertyChanged, this, &XlibBackend::propertyChanged);
    }
#if HAVE_SYNAPTICS
    if (getMode() == TouchpadInputBackendMode::XSynaptics && !m_keyboardMonitor) {
        m_keyboardMonitor = std::make_unique<XRecordKeyboardMonitor>(XDisplayString(m_display.get()));
        if (m_keyboardMonitor->isValid()) {
            connect(m_keyboardMonitor.get(), &XRecordKeyboardMonitor::keyboardActivityStarted, this, &TouchpadBackend::keyboardActivityStarted);
            connect(m_keyboardMonitor.get(), &XRecordKeyboardMonitor::keyboardActivityFinished, this, &TouchpadBackend::keyboardActivityFinished);
        } else {
            m_keyboardMonitor.reset();
        }
    }
#endif
}
