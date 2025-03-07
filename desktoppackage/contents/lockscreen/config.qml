import QtQuick 2.5
import QtQuick.Controls 2.5 as QQC2
import QtQuick.Layouts 1.1
import org.kde.kirigami 2.12 as Kirigami
import org.kde.kcmutils as KCM

Kirigami.FormLayout {
    id: configForm

    // TODO Plasma 7: Make this an enum.
    property bool cfg_alwaysShowClock
    property bool cfg_hideClockWhenIdle

    property alias cfg_showMediaControls: showMediaControls.checked
    property bool cfg_showMediaControlsDefault: false

    twinFormLayouts: parentLayout

    QQC2.RadioButton {
        id: clockAlwaysRadio
        Kirigami.FormData.label: i18ndc("plasma_shell_org.kde.plasma.desktop",
                                        "@title: group",
                                        "Show clock:")
        text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@option:radio Clock always shown", "Always")
        Accessible.name: i18nc("@option:radio", "Always show clock")
        checked: configForm.cfg_alwaysShowClock && !configForm.cfg_hideClockWhenIdle
        onToggled: {
            configForm.cfg_alwaysShowClock = true;
            configForm.cfg_hideClockWhenIdle = false;
        }

        KCM.SettingHighlighter {
            highlight: !clockAlwaysRadio.checked
        }
    }

    QQC2.RadioButton {
        text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@option:radio Clock shown only while unlock prompt is visible", "On unlocking prompt")
        Accessible.name: i18nc("@option:radio", "Show clock only on unlocking prompt")
        checked: configForm.cfg_alwaysShowClock && configForm.cfg_hideClockWhenIdle
        onToggled: {
            configForm.cfg_alwaysShowClock = true;
            configForm.cfg_hideClockWhenIdle = true;
        }
    }

    QQC2.RadioButton {
        text: i18ndc("plasma_shell_org.kde.plasma.desktop", "@option:radio Clock never shown", "Never")
        Accessible.name: i18nc("@option:radio", "Never show clock")
        checked: !configForm.cfg_alwaysShowClock
        onToggled: {
            configForm.cfg_alwaysShowClock = false;
        }
    }

    QQC2.CheckBox {
        id: showMediaControls
        Kirigami.FormData.label: i18ndc("plasma_shell_org.kde.plasma.desktop",
                                        "@title: group",
                                        "Media controls:")
        text: i18ndc("plasma_shell_org.kde.plasma.desktop",
                     "@option:check",
                     "Show under unlocking prompt")

        KCM.SettingHighlighter {
            highlight: cfg_showMediaControlsDefault != cfg_showMediaControls
        }
    }
}
