cmake_minimum_required(VERSION 3.10)

# set the project name
project(MakeTVPlaylist VERSION 1.1)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(MAKE_LIB "MAKE_LIB" 0)

if(MAKE_LIB)
	add_compile_definitions(MAKE_LIB=1)
	add_library(tvplaylist SHARED src/main.cpp)
	set_target_properties(tvplaylist PROPERTIES PUBLIC_HEADER src/libtvplaylist.h)
	install(TARGETS tvplaylist DESTINATION ${CMAKE_INSTALL_LIBDIR})
	install(TARGETS tvplaylist PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
else()
	add_executable(tvplaylist src/main.cpp)
	install(TARGETS tvplaylist DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
