# SPDX-FileCopyrightText: 2025 Nicolas Fella <nicolas.fella@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

plasma_add_applet(org.kde.plasma.windowlist
    QML_SOURCES
        ConfigGeneral.qml
        config.qml
        main.qml
        MenuButton.qml
    CPP_SOURCES
        ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)
sonic_logging_plugin(org.kde.plasma.windowlist org.sonicde.desktopinterface.windowlist)
