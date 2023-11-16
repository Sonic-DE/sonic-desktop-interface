#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import subprocess
import sys
import tempfile
import time
import unittest
from typing import Final

import gi
from appium import webdriver
from appium.options.common.base import AppiumOptions
from appium.webdriver.common.appiumby import AppiumBy
from desktoptest import start_kactivitymanagerd, start_kded
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.support.ui import WebDriverWait

gi.require_version('Gdk', '3.0')
from gi.repository import GdkPixbuf, GLib


def prepare_test_image(folder_path: str) -> None:
    """
    Creates an image file
    """
    bits_per_sample: Final = 8
    width = height = 16
    pixbuf = GdkPixbuf.Pixbuf.new(GdkPixbuf.Colorspace.RGB, True, bits_per_sample, width, height)
    pixbuf.fill(0xff0000ff)
    assert pixbuf.savev(os.path.join(folder_path, "test.png"), "png")


class FolderViewTest(unittest.TestCase):
    """
    Tests for the desktop folder view layout
    """

    driver: webdriver.Remote
    kactivitymanagerd: subprocess.Popen | None = None
    kded: subprocess.Popen | None = None
    plasmashell: subprocess.Popen | None = None
    desktop_dir: str

    @classmethod
    def setUpClass(cls) -> None:
        """
        Initializes the webdriver
        """
        cls.desktop_dir = GLib.get_user_special_dir(GLib.UserDirectory.DIRECTORY_DESKTOP)
        if not os.path.exists(cls.desktop_dir):
            os.mkdir(cls.desktop_dir)
        prepare_test_image(cls.desktop_dir)

        cls.kactivitymanagerd = start_kactivitymanagerd()
        cls.kded = start_kded()

        options = AppiumOptions()
        options.set_capability("app", "plasmashell -p org.kde.plasma.desktop --no-respawn")
        options.set_capability("timeouts", {'implicit': 30000})
        cls.driver = webdriver.Remote(command_executor='http://127.0.0.1:4723', options=options)

    def tearDown(self) -> None:
        """
        Take screenshot when the current test fails
        """
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file(f"failed_test_shot_folderview_#{self.id()}.png")

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

    def test_0_folderview_ready(self) -> None:
        """
        Waits until the folder view is ready
        """
        wait = WebDriverWait(self.driver, 30)
        wait.until(EC.presence_of_element_located((AppiumBy.NAME, "test.png")))

    def test_1_folderview_image_preview(self) -> None:
        """
        test.png is filled with red color
        """
        time.sleep(3)  # Make sure the desktop is ready

        with tempfile.TemporaryDirectory() as temp_dir:
            # Take desktop screenshot
            saved_image_path: str = os.path.join(temp_dir, "desktop.png")
            self.assertTrue(self.driver.get_screenshot_as_file(saved_image_path))

            pixbuf: GdkPixbuf.Pixbuf | None = GdkPixbuf.Pixbuf.new_from_file(saved_image_path)
            self.assertIsNotNone(pixbuf)
            self.assertGreater(pixbuf.get_width(), 1)
            self.assertGreater(pixbuf.get_height(), 1)
            self.assertEqual(pixbuf.get_n_channels(), 4)  # R, G, B, A

            pixel_data: bytes = pixbuf.get_pixels()  # R, G, B, A, R, G, B, A, ...

        # Now scan each pixels to match the expected colors
        i = 0
        count = 0
        while i < len(pixel_data):
            color = (pixel_data[i], pixel_data[i + 1], pixel_data[i + 2])  # 0~255
            i += 4
            if color == (255, 0, 0):  # Red
                count += 1
                if count >= 256:
                    return

        self.fail("Image preview is not available")


if __name__ == '__main__':
    unittest.main()
