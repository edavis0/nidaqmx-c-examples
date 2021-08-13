set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(toolchain_path C:/build/18.0/x64/sysroots)
set(CMAKE_C_COMPILER ${toolchain_path}/i686-nilrtsdk-mingw32/usr/bin/x86_64-nilrt-linux/x86_64-nilrt-linux-gcc.exe)
set(CMAKE_CXX_COMPILER ${toolchain_path}/i686-nilrtsdk-mingw32/usr/bin/x86_64-nilrt-linux/x86_64-nilrt-linux-g++.exe)
set(CMAKE_SYSROOT ${toolchain_path}/core2-64-nilrt-linux)
set(CMAKE_<LANG>_STANDARD_INCLUDE_DIRECTORIES ${toolchain_path}/core2-64-nilrt-linux/usr/include/c++/6.3.0 ${toolchain_path}
    /core2-64-nilrt-linux/usr/include/c++/6.3.0/x86_64-nilrt-linux)
set(CMAKE_<LANG>_FLAGS "-Wall -fmessage-length=0")
set(CMAKE_<LANG>_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_<LANG>_FLAGS_RELEASE "-O3")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Project specific information
cmake_minimum_required(VERSION 3.7.2)
project(ProjectName) # NOTE: Edit ProjectName to a project name of your choosing
set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_BUILD_TYPE Debug)

set(HEADER_DIR "C:/Program\ Files\ (x86)/National\ Instruments/NI-DAQ/DAQmx\ ANSI\ C\ Dev/include")
set(DAQMXLIBPATH "C:/Program\ Files\ (x86)/National\ Instruments/Shared/ExternalCompilerSupport/C/lib64/gcc")
add_executable(ProjectName ../src/SourceCodeName.c ${HEADER_DIR}/NIDAQMX.h) # NOTE: Replace ProjectName and SourceCodeName to match your chosen project name and source code name
target_include_directories(ProjectName PUBLIC ${HEADER_DIR})
target_link_libraries(ProjectName PUBLIC ${DAQMXLIBPATH}/libnidaqmx.so)