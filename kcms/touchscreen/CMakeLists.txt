add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchscreen\")

kcmutils_add_qml_kcm(kcm_touchscreen SOURCES kcmtouchscreen.cpp touchscreenmoduledata.cpp ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp)
sonic_logging_plugin(kcm_touchscreen org.sonicde.desktopinterface.touchscreen)

target_link_libraries(kcm_touchscreen PRIVATE
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::I18n
    KF6::KCMUtilsQuick

    Qt::Gui
    Qt::DBus
    Qt::Qml
    kwindevices
)
