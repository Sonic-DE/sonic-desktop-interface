#CMakeLists.txt
# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>

include(ECMConfiguredInstall)

########### Build localegen helper ############
set(stylus_Helper_SRCS
    stylushelper.cpp
)
qt_generate_dbus_interface(
    ${CMAKE_CURRENT_SOURCE_DIR}/stylushelper.h
    org.kde.stylushelper.xml
    OPTIONS -s -m
)
qt_add_dbus_adaptor(stylus_Helper_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.stylushelper.xml
                    ${CMAKE_CURRENT_SOURCE_DIR}/stylushelper.h StylusHelper)
add_executable(plasma-stylus-helper ${stylus_Helper_SRCS})
target_link_libraries(plasma-stylus-helper
    Qt::DBus
    PolkitQt${QT_MAJOR_VERSION}-1::Core
    KF5::I18n
    udev
    input
)
ecm_install_configured_files(INPUT org.kde.stylushelper.service.in DESTINATION ${KDE_INSTALL_DBUSSYSTEMSERVICEDIR} @ONLY)
install(TARGETS plasma-stylus-helper ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

option(USE_CMAKE_PREFIX "Use CMAKE_INSTALL_PREFIX instead of `/usr` for DBus and PolicyKit config files" ON)
set(POLICY_FILES_INSTALL_DIR "/usr")
if(USE_CMAKE_PREFIX)
    if(NOT ${CMAKE_INSTALL_PREFIX} STREQUAL POLICY_FILES_INSTALL_DIR)
        MESSAGE(WARNING "INSTALL_BROKEN_POLICY_FILES is enabled. The following files will be installed to ${CMAKE_INSTALL_PREFIX} instead of ${POLICY_FILES_INSTALL_DIR}.")
    endif()
    set(POLICY_FILES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

install(FILES org.kde.stylushelper.conf DESTINATION "${POLICY_FILES_INSTALL_DIR}/share/dbus-1/system.d")
install(FILES org.kde.stylushelper.policy DESTINATION "${POLICY_FILES_INSTALL_DIR}/share/polkit-1/actions")
