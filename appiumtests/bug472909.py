#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import unittest

from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_plasmashell
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait


class Bug472909Test(unittest.TestCase):
    """
    Kickoff is taking focus away after being closed with meta key
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None

    @classmethod
    def setUpClass(cls) -> None:
        cls.kactivitymanagerd, cls.kded, cls.plasmashell = start_plasmashell()

        options = AppiumOptions()
        options.set_capability("app", "Root")
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_bug472909_#{self.id()}.png")

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        subprocess.check_output(["kquitapp6", "plasmashell"], stderr=sys.stderr)
        if cls.kded:
            cls.kded.kill()
        if cls.kactivitymanagerd:
            cls.kactivitymanagerd.kill()
        cls.driver.quit()

    def test_bug472909(self) -> None:
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Application Launcher")))

        # Start active indicator window
        test_window = subprocess.Popen([os.path.join(os.path.dirname(os.path.abspath(__file__)), "resources", "bug472909_activewindow.py")], stdout=sys.stderr, stderr=sys.stderr)
        self.addCleanup(test_window.terminate)

        # Press Meta key twice to test focus restoration
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Active Window")))
        actions = ActionChains(self.driver)
        actions.send_keys(Keys.META).perform()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Inactive Window")))
        actions.send_keys(Keys.META).perform()
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Active Window")))


if __name__ == '__main__':
    unittest.main()
