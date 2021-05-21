/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

//#include "plugins.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include "kcm/touchpadconfigcontainer.h"
#include "kded/kded.h"

K_PLUGIN_FACTORY(TouchpadPluginFactory, registerPlugin<TouchpadDisabler>(); registerPlugin<TouchpadConfigContainer>("kcm");)
#include <plugins.moc>
// K_EXPORT_PLUGIN(TouchpadPluginFactory(buildAboutData()))
