# nidaqmx-c-examples

## Overview
This repository contains NI-DAQmx example programs. Each example contains C source code, and many examples are designed with new users in mind. The examples that have been refactored for new users (using constants and comments) can be found in the list below.

## Refactored Examples
* Analog_In:
  * Measure_Acceleration: ContAccelSamps-IntClk-AnlgStart
  * Measure_Current: Cont0-20mASamps-IntClk
  * Measure_Slow_Varying_Signal: ContAcqSamp-IntClk
  * Measure_Strain: ContStrainSamples
  * Measure_Temperature: ContThrmcplSamples-IntClk
  * Measure_Voltage: All examples
* Analog_Out:
  * Generate_Voltage: All examples
* Digital:
  * Generate_Values: All examples
  * Read_Values: ContReadDigChan-ExtClk, ReadDigChan, and ReadDigPort

## Building
To compile the source code on your host machine, you must install the GNU C/C++ Compile Tools for [x64 Linux][2] or [ARMv7 Linux][3]. Make sure to include the CMakeLists.txt file and .vscode directory when building the binary (included in "samplebuildfiles").

Note: It is highly recommended that you first learn how to cross-compile code and deploy to the NI Linux RTOS using Microsoft VSCode by visiting this [NI Forum Post][4]. Then, refer to the included [guide][5] for extra tips.

## Reference Material
* To review NI-DAQmx C Reference Help, visit this [link][6].
* To learn more about NI's driver software portfolio, visit this [link][7].
* To learn more about the NI Linux Real-Time OS, visit this [link][8].

[2]: https://www.ni.com/en-us/support/downloads/software-products/download.gnu-c---c---compile-tools-x64.html#338442 "x64 Linux Toolchain download" 
[3]: https://www.ni.com/en-us/support/downloads/software-products/download.gnu-c---c---compile-tools-for-armv7.html#338448 "ARMv7 Linux Toolchain download" 
[4]: https://forums.ni.com/t5/NI-Linux-Real-Time-Documents/NI-Linux-Real-Time-Cross-Compiling-Using-the-NI-Linux-Real-Time/ta-p/4026449 "NI forum post"
[5]: https://github.com/edavis0/nidaqmx-c-examples/blob/main/CrossCompileTips/NI-DAQmxLinuxCross-compileTips.md "guide"
[6]: https://zone.ni.com/reference/en-XX/help/370471AM-01/ "reference guide"
[7]: https://www.ni.com/en-us/innovations/white-papers/21/hardware-drivers-the-key-to-nis-software-connectedness.html "whitepaper"
[8]: https://www.ni.com/en-us/shop/linux.html "NI Linux homepage"
