// SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// Test binaries compile daemon sources that log with sonicDebug(); they declare
// a component but install no handler, so QtTest keeps its own.

#include <Sonic/Logging/Log>

SONIC_LOGGING_COMPONENT("org.sonicde.desktopinterface.keyboard.tests")
