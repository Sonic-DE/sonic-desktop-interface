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

#include <KLocalizedString>
#include <KAuthAction>
#include <KAuthExecuteJob>

#include "fingerprintmodel.h"

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

FingerprintModel::FingerprintModel(QObject* parent)
    : QObject(parent)
    , m_managerDbusInterface(new NetReactivatedFprintManagerInterface(QStringLiteral("net.reactivated.Fprint"), QStringLiteral("/net/reactivated/Fprint/Manager"), QDBusConnection::systemBus(), this))
{
    auto reply = m_managerDbusInterface->GetDefaultDevice();
    reply.waitForFinished();
    
    if (reply.isError()) {
        qDebug() << reply.error().message();
        setCurrentError(reply.error().message());
        return;
    }

    QDBusObjectPath path = reply.value();
    m_device = new FprintDevice(this, path);
    
    connect(m_device, &FprintDevice::enrollCompleted, this, &FingerprintModel::handleEnrollCompleted);
    connect(m_device, &FprintDevice::enrollStagePassed, this, &FingerprintModel::handleEnrollStagePassed);
    connect(m_device, &FprintDevice::enrollRetryStage, this, &FingerprintModel::handleEnrollRetryStage);
    connect(m_device, &FprintDevice::enrollFailed, this, &FingerprintModel::handleEnrollFailed);
}

FingerprintModel::~FingerprintModel()
{
    stopEnrolling();
    if (m_device != nullptr) delete m_device;
    delete m_managerDbusInterface;
}

void FingerprintModel::switchUser(QString username)
{
    m_username = username;
    
    if (deviceFound()) {
        stopEnrolling(); // stop enrolling if ongoing
        m_device->release(); // release from old user
        
        emit fingerprintsChanged();
    }
}

bool FingerprintModel::claimDevice()
{
    QVariantMap args;
    QVariant p;
    p.setValue(m_device);
    args["device"] = p;
    args["username"] = m_username;
    
    KAuth::Action action(m_username == "" ? "org.kde.kcontrol.kcmusers.fingerprint.claim" : "org.kde.kcontrol.kcmusers.fingerprint.claim.others");
    action.setArguments(args);
    KAuth::ExecuteJob *job = action.execute();
    if (!job->exec()) {
        setCurrentError(job->errorString());
        return false;
    }
    return true;
}

void FingerprintModel::startEnrolling(QString finger)
{
    setEnrollFeedback("");
    
    // claim device for user
    if (!claimDevice()) {
        return;
    }
    
    QVariantMap args;
    QVariant p;
    p.setValue(m_device);
    args["device"] = p;
    args["finger"] = finger;
    args["username"] = m_username;
    
    KAuth::Action action(m_username == "" ? "org.kde.kcontrol.kcmusers.fingerprint.enroll" : "org.kde.kcontrol.kcmusers.fingerprint.enroll.others");
    action.setArguments(args);
    KAuth::ExecuteJob *job = action.execute();
    if (!job->exec()) {
        setCurrentError(job->errorString());
        return;
    }
    
    m_currentlyEnrolling = true;
    emit currentlyEnrollingChanged();
}

void FingerprintModel::stopEnrolling()
{
    if (m_currentlyEnrolling) {
        m_currentlyEnrolling = false;
        emit currentlyEnrollingChanged();
        
        QDBusError error = m_device->stopEnrolling();
        if (error.isValid()) {
            qDebug() << "error stop enrolling:" << error.message();
            setCurrentError(error.message());
            return;
        }
        error = m_device->release();
        if (error.isValid()) {
            qDebug() << "error releasing:" << error.message();
            setCurrentError(error.message());
        }
    }
}

void FingerprintModel::clearFingerprints()
{
    // claim for user
    if (!claimDevice()) {
        return;
    }
    
    QDBusError error = m_device->deleteEnrolledFingers();
    if (error.isValid()) {
        qDebug() << "error clearing fingerprints:" << error.message();
        setCurrentError(error.message());
    }
    
    // release from user
    error = m_device->release();
    if (error.isValid()) {
        qDebug() << "error releasing:" << error.message();
        setCurrentError(error.message());
    }
    
    emit fingerprintsChanged();
}

QStringList FingerprintModel::fingerprints()
{
    QDBusPendingReply reply = m_device->listEnrolledFingers(m_username);
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "error listing enrolled fingers:" << reply.error().message();
        setCurrentError(reply.error().message());
        return QStringList();
    }
    
    return reply.value();
}

void FingerprintModel::handleEnrollCompleted()
{
    setEnrollFeedback("");
    stopEnrolling();
    emit fingerprintsChanged();
}

void FingerprintModel::handleEnrollStagePassed()
{
    setEnrollFeedback(i18n("Good!"));
}

void FingerprintModel::handleEnrollRetryStage(QString feedback)
{
    if (feedback == "enroll-retry-scan") {
        setEnrollFeedback(i18n("Retry scanning your finger."));
    } else if (feedback == "enroll-swipe-too-short") {
        setEnrollFeedback(i18n("Swipe too short. Try again."));
    } else if (feedback == "enroll-finger-not-centered") {
        setEnrollFeedback(i18n("Finger not centered on the reader. Try again."));
    } else if (feedback == "enroll-remove-and-retry") {
        setEnrollFeedback(i18n("Remove your finger from the reader, and try again."));
    }
}

void FingerprintModel::handleEnrollFailed(QString error)
{
    if (error == "enroll-failed") {
        setCurrentError(i18n("Fingerprint enrollment has failed."));
        stopEnrolling();
    } else if (error == "enroll-data-full") {
        setCurrentError(i18n("There is no space left for this device, delete other fingerprints to continue."));
        stopEnrolling();
    } else if (error == "enroll-disconnected") {
        setCurrentError(i18n("The device was disconnected."));
        m_currentlyEnrolling = false;
        emit currentlyEnrollingChanged();
    } else if (error == "enroll-unknown-error") {
        setCurrentError(i18n("An unknown error has occurred."));
        stopEnrolling();
    }
}
