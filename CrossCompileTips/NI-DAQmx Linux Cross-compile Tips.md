# Overview
Before using this document, refer to the NI forum [post][1], “NI Linux Real-Time Cross Compiling: Using the NI Linux Real-Time Cross Compile Toolchain with Visual Studio Code.” After working through the examples, choose your desired C code from the “nidaqmx-c-examples” repository. The following steps utilize the “ContThrmcplSamps-IntClk.c” c source file from “\Analog_In\Measure_Temperature\ContThrmcplSamples-IntClk”.
Note: this example was built with a Windows 10 host machine and cRIO-9040 (x64_Linux).

# Steps
1.  Install the NI-DAQmx driver on your host computer.
2.  Install the NI-DAQmx driver on your NI Linux Real-Time target.
3.  [Install][2] the correct GNU C & C++ Compile Tools for your NI Linux Real-Time target.
4.  Locate the NIDAQmx.h header file in the following directory on your host computer: C:\Program Files (x86)\National Instruments\NI-DAQ\DAQmx ANSI C Dev\include\.
5.  Add the NIDAQmx.h header file to the C:\build\18.0\x64\sysroots\core2-64-nilrt-linux\usr\include directory in your host computer.
6.  Add the contents of /usr/lib/x86_64-linux-gnu/ directory in your target to the C:\build\18.0\x64\sysroots\core2-64-nilrt-linux\usr\lib\ directory in your host computer.
7.  Add the contents of the target’s /usr/local/natinst/lib/ directory to the host’s C:\build\18.0\x64\sysroots\core2-64-nilrt-linux\usr\local\natinst\lib directory.
8.  Create a copy of the cross-compile project template in a directory of your choosing in the host computer, or use the sample set included in /samplebuildfiles.
9.  Open the directory in VSCode.

![Alt][3]

10. Modify the c_cpp_properties.json file, as shown below.
~~~
{
    "env": {
        "compilerSysroots": "C:/build/18.0/x64/sysroots"
    },  
    "configurations": [
        {
            "name": "NI Linux Real-Time x64",
            "compilerPath": "${compilerSysroots}/i686-nilrtsdk-mingw32/usr/bin/x86_64-nilrt-linux/x86_64-nilrt-linux-gcc",
            "compilerArgs": [
                "--sysroot=${compilerSysroots}/core2-64-nilrt-linux/"
            ],
            "includePath": [
                "${workspaceFolder}/**",
                "${compilerSysroots}/core2-64-nilrt-linux/usr/include"
            ],
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "gcc-x64"
        }
    ],
    "version": 4
}
~~~~
11. Modify the tasks.json file, as shown below.
~~~
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Generate Build Files",
            "type": "shell",
            "command": "cmake -G Ninja ${workspaceFolder}/build",
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "problemMatcher": []
        },
        {
            "label": "Ninja",
            "type": "shell",
            "command": "ninja",
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "clean",
            "type": "shell",
            "command": "ninja clean",
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "problemMatcher": []
               
        }
    ]
}
~~~
12. Add your Linux DAQmx example source code to the .src folder.
13.	Modify the CMakeList.txt file, as shown below. 
  * Note: ensure that the add_executable command references the correct source code file for your application.
  *	Note: The series of add_library and set_property commands reference the .so files necessary for DAQmx to properly execute.
~~~
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

# project specific information
cmake_minimum_required(VERSION 3.7.2)
project(ContThrmcplSamps-IntClk)
set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_BUILD_TYPE Debug)

include_directories(${toolchain_path}/core2-64-nilrt-linux/usr/include)
add_executable(ContThrmcplSamps-IntClk ../src/ContThrmcplSamps-IntClk.c)

# Note: ensure that you copy the contents of the real-time system's /usr/local/natinst/lib folder into the respective folder in your development toolchain
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

target_link_libraries(ContThrmcplSamps-IntClk nitargetcfg nirocoapi niprtsiu nisysapi niAvahiClient nimru2u nimhwcfu nimxdfu nicrtsiu 
                        nidimu nidmxfu nimdbgu niorbu nipalu nidaqmx )
~~~
14.	From the Command Palette (Ctrl + Shift + P), select Tasks: Run Task, and then “CMake Generate Build Files.” This will run the task created to allow Visual Studio Code to invoke CMake, as shown below.
![Alt][4]
15. From the Tasks: Run Task, select Ninja to build the executable, as shown below.
![Alt][5]
16.	Copy the directory from your host computer to your target.
17.	Through SSH, run the executable (located in /build/bin), as shown below.
![Alt][6]

[1]: https://forums.ni.com/t5/NI-Linux-Real-Time-Documents/NI-Linux-Real-Time-Cross-Compiling-Using-the-NI-Linux-Real-Time/ta-p/4026449?profile.language=en "cross compile forum post"
[2]: https://www.ni.com/en-us/innovations/white-papers/20/building-c-c---applications-for-ni-linux-real-time.html#section--1974177664 "compile tools download list"
[3]: https://github.com/edavis0/nidaqmx-c-examples/blob/main/CrossCompileTips/media/VSCode%20Directory%20Screenshot.png "VSCode directory screenshot"
[4]: https://github.com/edavis0/nidaqmx-c-examples/blob/main/CrossCompileTips/media/Build%20Console%20Output%20Screenshot%201.png "CMake build files screenshot"
[5]: https://github.com/edavis0/nidaqmx-c-examples/blob/main/CrossCompileTips/media/Build%20Console%20Output%20Screenshot%202.png "Ninja build screenshot"
[6]: https://github.com/edavis0/nidaqmx-c-examples/blob/main/CrossCompileTips/media/SSH%20Output%20Screenshot.png "SSH output screenshot"
