/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputdevice.h"

#include <config-build-options.h>

#include <QList>
#include <QObject>
#include <QVariantHash>

class InputBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<InputDevice *> inputDevices READ inputDevices NOTIFY inputDevicesChanged FINAL)
    Q_PROPERTY(bool isAnonymousInputDevice READ isAnonymousInputDevice CONSTANT FINAL)
    Q_PROPERTY(QVariantMap buttonMapping READ buttonMapping WRITE setButtonMapping NOTIFY buttonMappingChanged FINAL)
    Q_PROPERTY(int buttonMappingCount READ buttonMappingCount NOTIFY buttonMappingChanged STORED false FINAL)
    Q_PROPERTY(bool legacySettingsAvailable READ legacySettingsAvailable NOTIFY legacySettingsChanged FINAL)
    Q_PROPERTY(qreal legacyAcceleration READ legacyAcceleration WRITE setLegacyAcceleration NOTIFY legacySettingsChanged FINAL)
    Q_PROPERTY(int legacyThreshold READ legacyThreshold WRITE setLegacyThreshold NOTIFY legacySettingsChanged FINAL)
    Q_PROPERTY(int doubleClickInterval READ doubleClickInterval WRITE setDoubleClickInterval NOTIFY legacySettingsChanged FINAL)
    Q_PROPERTY(int dragStartTime READ dragStartTime WRITE setDragStartTime NOTIFY legacySettingsChanged FINAL)
    Q_PROPERTY(int dragStartDistance READ dragStartDistance WRITE setDragStartDistance NOTIFY legacySettingsChanged FINAL)
    Q_PROPERTY(int wheelScrollLines READ wheelScrollLines WRITE setWheelScrollLines NOTIFY legacySettingsChanged FINAL)

protected:
    explicit InputBackend() = default;

public:
    static std::unique_ptr<InputBackend> create();
    static void registerImplementationTypes(const char *uri);

    virtual void kcmInit()
    {
    }

    virtual bool save()
    {
        return false;
    }

    virtual bool load()
    {
        return false;
    }

    virtual bool defaults()
    {
        return false;
    }

    virtual bool isSaveNeeded() const
    {
        return false;
    }

    virtual QString errorString() const
    {
        return QString();
    }

    virtual int deviceCount() const
    {
        return 0;
    }

    virtual bool isAnonymousInputDevice() const
    {
        return false;
    }

    virtual QList<InputDevice *> inputDevices() const
    {
        return {};
    }

    virtual QVariantMap buttonMapping() const
    {
        return {};
    }

    virtual void setButtonMapping([[maybe_unused]] const QVariantMap &mapping)
    {
    }

    // Because QML can't QVariantMap::count() on its own.
    int buttonMappingCount() const
    {
        return buttonMapping().count();
    }

    virtual bool legacySettingsAvailable() const
    {
        return false;
    }
    virtual qreal legacyAcceleration() const
    {
        return 1.0;
    }
    virtual void setLegacyAcceleration(qreal)
    {
    }
    virtual int legacyThreshold() const
    {
        return 0;
    }
    virtual void setLegacyThreshold(int)
    {
    }
    virtual int doubleClickInterval() const
    {
        return 400;
    }
    virtual void setDoubleClickInterval(int)
    {
    }
    virtual int dragStartTime() const
    {
        return 500;
    }
    virtual void setDragStartTime(int)
    {
    }
    virtual int dragStartDistance() const
    {
        return 10;
    }
    virtual void setDragStartDistance(int)
    {
    }
    virtual int wheelScrollLines() const
    {
        return 3;
    }
    virtual void setWheelScrollLines(int)
    {
    }

Q_SIGNALS:
    void needsSaveChanged();

    void inputDevicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);
    void buttonMappingChanged();
    void legacySettingsChanged();
};
