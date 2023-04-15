add_definitions(-DTRANSLATION_DOMAIN=\"kcm_activities\")

set (KAMD_KCM_SRCS
   kcm_activities.cpp
   activityconfig.cpp
)

qt_add_dbus_interface (KAMD_KCM_SRCS
   common/dbus/org.kde.ActivityManager.Features.xml
   features_interface
)

kcmutils_add_qml_kcm(kcm_activities SOURCES ${KAMD_KCM_SRCS})

target_link_libraries (kcm_activities
   Qt::Quick
   Qt::Core
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
