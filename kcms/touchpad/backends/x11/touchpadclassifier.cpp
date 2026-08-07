/*
    SPDX-FileCopyrightText: 2026 SonicDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadclassifier.h"

#include <QList>
#include <algorithm>

ClassificationResult classifyNormal(const TouchpadCandidate &candidate)
{
    if (!candidate.isTouchpadType) {
        return {ClassificationOutcome::NotTouchpad, DriverFamily::None, candidate.deviceId, QStringLiteral("Device is not typed as XI_TOUCHPAD by Xorg")};
    }

    if (candidate.hasLibinputProperty) {
        return {ClassificationOutcome::Normal, DriverFamily::Libinput, candidate.deviceId, QString()};
    }

    if (candidate.hasSynapticsProperty) {
        return {ClassificationOutcome::Normal, DriverFamily::Synaptics, candidate.deviceId, QString()};
    }

    return {ClassificationOutcome::UnsupportedDriverProperties,
            DriverFamily::None,
            candidate.deviceId,
            QStringLiteral("XI_TOUCHPAD device lacks libinput or Synaptics configuration properties")};
}

ClassificationResult classifyUdevFallback(const TouchpadCandidate &candidate)
{
#if !HAVE_UDEV
    Q_UNUSED(candidate)
    return {ClassificationOutcome::UdevUnavailable, DriverFamily::None, -1, QStringLiteral("Udev fallback classification is not available in this build")};
#else
    // Udev resolution is performed by UdevResolver in udevresolver.cpp.
    // The classifier itself only checks the result of that resolution.
    // This function is called after udev resolution has been attempted
    // by the caller (XlibBackend) and the candidate's deviceNode has been
    // populated. The actual udev property checking is done in udevresolver.
    Q_UNUSED(candidate)
    return {ClassificationOutcome::UdevUnavailable, DriverFamily::None, -1, QStringLiteral("Udev fallback should be handled by UdevResolver")};
#endif
}

ClassificationResult classifyDiagnosticHeuristic(const TouchpadCandidate &candidate)
{
    // No verified StarLite touchpad identifiers are currently available.
    // 27c6:0111 is associated with the built-in Goodix touchscreen/keyboard
    // collection and must not be treated as evidence for the detachable touchpad.
    //
    // When issue-40 evidence provides correlated vendor/product IDs,
    // a narrowly scoped match can be added here. For now, all candidates
    // receive NoVerifiedHeuristic.
    Q_UNUSED(candidate)
    return {ClassificationOutcome::NoVerifiedHeuristic,
            DriverFamily::None,
            candidate.deviceId,
            QStringLiteral("No verified hardware heuristic matches this device. "
                           "Collect xinput, libinput, and udevadm data for diagnosis.")};
}

ClassificationResult classifyCandidate(const TouchpadCandidate &candidate)
{
    // Stage 1: Normal Xorg classification
    auto result = classifyNormal(candidate);
    if (result.outcome == ClassificationOutcome::Normal) {
        return result;
    }

    // Stage 2: Udev fallback (only for non-TOUCHPAD pointers)
    if (!candidate.isTouchpadType) {
        result = classifyUdevFallback(candidate);
        if (result.outcome == ClassificationOutcome::UdevFallback) {
            return result;
        }
    }

    // Stage 3: Diagnostic-only heuristic
    result = classifyDiagnosticHeuristic(candidate);
    return result;
}

ClassificationResult selectBestCandidate(const QList<TouchpadCandidate> &candidates, int currentlyTrackedDeviceId)
{
    ClassificationResult best;
    best.outcome = ClassificationOutcome::NotTouchpad;

    // Check if the currently tracked device is still present and valid
    for (const auto &c : candidates) {
        if (c.deviceId == currentlyTrackedDeviceId) {
            auto r = classifyNormal(c);
            if (r.outcome == ClassificationOutcome::Normal) {
                return r;
            }
        }
    }

    // Prefer normal classification, then udev fallback, then lowest device ID
    for (const auto &c : candidates) {
        auto r = classifyNormal(c);
        if (r.outcome == ClassificationOutcome::Normal) {
            if (best.outcome != ClassificationOutcome::Normal || c.deviceId < best.deviceId) {
                best = r;
            }
        }
    }

    if (best.outcome == ClassificationOutcome::Normal) {
        return best;
    }

    // Try udev fallback for non-TOUCHPAD pointers
    for (const auto &c : candidates) {
        if (!c.isTouchpadType) {
            auto r = classifyUdevFallback(c);
            if (r.outcome == ClassificationOutcome::UdevFallback) {
                if (best.outcome != ClassificationOutcome::UdevFallback || c.deviceId < best.deviceId) {
                    best = r;
                }
            }
        }
    }

    return best;
}
