// SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
//
// SPDX-License-Identifier: GPL-2.0-or-later

// The test compiles the lid detector, which logs with sonicWarning(); it declares
// a component but installs no handler, so QtTest keeps its own.

#include <Sonic/Logging/Log>

SONIC_LOGGING_COMPONENT("org.sonicde.desktopinterface.kickoff.tests")
