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

#include "fprinthelper.h"
#include "fprintdevice.h"

using namespace KAuth;

ActionReply FprintHelper::enroll(const QVariantMap &args)
{
    FprintDevice *device = args["device"].value<FprintDevice*>();
    
    QDBusError error = device->startEnrolling(args["finger"].toString());
    if (error.isValid()) {
        qDebug() << "error start enrolling:" << error.message();
        
        ActionReply reply(ActionReply::HelperErrorReply());
        reply.setErrorDescription(error.message());
        device->release();
        return reply;
    }
    return ActionReply::SuccessReply();
}

ActionReply FprintHelper::claim(const QVariantMap &args) 
{
    FprintDevice *device = args["device"].value<FprintDevice*>();
    
    // claim for user
    QDBusError error = device->claim(args["username"].toString());
    if (error.isValid()) {
        qDebug() << "error claiming:" << error.message();
        ActionReply reply(ActionReply::HelperErrorReply());
        reply.setErrorDescription(error.message());
        return reply;
    }
    return ActionReply::SuccessReply();
}

ActionReply FprintHelper::enrollothers(const QVariantMap &args)
{
    return enroll(args);
}

ActionReply FprintHelper::claimothers(const QVariantMap &args)
{
    return claim(args);
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmusers.fingerprint", FprintHelper)
