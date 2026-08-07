/*
    SPDX-FileCopyrightText: 2026 SonicDE Contributors

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "udevresolver.h"

#if HAVE_UDEV

#include "logging.h"

#include <QDebug>
#include <QRegularExpression>

#include <fcntl.h>
#include <libudev.h>
#include <sys/stat.h>
#include <unistd.h>

struct UdevDeleter {
    void operator()(udev *u) const
    {
        if (u) {
            udev_unref(u);
        }
    }
};

struct UdevDeviceDeleter {
    void operator()(udev_device *d) const
    {
        if (d) {
            udev_device_unref(d);
        }
    }
};

using UdevPtr = std::unique_ptr<udev, UdevDeleter>;
using UdevDevicePtr = std::unique_ptr<udev_device, UdevDeviceDeleter>;

static bool isValidEventNode(const QString &path)
{
    // Must be an absolute path of the form /dev/input/eventN
    static const QRegularExpression pattern(QStringLiteral("^/dev/input/event[0-9]+$"));
    if (!pattern.match(path).hasMatch()) {
        return false;
    }

    struct stat st;
    if (stat(path.toUtf8().constData(), &st) != 0) {
        return false;
    }
    if (!S_ISCHR(st.st_mode)) {
        return false;
    }
    return true;
}

UdevTouchpadResult resolveUdevTouchpad(const QString &deviceNode)
{
    UdevTouchpadResult result;
    result.deviceNode = deviceNode;

    if (deviceNode.isEmpty()) {
        result.diagnosticMessage = QStringLiteral("No device node property on XInput device");
        return result;
    }

    if (!isValidEventNode(deviceNode)) {
        result.diagnosticMessage = QStringLiteral("Invalid or non-event device node: %1").arg(deviceNode);
        return result;
    }

    struct stat st;
    if (stat(deviceNode.toUtf8().constData(), &st) != 0) {
        result.diagnosticMessage = QStringLiteral("Cannot stat device node: %1").arg(deviceNode);
        return result;
    }

    UdevPtr udev(udev_new());
    if (!udev) {
        result.diagnosticMessage = QStringLiteral("Cannot create udev context");
        return result;
    }

    UdevDevicePtr dev(udev_device_new_from_devnum(udev.get(), 'c', st.st_rdev));
    if (!dev) {
        result.diagnosticMessage = QStringLiteral("Cannot create udev device for: %1").arg(deviceNode);
        return result;
    }

    const char *idInput = udev_device_get_property_value(dev.get(), "ID_INPUT");
    if (!idInput || strcmp(idInput, "1") != 0) {
        result.diagnosticMessage = QStringLiteral("udev does not report ID_INPUT=1 for: %1").arg(deviceNode);
        return result;
    }

    const char *idTouchpad = udev_device_get_property_value(dev.get(), "ID_INPUT_TOUCHPAD");
    if (!idTouchpad || strcmp(idTouchpad, "1") != 0) {
        result.diagnosticMessage = QStringLiteral("udev does not report ID_INPUT_TOUCHPAD=1 for: %1").arg(deviceNode);
        return result;
    }

    result.isTouchpad = true;
    return result;
}

bool readUdevVendorProduct(const QString &deviceNode, quint32 &vendorId, quint32 &productId)
{
    if (!isValidEventNode(deviceNode)) {
        return false;
    }

    struct stat st;
    if (stat(deviceNode.toUtf8().constData(), &st) != 0) {
        return false;
    }

    UdevPtr udev(udev_new());
    if (!udev) {
        return false;
    }

    UdevDevicePtr dev(udev_device_new_from_devnum(udev.get(), 'c', st.st_rdev));
    if (!dev) {
        return false;
    }

    const char *vendor = udev_device_get_property_value(dev.get(), "ID_VENDOR_ID");
    const char *product = udev_device_get_property_value(dev.get(), "ID_MODEL_ID");

    if (!vendor || !product) {
        return false;
    }

    bool okVendor = false, okProduct = false;
    vendorId = QString::fromUtf8(vendor).toUInt(&okVendor, 16);
    productId = QString::fromUtf8(product).toUInt(&okProduct, 16);

    return okVendor && okProduct;
}

#endif // HAVE_UDEV
