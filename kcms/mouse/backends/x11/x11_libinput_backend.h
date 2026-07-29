/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputbackend.h"
#include "inputdevice.h"
#include "x11_libinput_dummydevice.h"

#include <QList>

#include <memory>

class X11LibinputBackend : public InputBackend
{
    Q_OBJECT

public:
    explicit X11LibinputBackend();
    ~X11LibinputBackend() = default;

    void kcmInit() override;

    bool save() override;
    bool load() override;
    bool defaults() override;
    bool isSaveNeeded() const override;
    QString errorString() const override;
    int deviceCount() const override;
    bool isAnonymousInputDevice() const override;
    QList<InputDevice *> inputDevices() const override;

    bool legacySettingsAvailable() const override;
    qreal legacyAcceleration() const override;
    void setLegacyAcceleration(qreal value) override;
    int legacyThreshold() const override;
    void setLegacyThreshold(int value) override;
    int doubleClickInterval() const override;
    void setDoubleClickInterval(int value) override;
    int dragStartTime() const override;
    void setDragStartTime(int value) override;
    int dragStartDistance() const override;
    void setDragStartDistance(int value) override;
    int wheelScrollLines() const override;
    void setWheelScrollLines(int value) override;

private:
    std::unique_ptr<X11LibinputDummyDevice> m_device;
    QString m_errorString;
    bool m_hasLegacyDevices = false;
    bool m_hasPointerDevices = false;
    qreal m_legacyAcceleration = 1.0;
    qreal m_savedLegacyAcceleration = 1.0;
    int m_legacyThreshold = 0;
    int m_savedLegacyThreshold = 0;
    int m_doubleClickInterval = 400;
    int m_savedDoubleClickInterval = 400;
    int m_dragStartTime = 500;
    int m_savedDragStartTime = 500;
    int m_dragStartDistance = 10;
    int m_savedDragStartDistance = 10;
    int m_wheelScrollLines = 3;
    int m_savedWheelScrollLines = 3;
};
