#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import time
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from gi.repository import Gio, GLib
from resources.testwindow import DesktopFileWrapper
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

WIDGET_ID: Final = "org.kde.plasma.taskmanager"


class WidgetTest(unittest.TestCase):
    """
    Tests for the task manager widget
    """

    driver: webdriver.Remote

    @classmethod
    def setUpClass(cls) -> None:
        """
        Opens the widget and initialize the webdriver
        """
        options = AppiumOptions()
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.nano {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{WIDGET_ID}_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.quit()

    def test_1_badge_count(self) -> None:
        """
        Can list running windows and show badge counts
        """
        with DesktopFileWrapper() as wrapper:
            os.environ["GDK_BACKEND"] = "wayland"

            time.sleep(5)  # Make sure KSycoca perceives the desktop file

            test_window = subprocess.Popen([wrapper.application_path], stdout=sys.stderr, stderr=sys.stderr)
            assert test_window.poll() is None
            self.addCleanup(test_window.terminate)

            # Wait until the window appears in the widget
            wait = WebDriverWait(self.driver, 30)
            wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Test Window")))

            # Set badge count and match again
            changed_properties = GLib.Variant('a{sv}', {
                "count": GLib.Variant('x', 123),
                "count-visible": GLib.Variant('b', True),
            })
            session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
            session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{wrapper.APPLICATION_ID}.desktop"), changed_properties))
            badge_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "123")))

            # Hide the badge count
            changed_properties = GLib.Variant('a{sv}', {
                "count-visible": GLib.Variant('b', False),
            })
            session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{wrapper.APPLICATION_ID}.desktop"), changed_properties))
            wait.until_not(lambda _: badge_element.is_displayed())


if __name__ == '__main__':

    unittest.main()
