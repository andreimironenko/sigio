cmake_minimum_required(VERSION 3.10)

project(sigio)

set(CMAKE_CXX_STANDARD 17)
set(BUILD_SHARED_LIBS YES)

# Add the cmake folder so the FindSphinx module is found
#set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(INCDIR "${PROJECT_SOURCE_DIR}/include")

# load git version module
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(VersionFromGit)

# retrieve version
version_from_git(GIT_EXECUTABLE "git" LOG ON TIMESTAMP "%Y%m%d%H%M%S")
project(sigio VERSION ${VERSION})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
#add_compile_options(-pthread)

set(sigio_headers ${INCDIR}/sigio.h)

include_directories(include)
include_directories(sources)

#add_subdirectory(sources)
#add_subdirectory(docs)
#add_subdirectory(python)

#add_dependencies(Doxygen ${CMAKE_PROJECT_NAME}_timer)
#add_dependencies(Sphinx Doxygen ${CMAKE_PROJECT_NAME}_timer)
#add_dependencies(pdf Sphinx Doxygen ${CMAKE_PROJECT_NAME}_timer)

add_library(sigio SHARED
  include/sigio.h
  sources/sigio.cpp
  sources/sigio_.cpp
  sources/sigio_.h
  )

target_link_libraries(sigio rt)
#add_subdirectory(tests)
#add_dependencies(${CMAKE_PROJECT_NAME}-test ${CMAKE_PROJECT_NAME} gtest gtest_main)

set_target_properties(sigio PROPERTIES PUBLIC_HEADER "${sigio_headers}")
set_target_properties(sigio PROPERTIES VERSION ${VERSION} SOVERSION 1)

# install sigio library and headers
install(TARGETS sigio EXPORT SigioTargets LIBRARY DESTINATION lib PUBLIC_HEADER DESTINATION include)

# export targets
set(EXPORT_DIR "${CMAKE_CURRENT_BINARY_DIR}/cmake")
export(EXPORT SigioTargets
  FILE "${EXPORT_DIR}/SigioTargets.cmake"
  NAMESPACE sigio::
  )

# install targets
install(EXPORT SigioTargets
  FILE SigioTargets.cmake
  NAMESPACE sigio::
  DESTINATION lib/cmake/sigio
  )
