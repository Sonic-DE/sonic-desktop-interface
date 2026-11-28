# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DTRANSLATION_DOMAIN="kcm_gamecontroller")

kcmutils_add_qml_kcm(kcm_gamecontroller INSTALL_NAMESPACE "plasma/kcms/systemsettings")

target_sources(kcm_gamecontroller PRIVATE
    devicemodel.cpp
    device.cpp
    axesmodel.cpp
    axesproxymodel.cpp
    buttonmodel.cpp
    hatmodel.cpp
    kcm.cpp
    gamecontrollermoduledata.cpp
    calibrationcontroller.cpp
    calibrationcontroller.h
    ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
)
sonic_logging_plugin(kcm_gamecontroller org.sonicde.desktopinterface.gamecontroller)

kconfig_target_kcfg_file(kcm_gamecontroller
    FILE gamecontrolleremulationsettings.kcfg
    CLASS_NAME GameControllerEmulationSettings
    MUTATORS
)

target_link_libraries(kcm_gamecontroller PRIVATE
    KF6::CoreAddons
    KF6::KCMUtils
    KF6::KCMUtilsQuick
    KF6::I18n
    Qt6::Quick
    Qt6::QuickWidgets
    SDL2::SDL2
    Qt::DBus
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" AND UDev_FOUND)
    target_sources(kcm_gamecontroller PRIVATE udevmatcher.cpp udevmatcher.h)
    target_link_libraries(kcm_gamecontroller PRIVATE UDev::UDev)
    target_compile_definitions(kcm_gamecontroller PRIVATE HAVE_UDEV=1)
else()
    target_compile_definitions(kcm_gamecontroller PRIVATE HAVE_UDEV=0)
endif()

add_subdirectory(kded)

if (BUILD_TESTING AND CMAKE_SYSTEM_NAME STREQUAL "Linux" AND UDev_FOUND)
    add_subdirectory(tests)
endif()
