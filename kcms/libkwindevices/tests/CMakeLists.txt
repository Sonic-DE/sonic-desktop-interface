ecm_add_test(
        TEST_NAME tst_inputdevice
        tst_inputdevice.cpp tst_inputdevice.cpp sonic_logging_component.cpp)
target_link_libraries(tst_inputdevice PRIVATE Qt::Test kwindevices)
