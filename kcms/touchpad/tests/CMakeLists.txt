ecm_add_test(
    touchpadclassifier_test.cpp
    TEST_NAME touchpadclassifier_test
    LINK_LIBRARIES touchpad_x11_backends Qt::Test
)
target_include_directories(touchpadclassifier_test PRIVATE
    ${CMAKE_SOURCE_DIR}/kcms/touchpad/backends/x11
    ${CMAKE_SOURCE_DIR}/kcms/touchpad/backends
    ${CMAKE_SOURCE_DIR}/kcms/touchpad
)
if (UDev_FOUND)
    target_compile_definitions(touchpadclassifier_test PRIVATE HAVE_UDEV=1)
else()
    target_compile_definitions(touchpadclassifier_test PRIVATE HAVE_UDEV=0)
endif()

# SimpleKCM smoke test using kcmshell6
# This test requires xvfb-run and the xcb platform
find_program(XVFB_RUN_EXECUTABLE xvfb-run)
if(XVFB_RUN_EXECUTABLE)
    add_test(
        NAME kcm_touchpad_smoketest
        COMMAND ${CMAKE_COMMAND} -E env
            QT_PLUGIN_PATH=${CMAKE_BINARY_DIR}/bin
            QT_QPA_PLATFORM=xcb
            ${XVFB_RUN_EXECUTABLE} -a
            kcmshell6 --smoke-test kcm_touchpad
    )
else()
    message(STATUS "xvfb-run not found, skipping kcm_touchpad_smoketest")
endif()
