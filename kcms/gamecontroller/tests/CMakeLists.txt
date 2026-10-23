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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    ecm_add_test(discoverytest.cpp ../devicemodel.cpp ../gamecontrollermoduledata.cpp
        ../device.cpp ../udevmatcher.cpp ../logging.cpp
        TEST_NAME gamecontroller_discoverytest)
    target_include_directories(gamecontroller_discoverytest PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/..)
    target_compile_definitions(gamecontroller_discoverytest PRIVATE HAVE_UDEV=1 TRANSLATION_DOMAIN="kcm_gamecontroller")
    target_link_libraries(gamecontroller_discoverytest PRIVATE Qt::Test Qt::Gui KF6::I18n KF6::KCMUtilsCore SDL2::SDL2 UDev::UDev)
    # SDL virtual joysticks emulate physical controllers in this test only.
    # Production's virtual-device exclusion remains enabled and is tested too.
    target_link_options(gamecontroller_discoverytest PRIVATE
        "LINKER:--wrap=SDL_JoystickIsVirtual" "LINKER:--wrap=SDL_JoystickPath")
endif()
