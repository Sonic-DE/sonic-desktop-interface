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

#include "polkitqt1-authority.h"

#include "fingerprintmodel.h"

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

using namespace PolkitQt1;

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

QString FingerprintModel::currentError()
{
    return m_currentError;
}

void FingerprintModel::setCurrentError(QString error)
{
    m_currentError = error;
    emit currentErrorChanged();
}

QString FingerprintModel::enrollFeedback()
{
    return m_enrollFeedback;
}

void FingerprintModel::setEnrollFeedback(QString feedback)
{
    m_enrollFeedback = feedback;
    emit enrollFeedbackChanged();
}

bool FingerprintModel::currentlyEnrolling()
{
    return m_currentlyEnrolling;
}

bool FingerprintModel::deviceFound()
{
    return m_device != nullptr;
}

double FingerprintModel::enrollProgress()
{
    return deviceFound() ? ((double) m_enrollStage) / m_device->numOfEnrollStages() : 0;
}

void FingerprintModel::setEnrollStage(int stage)
{
    m_enrollStage = stage;
    emit enrollProgressChanged();
}

QString FingerprintModel::dialogState()
{
    switch (m_dialogState) {
        case DialogState::FingerprintList:
            return "FingerprintList";
        case DialogState::PickFinger:
            return "PickFinger";
        case DialogState::Enrolling:
            return "Enrolling";
        case DialogState::EnrollComplete:
            return "EnrollComplete";
    }
    return "FingerprintList";
}

void FingerprintModel::setDialogState(DialogState dialogState)
{
    m_dialogState = dialogState;
    emit dialogStateChanged();
}

void FingerprintModel::setDialogState(QString dialogState)
{
    if (dialogState == "FingerprintList") {
        m_dialogState = DialogState::FingerprintList;
    } else if (dialogState == "PickFinger") {
        m_dialogState = DialogState::PickFinger;
    } else if (dialogState == "Enrolling") {
        m_dialogState = DialogState::Enrolling;
    } else if (dialogState == "EnrollComplete") {
        m_dialogState = DialogState::EnrollComplete;
    }
    emit dialogStateChanged();
}


void FingerprintModel::switchUser(QString username)
{
    m_username = username;
    
    if (deviceFound()) {
        stopEnrolling(); // stop enrolling if ongoing
        m_device->release(); // release from old user
        
        emit enrolledFingerprintsChanged();
    }
}

Authority::Result FingerprintModel::checkEditSelfPermission()
{
    UnixProcessSubject subject(static_cast<uint>(QCoreApplication::applicationPid()));
    return Authority::instance()->checkAuthorizationSync("net.reactivated.fprint.device.enroll", subject, Authority::AllowUserInteraction);
}


Authority::Result FingerprintModel::checkEditOthersPermission()
{
    UnixProcessSubject subject(static_cast<uint>(QCoreApplication::applicationPid()));
    return Authority::instance()->checkAuthorizationSync("net.reactivated.fprint.device.setusername", subject, Authority::AllowUserInteraction);
}


bool FingerprintModel::claimDevice()
{
    QDBusError error = m_device->claim(m_username);
    if (error.isValid()) {
        qDebug() << "error claiming:" << error.message();
        setCurrentError(error.message());
        return false;
    }
    return true;
}

void FingerprintModel::startEnrolling(QString finger)
{
    setEnrollStage(0);
    setEnrollFeedback("");
    
    // claim device for user
    if (!claimDevice()) {
        return;
    }
    
    // this may not be necessary, as fprintd should also checks polkit and prompts if necessary
    Authority::Result result = m_username == "" ? checkEditSelfPermission() : checkEditOthersPermission();
    if (result == Authority::No) {
        setCurrentError(i18n("Not authorized"));
        m_device->release();
        return;
    }
    
    QDBusError error = m_device->startEnrolling(finger);
    if (error.isValid()) {
        qDebug() << "error start enrolling:" << error.message();
        setCurrentError(error.message());
        m_device->release();
        return;
    }
    
    m_currentlyEnrolling = true;
    emit currentlyEnrollingChanged();
    
    setDialogState(DialogState::Enrolling);
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
        
        setDialogState(DialogState::FingerprintList);
    }
}

void FingerprintModel::clearFingerprints()
{
    // claim for user
    if (!claimDevice()) {
        return;
    }
 
    // this may not be necessary, as fprintd should also checks polkit and prompts if necessary
    Authority::Result result = m_username == "" ? checkEditSelfPermission() : checkEditOthersPermission();
    if (result == Authority::No) {
        setCurrentError(i18n("Not authorized"));
        m_device->release();
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
    
    emit enrolledFingerprintsChanged();
}

QStringList FingerprintModel::enrolledFingerprints()
{
    QDBusPendingReply reply = m_device->listEnrolledFingers(m_username);
    reply.waitForFinished();
    if (reply.isError()) {
        // ignore net.reactivated.Fprint.Error.NoEnrolledPrints, as it shows up when there are no fingerprints
        if (reply.error().name() != "net.reactivated.Fprint.Error.NoEnrolledPrints") {
            qDebug() << "error listing enrolled fingers:" << reply.error().message();
            setCurrentError(reply.error().message());
        }
        return QStringList();
    }
    
    return reply.value();
}

QStringList FingerprintModel::availableFingersToEnroll()
{
    QStringList list, enrolled = enrolledFingerprints();
    for (QString finger : FINGERS) {
        if (!enrolled.contains(finger)) {
            list.append(finger);
        }
    }
    return list;
}

void FingerprintModel::handleEnrollCompleted()
{
    setEnrollStage(m_device->numOfEnrollStages());
    setEnrollFeedback("");
    emit enrolledFingerprintsChanged();
    
    // stopEnrolling not called, as it is triggered only when the "complete" button is pressed
    // (only change dialog state change after button is pressed)
    setDialogState(DialogState::EnrollComplete);
}

void FingerprintModel::handleEnrollStagePassed()
{
    setEnrollStage(m_enrollStage + 1);
    setEnrollFeedback("");
    qDebug() << "fingerprint enroll stage pass:" << enrollProgress();
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
    qDebug() << "fingerprint enroll stage fail:" << feedback;
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
