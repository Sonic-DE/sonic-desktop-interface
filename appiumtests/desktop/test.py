#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

# pylint: disable=too-many-arguments

# For FreeBSD CI which only has python 3.9
from __future__ import annotations

import os
import subprocess
import sys
import tempfile
import threading
import time
import unittest
from typing import Final

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from desktoptest import name_has_owner, start_plasmashell
from gi.repository import Gio, GLib

KDE_VERSION: Final = 6
PLASMASHELL_SERVICE_NAME: Final = "org.kde.plasmashell"

class Bug482267Test(unittest.TestCase):
    """
    Startup slow with Picture of the day
    """

    temp_home: tempfile.TemporaryDirectory

    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    session_bus: Gio.DBusConnection

    @classmethod
    def setUpClass(cls) -> None:
        # create a throw-away XDG home, so the test starts with a clean slate
        # with every run, and doesn't mess with your local installation
        cls.temp_home = tempfile.TemporaryDirectory()
        cls.addClassCleanup(cls.temp_home.cleanup)
        os.environ["XDG_CACHE_HOME"] = os.path.join(cls.temp_home.name, ".cache")
        os.makedirs(os.environ["XDG_CACHE_HOME"])
        os.environ["XDG_CONFIG_HOME"] = os.path.join(cls.temp_home.name, ".config")
        os.makedirs(os.environ["XDG_CONFIG_HOME"])
        os.environ["XDG_DATA_HOME"] = os.path.join(cls.temp_home.name, ".local", "share")
        os.makedirs(os.environ["XDG_DATA_HOME"])
        os.environ["XDG_STATE_HOME"] = os.path.join(cls.temp_home.name, ".local", "state")
        os.makedirs(os.environ["XDG_STATE_HOME"])

        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()
        cls.session_bus = Gio.bus_get_sync(Gio.BusType.SESSION)

    @classmethod
    def tearDownClass(cls) -> None:
        if cls.plasmashell is not None:
            subprocess.check_output([f"kquitapp{KDE_VERSION}", "plasmashell"], stderr=sys.stderr)
            cls.plasmashell.wait(5)
        if cls.kded is not None:
            cls.kded.terminate()
            cls.kded.wait(5)
        if cls.kactivitymanagerd is not None:
            cls.kactivitymanagerd.terminate()
            cls.kactivitymanagerd.wait(5)

    def test_0_wait_until_ready(self) -> None:
        """
        Start plasmashell and wait until the DBus interface is ready
        """
        if not name_has_owner(self.session_bus, PLASMASHELL_SERVICE_NAME):
            for _ in range(10):
                if name_has_owner(self.session_bus, PLASMASHELL_SERVICE_NAME):
                    return
                print("waiting for plasmashell to appear on the DBus session")
                time.sleep(1)
            self.fail("plasmashell does not appear on DBus after 10 secs")
        time.sleep(5)

if __name__ == '__main__':
    if "KDECI_BUILD" not in os.environ:
        sys.exit(0)
    unittest.main()
