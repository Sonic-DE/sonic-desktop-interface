# Only used by emojier/data/update.sh
cmake_minimum_required(VERSION 3.16)

project(dictbuilder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

find_package(Qt5 CONFIG REQUIRED COMPONENTS
    Core
    Xml
)
find_package(KF5I18n)

add_executable(dictbuilder dictbuilder.cpp ../../emojicategory.cpp)
target_link_libraries(dictbuilder Qt5::Core Qt5::Xml KF5::I18n)
