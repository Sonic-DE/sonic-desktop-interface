if (BUILD_KCM_MOUSE_X11)
    target_sources(kcm_mouse
        PUBLIC
            x11/x11_libinput_backend.cpp
            x11/x11_libinput_dummydevice.cpp
            x11/libinput_settings.cpp
    )

    target_link_libraries(kcm_mouse
        PUBLIC
            X11::X11
            X11::Xi
            X11::Xcursor
            Qt::GuiPrivate
    )
    if (TARGET XOrg::Evdev)
        target_link_libraries(kcm_mouse PRIVATE XOrg::Evdev)
    endif()
endif()
