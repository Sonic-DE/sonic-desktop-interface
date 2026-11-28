// SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// The tests link the touchpad backends, which log through Sonic Logging; they
// declare a component but install no handler, so QtTest keeps its own.

#include <Sonic/Logging/Log>

SONIC_LOGGING_COMPONENT("org.sonicde.desktopinterface.touchpad.tests")
