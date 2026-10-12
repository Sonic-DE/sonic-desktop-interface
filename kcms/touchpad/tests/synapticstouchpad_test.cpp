/*
    SPDX-FileCopyrightText: 2026 SonicDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "synapticstouchpad.h"

#include <KConfigGroup>
#include <KSharedConfig>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTest>

#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <synaptics-properties.h>

class SynapticsTouchpadTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void tapMapping_data();
    void tapMapping();
    void loadLiveMapping_data();
    void loadLiveMapping();
    void restoreSavedTapping_data();
    void restoreSavedTapping();
    void restoreLegacyTapButtons();
    void restoreMappingWithTappingAlreadyEnabled();
    void changingMappingDoesNotEnableTapping();
    void disablingTappingAndChangingMapping();
    void restoreDragSettings();
    void preserveUnconfiguredTapActions();

private:
    void setProperty(const char *name, const QList<int> &values);
    QList<int> property(const char *name);
    KConfigGroup parameters();

    QTemporaryDir m_configHome;
    QProcess m_xvfb;
    Display *m_display = nullptr;
    int m_deviceId = -1;
};

void SynapticsTouchpadTest::initTestCase()
{
    // Never change properties on the user's X server or use their settings.
    const QString xvfb = QStandardPaths::findExecutable(QStringLiteral("Xvfb"));
    if (xvfb.isEmpty()) {
        QSKIP("Xvfb is required for isolated XInput property tests");
    }
    QVERIFY(m_configHome.isValid());
    qputenv("XDG_CONFIG_HOME", m_configHome.path().toUtf8());
    m_xvfb.start(xvfb,
                 {QStringLiteral("-displayfd"),
                  QStringLiteral("1"),
                  QStringLiteral("-screen"),
                  QStringLiteral("0"),
                  QStringLiteral("640x480x24"),
                  QStringLiteral("-nolisten"),
                  QStringLiteral("tcp")});
    QVERIFY(m_xvfb.waitForStarted());
    QVERIFY2(m_xvfb.waitForReadyRead(10000), qPrintable(m_xvfb.readAllStandardError()));
    const QByteArray displayNumber = m_xvfb.readAllStandardOutput().trimmed();
    bool valid = false;
    displayNumber.toInt(&valid);
    QVERIFY(valid);
    const QByteArray displayName = QByteArray(":") + displayNumber;
    m_display = XOpenDisplay(displayName.constData());
    QVERIFY(m_display);

    int major = 2;
    int minor = 0;
    QCOMPARE(XIQueryVersion(m_display, &major, &minor), Success);
    int count = 0;
    XIDeviceInfo *devices = XIQueryDevice(m_display, XIAllDevices, &count);
    QVERIFY(devices);
    for (int i = 0; i < count; ++i) {
        if (devices[i].use == XISlavePointer) {
            m_deviceId = devices[i].deviceid;
            break;
        }
    }
    XIFreeDeviceInfo(devices);
    QVERIFY(m_deviceId >= 0);
}

void SynapticsTouchpadTest::cleanupTestCase()
{
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
    m_xvfb.terminate();
    if (!m_xvfb.waitForFinished(5000)) {
        m_xvfb.kill();
        m_xvfb.waitForFinished();
    }
}

KConfigGroup SynapticsTouchpadTest::parameters()
{
    return KSharedConfig::openConfig(QStringLiteral("touchpadrc"))->group(QStringLiteral("parameters"));
}

void SynapticsTouchpadTest::setProperty(const char *name, const QList<int> &values)
{
    QByteArray bytes;
    for (int value : values) {
        bytes.append(char(value));
    }
    XIChangeProperty(m_display,
                     m_deviceId,
                     XInternAtom(m_display, name, False),
                     XA_INTEGER,
                     8,
                     XIPropModeReplace,
                     reinterpret_cast<unsigned char *>(bytes.data()),
                     bytes.size());
    XSync(m_display, False);
}

QList<int> SynapticsTouchpadTest::property(const char *name)
{
    Atom type = None;
    int format = 0;
    unsigned long count = 0;
    unsigned long remaining = 0;
    unsigned char *bytes = nullptr;
    const int status =
        XIGetProperty(m_display, m_deviceId, XInternAtom(m_display, name, True), 0, 32, False, XA_INTEGER, &type, &format, &count, &remaining, &bytes);
    QList<int> values;
    if (status == Success && type == XA_INTEGER && format == 8 && bytes) {
        for (unsigned long i = 0; i < count; ++i) {
            values.append(bytes[i]);
        }
    }
    XFree(bytes);
    return values;
}

void SynapticsTouchpadTest::init()
{
    const auto config = KSharedConfig::openConfig(QStringLiteral("touchpadrc"));
    config->deleteGroup(QStringLiteral("parameters"));
    config->deleteGroup(QStringLiteral("autodisable"));
    config->sync();
    // Nonzero corner actions must survive writes to the finger actions.
    setProperty(SYNAPTICS_PROP_TAP_ACTION, {1, 2, 3, 0, 0, 0, 0});
    setProperty(SYNAPTICS_PROP_GESTURES, {0});
    setProperty(SYNAPTICS_PROP_LOCKED_DRAGS, {0});
}

void SynapticsTouchpadTest::tapMapping_data()
{
    QTest::addColumn<bool>("lmr");
    QTest::addColumn<int>("two");
    QTest::addColumn<int>("three");
    QTest::newRow("right-click") << false << 3 << 2;
    QTest::newRow("middle-click") << true << 2 << 3;
}

void SynapticsTouchpadTest::tapMapping()
{
    QFETCH(bool, lmr);
    QFETCH(int, two);
    QFETCH(int, three);
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    touchpad.setTapToClick(true);
    touchpad.setLmrTapButtonMap(lmr);
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 1, two, three}));
    QCOMPARE(parameters().readEntry("LmrTapButtonMap", !lmr), lmr);
    QVERIFY(!touchpad.isSaveNeeded());

    // Also exercise changing only the mapping while tapping remains enabled.
    touchpad.setLmrTapButtonMap(!lmr);
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 1, three, two}));
    QVERIFY(!touchpad.isSaveNeeded());
}

void SynapticsTouchpadTest::loadLiveMapping_data()
{
    tapMapping_data();
}

void SynapticsTouchpadTest::loadLiveMapping()
{
    QFETCH(bool, lmr);
    QFETCH(int, two);
    QFETCH(int, three);
    setProperty(SYNAPTICS_PROP_TAP_ACTION, {1, 2, 3, 0, 1, two, three});
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    QCOMPARE(touchpad.lmrTapButtonMap(), lmr);
    QVERIFY(!touchpad.isSaveNeeded());
}

void SynapticsTouchpadTest::restoreSavedTapping_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("lmr");
    QTest::newRow("enabled-right-click") << true << false;
    QTest::newRow("enabled-middle-click") << true << true;
    QTest::newRow("disabled-right-click") << false << false;
    QTest::newRow("disabled-middle-click") << false << true;
}

void SynapticsTouchpadTest::restoreSavedTapping()
{
    QFETCH(bool, enabled);
    QFETCH(bool, lmr);
    auto group = parameters();
    group.writeEntry("OneFingerTapButton", enabled ? 1 : 0);
    group.writeEntry("LmrTapButtonMap", lmr);
    QVERIFY(group.sync());
    const QList<int> expected{1, 2, 3, 0, enabled ? 1 : 0, enabled ? (lmr ? 2 : 3) : 0, enabled ? (lmr ? 3 : 2) : 0};

    // Repeat construction and load/save as happens at login and reconnect.
    for (int i = 0; i < 2; ++i) {
        setProperty(SYNAPTICS_PROP_TAP_ACTION, {1, 2, 3, 0, enabled ? 0 : 1, enabled ? 0 : 3, enabled ? 0 : 2});
        SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
        QVERIFY(touchpad.load());
        QCOMPARE(touchpad.isTapToClick(), enabled);
        QCOMPARE(touchpad.lmrTapButtonMap(), lmr);
        QVERIFY(touchpad.isSaveNeeded());
        QVERIFY(touchpad.save());
        QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), expected);
        QVERIFY(!touchpad.isSaveNeeded());
        QVERIFY(touchpad.save());
        QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), expected);
    }
}

void SynapticsTouchpadTest::restoreLegacyTapButtons()
{
    auto group = parameters();
    group.writeEntry("OneFingerTapButton", 1);
    group.writeEntry("TwoFingerTapButton", 2);
    group.writeEntry("ThreeFingerTapButton", 3);
    QVERIFY(group.sync());
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    QVERIFY(touchpad.lmrTapButtonMap());
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 1, 2, 3}));
}

void SynapticsTouchpadTest::restoreMappingWithTappingAlreadyEnabled()
{
    auto group = parameters();
    group.writeEntry("OneFingerTapButton", 1);
    group.writeEntry("LmrTapButtonMap", false);
    QVERIFY(group.sync());
    // The live mapping appears LRM to a boolean reader, but its third action
    // has been lost. Restoration must compare the complete tap action tuple.
    setProperty(SYNAPTICS_PROP_TAP_ACTION, {1, 2, 3, 0, 1, 3, 0});
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    QVERIFY(touchpad.isSaveNeeded());
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 1, 3, 2}));
    QVERIFY(!touchpad.isSaveNeeded());
}

void SynapticsTouchpadTest::changingMappingDoesNotEnableTapping()
{
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    touchpad.setLmrTapButtonMap(true);
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 0, 0, 0}));
    QVERIFY(!touchpad.isSaveNeeded());
    touchpad.setTapToClick(true);
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 1, 2, 3}));
}

void SynapticsTouchpadTest::disablingTappingAndChangingMapping()
{
    setProperty(SYNAPTICS_PROP_TAP_ACTION, {1, 2, 3, 0, 1, 3, 2});
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    touchpad.setTapToClick(false);
    touchpad.setLmrTapButtonMap(true);
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), (QList<int>{1, 2, 3, 0, 0, 0, 0}));
    QVERIFY(!touchpad.isSaveNeeded());
}

void SynapticsTouchpadTest::restoreDragSettings()
{
    auto group = parameters();
    group.writeEntry("TapAndDragGesture", true);
    group.writeEntry("LockedDrags", true);
    QVERIFY(group.sync());
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    QVERIFY(touchpad.isSaveNeeded());
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_GESTURES), (QList<int>{1}));
    QCOMPARE(property(SYNAPTICS_PROP_LOCKED_DRAGS), (QList<int>{1}));
    QVERIFY(!touchpad.isSaveNeeded());
}

void SynapticsTouchpadTest::preserveUnconfiguredTapActions()
{
    const QList<int> actions{1, 2, 3, 0, 2, 1, 3};
    setProperty(SYNAPTICS_PROP_TAP_ACTION, actions);
    SynapticsTouchpad touchpad(m_display, m_deviceId, QStringLiteral("Test touchpad"));
    QVERIFY(!touchpad.isSaveNeeded());
    QVERIFY(touchpad.save());
    QCOMPARE(property(SYNAPTICS_PROP_TAP_ACTION), actions);
}

QTEST_GUILESS_MAIN(SynapticsTouchpadTest)

#include "synapticstouchpad_test.moc"
