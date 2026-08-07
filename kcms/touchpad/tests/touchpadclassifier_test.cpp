/*
    SPDX-FileCopyrightText: 2026 SonicDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadclassifier.h"

#include <QTest>

class TouchpadClassifierTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testNormalLibinputAcceptance();
    void testNormalSynapticsAcceptance();
    void testNonTouchpadRejected();
    void testUnsupportedDriverProperties();
    void testUdevUnavailableWhenDisabled();
    void testDiagnosticHeuristicNoVerifiedMatch();
    void testVendorProductAloneInsufficient();
    void testSelectBestCandidatePrefersNormal();
    void testSelectBestCandidateLowestId();
    void testSelectBestCandidateKeepsTracked();
    void testSelectBestCandidateEmpty();
};

void TouchpadClassifierTest::testNormalLibinputAcceptance()
{
    TouchpadCandidate c;
    c.deviceId = 5;
    c.isTouchpadType = true;
    c.hasLibinputProperty = true;

    auto result = classifyNormal(c);
    QCOMPARE(result.outcome, ClassificationOutcome::Normal);
    QCOMPARE(result.driver, DriverFamily::Libinput);
    QCOMPARE(result.deviceId, 5);
    QVERIFY(result.diagnosticMessage.isEmpty());
}

void TouchpadClassifierTest::testNormalSynapticsAcceptance()
{
    TouchpadCandidate c;
    c.deviceId = 7;
    c.isTouchpadType = true;
    c.hasSynapticsProperty = true;

    auto result = classifyNormal(c);
    QCOMPARE(result.outcome, ClassificationOutcome::Normal);
    QCOMPARE(result.driver, DriverFamily::Synaptics);
}

void TouchpadClassifierTest::testNonTouchpadRejected()
{
    TouchpadCandidate c;
    c.deviceId = 3;
    c.isTouchpadType = false;
    c.hasLibinputProperty = true; // has property but wrong type

    auto result = classifyNormal(c);
    QCOMPARE(result.outcome, ClassificationOutcome::NotTouchpad);
}

void TouchpadClassifierTest::testUnsupportedDriverProperties()
{
    TouchpadCandidate c;
    c.deviceId = 9;
    c.isTouchpadType = true;
    c.hasLibinputProperty = false;
    c.hasSynapticsProperty = false;

    auto result = classifyNormal(c);
    QCOMPARE(result.outcome, ClassificationOutcome::UnsupportedDriverProperties);
    QVERIFY(!result.diagnosticMessage.isEmpty());
}

void TouchpadClassifierTest::testUdevUnavailableWhenDisabled()
{
    TouchpadCandidate c;
    c.deviceId = 11;
    c.isTouchpadType = false;
    c.deviceNode = QStringLiteral("/dev/input/event5");

    auto result = classifyUdevFallback(c);
#if HAVE_UDEV
    // When udev is available, the classifier delegates to UdevResolver.
    // The test only verifies the classification logic, not real udev resolution.
    QSKIP("Udev resolution is tested through integration tests with real hardware");
#else
    QCOMPARE(result.outcome, ClassificationOutcome::UdevUnavailable);
#endif
}

void TouchpadClassifierTest::testDiagnosticHeuristicNoVerifiedMatch()
{
    TouchpadCandidate c;
    c.deviceId = 13;
    c.isTouchpadType = false;
    c.name = QStringLiteral("StarLite Keyboard Touchpad");
    c.vendorId = 0x27c6;
    c.productId = 0x0111;
    c.vendorIdValid = true;
    c.productIdValid = true;

    auto result = classifyDiagnosticHeuristic(c);
    QCOMPARE(result.outcome, ClassificationOutcome::NoVerifiedHeuristic);
    QVERIFY(!result.diagnosticMessage.isEmpty());
}

void TouchpadClassifierTest::testVendorProductAloneInsufficient()
{
    // 27c6:0111 alone must not classify as a touchpad
    TouchpadCandidate c;
    c.deviceId = 15;
    c.isTouchpadType = false;
    c.vendorId = 0x27c6;
    c.productId = 0x0111;
    c.vendorIdValid = true;
    c.productIdValid = true;

    // Full pipeline should end at NoVerifiedHeuristic
    auto result = classifyCandidate(c);
    QCOMPARE(result.outcome, ClassificationOutcome::NoVerifiedHeuristic);
    QVERIFY(result.driver == DriverFamily::None);
}

void TouchpadClassifierTest::testSelectBestCandidatePrefersNormal()
{
    QList<TouchpadCandidate> candidates;

    // Udev fallback candidate (non-TOUCHPAD with node)
    TouchpadCandidate fallback;
    fallback.deviceId = 2;
    fallback.isTouchpadType = false;
    fallback.deviceNode = QStringLiteral("/dev/input/event3");

    // Normal candidate (TOUCHPAD with libinput)
    TouchpadCandidate normal;
    normal.deviceId = 5;
    normal.isTouchpadType = true;
    normal.hasLibinputProperty = true;

    candidates << fallback << normal;

    auto result = selectBestCandidate(candidates, -1);
    QCOMPARE(result.outcome, ClassificationOutcome::Normal);
    QCOMPARE(result.deviceId, 5);
}

void TouchpadClassifierTest::testSelectBestCandidateLowestId()
{
    QList<TouchpadCandidate> candidates;

    TouchpadCandidate c1;
    c1.deviceId = 10;
    c1.isTouchpadType = true;
    c1.hasLibinputProperty = true;

    TouchpadCandidate c2;
    c2.deviceId = 3;
    c2.isTouchpadType = true;
    c2.hasLibinputProperty = true;

    candidates << c1 << c2;

    auto result = selectBestCandidate(candidates, -1);
    QCOMPARE(result.outcome, ClassificationOutcome::Normal);
    QCOMPARE(result.deviceId, 3);
}

void TouchpadClassifierTest::testSelectBestCandidateKeepsTracked()
{
    QList<TouchpadCandidate> candidates;

    TouchpadCandidate c1;
    c1.deviceId = 3;
    c1.isTouchpadType = true;
    c1.hasLibinputProperty = true;

    TouchpadCandidate c2;
    c2.deviceId = 7;
    c2.isTouchpadType = true;
    c2.hasLibinputProperty = true;

    candidates << c1 << c2;

    // Currently tracking device 7 - should keep it
    auto result = selectBestCandidate(candidates, 7);
    QCOMPARE(result.outcome, ClassificationOutcome::Normal);
    QCOMPARE(result.deviceId, 7);
}

void TouchpadClassifierTest::testSelectBestCandidateEmpty()
{
    QList<TouchpadCandidate> candidates;

    auto result = selectBestCandidate(candidates, -1);
    QCOMPARE(result.outcome, ClassificationOutcome::NotTouchpad);
}

QTEST_MAIN(TouchpadClassifierTest)
#include "touchpadclassifier_test.moc"
