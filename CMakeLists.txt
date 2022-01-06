cmake_minimum_required(VERSION 3.10)

# set the project name
project(MakeTVPlaylist VERSION 1.0)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(MAKE_LIB "MAKE_LIB" 0)

if(MAKE_LIB)
	add_compile_definitions(MAKE_LIB=1)
	add_library(tvplaylist SHARED src/main.cpp)
else()
	add_executable(tvplaylist src/main.cpp)
endif()

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
