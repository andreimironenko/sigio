find_package(PythonLibs REQUIRED)
message("Include dirs of Python: " ${PYTHON_INCLUDE_DIRS})
get_filename_component(PYTHON_LIBRARIES_PATH ${PYTHON_LIBRARIES} DIRECTORY)
get_filename_component(PYTHON_LIBRARIES_NAME ${PYTHON_LIBRARIES} NAME)
message("Path Libs of Python: " ${PYTHON_LIBRARIES_PATH})
message("Python Libs: " ${PYTHON_LIBRARIES_NAME})

include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARIES})


add_library(posix_timer SHARED
  ../../include/timer.h
  timer/python_timer.cpp
  )


target_link_libraries(posix_timer ${CMAKE_PROJECT_NAME}_timer rt boost_python38 python3.8)
