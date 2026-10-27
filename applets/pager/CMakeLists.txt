add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.pager\")
plasma_add_applet(org.kde.plasma.pager
    QML_SOURCES
        qml/main.qml
        qml/config.qml
        qml/configGeneral.qml
    CPP_SOURCES
        pagermodel.cpp
        windowmodel.cpp
        ${CMAKE_SOURCE_DIR}/sonic_logging_autoinstall.cpp
    RESOURCES
        main.xml
    GENERATE_APPLET_CLASS
    QML_ARGS
        NO_LINT # https://qt-project.atlassian.net/browse/QTBUG-145572
)
sonic_logging_plugin(org.kde.plasma.pager org.sonicde.desktopinterface.pager)

target_link_libraries(org.kde.plasma.pager PRIVATE
    KF6::WindowSystem
    Plasma::Plasma
    Plasma::Activities
    PW::LibTaskManager
    Qt::Core
    Qt::DBus
    Qt::Qml
    Qt::Quick
)
