set(kcm_users_SRCS
    kcm.cpp
    user.cpp
    usermodel.cpp
    fprintdevice.cpp
    fingerprintmodel.cpp
)

set_source_files_properties(org.freedesktop.Accounts.xml
                        PROPERTIES NO_NAMESPACE TRUE)

set_source_files_properties(org.freedesktop.Accounts.User.xml
                        PROPERTIES NO_NAMESPACE TRUE)

set_source_files_properties(net.reactivated.Fprint.Device.xml
                        PROPERTIES NO_NAMESPACE TRUE)

set_source_files_properties(net.reactivated.Fprint.Manager.xml
                        PROPERTIES NO_NAMESPACE TRUE)
                        
qt5_add_dbus_interface(kcm_users_SRCS
    org.freedesktop.Accounts.xml
    accounts_interface
)

qt5_add_dbus_interface(kcm_users_SRCS
    org.freedesktop.Accounts.User.xml
    user_interface
)

qt5_add_dbus_interface(kcm_users_SRCS
    net.reactivated.Fprint.Device.xml
    fprint_device_interface
)

qt5_add_dbus_interface(kcm_users_SRCS
    net.reactivated.Fprint.Manager.xml
    fprint_manager_interface
)

set(login1_manager_xml org.freedesktop.login1.Manager.xml)
set_source_files_properties(${login1_manager_xml} PROPERTIES INCLUDE "usersessions.h")
qt5_add_dbus_interface(kcm_users_SRCS
    ${login1_manager_xml}
    login1_interface
)

ecm_qt_declare_logging_category(kcm_users_SRCS
    HEADER kcmusers_debug.h
    IDENTIFIER KCMUSERS
    CATEGORY_NAME org.kde.kcm_users
    EXPORT KCMUSERS
    DESCRIPTION "System Settings - Users"
)

ecm_qt_install_logging_categories(
    EXPORT KCMUSERS
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

add_library(kcm_users MODULE ${kcm_users_SRCS})

target_link_libraries(kcm_users
    Qt5::Quick
    Qt5::QuickWidgets
    Qt5::Concurrent
    Qt5::DBus
    KF5::CoreAddons
    KF5::Declarative
    KF5::I18n
    KF5::QuickAddons
    KF5::AuthCore
    crypt
)

kcoreaddons_desktop_to_json(kcm_users "../kcm_users.desktop" SERVICE_TYPES kcmodule.desktop)

### KAuth actions

set(kcmusersfprinthelper_SRCS
    fprintdevice.cpp
    fprinthelper.cpp
)

qt5_add_dbus_interface(kcmusersfprinthelper_SRCS
    net.reactivated.Fprint.Device.xml
    fprint_device_interface
)

qt5_add_dbus_interface(kcmusersfprinthelper_SRCS
    net.reactivated.Fprint.Manager.xml
    fprint_manager_interface
)

add_executable(kcmusersfprinthelper ${kcmusersfprinthelper_SRCS})
target_link_libraries(kcmusersfprinthelper 
    KF5::AuthCore 
    KF5::CoreAddons 
    Qt5::DBus)

install(TARGETS kcmusersfprinthelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
kauth_install_helper_files(kcmusersfprinthelper org.kde.kcontrol.kcmusers.fingerprint root)

kauth_install_actions(org.kde.kcontrol.kcmusers.fingerprint kcmusers_actions.actions)

#####

install(TARGETS kcm_users DESTINATION ${PLUGIN_INSTALL_DIR}/kcms)
