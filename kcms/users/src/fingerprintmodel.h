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

#pragma once

#include "fprintdevice.h"

#include <KLocalizedString>

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

class FingerprintModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString scanType READ scanType)
    Q_PROPERTY(QString currentError READ currentError WRITE setCurrentError NOTIFY currentErrorChanged) // error for ui to display
    Q_PROPERTY(QString enrollFeedback READ enrollFeedback WRITE setEnrollFeedback NOTIFY enrollFeedbackChanged)
    Q_PROPERTY(QStringList enrolledFingerprints READ enrolledFingerprints NOTIFY enrolledFingerprintsChanged)
    Q_PROPERTY(QStringList availableFingersToEnroll READ availableFingersToEnroll NOTIFY enrolledFingerprintsChanged)
    Q_PROPERTY(bool deviceFound READ deviceFound NOTIFY devicesFoundChanged)
    Q_PROPERTY(bool currentlyEnrolling READ currentlyEnrolling NOTIFY currentlyEnrollingChanged)
    Q_PROPERTY(double enrollProgress READ enrollProgress NOTIFY enrollProgressChanged)
    Q_PROPERTY(QString dialogState READ dialogState WRITE setDialogState NOTIFY dialogStateChanged)
    
public:
    explicit FingerprintModel(QObject* parent = nullptr);
    ~FingerprintModel();

    enum DialogState {
        FingerprintList,
        PickFinger,
        Enrolling,
        EnrollComplete,
    };
    
    const QList<QString> FINGERS = {"left-thumb", "left-index-finger", "left-middle-finger", "left-ring-finger", "left-little-finger", "right-thumb", "right-index-finger", "right-middle-finger", "right-ring-finger", "right-little-finger"};
    const QList<QString> FINGERS_FRIENDLY_NAME = {i18n("Left Thumb"), i18n("Left Index Finger"), i18n("Left Middle Finger"), i18n("Left Ring Finger"), i18n("Left Little Finger"), i18n("Right Thumb"), i18n("Right Index Finger"), i18n("Right Middle Finger"), i18n("Right Ring Finger"), i18n("Right Little Finger")};
    
    Q_INVOKABLE void switchUser(QString username);
    bool claimDevice();
    
    Q_INVOKABLE void startEnrolling(QString finger);
    Q_INVOKABLE void stopEnrolling();
    Q_INVOKABLE void clearFingerprints();
    
    QStringList enrolledFingerprints();
    QStringList availableFingersToEnroll();
    
    QString scanType();
    QString currentError();
    void setCurrentError(QString error);
    QString enrollFeedback();
    void setEnrollFeedback(QString feedback);
    bool currentlyEnrolling();
    bool deviceFound();
    double enrollProgress();
    void setEnrollStage(int stage);
    QString dialogState();
    void setDialogState(DialogState dialogState);
    void setDialogState(QString dialogState);
    
public Q_SLOTS:
    void handleEnrollCompleted();
    void handleEnrollStagePassed();
    void handleEnrollRetryStage(QString feedback);
    void handleEnrollFailed(QString error);

Q_SIGNALS:
    void currentErrorChanged();
    void enrollFeedbackChanged();
    void enrolledFingerprintsChanged();
    void devicesFoundChanged();
    void currentlyEnrollingChanged();
    void enrollProgressChanged();
    void dialogStateChanged();
    
private:
    QString m_username; // set to "" if it is the currently logged in user
    QString m_currentError, m_enrollFeedback;
    
    DialogState m_dialogState = DialogState::FingerprintList;
    
    bool m_currentlyEnrolling = false;
    int m_enrollStage = 0;
    
    FprintDevice *m_device = nullptr;
    
    NetReactivatedFprintManagerInterface *m_managerDbusInterface;
};
