#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2021 Red Hat Inc.
# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

import os
import unittest
from typing import Final

import gi
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy

gi.require_version('UMockdev', '1.0')
from gi.repository import UMockdev

KDE_VERSION: Final = 6
KCM_ID: Final = "kcm_tablet"


class KCMTest(unittest.TestCase):
    """
    Tests for kcm_tablet
    """

    driver: webdriver.Remote
    testbed: UMockdev.Testbed

    @classmethod
    def setUpClass(cls) -> None:
        """
        Sets up umockdev and opens the KCM
        """
        assert UMockdev.in_mock_environment()
        cls.testbed = UMockdev.Testbed.new()
        parent = cls.testbed.add_device('hid', '/devices/pci0000:00/0000:00:14.0/usb3/3-10/3-10:1.2/0003:046D:C52B.0009/0003:046D:4101.000A', None, [], [])
        cls.testbed.add_device('input', '/devices/pci0000:00/0000:00:14.0/usb3/3-10/3-10:1.2/0003:046D:C52B.0009/0003:046D:4101.000A/input/input3', parent, ['name', 'Wacom Cintiq 24HD Pad'], ['DEVNAME', 'input/event3', 'ID_INPUT', '1', 'ID_INPUT_TABLET', '1', 'ID_INPUT_TABLET_PAD', '1', 'ID_VENDOR_ID', '0x56a', 'ID_MODEL_ID', '0x0f4', 'ID_INPUT_WIDTH_MM', '50', 'ID_INPUT_HEIGHT_MM', '40', 'PRODUCT', '3/56a/f4/100', 'LIBINPUT_DEVICE_GROUP', '3/56a/f4:usb-0000:00:14.0-5'])
        cls.testbed.add_device('input', '/devices/pci0000:00/0000:00:14.0/usb3/3-10/3-10:1.2/0003:046D:C52B.0009/0003:046D:4101.000A/input/input4', parent, ['name', 'Wacom Cintiq 24HD Pen'], ['DEVNAME', 'input/event4', 'ID_INPUT', '1', 'ID_INPUT_TABLET', '1', 'ID_VENDOR_ID', '0x56a', 'ID_MODEL_ID', '0x0f4', 'ID_INPUT_WIDTH_MM', '50', 'ID_INPUT_HEIGHT_MM', '40', 'PRODUCT', '3/56a/f4/100', 'LIBINPUT_DEVICE_GROUP', '3/56a/f4:usb-0000:00:14.0-5'])

        options = AppiumOptions()
        options.set_capability("app", f"kcmshell{KDE_VERSION} {KCM_ID}")
        options.set_capability("timeouts", {'implicit': 10000})
        options.set_capability("environ", {
            "UMOCKDEV_DIR": cls.testbed.get_root_dir(),
            "LD_PRELOAD": os.environ["LD_PRELOAD"],
        })
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
        cls.driver.quit()

    def test_0_open(self) -> None:
        self.driver.find_element(AppiumBy.NAME, "Wacom Cintiq 24HD Pad")


if __name__ == '__main__':
    unittest.main()
