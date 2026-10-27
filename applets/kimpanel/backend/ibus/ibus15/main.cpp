/*
    This file is part of KIMToy, an input method frontend for KDE
    SPDX-FileCopyrightText: 2011 Ni Hui <shuizhuyuanluo@126.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "app.h"
#include "panel.h"

#include <QSessionManager>
#include <Sonic/Logging/Logging>
#include <ibus.h>
#include <locale.h>
#include <stdlib.h>

SONIC_LOGGING_COMPONENT("org.sonicde.desktopinterface.kimpanel-ibus-panel")

int main(int argc, char *argv[])
{
    ibus_init();
    // we don't need im in this.
    qputenv("QT_IM_MODULE", "compose");
    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    App app(argc, argv);
    Sonic::Logging::install();

    return app.exec();
}
