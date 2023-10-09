cmake_minimum_required(VERSION 3.16)
project(logsystem)

set(CXX_STANDARD_REQUIRED 11)

include_directories(${PROJECT_SOURCE_DIR}/include/)
add_library(log_module SHARED src/log.cpp src/util.cpp src/config.cpp)

add_executable(test test/log_config_test.cpp)
add_dependencies(test log_module)
target_link_libraries(test log_module)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)