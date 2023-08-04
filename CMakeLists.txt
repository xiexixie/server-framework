cmake_minimum_required(VERSION 3.16)
project(logsystem)

set(CXX_STANDARD_REQUIRED 11)

include_directories(${PROJECT_SOURCE_DIR}/include/)
add_library(log SHARED src/log.cpp)

add_executable(test test/logtest.cpp src/util.cpp)
add_dependencies(test log)
target_link_libraries(test log)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)