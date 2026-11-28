add_definitions(-DTRANSLATION_DOMAIN=\"kcm_activities\")

kcmutils_add_qml_kcm(kcm_activities
   SOURCES
      kcm_activities.cpp
      activityconfig.cpp
      ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
)
sonic_logging_plugin(kcm_activities org.sonicde.desktopinterface.activities)

target_link_libraries (kcm_activities PRIVATE
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
   Plasma::Activities
)
