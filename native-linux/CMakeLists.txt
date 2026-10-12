cmake_minimum_required(VERSION 3.10)
project(MtLinux CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

find_package(JNI REQUIRED COMPONENTS JVM)
find_package(X11 REQUIRED)

if(NOT X11_Xi_FOUND)
	message(FATAL_ERROR "XInput2 (libXi) development headers were not found")
endif()

add_library(MtLinux64 SHARED
	LinuxMultitouch.cpp
	WindowManager.cpp
	MtWindow.cpp
)

target_include_directories(MtLinux64 PRIVATE ${JNI_INCLUDE_DIRS} ${X11_INCLUDE_DIR})
target_link_libraries(MtLinux64 PRIVATE ${X11_LIBRARIES} ${X11_Xi_LIB} pthread)

set_target_properties(MtLinux64 PROPERTIES
	PREFIX "lib"
	OUTPUT_NAME "MtLinux64"
)
