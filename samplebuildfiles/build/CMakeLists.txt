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

include_directories(${toolchain_path}/core2-64-nilrt-linux/usr/include)
add_executable(ProjectName ../src/SourceCodeName.c) # NOTE: Replace ProjectName and SourceCodeName to match your chosen project name and source code name

# NOTE: ensure that you copy the contents of the real-time system's /usr/lib/x86_64-linux-gnu/ and /usr/local/natinst/lib folders and into the respective folder in your development toolchain
add_library(nitargetcfg SHARED IMPORTED)
set_property(TARGET nitargetcfg PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/local/natinst/lib/libnitargetcfg.so.7.5.0)
add_library(nirocoapi SHARED IMPORTED)
set_property(TARGET nirocoapi PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnirocoapi.so.20.2.0)
add_library(niprtsiu SHARED IMPORTED)
set_property(TARGET niprtsiu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/ni-rtsi/libniprtsiu.so.19.6.0)
add_library(nisysapi SHARED IMPORTED)
set_property(TARGET nisysapi PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnisysapi.so.20.0.0)
add_library(niAvahiClient SHARED IMPORTED)
set_property(TARGET niAvahiClient PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libniAvahiClient.so.19.0.0)
add_library(nimru2u SHARED IMPORTED)
set_property(TARGET nimru2u PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnimru2u.so.20.0.0)
add_library(nimhwcfu SHARED IMPORTED)
set_property(TARGET nimhwcfu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnimhwcfu.so.20.1.0)
add_library(nimxdfu SHARED IMPORTED)
set_property(TARGET nimxdfu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnimxdfu.so.20.0.0)
add_library(nicrtsiu SHARED IMPORTED)
set_property(TARGET nicrtsiu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/ni-rtsi/libnicrtsiu.so.20.0.0)
add_library(nidimu SHARED IMPORTED)
set_property(TARGET nidimu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnidimu.so.20.0.0)
add_library(nidmxfu SHARED IMPORTED)
set_property(TARGET nidmxfu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnidmxfu.so.1)
add_library(nimdbgu SHARED IMPORTED)
set_property(TARGET nimdbgu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnimdbgu.so.20.0.0)
add_library(niorbu SHARED IMPORTED)
set_property(TARGET niorbu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libniorbu.so.20.0.0)
add_library(nipalu SHARED IMPORTED)
set_property(TARGET nipalu PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnipalu.so.20.0.0)
add_library(nidaqmx SHARED IMPORTED)
set_property(TARGET nidaqmx PROPERTY IMPORTED_LOCATION ${toolchain_path}/core2-64-nilrt-linux/usr/lib/x86_64-linux-gnu/libnidaqmx.so)

target_link_libraries(ProjectName nitargetcfg nirocoapi niprtsiu nisysapi niAvahiClient nimru2u nimhwcfu nimxdfu nicrtsiu 
                        nidimu nidmxfu nimdbgu niorbu nipalu nidaqmx ) # NOTE: Replace ProjectName to match your chosen project name