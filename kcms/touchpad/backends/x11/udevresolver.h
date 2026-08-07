/*
    SPDX-FileCopyrightText: 2026 SonicDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "touchpadclassifier.h"

#include <QString>

// Resolve an XInput device's event node through udev to determine
// if udev classifies it as a touchpad (ID_INPUT_TOUCHPAD=1).
// This is only compiled when HAVE_UDEV is defined.

struct UdevTouchpadResult {
    bool isTouchpad = false;
    QString deviceNode;
    QString diagnosticMessage;
};

#if HAVE_UDEV
// Check if the device at the given node path is classified as a touchpad by udev.
UdevTouchpadResult resolveUdevTouchpad(const QString &deviceNode);

// Read vendor/product IDs from udev for diagnostic purposes.
// Returns true if both IDs were successfully read.
bool readUdevVendorProduct(const QString &deviceNode, quint32 &vendorId, quint32 &productId);
#endif
