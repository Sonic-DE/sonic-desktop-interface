# SPDX-License-Identifier: BSD-3-Clause

# Exercise the real helper without applying a cursor to a running display.
add_library(kapplymousetheme_test_helper OBJECT ../kapplymousetheme.cpp)
target_compile_definitions(kapplymousetheme_test_helper PRIVATE main=kapplymousetheme_main)
target_link_libraries(kapplymousetheme_test_helper PRIVATE Qt::Gui Sonic::Logging)

add_executable(kapplymousethemetest kapplymousethemetest.cpp)
target_link_libraries(kapplymousethemetest PRIVATE kapplymousetheme_test_helper Qt::Gui Qt::Test)
add_test(NAME kapplymousethemetest COMMAND kapplymousethemetest)
set_tests_properties(kapplymousethemetest PROPERTIES TIMEOUT 60)
ecm_mark_as_test(kapplymousethemetest)
