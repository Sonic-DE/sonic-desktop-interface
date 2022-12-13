#!/usr/bin/env python3

# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

import unittest
from appium import webdriver
from appium.webdriver.common.appiumby import AppiumBy
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.ui import WebDriverWait
import time
from selenium.webdriver.support.events import EventFiringWebDriver, AbstractEventListener


class KickoffTests(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        desired_caps = {}
        desired_caps["app"] = "plasmawindowed org.kde.plasma.kickoff"
        desired_caps["timeouts"] = {'implicit': 10000}
        self.driver = webdriver.Remote(
            command_executor='http://127.0.0.1:4723',
            desired_capabilities=desired_caps)


    @classmethod
    def tearDownClass(self):
        self.driver.quit()


    def setUp(self):
        self.driver.press_keycode(Keys.ESCAPE);


    def tearDown(self):
        if not self._outcome.result.wasSuccessful():
            self.driver.get_screenshot_as_file("failed_test_shot_{}.png".format(self.id()))


    def test_categories(self):
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | All Applications]").click()


    # broke until this lands: https://invent.kde.org/plasma/kwin/-/merge_requests/3300
    # def test_search_and_open(self):
    #     self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
    #     self.driver.find_element(by=AppiumBy.NAME, value="Search").send_keys("Info Center")
    #     self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[list item | Info Center]").click()
    #     WebDriverWait(self.driver, 10).until(lambda x: self.driver.query_app_state('org.kde.kinfocenter.desktop') == 4)
    #     self.assertEqual(self.driver.query_app_state('org.kde.kinfocenter.desktop'), 4)
    #     self.driver.terminate_app('org.kde.kinfocenter.desktop')


    def test_keyboard_navigation(self):
        self.driver.find_element(by=AppiumBy.CLASS_NAME, value="[push button | Application Launcher]").click()
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        first_favorite = focused_elements[0].id
        self.assertIn("KickoffGridDelegate", focused_elements[0].get_attribute('accessibility-id'))

        self.driver.press_keycode(Keys.RIGHT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_favorite, focused_elements[0].id)

        self.driver.press_keycode(Keys.LEFT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertEqual(first_favorite, focused_elements[0].id)

        self.driver.press_keycode(Keys.LEFT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_favorite, focused_elements[0].id)
        favorites_category = focused_elements[0].id

        self.driver.press_keycode(Keys.DOWN)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(favorites_category, focused_elements[0].id)

        self.driver.press_keycode(Keys.RIGHT)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(favorites_category, focused_elements[0].id)
        self.assertNotEqual(first_favorite, focused_elements[0].id)
        self.assertNotIn("KickoffGridDelegate", focused_elements[0].get_attribute('accessibility-id'))
        first_all_app = focused_elements[0].id

        self.driver.press_keycode(Keys.DOWN)
        focused_elements = self.driver.find_elements(by=AppiumBy.XPATH, value="//list_item[contains(@states, 'focused')]")
        self.assertEqual(len(focused_elements), 1)
        self.assertNotEqual(first_all_app, focused_elements[0].id)

        self.driver.press_keycode(Keys.TAB)
        self.assertEqual(True, self.driver.find_element(by=AppiumBy.NAME, value="Applications").is_selected())


if __name__ == '__main__':
    unittest.main()
