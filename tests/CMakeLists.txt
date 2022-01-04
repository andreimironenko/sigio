# adding the Google_Tests_run target


include_directories(../include)
include(FetchContent)


# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
endif()


include_directories(${json_SOURCE_DIR}/include)

add_executable(${CMAKE_PROJECT_NAME}-test sigio-test.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}-test sigio gtest gtest_main)
#target_link_libraries(timer-test rt posixcpp_timer)

add_executable(${CMAKE_PROJECT_NAME}-example sigio-example.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}-example sigio)
set_property(TARGET ${CMAKE_PROJECT_NAME}-example PROPERTY CXX_STANDARD 17)

add_executable(${CMAKE_PROJECT_NAME}-example-inotify sigio-example-inotify.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME}-example-inotify sigio)
set_property(TARGET ${CMAKE_PROJECT_NAME}-example-inotify PROPERTY CXX_STANDARD 17)
