# // krazy:excludeall=copyright,license
find_package(KF6I18n REQUIRED)

target_sources(touchpad_common PRIVATE libinputcommon.cpp)

if (BUILD_KCM_TOUCHPAD_X11)
    # Separate static lib for the X11 backend .cpp files. Keeping them out of
    # touchpad_common's AUTOMOC unit prevents X11 system headers (Xlib/XCB)
    # from being pulled into touchpad_common's mocs_compilation.cpp, which
    # would break moc output that uses 5-arg SignalData constructor calls
    # (inherited via `using` from FunctionData without default arguments).
    add_library(touchpad_x11_backends STATIC
        x11/propertyinfo.cpp
        x11/xlibbackend.cpp
        x11/xlibtouchpad.cpp
        x11/xcbatom.cpp
        x11/xlibnotifications.cpp
        x11/xrecordkeyboardmonitor.cpp
        x11/libinputtouchpad.cpp
    )
    if (TARGET XOrg::Synaptics)
        target_sources(touchpad_x11_backends PRIVATE x11/synapticstouchpad.cpp)
        target_link_libraries(touchpad_x11_backends PRIVATE XOrg::Synaptics)
    endif()
    set_property(TARGET touchpad_x11_backends PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_include_directories(touchpad_x11_backends PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/kcms/touchpad
    )

    target_link_libraries(touchpad_x11_backends
        PUBLIC
            touchpad_common
        PRIVATE
            Qt::GuiPrivate
            XCB::ATOM
            XCB::RECORD
            XCB::RECORD
            X11::X11
            X11::Xi
            X11::XCB
            PkgConfig::XORGSERVER
            PkgConfig::XORGLIBINPUT
            KF6::I18n
    )

    target_link_libraries(touchpad_common PRIVATE touchpad_x11_backends)

    add_executable(kcm-touchpad-list-devices x11/listdevices.cpp)
    target_link_libraries(kcm-touchpad-list-devices
        X11::X11
        X11::Xi
    )
    install(TARGETS kcm-touchpad-list-devices
            DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
    )
endif()
