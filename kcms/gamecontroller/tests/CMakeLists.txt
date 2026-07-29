ecm_add_test(
    modeltest.cpp
    ../axesmodel.cpp
    ../buttonmodel.cpp
    ../hatmodel.cpp
    ../device.cpp
    ../udevmatcher.cpp
    ../logging.cpp
    TEST_NAME gamecontroller_modeltest
)

target_include_directories(gamecontroller_modeltest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_BINARY_DIR}/..
)
target_compile_definitions(gamecontroller_modeltest PRIVATE HAVE_UDEV=1 TRANSLATION_DOMAIN="kcm_gamecontroller")
target_link_libraries(gamecontroller_modeltest PRIVATE
    Qt::Test
    Qt::Gui
    KF6::I18n
    SDL2::SDL2
    UDev::UDev
)
