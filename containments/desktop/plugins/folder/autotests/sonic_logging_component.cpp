// SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// The tests compile the folder plugin's sources, which log with sonicDebug(); they
// declare a component but install no handler, so QtTest keeps its own.

#include <Sonic/Logging/Log>

SONIC_LOGGING_COMPONENT("org.sonicde.desktopinterface.folder.tests")
