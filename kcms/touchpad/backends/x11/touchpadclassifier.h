/*
    SPDX-FileCopyrightText: 2026 SonicDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QFlags>
#include <QString>

// Value-only touchpad candidate classification.
// This header contains no Xlib/XCB types so it can be unit-tested
// without an X server or X11 development headers.

enum class DriverFamily {
    None = 0,
    Libinput = 1,
    Synaptics = 2,
};

enum class ClassificationOutcome {
    NotTouchpad,
    Normal,
    UdevFallback,
    NoDeviceNode,
    InvalidDeviceNode,
    UdevUnavailable,
    UnsupportedDriverProperties,
    XError,
    HeuristicLikelyTouchpad,
    NoVerifiedHeuristic,
};

struct TouchpadCandidate {
    int deviceId = -1;
    int xinputUse = 0; // IsXExtensionPointer etc., encoded as int
    bool isTouchpadType = false; // whether XInput type atom == XI_TOUCHPAD
    QString name;
    bool hasLibinputProperty = false;
    bool hasSynapticsProperty = false;
    QString deviceNode; // /dev/input/eventN
    quint32 vendorId = 0;
    quint32 productId = 0;
    bool vendorIdValid = false;
    bool productIdValid = false;
};

struct ClassificationResult {
    ClassificationOutcome outcome = ClassificationOutcome::NotTouchpad;
    DriverFamily driver = DriverFamily::None;
    int deviceId = -1;
    QString diagnosticMessage;
};

// Classify a single candidate through normal Xorg typing.
// Returns Normal if the candidate is XI_TOUCHPAD with a supported driver property.
ClassificationResult classifyNormal(const TouchpadCandidate &candidate);

// Attempt udev fallback classification for a non-TOUCHPAD pointer.
// When HAVE_UDEV is disabled at compile time, returns UdevUnavailable.
ClassificationResult classifyUdevFallback(const TouchpadCandidate &candidate);

// Run diagnostic-only hardware heuristics after both authoritative stages fail.
// This never classifies a device as a touchpad; it only produces a diagnostic
// outcome (HeuristicLikelyTouchpad or NoVerifiedHeuristic).
ClassificationResult classifyDiagnosticHeuristic(const TouchpadCandidate &candidate);

// Full classification pipeline: normal -> udev fallback -> diagnostic heuristic.
ClassificationResult classifyCandidate(const TouchpadCandidate &candidate);

// Select the best candidate from a list, preferring normal classification
// over udev fallback, then by lowest device ID as deterministic tie-breaker.
// The candidate list is not modified.
ClassificationResult selectBestCandidate(const QList<TouchpadCandidate> &candidates, int currentlyTrackedDeviceId);
