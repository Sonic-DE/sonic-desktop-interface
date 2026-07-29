#pragma once

#include "libinputtouchpad.h"

class SynapticsTouchpad : public LibinputTouchpad
{
    Q_OBJECT

public:
    SynapticsTouchpad(Display *display, int deviceId, const QString &name);

    bool load() override;
    bool save() override;
    bool defaults() override;
    bool isSaveNeeded() const override;

    QString name() const override
    {
        return m_synapticsName;
    }
    bool supportsDisableEvents() const override
    {
        return true;
    }
    bool isEnabled() const override
    {
        return !const_cast<SynapticsTouchpad *>(this)->isSuspended();
    }
    void setEnabled(bool enabled) override
    {
        setSuspended(!enabled);
        Q_EMIT enabledChanged();
    }
    bool supportsLeftHanded() const override
    {
        return false;
    }
    bool supportsDisableEventsOnExternalMouse() const override
    {
        return true;
    }
    bool supportsDisableWhileTyping() const override
    {
        return true;
    }
    bool supportsMiddleEmulation() const override
    {
        return m_middleEmulation.avail;
    }
    bool supportsPointerAcceleration() const override
    {
        return m_pointerAcceleration.avail;
    }
    bool supportsPointerAccelerationProfileFlat() const override
    {
        return false;
    }
    bool supportsPointerAccelerationProfileAdaptive() const override
    {
        return false;
    }
    bool supportsNaturalScroll() const override
    {
        return m_naturalScroll.avail;
    }
    bool supportsHorizontalScrolling() const override
    {
        return m_horizontalScrolling.avail;
    }
    bool supportsScrollTwoFinger() const override
    {
        return m_supportsScrollTwoFinger.val;
    }
    bool supportsScrollEdge() const override
    {
        return m_supportsScrollEdge.val;
    }
    bool supportsScrollOnButtonDown() const override
    {
        return false;
    }
    bool supportsClickMethodAreas() const override
    {
        return false;
    }
    bool supportsClickMethodClickfinger() const override
    {
        return false;
    }
    bool supportsScrollFactor() const override
    {
        return false;
    }
    void setLmrTapButtonMap(bool set) override
    {
        m_lmrTapButtonMap.set(set);
    }
    XcbAtom &touchpadOffAtom() override
    {
        return m_touchpadOffAtom;
    }

private:
    void resetBool(PropBool &property, bool value, bool available = true);
    void resetReal(Prop<qreal> &property, qreal value, bool available = true);
    QVariant propertyValue(const char *name, int offset, Atom expectedType = 0) const;
    bool writeProperty(const char *name, int offset, const QVariant &value, Atom expectedType = 0);

    QString m_synapticsName;
    XcbAtom m_touchpadOffAtom;
    int m_verticalDistance = 0;
    int m_horizontalDistance = 0;
};
