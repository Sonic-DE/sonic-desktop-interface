add_definitions(-DTRANSLATION_DOMAIN=\"attica_kde_plugin\")

add_library(attica_kde MODULE kdeplatformdependent.cpp)
target_include_directories(attica_kde PRIVATE "${ACCOUNTSQT_INCLUDE_DIRS}")
target_link_libraries(attica_kde
    PUBLIC
        Qt::Widgets
        Qt::Xml
        KF6::KIOWidgets
        KF6::KCMUtils
        KF6::Attica
        KF6::I18n
        KF6::Service
        KAccounts6
        ${ACCOUNTSQT_LIBRARIES}
    PRIVATE
        Sonic::Logging
)

install(TARGETS attica_kde                 DESTINATION ${KDE_INSTALL_PLUGINDIR})
