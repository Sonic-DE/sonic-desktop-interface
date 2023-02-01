# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_joystick\")

set(kcm_joystick2_SRCS
  kcm.cpp
  devicemodel.cpp
  joydevice.cpp
)

kcoreaddons_add_plugin(kcm_joystick2 SOURCES ${kcm_joystick2_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_joystick2)

target_link_libraries(kcm_joystick2
  KF5::KIOWidgets
  KF5::CoreAddons
  KF5::ConfigWidgets
  KF5::KCMUtils
  KF5::QuickAddons
  KF5::I18n
  KF5::Plasma
  KF5::PlasmaQuick
  KF5::NewStuff
  KF5::Package

  Qt::DBus
  Qt::Widgets
  Qt::QuickWidgets
)

kpackage_install_package(package kcm_joystick2 kcms)
