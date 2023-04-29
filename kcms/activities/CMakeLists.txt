add_definitions(-DTRANSLATION_DOMAIN=\"kcm_activities\")

set(KAMD_KCM_DATADIR ${KDE_INSTALL_DATADIR}/kactivitymanagerd/workspace/settings/)
if(NOT IS_ABSOLUTE "${KDE_INSTALL_DATADIR}")
   set (KAMD_KCM_DATADIR "${CMAKE_INSTALL_PREFIX}/${KAMD_KCM_DATADIR}")
endif ()

configure_file(kactivities-kcm-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kactivities-kcm-features.h)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}) # utils folder

set (KAMD_KCM_SRCS
   kcm_activities.cpp
   dialog.cpp
)

qt_add_dbus_interface (KAMD_KCM_SRCS
   common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
)

kcoreaddons_add_plugin(kcm_activities SOURCES ${KAMD_KCM_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")
kcmutils_generate_desktop_file(kcm_activities)

target_link_libraries (kcm_activities
   Qt::Quick
   Qt::QuickWidgets
   Qt::Core
   Qt::Sql
   Qt::Qml
   Qt::DBus
   KF6::CoreAddons
   KF6::KCMUtils
   KF6::I18n
   KF6::Service
   KF6::XmlGui
   KF6::GlobalAccel
   KF6::Activities
)
