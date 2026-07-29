kcoreaddons_add_plugin(kded_gamecontroller_calibration
    SOURCES calibrationapplier.cpp
    INSTALL_NAMESPACE kf6/kded
)

target_link_libraries(kded_gamecontroller_calibration PRIVATE
    Qt::Core
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::DBusAddons
)
