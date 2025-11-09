#!/usr/bin/env python3

# SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
# SPDX-License-Identifier: MIT

import sys

from PySide6.QtWidgets import QApplication, QMainWindow, QPushButton
from PySide6.QtCore import QTimer, QEvent


class TestWindow(QMainWindow):

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self.setWindowTitle("Test Window")

        self.button = QPushButton("Active Window")
        self.button.clicked.connect(self.on_button_clicked)
        self.setCentralWidget(self.button)
        self.installEventFilter(self)
        QTimer.singleShot(60000, self.close)

    def on_button_clicked(self) -> None:
        self.close()

    def eventFilter(self, watched, event):
        if watched == self:
            if event.type() == QEvent.WindowActivate:
                self.button.setText("Active Window")
            elif event.type() == QEvent.WindowDeactivate:
                self.button.setText("Inactive Window")
        return super().eventFilter(watched, event)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = TestWindow()
    win.show()
    sys.exit(app.exec())
