#!/usr/bin/env python3

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

import os
import subprocess
import sys
import time
import unittest
from appium import webdriver
from appium.webdriver.common.appiumby import AppiumBy
from appium.webdriver.applicationstate import ApplicationState
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.events import EventFiringWebDriver, AbstractEventListener


class EventListeners(AbstractEventListener):
    def __init__(self) -> None:
        super().__init__()
        self.i = 0


    def before_click(self, element, driver):
        driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.i))
        self.i += 1


    def after_click(self, element, driver):
        driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.i))
        self.i += 1


class KickoffTests(unittest.TestCase):
    LIBEXEC = None

    @classmethod
    def setUpClass(self):
        self.kactivitymanagerd = subprocess.Popen(["{}/kactivitymanagerd".format(KickoffTests.LIBEXEC)])

        desired_caps = {}
        desired_caps["app"] = "plasmawindowed org.kde.plasma.kickoff"
        desired_caps["timeouts"] = {'implicit': 10000}
        self.remote = webdriver.Remote(
            command_executor='http://127.0.0.1:4723',
            desired_capabilities=desired_caps)
        self.driver = EventFiringWebDriver(self.remote, EventListeners())


    @classmethod
    def tearDownClass(self):
        self.driver.quit()
        self.kactivitymanagerd.kill()
        self.kactivitymanagerd.wait()


    def setUp(self):
        self.driver.press_keycode(Keys.ESCAPE)


    def tearDown(self):
        self.driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.id()))


    def test_categories(self):
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | All Applications]").click()


    def test_search_and_open(self):
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        # Emoji Selector is the only actual application we install from workspace :|
        self.driver.find_element(by=AppiumBy.NAME, value="Search").send_keys("Emoji Selector")
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | Emoji Selector]").click()
        WebDriverWait(self.driver, 10).until(lambda x: self.driver.query_app_state('org.kde.plasma.emojier.desktop') == ApplicationState.RUNNING_IN_FOREGROUND)
        self.assertEqual(self.driver.query_app_state('org.kde.plasma.emojier.desktop'), ApplicationState.RUNNING_IN_FOREGROUND)
        self.driver.terminate_app('org.kde.plasma.emojier.desktop')


    def test_keyboard_navigation(self):
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        first_favorite = focused_elements[0].id
        self.assertIn("KickoffGridDelegate", focused_elements[0].get_attribute('accessibility-id'))

        # Go right to second favorite
        self.driver.press_keycode(Keys.RIGHT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_favorite, focused_elements[0].id)

        # Go left to first favorite again
        self.driver.press_keycode(Keys.LEFT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertEqual(first_favorite, focused_elements[0].id)

        # Go further left to the category list
        self.driver.press_keycode(Keys.LEFT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_favorite, focused_elements[0].id)
        favorites_category = focused_elements[0].id

        # Go down to the 'all apps' category
        self.driver.press_keycode(Keys.DOWN)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(favorites_category, focused_elements[0].id)

        # Go right to the first all app. This must not be a grid delegate anymore (favorites are griddelegates)
        self.driver.press_keycode(Keys.RIGHT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(favorites_category, focused_elements[0].id)
        self.assertNotEqual(first_favorite, focused_elements[0].id)
        self.assertNotIn("KickoffGridDelegate", focused_elements[0].get_attribute('accessibility-id'))
        first_all_app = focused_elements[0].id

        # Go down to second all app
        self.driver.press_keycode(Keys.DOWN)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_all_app, focused_elements[0].id)

        # Hitting Tab should get us to the "menu bar" at the bottom
        self.driver.press_keycode(Keys.TAB)
        self.assertEqual(True, self.driver.find_element(by=AppiumBy.NAME, value="Applications").is_selected())


if __name__ == '__main__':
    KickoffTests.LIBEXEC = sys.argv.pop()
    unittest.main()
