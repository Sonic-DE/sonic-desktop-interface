add_executable(liddetectortest
    liddetectortest.cpp
    ../liddetector.cpp
    sonic_logging_component.cpp
)
target_include_directories(liddetectortest PRIVATE ..)
target_link_libraries(liddetectortest PRIVATE Qt::Core Qt::DBus Qt::Qml Qt::Test Sonic::Logging)
add_test(NAME liddetectortest COMMAND dbus-run-session $<TARGET_FILE:liddetectortest>)
