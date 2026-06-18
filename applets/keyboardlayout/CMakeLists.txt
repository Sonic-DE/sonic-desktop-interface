# SPDX-FileCopyrightText: 2025 Tobias Fella <tobias.fella@kde.org>
# SPDX-FileCopyrightText: 2026 Kristen McWilliam <kristen@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

# Shared IPC protocol library. The kded daemon and the applet's QML
# module both link against this so that IpcProtocol.cpp's QDataStream
# operators and keyboardIpcSocketPath() are defined exactly once.
add_library(keyboardlayout_ipc_protocol STATIC
    ipc/IpcProtocol.cpp
    ipc/IpcProtocol.h
)

target_include_directories(keyboardlayout_ipc_protocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/ipc
)

target_link_libraries(keyboardlayout_ipc_protocol
    PUBLIC
        Qt::Core
)

ecm_add_qml_module(org.kde.plasma.keyboardlayout_ipc
    URI "org.kde.plasma.keyboardlayout.ipc"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
        QtCore
        QtQuick
)

target_sources(org.kde.plasma.keyboardlayout_ipc PRIVATE
    ipc/KeyboardLayoutSocket.cpp
    ipc/KeyboardLayoutSocket.h
)

target_link_libraries(org.kde.plasma.keyboardlayout_ipc
    PUBLIC
        keyboardlayout_ipc_protocol
        Qt::Qml
        Qt::Network
)

ecm_qt_declare_logging_category(org.kde.plasma.keyboardlayout_ipc
    HEADER debug.h
    IDENTIFIER KCM_KEYBOARD
    CATEGORY_NAME org.kde.kcm_keyboard
)

ecm_finalize_qml_module(org.kde.plasma.keyboardlayout_ipc)

plasma_add_applet(org.kde.plasma.keyboardlayout
    QML_SOURCES
        qml/main.qml
        qml/config.qml
        qml/configGeneral.qml
    CPP_SOURCES
        ${CMAKE_SOURCE_DIR}/messagelogger.cpp
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
)

add_dependencies(org.kde.plasma.keyboardlayout org.kde.plasma.keyboardlayout_ipc)
