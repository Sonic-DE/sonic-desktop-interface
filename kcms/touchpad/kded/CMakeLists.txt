kcoreaddons_add_plugin(kded_touchpad SOURCES kded.cpp INSTALL_NAMESPACE kf6/kded)
target_include_directories(kded_touchpad PRIVATE ${CMAKE_SOURCE_DIR}/kcms/touchpad)

target_link_libraries(kded_touchpad
    PRIVATE
    Qt::DBus
    KF6::CoreAddons
    KF6::DBusAddons
    touchpad_common
    touchpad_x11_backends
)
