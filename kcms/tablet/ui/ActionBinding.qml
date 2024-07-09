/*
    SPDX-FileCopyrightText: 2024 Joshua Goins <joshua.goins@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.plasma.tablet.kcm
import org.kde.kcmutils
import org.kde.kquickcontrols

QQC2.Button {
    id: root

    property InputSequence inputSequence

    signal gotInputSequence(sequence: InputSequence)

    icon.name: {
        switch (inputSequence.type) {
            case InputSequence.Disabled:
                return "action-unavailable-symbolic";
            case InputSequence.Keyboard:
                return "input-keyboard-symbolic";
            case InputSequence.Mouse:
                return "input-mouse-symbolic";
            case InputSequence.ApplicationDefined:
                return "applications-all-symbolic";
        }
    }
    text: inputSequence.toString()

    function moveUpSequence(sequence: InputSequence): void {
        root.gotInputSequence(sequence);
        actionDialog.gotInputSequence.disconnect(moveUpSequence);
    }

    onClicked: {
        actionDialog.inputSequence = inputSequence;
        actionDialog.gotInputSequence.connect(moveUpSequence);
        actionDialog.open();
        actionDialog.refreshDialogData();
    }
}