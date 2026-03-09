/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabletevents.h"

#include <QGuiApplication>
#include <QQuickWindow>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

class TabletPadDial : public QObject
{
public:
    TabletPadDial(TabletEvents *events, int deviceId)
        : QObject(events)
        , m_events(events)
        , m_deviceId(deviceId)
    {
    }

    void handleDelta(int32_t value120)
    {
        Q_EMIT m_events->dialDelta(value120);
    }

    TabletEvents *const m_events;
    int m_deviceId;
};

class TabletPadGroup : public QObject
{
public:
    TabletPadGroup(TabletEvents *events, int deviceId)
        : QObject(events)
        , m_events(events)
        , m_deviceId(deviceId)
    {
    }

    TabletEvents *const m_events;
    int m_deviceId;
    TabletPadDial *m_dial = nullptr;
};

class TabletPad : public QObject
{
public:
    TabletPad(TabletEvents *events, int deviceId, const QString &path)
        : QObject(events)
        , m_events(events)
        , m_deviceId(deviceId)
        , m_path(path)
    {
    }

    void handleButton(uint32_t button, uint32_t state)
    {
        Q_EMIT m_events->padButtonReceived(m_path, button, state);
    }

    TabletEvents *const m_events;
    int m_deviceId;
    QString m_path;
    uint m_buttons = 0;
};

class TabletTool : public QObject
{
public:
    TabletTool(TabletEvents *events, int deviceId)
        : QObject(events)
        , m_events(events)
        , m_deviceId(deviceId)
    {
    }

    void setHardwareSerial(uint32_t hi, uint32_t lo)
    {
        m_hardware_serial_hi = hi;
        m_hardware_serial_lo = lo;
    }

    void handleButton(uint32_t button, uint32_t state)
    {
        Q_EMIT m_events->toolButtonReceived(m_hardware_serial_hi, m_hardware_serial_lo, button, state);
    }

    void handleMotion(double x, double y, double pressure, double tiltX, double tiltY)
    {
        m_x = x;
        m_y = y;
        m_pressure = pressure;
        m_tilt_x = tiltX;
        m_tilt_y = tiltY;
        Q_EMIT m_events->toolMotion(m_hardware_serial_hi, m_hardware_serial_lo, x, y, pressure, tiltX, tiltY);
    }

    void handleDown(uint32_t serial)
    {
        Q_UNUSED(serial)
        Q_EMIT m_events->toolDown(m_hardware_serial_hi, m_hardware_serial_lo, m_x, m_y);
    }

    void handleUp()
    {
        Q_EMIT m_events->toolUp(m_hardware_serial_hi, m_hardware_serial_lo, m_x, m_y);
    }

    uint32_t m_hardware_serial_hi = 0;
    uint32_t m_hardware_serial_lo = 0;
    double m_x = 0;
    double m_y = 0;
    double m_pressure = 0;
    double m_tilt_x = 0;
    double m_tilt_y = 0;
    TabletEvents *const m_events;
    int m_deviceId;
};

class TabletSeat : public QObject
{
public:
    TabletSeat(TabletEvents *events)
        : QObject(events)
        , m_events(events)
    {
    }

    ~TabletSeat()
    {
        qDeleteAll(m_tools);
        qDeleteAll(m_pads);
    }

    TabletTool *addTool(int deviceId)
    {
        auto tool = new TabletTool(m_events, deviceId);
        m_tools.append(tool);
        return tool;
    }

    TabletPad *addPad(int deviceId, const QString &path)
    {
        auto pad = new TabletPad(m_events, deviceId, path);
        m_pads.append(pad);
        return pad;
    }

    TabletTool *findTool(int deviceId)
    {
        for (auto tool : m_tools) {
            if (tool->m_deviceId == deviceId) {
                return tool;
            }
        }
        return nullptr;
    }

    TabletPad *findPad(int deviceId)
    {
        for (auto pad : m_pads) {
            if (pad->m_deviceId == deviceId) {
                return pad;
            }
        }
        return nullptr;
    }

    TabletEvents *const m_events;
    QList<TabletTool *> m_tools;
    QList<TabletPad *> m_pads;
};

class TabletManager : public QObject
{
public:
    explicit TabletManager(TabletEvents *q)
        : QObject(q)
        , m_events(q)
        , m_seat(new TabletSeat(q))
        , m_display(nullptr)
        , m_rootWindow(0)
    {
        // Don't open display here - only initialize when a tablet is plugged in
    }

    ~TabletManager()
    {
        if (m_display) {
            XCloseDisplay(m_display);
        }
    }

    // Lazy initialization - only opens display and queries devices when needed
    void ensureInitialized()
    {
        if (m_display) {
            return; // Already initialized
        }

        // Get display from environment
        const char *displayName = getenv("DISPLAY");
        if (!displayName) {
            return;
        }

        m_display = XOpenDisplay(displayName);
        if (!m_display) {
            return;
        }

        m_rootWindow = DefaultRootWindow(m_display);

        // Query X Input devices to find tablets
        queryDevices();
    }

    bool hasTablet()
    {
        ensureInitialized();
        return m_seat->m_tools.size() > 0 || m_seat->m_pads.size() > 0;
    }

    TabletSeat *seat()
    {
        ensureInitialized();
        return m_seat;
    }

    void queryDevices()
    {
        int ndevices = 0;
        XDeviceInfo *devices = XListInputDevices(m_display, &ndevices);
        if (!devices) {
            return;
        }

        // Get the wacom tool type atom
        Atom toolTypeAtom = XInternAtom(m_display, "Wacom Tool Type", True);
        if (!toolTypeAtom) {
            return;
        }

        for (int i = 0; i < ndevices; i++) {
            XDeviceInfo &dev = devices[i];

            // Query the Wacom Tool Type property to identify tablet devices
            Atom actualType = None;
            int actualFormat = 0;
            unsigned long nitems = 0, bytesAfter = 0;
            unsigned char *data = nullptr;

            Status status =
                XIGetProperty(m_display, dev.id, toolTypeAtom, 0, 100, False, AnyPropertyType, &actualType, &actualFormat, &nitems, &bytesAfter, &data);

            if (status == Success && data && nitems > 0) {
                // This is a Wacom tablet device - get the tool type from the atom
                Atom *toolType = reinterpret_cast<Atom *>(data);
                if (nitems > 0) {
                    char *toolTypeName = XGetAtomName(m_display, toolType[0]);
                    if (toolTypeName) {
                        bool isPad = (strcmp(toolTypeName, "PAD") == 0);
                        if (isPad) {
                            m_seat->addPad(dev.id, QString::fromLatin1(dev.name));
                        } else {
                            m_seat->addTool(dev.id);
                        }
                        XFree(toolTypeName);

                        // Select for events on this device
                        selectEvents(dev.id);
                    }
                }
                XFree(data);
            }
        }

        if (devices) {
            XFreeDeviceList(devices);
        }
    }

    void selectEvents(int deviceId)
    {
        unsigned char mask[XI_LASTEVENT];
        memset(mask, 0, sizeof(mask));

        XISetMask(mask, XI_DeviceChanged);
        XISetMask(mask, XI_ButtonPress);
        XISetMask(mask, XI_ButtonRelease);
        XISetMask(mask, XI_Motion);

        XIEventMask eventMask;
        eventMask.deviceid = deviceId;
        eventMask.mask = mask;
        eventMask.mask_len = sizeof(mask);

        XISelectEvents(m_display, m_rootWindow, &eventMask, 1);
    }

    TabletEvents *const m_events;
    TabletSeat *m_seat = nullptr;
    Display *m_display = nullptr;
    Window m_rootWindow = 0;
};

TabletEvents::TabletEvents(QQuickItem *parent)
    : QQuickItem(parent)
{
    if (qGuiApp->platformName() != QLatin1String("xcb")) {
        return;
    }

    // Create X11 tablet manager
    auto tabletClient = new TabletManager(this);

    if (!tabletClient->hasTablet()) {
        return;
    }

    // Install event filter to handle X11 events
    QCoreApplication::instance()->installEventFilter(this);
}

TabletEvents::~TabletEvents() = default;

bool TabletEvents::eventFilter(QObject *watched, QEvent *event)
{
    return QQuickItem::eventFilter(watched, event);
}
