project (KCMActivities)
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_activities5\")

find_package (Qt${QT_MAJOR_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Sql Qml DBus)
find_package (KF6KCMUtils ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6XmlGui ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6GlobalAccel ${KF6_MIN_VERSION} CONFIG REQUIRED)
find_package (KF6Activities ${KF6_MIN_VERSION} CONFIG REQUIRED)

set (KAMD_KCM_DATADIR
   ${KDE_INSTALL_DATADIR}/kpackage/kcms/kcm_activities/
)
if (NOT IS_ABSOLUTE "${KDE_INSTALL_DATADIR}")
   set (KAMD_KCM_DATADIR "${CMAKE_INSTALL_PREFIX}/${KAMD_KCM_DATADIR}")
endif ()

configure_file (kactivities-kcm-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/kactivities-kcm-features.h)

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

kpackage_install_package(package kcm_activities kcms)


