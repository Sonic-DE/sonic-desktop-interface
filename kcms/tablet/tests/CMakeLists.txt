ecm_add_test(
        TEST_NAME tst_calibrationtool
        tst_calibrationtool.cpp ../calibrationtool.cpp sonic_logging_component.cpp)
target_link_libraries(tst_calibrationtool PRIVATE Qt::Test KF6::ConfigCore Canberra::Canberra kwindevices)
