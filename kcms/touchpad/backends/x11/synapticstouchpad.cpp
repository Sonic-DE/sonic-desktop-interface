#include "synapticstouchpad.h"

#include <KConfigGroup>
#include <KSharedConfig>

#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <synaptics-properties.h>

namespace
{
struct XFreeDeleter {
    void operator()(unsigned char *value) const
    {
        XFree(value);
    }
};
}

SynapticsTouchpad::SynapticsTouchpad(Display *display, int deviceId, const QString &name)
    : LibinputTouchpad(display, deviceId)
    , m_synapticsName(name)
{
    m_touchpadOffAtom.intern(m_connection, SYNAPTICS_PROP_OFF);
    m_supportedButtons.avail = true;
    m_supportedButtons.old = m_supportedButtons.val = Qt::LeftButton | Qt::MiddleButton | Qt::RightButton;
    load();
}

void SynapticsTouchpad::resetBool(PropBool &property, bool value, bool available)
{
    property.avail = available;
    property.old = property.val = value;
}

void SynapticsTouchpad::resetReal(Prop<qreal> &property, qreal value, bool available)
{
    property.avail = available;
    property.old = property.val = value;
}

QVariant SynapticsTouchpad::propertyValue(const char *name, int offset, Atom expectedType) const
{
    if (expectedType == 0)
        expectedType = XA_INTEGER;
    const Atom property = XInternAtom(m_display, name, True);
    if (property == None) {
        return {};
    }
    Atom type = None;
    int format = 0;
    unsigned long count = 0;
    unsigned long remaining = 0;
    unsigned char *raw = nullptr;
    if (XIGetProperty(m_display, m_deviceId, property, 0, 32, False, expectedType, &type, &format, &count, &remaining, &raw) != Success) {
        return {};
    }
    std::unique_ptr<unsigned char, XFreeDeleter> data(raw);
    if (!data || type != expectedType || offset < 0 || static_cast<unsigned long>(offset) >= count) {
        return {};
    }
    if (format == 8)
        return int(reinterpret_cast<qint8 *>(data.get())[offset]);
    if (format == 32 && expectedType == XA_INTEGER)
        return reinterpret_cast<qint32 *>(data.get())[offset];
    if (format == 32)
        return reinterpret_cast<float *>(data.get())[offset];
    return {};
}

bool SynapticsTouchpad::writeProperty(const char *name, int offset, const QVariant &value, Atom expectedType)
{
    if (expectedType == 0)
        expectedType = XA_INTEGER;
    const Atom property = XInternAtom(m_display, name, True);
    if (property == None)
        return false;
    Atom type = None;
    int format = 0;
    unsigned long count = 0;
    unsigned long remaining = 0;
    unsigned char *raw = nullptr;
    if (XIGetProperty(m_display, m_deviceId, property, 0, 32, False, expectedType, &type, &format, &count, &remaining, &raw) != Success)
        return false;
    std::unique_ptr<unsigned char, XFreeDeleter> data(raw);
    if (!data || type != expectedType || offset < 0 || static_cast<unsigned long>(offset) >= count)
        return false;
    if (format == 8)
        reinterpret_cast<qint8 *>(data.get())[offset] = value.toInt();
    else if (format == 32 && expectedType == XA_INTEGER)
        reinterpret_cast<qint32 *>(data.get())[offset] = value.toInt();
    else if (format == 32)
        reinterpret_cast<float *>(data.get())[offset] = value.toFloat();
    else
        return false;
    XIChangeProperty(m_display, m_deviceId, property, type, format, XIPropModeReplace, data.get(), count);
    return true;
}

bool SynapticsTouchpad::load()
{
    const auto config = KSharedConfig::openConfig(QStringLiteral("touchpadrc"));
    KConfigGroup group(config, QStringLiteral("parameters"));
    KConfigGroup autoDisable(config, QStringLiteral("autodisable"));
    const QVariant oneTap = propertyValue(SYNAPTICS_PROP_TAP_ACTION, 4);
    const QVariant twoTap = propertyValue(SYNAPTICS_PROP_TAP_ACTION, 5);
    const QVariant threeTap = propertyValue(SYNAPTICS_PROP_TAP_ACTION, 6);
    const QVariant gestures = propertyValue(SYNAPTICS_PROP_GESTURES, 0);
    const QVariant locked = propertyValue(SYNAPTICS_PROP_LOCKED_DRAGS, 0);
    const QVariant twoFinger = propertyValue(SYNAPTICS_PROP_SCROLL_TWOFINGER, 0);
    const QVariant edge = propertyValue(SYNAPTICS_PROP_SCROLL_EDGE, 0);
    const QVariant verticalDistance = propertyValue(SYNAPTICS_PROP_SCROLL_DISTANCE, 0);
    const QVariant horizontalDistance = propertyValue(SYNAPTICS_PROP_SCROLL_DISTANCE, 1);
    const Atom floatType = XInternAtom(m_display, "FLOAT", False);
    const QVariant acceleration = propertyValue(SYNAPTICS_PROP_SPEED, 2, floatType);
    const QVariant middleTimeout = propertyValue(SYNAPTICS_PROP_MIDDLE_TIMEOUT, 0);

    resetBool(m_tapToClick, group.readEntry("OneFingerTapButton", oneTap.toInt()) != 0, oneTap.isValid());
    // Keep the live driver value as the baseline, not the saved preference.
    // Session initialization must be able to apply settings after a driver reset.
    m_tapToClick.old = oneTap.toInt() != 0;
    m_tapToClickEnabledByDefault.avail = oneTap.isValid();
    m_tapToClickEnabledByDefault.old = m_tapToClickEnabledByDefault.val = m_tapToClick.val;
    resetBool(m_tapAndDrag, group.readEntry("TapAndDragGesture", gestures.toBool()), gestures.isValid());
    m_tapAndDrag.old = gestures.toBool();
    resetBool(m_tapDragLock, group.readEntry("LockedDrags", locked.toBool()), locked.isValid());
    m_tapDragLock.old = locked.toBool();
    resetBool(m_lmrTapButtonMap,
              group.readEntry("LmrTapButtonMap",
                              group.readEntry("TwoFingerTapButton", twoTap.toInt()) == 2 && group.readEntry("ThreeFingerTapButton", threeTap.toInt()) == 3),
              oneTap.isValid() && twoTap.isValid() && threeTap.isValid());
    // LMR means left/middle/right: X11 buttons 1/2/3, not 1/3/2.
    // Disabled tapping has no live button order; retain the saved preference.
    m_lmrTapButtonMap.old = oneTap.toInt() != 0 ? (twoTap.toInt() == 2 && threeTap.toInt() == 3) : m_lmrTapButtonMap.val;
    m_lmrTapButtonMapEnabledByDefault.avail = m_lmrTapButtonMap.avail;
    m_lmrTapButtonMapEnabledByDefault.old = m_lmrTapButtonMapEnabledByDefault.val = true;
    const bool hasSavedTapActions =
        group.hasKey("OneFingerTapButton") || group.hasKey("TwoFingerTapButton") || group.hasKey("ThreeFingerTapButton") || group.hasKey("LmrTapButtonMap");
    const int desiredOne = m_tapToClick.val ? 1 : 0;
    const int desiredTwo = m_tapToClick.val ? (m_lmrTapButtonMap.val ? 2 : 3) : 0;
    const int desiredThree = m_tapToClick.val ? (m_lmrTapButtonMap.val ? 3 : 2) : 0;
    m_tapActionsNeedApply =
        m_lmrTapButtonMap.avail && hasSavedTapActions && (oneTap.toInt() != desiredOne || twoTap.toInt() != desiredTwo || threeTap.toInt() != desiredThree);
    resetBool(m_isScrollTwoFinger, group.readEntry("VertTwoFingerScroll", twoFinger.toBool()), twoFinger.isValid());
    resetBool(m_isScrollEdge, group.readEntry("VertEdgeScroll", edge.toBool()), edge.isValid());
    m_supportsScrollTwoFinger.avail = true;
    m_supportsScrollTwoFinger.old = m_supportsScrollTwoFinger.val = twoFinger.isValid();
    m_supportsScrollEdge.avail = true;
    m_supportsScrollEdge.old = m_supportsScrollEdge.val = edge.isValid();
    m_verticalDistance = verticalDistance.toInt();
    m_horizontalDistance = horizontalDistance.toInt();
    resetBool(m_naturalScroll, group.readEntry("InvertVertScroll", m_verticalDistance < 0), verticalDistance.isValid());
    resetBool(m_horizontalScrolling, group.readEntry("HorizTwoFingerScroll", horizontalDistance.isValid()), horizontalDistance.isValid());
    resetReal(m_pointerAcceleration, group.readEntry("AccelFactor", acceleration.toReal()), acceleration.isValid());
    resetBool(m_middleEmulation, group.readEntry("MiddleButtonEmulation", middleTimeout.toInt() > 0), middleTimeout.isValid());
    resetBool(m_disableWhileTyping, autoDisable.readEntry("DisableOnKeyboardActivity", true));
    m_disableWhileTypingEnabledByDefault.avail = true;
    m_disableWhileTypingEnabledByDefault.old = m_disableWhileTypingEnabledByDefault.val = true;
    resetBool(m_disableEventsOnExternalMouse, autoDisable.readEntry("DisableWhenMousePluggedIn", false));
    m_supportsDisableEventsOnExternalMouse.avail = true;
    m_supportsDisableEventsOnExternalMouse.old = m_supportsDisableEventsOnExternalMouse.val = true;
    m_disableEventsOnExternalMouseEnabledByDefault.avail = true;
    m_disableEventsOnExternalMouseEnabledByDefault.old = m_disableEventsOnExternalMouseEnabledByDefault.val = false;
    m_tapFingerCount.avail = true;
    m_tapFingerCount.old = m_tapFingerCount.val = 3;
    return true;
}

bool SynapticsTouchpad::save()
{
    const auto config = KSharedConfig::openConfig(QStringLiteral("touchpadrc"));
    KConfigGroup group(config, QStringLiteral("parameters"));
    KConfigGroup autoDisable(config, QStringLiteral("autodisable"));
    bool ok = true;
    if (m_tapToClick.changed() || m_lmrTapButtonMap.changed() || m_tapActionsNeedApply) {
        const int one = m_tapToClick.val ? 1 : 0;
        const int two = m_tapToClick.val ? (m_lmrTapButtonMap.val ? 2 : 3) : 0;
        const int three = m_tapToClick.val ? (m_lmrTapButtonMap.val ? 3 : 2) : 0;
        ok &= writeProperty(SYNAPTICS_PROP_TAP_ACTION, 4, one);
        ok &= writeProperty(SYNAPTICS_PROP_TAP_ACTION, 5, two);
        ok &= writeProperty(SYNAPTICS_PROP_TAP_ACTION, 6, three);
        group.writeEntry("OneFingerTapButton", one);
        group.writeEntry("TwoFingerTapButton", two);
        group.writeEntry("ThreeFingerTapButton", three);
        group.writeEntry("LmrTapButtonMap", m_lmrTapButtonMap.val);
    }
    if (m_tapAndDrag.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_GESTURES, 0, m_tapAndDrag.val);
        group.writeEntry("TapAndDragGesture", m_tapAndDrag.val);
    }
    if (m_tapDragLock.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_LOCKED_DRAGS, 0, m_tapDragLock.val);
        group.writeEntry("LockedDrags", m_tapDragLock.val);
    }
    if (m_isScrollTwoFinger.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_SCROLL_TWOFINGER, 0, m_isScrollTwoFinger.val);
        group.writeEntry("VertTwoFingerScroll", m_isScrollTwoFinger.val);
    }
    if (m_isScrollEdge.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_SCROLL_EDGE, 0, m_isScrollEdge.val);
        group.writeEntry("VertEdgeScroll", m_isScrollEdge.val);
    }
    if (m_horizontalScrolling.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_SCROLL_TWOFINGER, 1, m_horizontalScrolling.val && m_isScrollTwoFinger.val);
        ok &= writeProperty(SYNAPTICS_PROP_SCROLL_EDGE, 1, m_horizontalScrolling.val && m_isScrollEdge.val);
        group.writeEntry("HorizTwoFingerScroll", m_horizontalScrolling.val);
    }
    if (m_naturalScroll.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_SCROLL_DISTANCE, 0, m_naturalScroll.val ? -qAbs(m_verticalDistance) : qAbs(m_verticalDistance));
        ok &= writeProperty(SYNAPTICS_PROP_SCROLL_DISTANCE, 1, m_naturalScroll.val ? -qAbs(m_horizontalDistance) : qAbs(m_horizontalDistance));
        group.writeEntry("InvertVertScroll", m_naturalScroll.val);
    }
    if (m_middleEmulation.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_MIDDLE_TIMEOUT, 0, m_middleEmulation.val ? 75 : 0);
        group.writeEntry("MiddleButtonEmulation", m_middleEmulation.val);
    }
    if (m_pointerAcceleration.changed()) {
        ok &= writeProperty(SYNAPTICS_PROP_SPEED, 2, m_pointerAcceleration.val, XInternAtom(m_display, "FLOAT", False));
        group.writeEntry("AccelFactor", m_pointerAcceleration.val);
    }
    if (m_disableWhileTyping.changed()) {
        autoDisable.writeEntry("DisableOnKeyboardActivity", m_disableWhileTyping.val);
    }
    if (m_disableEventsOnExternalMouse.changed()) {
        autoDisable.writeEntry("DisableWhenMousePluggedIn", m_disableEventsOnExternalMouse.val);
    }
    XSync(m_display, False);
    ok &= group.sync() && autoDisable.sync();
    if (ok)
        load();
    return ok;
}

bool SynapticsTouchpad::defaults()
{
    m_tapToClick.set(true);
    m_tapAndDrag.set(true);
    m_tapDragLock.set(false);
    m_lmrTapButtonMap.set(true);
    m_isScrollTwoFinger.set(true);
    m_isScrollEdge.set(false);
    m_naturalScroll.set(false);
    m_horizontalScrolling.set(true);
    m_middleEmulation.set(false);
    m_disableWhileTyping.set(true);
    m_disableEventsOnExternalMouse.set(false);
    m_pointerAcceleration.set(0.035);
    return true;
}

bool SynapticsTouchpad::isSaveNeeded() const
{
    return m_tapToClick.changed() || m_lmrTapButtonMap.changed() || m_tapActionsNeedApply || m_tapAndDrag.changed() || m_tapDragLock.changed()
        || m_isScrollTwoFinger.changed() || m_isScrollEdge.changed() || m_naturalScroll.changed() || m_horizontalScrolling.changed()
        || m_middleEmulation.changed() || m_pointerAcceleration.changed() || m_disableWhileTyping.changed() || m_disableEventsOnExternalMouse.changed();
}
