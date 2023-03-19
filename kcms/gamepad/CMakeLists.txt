# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_gamepad\")

set(kcm_gamepad_SRCS
        kcm.cpp
        devicemodel.cpp
        devicetypemodel.cpp
        gamepad.cpp
        gamepadbutton.cpp
        gamepadstick.cpp
        gamepadtrigger.cpp
        axesmodel.cpp
        buttonmodel.cpp
        )

kcoreaddons_add_plugin(kcm_gamepad SOURCES ${kcm_gamepad_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_gamepad)

target_link_libraries(kcm_gamepad
  KF6::CoreAddons
  KF6::ConfigWidgets
  KF6::KCMUtils
  KF6::KCMUtilsQuick
  KF6::I18n
  KF6::Plasma
  KF6::PlasmaQuick
  KF6::Solid

  Qt::Widgets
  Qt::Quick
  Qt::QuickWidgets

  udev
  evdev

  SDL2 # TODO: find_package
)

kpackage_install_package(package kcm_gamepad kcms)
