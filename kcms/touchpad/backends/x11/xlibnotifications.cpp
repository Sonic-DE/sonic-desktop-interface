/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xlibnotifications.h"
#include "c_ptr.h"

#include <cstring>
#include <vector>

// X11 headers must be included before Qt headers because X11 #defines Status
// as a macro, which conflicts with QTextStream::Status enum.
#include <X11/Xlib-xcb.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XI2proto.h>
#include <X11/extensions/XInput2.h>

XlibNotifications::XlibNotifications(Display *display, int device)
    : m_display(display)
    , m_device(device)
{
    if (!display) {
        return;
    }

    m_connection = XGetXCBConnection(display);
    if (!m_connection || xcb_connection_has_error(m_connection)) {
        return;
    }

    const int fd = xcb_get_file_descriptor(m_connection);
    if (fd < 0) {
        return;
    }

    xcb_query_extension_cookie_t inputExtCookie = xcb_query_extension(m_connection, std::strlen(INAME), INAME);
    std::unique_ptr<xcb_query_extension_reply_t, CDeleter> inputExt(xcb_query_extension_reply(m_connection, inputExtCookie, nullptr));
    if (!inputExt || !inputExt->present) {
        return;
    }
    m_inputOpcode = inputExt->major_opcode;

    // Select events on the root window (not on a separate input-only window)
    const int maskLen = XIMaskLen(XI_LASTEVENT);

    if (device != XIAllDevices && device >= 0) {
        // Tracked device: select hierarchy + property events
        XIEventMask masks[2];

        // Property event mask for the tracked device
        std::vector<unsigned char> touchpadMask(maskLen, 0);
        masks[0].deviceid = device;
        masks[0].mask = touchpadMask.data();
        masks[0].mask_len = maskLen;
        XISetMask(touchpadMask.data(), XI_PropertyEvent);

        // Hierarchy mask for all devices
        std::vector<unsigned char> allMask(maskLen, 0);
        masks[1].deviceid = XIAllDevices;
        masks[1].mask = allMask.data();
        masks[1].mask_len = maskLen;
        XISetMask(allMask.data(), XI_HierarchyChanged);

        XISelectEvents(display, XDefaultRootWindow(display), masks, 2);
    } else {
        // No tracked device: select only hierarchy events on all devices
        XIEventMask mask;
        std::vector<unsigned char> allMask(maskLen, 0);
        mask.deviceid = XIAllDevices;
        mask.mask = allMask.data();
        mask.mask_len = maskLen;
        XISetMask(allMask.data(), XI_HierarchyChanged);

        XISelectEvents(display, XDefaultRootWindow(display), &mask, 1);
    }

    XFlush(display);

    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &XlibNotifications::processEvents);
    m_notifier->setEnabled(true);

    m_valid = true;
}

void XlibNotifications::processEvents()
{
    while (XPending(m_display)) {
        XEvent event;
        XNextEvent(m_display, &event);
        processEvent(&event);
    }
}

struct XEventDataDeleter {
    XEventDataDeleter(Display *display, XGenericEventCookie *cookie)
        : m_display(display)
        , m_cookie(cookie)
    {
        XGetEventData(m_display, m_cookie);
    }

    ~XEventDataDeleter()
    {
        if (m_cookie->data) {
            XFreeEventData(m_display, m_cookie);
        }
    }

    Display *m_display;
    XGenericEventCookie *m_cookie;
};

void XlibNotifications::processEvent(XEvent *event)
{
    if (event->xcookie.type != GenericEvent) {
        return;
    }
    if (event->xcookie.extension != m_inputOpcode) {
        return;
    }

    if (event->xcookie.evtype == XI_PropertyEvent) {
        XEventDataDeleter helper(m_display, &event->xcookie);
        if (!event->xcookie.data) {
            return;
        }

        XIPropertyEvent *propEvent = reinterpret_cast<XIPropertyEvent *>(event->xcookie.data);
        Q_EMIT propertyChanged(propEvent->property);
    } else if (event->xcookie.evtype == XI_HierarchyChanged) {
        XEventDataDeleter helper(m_display, &event->xcookie);
        if (!event->xcookie.data) {
            return;
        }

        XIHierarchyEvent *hierarchyEvent = reinterpret_cast<XIHierarchyEvent *>(event->xcookie.data);
        for (uint16_t i = 0; i < hierarchyEvent->num_info; i++) {
            if (hierarchyEvent->info[i].deviceid == m_device) {
                if (hierarchyEvent->info[i].flags & XISlaveRemoved) {
                    Q_EMIT touchpadDetached();
                    return;
                }
            }
            if (hierarchyEvent->info[i].use != XISlavePointer) {
                continue;
            }
            // Request rescan for slave additions and device enables.
            // Do not treat XIDeviceDisabled as physical removal.
            if (hierarchyEvent->info[i].flags & (XISlaveAdded | XIDeviceEnabled)) {
                Q_EMIT devicePlugged(hierarchyEvent->info[i].deviceid);
            }
        }
    }
}

XlibNotifications::~XlibNotifications()
{
    // No input-only window to destroy anymore. The notifier is parented to
    // this QObject and deleted automatically. The XCB connection is borrowed
    // from the owning Xlib display and must not be disconnected here.
}
