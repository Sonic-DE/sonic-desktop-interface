set_source_files_properties(ActionMenu.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

plasma_add_applet(org.kde.plasma.kicker
    QML_SOURCES
    config.qml
    ActionMenu.qml
    CompactRepresentation.qml
    ConfigGeneral.qml
    DashboardRepresentation.qml
    HoverBlocker.qml
    ItemAbstractDelegate.qml
    ItemGridDelegate.qml
    ItemGridView.qml
    ItemListDelegate.qml
    ItemListDialog.qml
    ItemListView.qml
    ItemMultiGridView.qml
    MenuRepresentation.qml
    RunnerResultsList.qml
    SideBarItem.qml
    SideBarSection.qml
    main.qml

    RESOURCES
    main.xml

    CPP_SOURCES
        ${CMAKE_SOURCE_DIR}/messagelogger.cpp

    GENERATE_APPLET_CLASS
)

# Suppress warnings from auto-generated qmlcache files
target_compile_options(org.kde.plasma.kicker PRIVATE -Wno-unused-parameter)

set_source_files_properties(code/tools.js PROPERTIES QT_QML_SKIP_QMLDIR_ENTRY TRUE)
ecm_target_qml_sources(org.kde.plasma.kicker SOURCES code/tools.js PATH code)
