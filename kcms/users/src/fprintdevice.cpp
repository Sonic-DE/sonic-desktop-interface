/*
    Copyright 2020  Devin Lin <espidev@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fprintdevice.h"

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

FprintDevice::FprintDevice(QObject* parent, QDBusObjectPath path)
    : QObject(parent)
    , m_dbusInterface(new NetReactivatedFprintDeviceInterface(QStringLiteral("net.reactivated.Fprint"), path.path(), QDBusConnection::systemBus(), this))
{
    connect(m_dbusInterface, &NetReactivatedFprintDeviceInterface::EnrollStatus, this, &FprintDevice::enrollStatus);
}

FprintDevice::~FprintDevice()
{
    delete m_dbusInterface;
}

QDBusPendingReply<QStringList> FprintDevice::listEnrolledFingers(QString username)
{
    return m_dbusInterface->ListEnrolledFingers(username);
}

QDBusError FprintDevice::claim(QString username)
{
    auto reply = m_dbusInterface->Claim(username);
    reply.waitForFinished();
    return reply.error();
}

QDBusError FprintDevice::release()
{
    auto reply = m_dbusInterface->Release();
    reply.waitForFinished();
    return reply.error();
}

QDBusError FprintDevice::deleteEnrolledFingers()
{
    auto reply = m_dbusInterface->DeleteEnrolledFingers2();
    reply.waitForFinished();
    return reply.error();
}

QDBusError FprintDevice::startEnrolling(QString finger)
{
    auto reply = m_dbusInterface->EnrollStart(finger);
    reply.waitForFinished();
    return reply.error();
}

QDBusError FprintDevice::stopEnrolling()
{
    auto reply = m_dbusInterface->EnrollStop();
    reply.waitForFinished();
    return reply.error();
}

void FprintDevice::enrollStatus(QString result, bool done)
{
    if (result == "enroll-completed") {
        emit enrollCompleted();
    } else if (result == "enroll-failed" || result == "enroll-data-full" || result == "enroll-disconnected" || result == "enroll-unknown-error") {
        emit enrollFailed(result);
    } else if (result == "enroll-stage-passed") {
        emit enrollStagePassed();
    } else if (result == "enroll-retry-scan" || result == "enroll-swipe-too-short" || result == "enroll-finger-not-centered" || result == "enroll-remove-and-retry") {
        emit enrollRetryStage(result);
    }
}

int FprintDevice::numOfEnrollStages()
{
    return m_dbusInterface->num_enroll_stages();
}


