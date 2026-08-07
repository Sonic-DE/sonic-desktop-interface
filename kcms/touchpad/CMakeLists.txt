# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_touchpad\")

if (BUILD_KCM_TOUCHPAD_X11)
    if (NOT XORGLIBINPUT_FOUND AND NOT TARGET XOrg::Synaptics)
        list(APPEND MISSING_DEPS "xorg-libinput or Synaptics property headers")
    endif()
    if (NOT X11_XCB_FOUND)
        list(APPEND MISSING_DEPS "X11_XCB")
    endif()
    if (NOT X11_Xi_FOUND)
        list(APPEND MISSING_DEPS "X11_Xi")
    endif()
    if (NOT XORGSERVER_FOUND)
        list(APPEND MISSING_DEPS "xorg-server")
    endif()
    if (MISSING_DEPS)
        message(FATAL_ERROR "Missing X11 dependencies for kcm_touchpad: ${MISSING_DEPS}. Install dependencies or set CMake option -DBUILD_KCM_TOUCHPAD_X11=OFF.")
    endif()
endif()

add_subdirectory(icon)

qt_generate_dbus_interface(kded/kded.h org.kde.touchpad.xml)
qt_add_dbus_interfaces(TOUCHPAD_DBUS_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.touchpad.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
)

add_library(touchpad_common STATIC
    touchpadbackend.cpp
    logging.cpp
    ${TOUCHPAD_DBUS_SRCS}
)
set_property(TARGET touchpad_common PROPERTY POSITION_INDEPENDENT_CODE ON)
# touchpadbackend.h is private to touchpad_common. Consumers (kcm_touchpad,
# kded_touchpad) must not see it, so AUTOMOC for those targets does not scan
# it. This prevents duplicate moc_touchpadbackend.cpp generation across targets.
target_include_directories(touchpad_common PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(touchpad_common
    PUBLIC
    Qt::Core
    KF6::XmlGui
    PRIVATE
    KF6::ConfigCore
    KF6::WindowSystem
    KF6::I18n
)
if (BUILD_KCM_TOUCHPAD_X11)
    # touchpadbackend.cpp includes xlibbackend.h under the X11 guard, which
    # pulls in X11 system headers. Link the X11 deps privately so they
    # don't propagate to consumers.
    target_link_libraries(touchpad_common
        PRIVATE
            Qt::GuiPrivate
            X11::X11
            X11::Xi
            X11::XCB
            PkgConfig::XORGSERVER
    )
    if (XORGLIBINPUT_FOUND)
        target_link_libraries(touchpad_common PRIVATE PkgConfig::XORGLIBINPUT)
    endif()
endif()

add_subdirectory(backends)
add_subdirectory(kded)

kcmutils_add_qml_kcm(kcm_touchpad)
target_sources(kcm_touchpad PRIVATE kcm.cpp touchpadmoduledata.cpp)

target_link_libraries(kcm_touchpad
    PRIVATE
    Qt::Core
    Qt::DBus
    Qt::Qml
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtils
    KF6::KCMUtilsQuick
    Plasma::Plasma
    KF6::WindowSystem
    touchpad_common
    touchpad_x11_backends
)

add_custom_command(TARGET kcm_touchpad POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink "../kcms/systemsettings/kcm_touchpad.so" "kcm_touchpad_init.so")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kcm_touchpad_init.so DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcminit)

install(FILES org.kde.touchpadshortcuts.desktop DESTINATION ${KDE_INSTALL_DATADIR}/kglobalaccel)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
