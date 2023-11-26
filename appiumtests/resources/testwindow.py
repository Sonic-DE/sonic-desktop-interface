#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import os
import stat
from typing import Final, Self

import gi

gi.require_version('GdkPixbuf', '2.0')
gi.require_version('Gtk', '4.0')
from gi.repository import GdkPixbuf, GLib, Gtk


class DesktopFileWrapper(object):

    APPLICATION_ID: Final = "org.kde.testwindow"

    def __init__(self) -> None:
        self.path: str = os.path.join(GLib.get_user_data_dir(), "applications", f"{self.APPLICATION_ID}.desktop")
        self.application_path: str = os.path.realpath(__file__)
        self.icon_path: str = os.path.join(os.path.join(GLib.get_user_data_dir(), "icons"), "test.png")

    def __enter__(self) -> Self:
        os.makedirs(os.path.join(GLib.get_user_data_dir(), "applications"))
        os.makedirs(os.path.join(GLib.get_user_data_dir(), "icons"))

        pixbuf = GdkPixbuf.Pixbuf.new(GdkPixbuf.Colorspace.RGB, True, 8, 16, 16)
        pixbuf.fill(0xff0000ff)
        assert pixbuf.savev(self.icon_path, "png")

        with open(self.path, "w", encoding="utf-8") as file_handler:
            file_handler.writelines([
                "[Desktop Entry]\n",
                "Type=Application\n",
                f"Icon={self.icon_path}\n",
                "Name=Software Center\n",
                f"Exec={self.application_path}\n",
            ])
            file_handler.flush()
        os.chmod(self.path, os.stat(self.path).st_mode | stat.S_IEXEC)

        return self

    def __exit__(self, *args) -> None:
        os.remove(self.path)
        os.remove(self.icon_path)


class TestWindow(Gtk.ApplicationWindow):

    def __init__(self, _app: Gtk.Application) -> None:
        super().__init__(application=_app, title="Test Window")

        self.button = Gtk.Button(label="Install Software")
        self.button.connect("clicked", self.on_button_clicked)
        self.set_child(self.button)

        GLib.timeout_add_seconds(60, self.close)

    def on_button_clicked(self, widget) -> None:
        self.close()


def on_activate(_app: Gtk.Application) -> None:
    win = TestWindow(_app)
    win.present()


if __name__ == "__main__":
    app = Gtk.Application(application_id=DesktopFileWrapper.APPLICATION_ID)
    app.connect('activate', on_activate)
    app.run(None)
