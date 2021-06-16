cmake_minimum_required(VERSION 3.14)
project(posixcpp)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS YES)

# Add the cmake folder so the FindSphinx module is found
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include_directories(include)
include_directories(timer)

add_subdirectory(timer)
add_subdirectory(tests)
add_subdirectory(docs)
add_subdirectory(python)

add_dependencies(timer-test ${CMAKE_PROJECT_NAME}_timer gtest gtest_main)
add_dependencies(Doxygen ${CMAKE_PROJECT_NAME}_timer)
add_dependencies(Sphinx Doxygen ${CMAKE_PROJECT_NAME}_timer)
add_dependencies(pdf Sphinx Doxygen ${CMAKE_PROJECT_NAME}_timer)


