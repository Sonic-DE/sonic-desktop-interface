# Sonic Desktop Interface Release Notes

Sonic Desktop Interface is the heart of the SonicDE desktop: the panel, application launcher, task manager, desktop icons, lock screen look, and the settings pages for your keyboard, mouse, touchpad, tablet and more.

## Unreleased

- Keyboard layout switching is more dependable: switching between layouts that type the same characters (such as US and Australian English) no longer gets stuck, and your first layout change now takes effect right away even on a brand-new account.
- The keyboard layout switcher and keyboard settings now talk to the keyboard service through SonicDE's new fast, robust communication framework.
- The desktop's widgets and tools now record their logs through SonicDE's new unified logging system instead of a separate log file.
- The desktop and its shell integration now consistently identify themselves as SonicDE, with SonicDE names throughout.
- The lock screen keeps your picture visible and centers your picture and password field above the on-screen keyboard when it is open.
- Game controller settings now detect connected gamepads more reliably and show only the devices that matter.
- Fixed a possible crash when the mouse settings finish applying your cursor theme.
- Behind the scenes: new automated checks for applying mouse cursor themes, and the accessibility helper and keyboard preview use QXLibre for X11 when it's installed, going back to Qt's own X11 support when it isn't. Panels can also leave their background out behind part of a widget.

## 6.8.0.3 (2026-10-25)

- Fixed a possible crash when the mouse settings finish applying your cursor theme.
- Behind the scenes: new automated checks for applying mouse cursor themes.

## 6.8.0.2 (2026-10-23)

- Game controller settings now detect connected gamepads more reliably and show only the devices that matter.

## 6.8.0.1 (2026-10-21)

- The lock screen keeps your picture visible and centers your picture and password field above the on-screen keyboard when it is open.

## 6.8.0 (2026-10-21)

- After a wrong password, the lock screen now briefly disables the password field and unlock button while the system's security delay runs, so nothing you type in the meantime is lost or misread.
- The password field stays visible on the lock screen when the on-screen keyboard is open.
- The lock screen now shares the same footer as the rest of SonicDE's sign-in screens for a consistent look.
- Window previews in the task manager now use the native X11 thumbnails, removing a dependency on screen-casting services.
- Tap-to-click and touchpad button mappings now work as configured.
- Behind the scenes: lock screen password handling moved to the screen locker itself, build fixes for systems without the Synaptics touchpad driver, code cleanups and the 6.8.0 release update.

## 6.7.5.4 (2026-10-12)

- Tap-to-click and touchpad button mappings now work as configured.

## 6.7.5.3 (2026-10-08)

- Behind the scenes: code cleanups in the desktop's background services.

## 6.7.5.2 (2026-09-25)

- After a wrong password, the lock screen now briefly disables the password field and unlock button while the system's security delay runs, so nothing you type in the meantime is lost or misread.

## 6.7.5.1 (2026-09-14)

- Window previews in the task manager now use the native X11 thumbnails, removing a dependency on screen-casting services.

## 6.7.5 (2026-09-12)

- SonicDE no longer needs the old Plasma and Qt 5 compatibility libraries, making installs leaner; as part of this, the launcher's Places page now shows a laptop icon on laptops.
- Behind the scenes: the 6.7.5 release update.

## 6.7.4.2 (2026-08-19)

- Desktop icons now keep their positions when you create new files or folders on an unsorted desktop, along with other icon placement fixes.

## 6.7.4.1 (2026-08-15)

- Grouped task previews in the task manager now close properly when you move the mouse away, and you can press Escape to close them.
- Behind the scenes: new automated checks for grouped task previews.

## 6.7.4 (2026-08-04)

- Behind the scenes: fixed a missing build requirement for the mouse settings.

## 6.7.3.5 (2026-08-08)

- Touchpad settings now find your touchpad reliably and load without display glitches.

## 6.7.3.4 (2026-08-02)

- SonicDE logos now replace the old KDE logos throughout the desktop.

## 6.7.3.3 (2026-08-01)

- The new Silver theme is now the default look of the desktop, replacing Breeze.
- The SDDM login theme has been removed, as SonicDE now ships its own login manager.
- Behind the scenes: code formatting now follows SonicDE's shared settings.

## 6.7.3.2 (2026-07-30)

- Desktop log files now live in your personal state folder and are readable only by you, keeping them private.

## 6.7.3.1 (2026-07-30)

- Game controller calibration is back, and mouse and touchpad settings once again support devices that don't use the libinput driver.
- Fixed a crash when closing the search plugin installer.

## 6.7.3 (2026-07-14)

- Behind the scenes: code formatting rules and developer editor support.

## 6.7.2 (2026-06-30)

No SonicDE-specific changes in this release; it brings the package up to date with upstream.

## 6.7.1.1 (2026-06-27)

- Behind the scenes: a build fix for the game controller settings.

## 6.7.1 (2026-06-23)

No SonicDE-specific changes in this release; it brings the package up to date with upstream.

## 6.7.0 (2026-06-20)

- Behind the scenes: a build fix for the desktop icons view and developer housekeeping.

## 6.6.5.3 (2026-06-18)

- The keyboard layout switcher and its settings page now show crisp flag icons instead of image files or plain country codes.
- The keyboard layout switcher now works on systems without systemd, and reacts more quickly to changes made in the keyboard settings.
- Logs from the desktop's widgets and settings pages are now all collected in one place.
- Behind the scenes: more detailed logging on development builds.

## 6.6.5.2 (2026-06-15)

- Fixed panel widgets not responding to some clicks and key presses.
- Fixed an input problem in the application launcher.
- Behind the scenes: development builds keep detailed logs without noisy display-system messages.

## 6.6.5.1 (2026-06-13)

- Desktop icons no longer jump to a different screen on multi-monitor setups.
- The desktop now has its own logging system, so problems can be diagnosed even without systemd.
- The keyboard settings no longer show a false "not found" warning.
- Behind the scenes: a large modernization of the build setup, removal of leftover Wayland code, build fixes and new automated checks for desktop icons and drag and drop across screens.

## 6.6.5 (2026-05-12)

No SonicDE-specific changes in this release; it brings the package up to date with upstream.

## 6.6.4.1 (2026-04-08)

- The Discover software center has been removed from the default panel, widgets and file handling.
- Behind the scenes: a small correction in the application launcher.

## 6.6.4 (2026-04-07)

No SonicDE-specific changes in this release; it brings the package up to date with upstream.

## 6.6.3.1 (2026-03-17)

- Graphics tablet settings now work on X11 with the XLibre Wacom driver, and no longer leak memory.
- The desktop no longer depends on Wayland at all.
- Desktop icons use memory more carefully: icon positions, previews and folder information are kept within sensible limits, and leftover resources are cleaned up properly.
- Behind the scenes: compatibility updates for newer Qt versions, better error handling, build fixes and code cleanups.

## 6.6.3 (2026-03-17)

No SonicDE-specific changes in this release; it brings the package up to date with upstream.

## 6.6.2.6 (2026-03-17)

No SonicDE-specific changes in this release; it brings the package up to date with upstream.

## 6.6.2.5 (2026-01-22)

- Desktop icons use memory more carefully: icon positions, previews and folder information are kept within sensible limits, and leftover resources are cleaned up properly.
- Behind the scenes: removal of leftover Wayland checks.

## 6.6.2.4 (2026-03-10)

- Graphics tablet settings now work on X11 with the XLibre Wacom driver.
- The desktop no longer depends on Wayland at all.

## 6.6.2.3 (2026-03-08)

- Graphics tablet settings no longer leak memory.
- Behind the scenes: compatibility updates for newer Qt versions, better error handling and code cleanups.

## 6.6.2.2 (2026-03-06)

- Wayland-only settings and widgets have been removed, keeping the desktop focused on X11.
- Behind the scenes: build fixes, README updates pointing to SonicDE and code cleanups.

## 6.6.2.1 (2026-03-02)

- Settings pages are now always built for X11, with Wayland support removed.
- Behind the scenes: a build fix for the touchpad settings.

## 6.6.2 (2026-03-02)

- Behind the scenes: README updates pointing to SonicDE, developer editor support and a build fix.

## 6.6.1 (2026-03-01)

- Behind the scenes: README updates pointing to SonicDE and developer editor support.
