# SPDX-FileCopyrightText: 2026 Sebastian Sauer <dipesh@gmx.de>
# SPDX-FileCopyrightText: 2026 Joseph Crowell <joseph.w.crowell@gmail.com>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

ecm_add_qml_module(org.sonicde.dwellclicker_ipc
    URI "org.sonicde.dwellclicker.ipc"
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES
        QtCore
)

target_sources(org.sonicde.dwellclicker_ipc PRIVATE
    ipc/DwellClickerSocket.cpp
    ipc/DwellClickerSocket.h
    ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
)
sonic_logging_plugin(org.sonicde.dwellclicker_ipc org.sonicde.desktopinterface.dwellclicker)

# The generated QML type registrations include the header by its bare name.
target_include_directories(org.sonicde.dwellclicker_ipc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ipc)

target_link_libraries(org.sonicde.dwellclicker_ipc
    PRIVATE
        Qt::Qml
        SonicWin::DwellClickerIpc
)

ecm_finalize_qml_module(org.sonicde.dwellclicker_ipc)

plasma_add_applet(org.kde.plasma.dwellclicker
    QML_SOURCES
        main.qml
    GENERATE_APPLET_CLASS
)

add_dependencies(org.kde.plasma.dwellclicker org.sonicde.dwellclicker_ipc)
