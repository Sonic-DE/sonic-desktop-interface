# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_kded\")

set(kcm_kded_SRCS
    kcmkded.cpp
    modulesmodel.cpp
    filterproxymodel.cpp
    kdedconfigdata.cpp
    ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
)

qt_add_dbus_interface(kcm_kded_SRCS ${KDED_DBUS_INTERFACE} kded_interface)

kcmutils_add_qml_kcm(kcm_kded SOURCES ${kcm_kded_SRCS})
target_link_libraries(kcm_kded PRIVATE KF6::KCMUtilsQuick KF6::I18n Qt::DBus Qt::Qml)
sonic_logging_plugin(kcm_kded org.sonicde.desktopinterface.kded)
