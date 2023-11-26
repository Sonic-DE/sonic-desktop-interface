#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import base64
import os
import subprocess
import sys
import tempfile
import time
import unittest
from typing import Final

import cv2 as cv
import numpy as np
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

    def take_screenshot(self) -> str:
        time.sleep(3)
        with tempfile.TemporaryDirectory() as temp_dir:
            saved_image_path: str = os.path.join(temp_dir, "kcm_window.png")
            self.assertTrue(self.driver.get_screenshot_as_file(saved_image_path))

            cv_first_image = cv.imread(saved_image_path, cv.IMREAD_COLOR)
            return base64.b64encode(cv.imencode('.png', cv_first_image)[1].tobytes()).decode()

    def test_1_window(self) -> None:
        """
        Can list running windows and show badge counts
        """
        with DesktopFileWrapper() as wrapper:
            os.environ["GDK_BACKEND"] = "wayland"
            os.environ["WAYLAND_DEBUG"] = "1"

            time.sleep(3)  # Make sure KSycoca perceives the desktop file

            test_window = subprocess.Popen([wrapper.application_path], stdout=sys.stderr, stderr=sys.stderr)
            assert test_window.poll() is None
            self.addCleanup(test_window.terminate)

            # Wait until the window appears in the widget
            wait = WebDriverWait(self.driver, 30)
            wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Test Window")))

            # Match icon part
            cv_second_image = np.zeros((16, 16, 3), dtype=np.uint8)
            cv_second_image[:, :] = [0, 0, 255]
            second_image = base64.b64encode(cv.imencode('.png', cv_second_image)[1].tobytes()).decode()
            self.driver.find_image_occurrence(self.take_screenshot(), second_image)

            # Set badge count and match again
            changed_properties = GLib.Variant('a{sv}', {
                "count": GLib.Variant('x', 123),
                "count-visible": GLib.Variant('b', True),
            })
            session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
            session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{wrapper.APPLICATION_ID}.desktop"), changed_properties))
            badge_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "123")))
            self.driver.find_image_occurrence(self.take_screenshot(), second_image)
            # Hide the badge count
            changed_properties = GLib.Variant('a{sv}', {
                "count-visible": GLib.Variant('b', False),
            })
            session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{wrapper.APPLICATION_ID}.desktop"), changed_properties))
            wait.until_not(lambda _: badge_element.is_displayed())


if __name__ == '__main__':

    unittest.main()
