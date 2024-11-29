#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import unittest
from typing import Final

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.webelement import WebElement
from gi.repository import Gio, GLib
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))
from desktoptest import start_kded

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_keys"


class KCMTest(unittest.TestCase):
    """
    Tests for kcm_keys
    """

    driver: webdriver.Remote
    kded: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        cls.kded = start_kded()

        options = AppiumOptions()
        options.set_capability("app", f"kcmshell{KDE_VERSION} {KCM_ID}")
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "0",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;qt.qml.typeresolution.cycle.warning=false;qt.qpa.wayland.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false",
        })
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{KCM_ID}_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        cls.driver.find_element(AppiumBy.XPATH, "//*[@name='Cancel' and contains(@accessibility-id, 'Button')]").click()
        if cls.kded is not None:
            subprocess.check_output([f"kquitapp{KDE_VERSION}", f"kded{KDE_VERSION}"], stderr=sys.stderr)
            cls.kded.wait()
        cls.driver.quit()

    def test_0_open(self) -> None:
        self.driver.find_element(AppiumBy.NAME, "Select an item from the list to view its shortcuts here")
        self.driver.get_screenshot_as_file(f"appium_artifact_{KCM_ID}.png")
        print(self.driver.page_source, file=sys.stderr)


if __name__ == '__main__':
    assert os.getenv("USE_CUSTOM_BUS") == "1"
    unittest.main()
