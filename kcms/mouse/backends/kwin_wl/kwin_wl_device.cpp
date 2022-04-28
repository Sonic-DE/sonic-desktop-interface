/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_device.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusReply>
#include <QVector>

#include "logging.h"

Q_DECLARE_METATYPE(Qt::MouseButton)

QDBusArgument &operator<<(QDBusArgument &argument, const QMap<Qt::MouseButton, QKeySequence> &mapping)
{
    argument.beginMap(qMetaTypeId<std::underlying_type_t<Qt::MouseButton>>(), qMetaTypeId<int>());
    std::for_each(mapping.keyValueBegin(), mapping.keyValueEnd(), [&argument](const std::pair<Qt::MouseButton, QKeySequence> &entry) {
        argument.beginMapEntry();
        argument << entry.first << entry.second[0];
        argument.endMapEntry();
    });
    argument.endMap();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QMap<Qt::MouseButton, QKeySequence> &mapping)
{
    mapping.clear();
    argument.beginMap();
    while (!argument.atEnd()) {
        int key;
        int value;
        argument.beginMapEntry();
        argument >> key >> value;
        argument.endMapEntry();
        mapping.insert(static_cast<Qt::MouseButton>(key), value);
    };
    argument.endMap();
    return argument;
}

namespace
{
template<typename T>
T valueLoaderPart(QVariant const &reply)
{
    return reply.value<T>();
}

template<>
Qt::MouseButtons valueLoaderPart(QVariant const &reply)
{
    return static_cast<Qt::MouseButtons>(reply.toInt());
}

template<>
QMap<Qt::MouseButton, QKeySequence> valueLoaderPart(QVariant const &reply)
{
    return qdbus_cast<QMap<Qt::MouseButton, QKeySequence>>(reply);
}
}

KWinWaylandDevice::KWinWaylandDevice(QString dbusName)
    : m_dbusName(dbusName)
{
    qDBusRegisterMetaType<QMap<Qt::MouseButton, QKeySequence>>();
}

KWinWaylandDevice::~KWinWaylandDevice()
{
}

bool KWinWaylandDevice::init()
{
    bool success = true;

    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("GetAll"));
    message << QStringLiteral("org.kde.KWin.InputDevice");
    QDBusReply<QVariantMap> reply = QDBusConnection::sessionBus().call(message);

    if (!reply.isValid()) {
        return false;
    }

    auto valueLoader = [properties = reply.value(), this](auto &prop) {
        if (QVariant variant = properties.value(prop.dbus); variant.isValid()) {
            prop.avail = true;
            prop.old = valueLoaderPart<typename std::remove_reference_t<decltype(prop)>::value_type>(variant);
            prop.val = prop.old;
            return true;
        }
        qCCritical(KCM_MOUSE) << "Device" << m_dbusName << "does not have property on d-bus read of" << prop.dbus;
        prop.avail = false;
        return false;
    };

    // general
    success &= valueLoader(m_name);
    success &= valueLoader(m_sysName);
    success &= valueLoader(m_supportsDisableEvents);
    success &= valueLoader(m_enabled);
    // advanced
    success &= valueLoader(m_supportedButtons);
    success &= valueLoader(m_buttonMapping);
    success &= valueLoader(m_supportsLeftHanded);
    success &= valueLoader(m_leftHandedEnabledByDefault);
    success &= valueLoader(m_leftHanded);
    success &= valueLoader(m_supportsMiddleEmulation);
    success &= valueLoader(m_middleEmulationEnabledByDefault);
    success &= valueLoader(m_middleEmulation);
    // acceleration
    success &= valueLoader(m_supportsPointerAcceleration);
    success &= valueLoader(m_supportsPointerAccelerationProfileFlat);
    success &= valueLoader(m_supportsPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_defaultPointerAcceleration);
    success &= valueLoader(m_defaultPointerAccelerationProfileFlat);
    success &= valueLoader(m_defaultPointerAccelerationProfileAdaptive);
    success &= valueLoader(m_pointerAcceleration);
    success &= valueLoader(m_pointerAccelerationProfileFlat);
    success &= valueLoader(m_pointerAccelerationProfileAdaptive);
    // natural scroll
    success &= valueLoader(m_supportsNaturalScroll);
    success &= valueLoader(m_naturalScrollEnabledByDefault);
    success &= valueLoader(m_naturalScroll);

    success &= valueLoader(m_scrollFactor);

    return success;
}

bool KWinWaylandDevice::getDefaultConfig()
{
    m_enabled.set(true);
    m_leftHanded.set(false);

    m_pointerAcceleration.set(m_defaultPointerAcceleration);
    m_pointerAccelerationProfileFlat.set(m_defaultPointerAccelerationProfileFlat);
    m_pointerAccelerationProfileAdaptive.set(m_defaultPointerAccelerationProfileAdaptive);

    m_middleEmulation.set(m_middleEmulationEnabledByDefault);
    m_naturalScroll.set(m_naturalScrollEnabledByDefault);

    m_scrollFactor.set(1.0);

    return true;
}

bool KWinWaylandDevice::applyConfig()
{
    QVector<QString> msgs;

    msgs << valueWriter(m_enabled) << valueWriter(m_leftHanded) << valueWriter(m_pointerAcceleration) << valueWriter(m_pointerAccelerationProfileFlat)
         << valueWriter(m_pointerAccelerationProfileAdaptive) << valueWriter(m_middleEmulation) << valueWriter(m_naturalScroll) << valueWriter(m_scrollFactor)
         << valueWriter(m_buttonMapping);

    bool success = true;
    QString error_msg;

    for (QString m : msgs) {
        if (!m.isNull()) {
            qCCritical(KCM_MOUSE) << "in error:" << m;
            if (!success) {
                error_msg.append("\n");
            }
            error_msg.append(m);
            success = false;
        }
    }

    if (!success) {
        qCCritical(KCM_MOUSE) << error_msg;
    }
    return success;
}

bool KWinWaylandDevice::isChangedConfig() const
{
    return m_enabled.changed() || m_leftHanded.changed() || m_pointerAcceleration.changed() || m_pointerAccelerationProfileFlat.changed()
        || m_pointerAccelerationProfileAdaptive.changed() || m_middleEmulation.changed() || m_scrollFactor.changed() || m_naturalScroll.changed()
        || m_buttonMapping.changed();
}

template<typename T>
QString KWinWaylandDevice::valueWriter(const Prop<T> &prop)
{
    if (!prop.changed()) {
        return QString();
    }
    auto message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                  QStringLiteral("/org/kde/KWin/InputDevice/") + m_dbusName,
                                                  QStringLiteral("org.freedesktop.DBus.Properties"),
                                                  QStringLiteral("Set"));
    message << QStringLiteral("org.kde.KWin.InputDevice") << prop.dbus << QVariant::fromValue(QDBusVariant(QVariant::fromValue(prop.val)));
    QDBusReply<void> reply = QDBusConnection::sessionBus().call(message);
    if (reply.error().isValid()) {
        qCCritical(KCM_MOUSE) << reply.error().message();
        return reply.error().message();
    }
    return QString();
}
