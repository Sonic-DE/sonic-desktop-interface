add_definitions(-DTRANSLATION_DOMAIN=\"kcm_tablet\")

kcmutils_add_qml_kcm(kcm_tablet INSTALL_NAMESPACE "plasma/kcms/systemsettings")

if (Qt6_VERSION VERSION_GREATER_EQUAL "6.8.0")
    set(private_code_option  "PRIVATE_CODE")
endif()

target_sources(kcm_tablet PRIVATE
    kcmtablet.cpp
    tabletevents.cpp
    calibrationtool.cpp
    inputsequence.cpp
    tabletsmodel.cpp
    tabletmoduledata.cpp
    ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
)
sonic_logging_plugin(kcm_tablet org.sonicde.desktopinterface.tablet)

target_link_libraries(kcm_tablet PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::GuiAddons
    KF6::I18n
    KF6::KCMUtils
    KF6::KCMUtilsQuick

    Qt::DBus
    kwindevices
    Canberra::Canberra
    PkgConfig::LIBWACOM

    Qt::GuiPrivate
    X11::X11
    X11::Xi
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

