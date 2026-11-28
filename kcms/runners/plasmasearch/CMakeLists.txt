# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-3-Clause

kcmutils_add_qml_kcm(kcm_plasmasearch SOURCES kcm.cpp ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp)
sonic_logging_plugin(kcm_plasmasearch org.sonicde.desktopinterface.plasmasearch)
target_link_libraries(kcm_plasmasearch PRIVATE
  KF6::CoreAddons
  KF6::KCMUtils
  KF6::KCMUtilsQuick
  KF6::Runner
  KF6::I18n
  Qt::DBus
)
