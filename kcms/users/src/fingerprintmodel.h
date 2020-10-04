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

#include "polkitqt1-authority.h"

#include "fprint_device_interface.h"
#include "fprint_manager_interface.h"

class FingerprintModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentError READ currentError WRITE setCurrentError NOTIFY currentErrorChanged) // error for ui to display
    Q_PROPERTY(QString enrollFeedback READ enrollFeedback WRITE setEnrollFeedback NOTIFY enrollFeedbackChanged)
    Q_PROPERTY(QStringList fingerprints READ fingerprints NOTIFY fingerprintsChanged)
    Q_PROPERTY(bool deviceFound READ deviceFound NOTIFY devicesFoundChanged)
    Q_PROPERTY(bool currentlyEnrolling READ currentlyEnrolling NOTIFY currentlyEnrollingChanged)
    
public:
    explicit FingerprintModel(QObject* parent = nullptr);
    ~FingerprintModel();

    Q_INVOKABLE void switchUser(QString username);
    bool claimDevice();
    
    Q_INVOKABLE void startEnrolling(QString finger);
    Q_INVOKABLE void stopEnrolling();
    Q_INVOKABLE void clearFingerprints();
    
    PolkitQt1::Authority::Result checkEditSelfPermission();
    PolkitQt1::Authority::Result checkEditOthersPermission();
    
    QStringList fingerprints();
    
    QString currentError()
    {
        return m_currentError;
    }
    void setCurrentError(QString error)
    {
        m_currentError = error;
        emit currentErrorChanged();
    }
    QString enrollFeedback()
    {
        return m_enrollFeedback;
    }
    void setEnrollFeedback(QString feedback)
    {
        m_enrollFeedback = feedback;
        emit enrollFeedbackChanged();
    }
    bool currentlyEnrolling()
    {
        return m_currentlyEnrolling;
    }
    bool deviceFound()
    {
        return m_device != nullptr;
    }

public Q_SLOTS:
    void handleEnrollCompleted();
    void handleEnrollStagePassed();
    void handleEnrollRetryStage(QString feedback);
    void handleEnrollFailed(QString error);

Q_SIGNALS:
    void currentErrorChanged();
    void enrollFeedbackChanged();
    void fingerprintsChanged();
    void devicesFoundChanged();
    void currentlyEnrollingChanged();
    
private:
    QString m_username; // set to "" if it is the currently logged in user
    QString m_currentError, m_enrollFeedback;
    
    bool m_currentlyEnrolling = false;
    
    FprintDevice *m_device = nullptr;
    
    NetReactivatedFprintManagerInterface *m_managerDbusInterface;
};
