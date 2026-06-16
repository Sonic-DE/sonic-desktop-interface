set_source_files_properties(LayoutManager.js PROPERTIES QT_QML_SKIP_QMLDIR_ENTRY TRUE)

plasma_add_applet(org.kde.panel
    QML_SOURCES
        AppletContainer.qml
        ConfigOverlay.qml
        main.qml
        LayoutManager.js
    CPP_SOURCES
        ${CMAKE_SOURCE_DIR}/messagelogger.cpp
    RESOURCES
        main.xml
    GENERATE_CONTAINMENT_CLASS
)
