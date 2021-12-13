/*
    SPDX-FileCopyrightText: 2017 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.private.volume 0.1

Item {
    id: audioStreamIconBox
    property int streamDuration: 0

    opacity: 0
    visible: opacity > 0

    HoverHandler {
        id: hoverHandler
        onHoveredChanged: if (hovered) {
            
        } else {
            
        }
    }
    TapHandler {
        id: tapHandler
        onSingleTapped: toggleMuted()
    }
    WheelHandler {
        id: wheelHandler
    }
    DragHandler {
        id: dragHandler
        target: null
    }

    PlasmaCore.Dialog {
        property point taskPoint: task.mapToGlobal(task.x, task.y)
        x: taskPoint.x - (false ? width + PlasmaCore.Units.smallSpacing * 2: -PlasmaCore.Units.smallSpacing * 2)
        y: taskPoint.y - (true ? height + PlasmaCore.Units.smallSpacing * 2 : -PlasmaCore.Units.smallSpacing * 2)
        visible: hoverHandler.hovered
        location: PlasmaCore.Types.Floating
        mainItem: Row {
            spacing: PlasmaCore.Units.smallSpacing
            PlasmaCore.IconItem {
                implicitHeight: PlasmaCore.Units.iconSizes.small
                implicitWidth: implicitHeight
                source: "audio-volume-high-symbolic" //temp
            }
            PC3.Slider {
                id: popupVolumeSlider
                from: PulseAudio.MinimalVolume
                to: PulseAudio.NormalVolume
                stepSize: to / (to / PulseAudio.NormalVolume * 100.0)
            }
            PC3.Label {
                text: `${popupVolumeSlider.value}${popupVolumeSlider.locale.percent}`
            }
            PC3.ToolButton {
                icon.name: "open-menu-symbolic"
                text: "More options"
                display: PC3.AbstractButton.IconOnly
            }
        }
//             flags: notificationItem.replying || focusListener.wantsFocus ? 0 : Qt.WindowDoesNotAcceptFocus
    }

    PlasmaCore.FrameSvgItem {
        z: -1
        anchors.fill: audioStreamIcon
        visible: opacity > 0
        imagePath: "widgets/button"
        prefix: ["toolbutton-hover", "normal"]
        opacity: hoverHandler.hovered
        Behavior on opacity {
            enabled: hoverHandler.hovered
            OpacityAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        z: -1
        anchors.fill: audioStreamIcon
        visible: opacity > 0
        imagePath: "widgets/button"
        prefix: ["toolbutton-pressed", "pressed"]
        opacity: tapHandler.pressed
        Behavior on opacity {
            enabled: tapHandler.pressed
            OpacityAnimator {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    PlasmaCore.Svg {
        id: audioSvg
        imagePath: "icons/audio"
    }

    PlasmaCore.SvgItem {
        id: audioStreamIcon

        // Need audio indicator twice, to keep iconBox in the center.
        readonly property var requiredSpace: Math.min(iconBox.width, iconBox.height)
                                             + Math.min(Math.min(iconBox.width, iconBox.height), PlasmaCore.Units.iconSizes.smallMedium) * 2
        svg: audioSvg
        smooth: false

        height: Math.round(Math.min(parent.height * indicatorScale, PlasmaCore.Units.iconSizes.smallMedium))
        width: height

        anchors {
            verticalCenter: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }

        states: [
            State {
                name: "verticalIconsOnly"
                when: tasks.vertical && frame.width < audioStreamIcon.requiredSpace

                PropertyChanges {
                    target: audioStreamIconLoader
                    anchors.rightMargin: Math.round(taskFrame.margins.right * indicatorScale)
                }
            },

            State {
                name: "horizontal"
                when: frame.width > audioStreamIcon.requiredSpace

                AnchorChanges {
                    target: audioStreamIconLoader

                    anchors.top: undefined
                    anchors.verticalCenter: frame.verticalCenter
                }

                PropertyChanges {
                    target: audioStreamIconLoader
                    width: PlasmaCore.Units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), PlasmaCore.Units.iconSizes.smallMedium))
                }

                PropertyChanges {
                    target: audioStreamIcon

                    height: parent.height
                    width: parent.width
                }
            },

            State {
                name: "vertical"
                when: frame.height > audioStreamIcon.requiredSpace

                AnchorChanges {
                    target: audioStreamIconLoader

                    anchors.right: undefined
                    anchors.horizontalCenter: frame.horizontalCenter
                }

                PropertyChanges {
                    target: audioStreamIconLoader

                    anchors.topMargin: taskFrame.margins.top
                    width: PlasmaCore.Units.roundToIconSize(Math.min(Math.min(iconBox.width, iconBox.height), PlasmaCore.Units.iconSizes.smallMedium))
                }

                PropertyChanges {
                    target: audioStreamIcon

                    height: parent.height
                    width: parent.width
                }
            }
        ]
    }

    // Using States rather than a simple Behavior we can apply different transitions,
    // which allows us to delay showing the icon but hide it instantly still.
    states: [
        State {
            name: "playing"
            when: task.playingAudio && !task.muted
            PropertyChanges {
                target: audioStreamIconBox
                opacity: 1
            }
            PropertyChanges {
                target: audioStreamIcon
                elementId: "audio-volume-high"
            }
        },
        State {
            name: "muted"
            when: task.muted
            PropertyChanges {
                target: audioStreamIconBox
                opacity: 1
            }
            PropertyChanges {
                target: audioStreamIcon
                elementId: "audio-volume-muted"
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to: "playing"
            SequentialAnimation {
                // Delay showing the play indicator so we don't flash it for brief sounds.
                PauseAnimation {
                    duration: !task.delayAudioStreamIndicator || inPopup ? 0 : 2000
                }
                NumberAnimation {
                    property: "opacity"
                    duration: PlasmaCore.Units.longDuration
                }
            }
        },
        Transition {
            from: ""
            to: "muted"
            NumberAnimation {
                property: "opacity"
                duration: PlasmaCore.Units.longDuration
            }
        },
        Transition {
            to: ""
            SequentialAnimation {
                // Delay hiding the play indicator so we don't avoid showing the icon for looped brief sounds
                PauseAnimation {
                    duration: 50
                }
                NumberAnimation {
                    property: "opacity"
                    duration: PlasmaCore.Units.longDuration
                }
            }
        }
    ]
}
