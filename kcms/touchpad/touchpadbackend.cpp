/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadbackend.h"

#if BUILD_KCM_TOUCHPAD_X11
#include "backends/x11/xlibbackend.h"
#endif

#include <Sonic/Logging/Log>

#include <QThreadStorage>

#include <KWindowSystem>

void TouchpadBackend::setMode(TouchpadInputBackendMode mode)
{
    m_mode = mode;
}

TouchpadBackend *TouchpadBackend::implementation()
{
    // There are multiple possible backends
#if BUILD_KCM_TOUCHPAD_X11
    {
        static QThreadStorage<std::shared_ptr<XlibBackend>> backend;
        if (!backend.hasLocalData()) {
            sonicDebug() << "Using X11 backend";
            backend.setLocalData(std::shared_ptr<XlibBackend>(XlibBackend::initialize()));
        }
        return backend.localData().get();
    }
#endif
    sonicCritical() << "Not able to select appropriate backend.";
    return nullptr;
}
