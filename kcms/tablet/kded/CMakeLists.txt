# SPDX-FileCopyrightText: 2025 KDE Contributors
# SPDX-License-Identifier: BSD-2-Clause

kcoreaddons_add_plugin(kded_tabletdaemon
    SOURCES
        tabletdaemon.cpp
        tabletdaemon.h
    INSTALL_NAMESPACE kf6/kded
)
set_target_properties(kded_tabletdaemon PROPERTIES OUTPUT_NAME tabletdaemon)

target_link_libraries(kded_tabletdaemon
    Qt::DBus
    Qt::Gui
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::GlobalAccel
    KF6::I18n
    # kwindevices: Static library providing InputDevice wrapper for KWin's D-Bus input device interface.
    # Defined in kcms/libkwindevices/CMakeLists.txt and shared across multiple KCMs (mouse, touchpad, tablet).
    kwindevices
)
