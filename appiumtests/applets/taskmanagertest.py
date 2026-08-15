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
from appium.webdriver.webelement import WebElement
from gi.repository import Gio, GLib
from selenium.common.exceptions import NoSuchElementException
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

sys.path.append(os.path.join(os.path.dirname(os.path.abspath(__file__)), os.pardir, "desktop"))
from desktoptest import (DesktopFileWrapper, start_kactivitymanagerd)

WIDGET_ID: Final = "org.kde.plasma.taskmanager"
KDE_VERSION: Final = 6

# The four options for "Clicking grouped task" in the Behavior config tab,
# in the same order they appear in the ComboBox (ConfigBehavior.qml:83-88).
GROUPED_TASK_OPTIONS: Final = [
    "Cycles through tasks",
    "Shows small window previews",
    "Shows large window previews",
    "Shows textual list",
]


class Bug487023Test(unittest.TestCase):
    """
    Tests for the task manager widget
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    wrapper: DesktopFileWrapper

    @classmethod
    def setUpClass(cls) -> None:
        """
        Opens the widget and initialize the webdriver
        """
        cls.wrapper = DesktopFileWrapper()
        cls.wrapper.create()
        cls.kactivitymanagerd = start_kactivitymanagerd()

        options = AppiumOptions()
        options.set_capability("environ", {
            "LC_ALL": "en_US.UTF-8",
            "QT_FATAL_WARNINGS": "0",
            "QT_LOGGING_RULES": "qt.accessibility.atspi.warning=false;kf.plasma.core.warning=false;kf.windowsystem.warning=false;kf.kirigami.platform.warning=false;org.kde.plasma.notificationmanager.warning=false;org.kde.plasma.taskmanager.debug=true",
        })
        options.set_capability("app", f"plasmawindowed -p org.kde.plasma.desktop {WIDGET_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

        os.environ["GDK_BACKEND"] = "wayland"
        os.environ["GSK_RENDERER"] = "cairo"
        os.environ["AT_SPI_BUS_ADDRESS"] = "org:foo=bar"  # Disable atspi for the test window

    def tearDown(self) -> None:
        """
        Take screenshot and dump page source when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_{WIDGET_ID}_#{self.id()}.png")
            print(self.driver.page_source, file=sys.stderr)

    @classmethod
    def tearDownClass(cls) -> None:
        """
        Make sure to terminate the driver again, lest it dangles.
        """
        subprocess.check_call([f"kquitapp{KDE_VERSION}", "plasmawindowed"])
        for _ in range(10):
            try:
                subprocess.check_call(["pidof", "plasmawindowed"])
            except subprocess.CalledProcessError:
                break
            time.sleep(1)
        cls.driver.quit()
        if cls.kactivitymanagerd is not None:
            cls.kactivitymanagerd.terminate()
            cls.kactivitymanagerd.wait()

    def test_0_open(self) -> None:
        """
        Open the widget
        """
        self.driver.find_element(AppiumBy.NAME, "Icons-and-Text Task Manager")

    def test_1_badge_count(self) -> None:
        """
        Can list running windows and show badge counts based on app id
        """
        test_window = subprocess.Popen([self.wrapper.application_path], stdout=sys.stderr, stderr=sys.stderr)
        assert test_window.poll() is None
        self.addCleanup(test_window.kill)

        # Wait until the window appears in the widget
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Test Window")))

        # Set badge count and match again
        changed_properties = GLib.Variant('a{sv}', {
            "count": GLib.Variant('x', 123),
            "count-visible": GLib.Variant('b', True),
        })
        session_bus: Gio.DBusConnection = Gio.bus_get_sync(Gio.BusType.SESSION)
        session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{self.wrapper.APPLICATION_ID}.desktop"), changed_properties))
        badge_element = wait.until(EC.presence_of_element_located((AppiumBy.NAME, "123")))

        # Hide the badge count
        changed_properties = GLib.Variant('a{sv}', {
            "count-visible": GLib.Variant('b', False),
        })
        session_bus.emit_signal(None, "/com/canonical/unity/launcherentry/1", "com.canonical.Unity.LauncherEntry", "Update", GLib.Variant.new_tuple(GLib.Variant('s', f"application://{self.wrapper.APPLICATION_ID}.desktop"), changed_properties))
        wait.until_not(lambda _: badge_element.is_displayed())

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _set_grouped_task_visualization(self, wait: WebDriverWait, option_name: str) -> None:
        """
        Opens the config dialog, selects the named grouped-task visualization
        option, unchecks "Group only when full" so the group is always combined,
        and applies the change.  Uses a bounded number of combobox clicks
        rather than a hard-coded count so it is robust regardless of the
        option that was previously selected.
        """
        assert option_name in GROUPED_TASK_OPTIONS, f"unknown option {option_name!r}"

        subprocess.check_call(["plasmawindowed", "--config"])
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "Behavior"))).click()

        # The combobox accessible name is the text of the currently selected
        # item.  Find it by trying each known option name.
        combobox: WebElement | None = None
        for current_name in GROUPED_TASK_OPTIONS:
            try:
                combobox = self.driver.find_element(AppiumBy.NAME, current_name)
                break
            except NoSuchElementException:
                pass
        assert combobox is not None, "could not find the grouped-task combobox"
        combobox.click()

        # Cycle through options until the desired one is shown, bounded by
        # the total number of options so we never loop forever.
        for _ in range(len(GROUPED_TASK_OPTIONS)):
            try:
                wait.until(EC.presence_of_element_located((AppiumBy.NAME, option_name)))
                break
            except Exception:
                combobox.click()
        else:
            self.fail(f"could not select {option_name!r} in the grouped-task combobox")

        # Uncheck "Group only when the Task Manager is full" so grouped tasks
        # always combine into a single button regardless of panel fullness.
        try:
            group_only_checkbox = self.driver.find_element(AppiumBy.NAME, "Group only when the Task Manager is full")
            if group_only_checkbox.is_selected():
                group_only_checkbox.click()
        except NoSuchElementException:
            # icontasks variant hides this checkbox; that is fine.
            pass

        self.driver.find_element(AppiumBy.NAME, "OK").click()
        wait.until_not(lambda _: combobox.is_displayed())

    def _start_grouped_windows(self, wait: WebDriverWait) -> None:
        """
        Starts two test windows so they group into a single task button,
        and waits for the grouped task to appear.
        """
        window_1 = subprocess.Popen(["python3", self.wrapper.application_path])
        window_2 = subprocess.Popen(["python3", self.wrapper.application_path])
        self.addCleanup(window_1.kill)
        self.addCleanup(window_2.kill)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, self.wrapper.APPLICATION_NAME)))

    def _open_group_popup(self, wait: WebDriverWait) -> None:
        """Clicks the grouped task to open its popup and waits for child entries."""
        self.driver.find_element(AppiumBy.NAME, self.wrapper.APPLICATION_NAME).click()
        wait.until(lambda _: len(self.driver.find_elements(AppiumBy.NAME, "Test Window")) >= 2)

    def _assert_popup_closed(self, wait: WebDriverWait) -> None:
        """Waits until no popup child entries are present."""
        wait.until(lambda _: len(self.driver.find_elements(AppiumBy.NAME, "Test Window")) == 0)

    # ------------------------------------------------------------------
    # Textual list (groupedTaskVisualization mode 3)
    # ------------------------------------------------------------------

    def test_2_bug487023_group_dialog(self) -> None:
        """
        Tests the group dialog can be opened and then closed with Escape.
        """
        wait = WebDriverWait(self.driver, 30)
        self._set_grouped_task_visualization(wait, "Shows textual list")
        self._start_grouped_windows(wait)

        # Open the textual list and verify two child tasks appear.
        self._open_group_popup(wait)
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)

    def test_3_textual_escape_closes(self) -> None:
        """
        Escape closes the textual group dialog.
        """
        wait = WebDriverWait(self.driver, 30)
        ActionChains(self.driver).send_keys(Keys.ESCAPE).perform()
        self._assert_popup_closed(wait)

    def test_4_textual_reopen_after_escape(self) -> None:
        """
        The textual group dialog can be reopened after Escape closed it.
        """
        wait = WebDriverWait(self.driver, 30)
        self._open_group_popup(wait)
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)

    def test_5_textual_click_toggle_closes(self) -> None:
        """
        Clicking the same grouped task again closes the textual dialog.
        """
        wait = WebDriverWait(self.driver, 30)
        self.driver.find_element(AppiumBy.NAME, self.wrapper.APPLICATION_NAME).click()
        self._assert_popup_closed(wait)

    def test_6_textual_reopen_after_click_toggle(self) -> None:
        """
        The textual group dialog can be reopened after click-toggle closed it.
        """
        wait = WebDriverWait(self.driver, 30)
        self._open_group_popup(wait)
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)

    def test_7_textual_hover_leave_closes(self) -> None:
        """
        Moving the pointer outside both the grouped task and the popup
        closes the textual dialog after the grace interval.
        """
        wait = WebDriverWait(self.driver, 30)
        # Move the pointer well outside the task manager and popup areas.
        ActionChains(self.driver).move_by_offset(500, 500).perform()
        self._assert_popup_closed(wait)

    # ------------------------------------------------------------------
    # Small window previews (groupedTaskVisualization mode 1)
    # ------------------------------------------------------------------

    def test_8_switch_to_preview_mode(self) -> None:
        """
        Switches grouped-task visualization to "Shows small window previews"
        and opens the grouped preview tooltip.
        """
        wait = WebDriverWait(self.driver, 30)
        self._set_grouped_task_visualization(wait, "Shows small window previews")

        # Open the grouped preview tooltip and verify two child entries appear.
        self._open_group_popup(wait)
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)

    def test_9_preview_escape_closes(self) -> None:
        """
        Escape closes the grouped preview tooltip.
        """
        wait = WebDriverWait(self.driver, 30)
        ActionChains(self.driver).send_keys(Keys.ESCAPE).perform()
        self._assert_popup_closed(wait)

    def test_10_preview_reopen_after_escape(self) -> None:
        """
        The grouped preview tooltip can be reopened after Escape closed it.
        """
        wait = WebDriverWait(self.driver, 30)
        self._open_group_popup(wait)
        self.assertEqual(len(self.driver.find_elements(AppiumBy.NAME, "Test Window")), 2)


if __name__ == '__main__':
    unittest.main()
