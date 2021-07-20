/*********************************************************************
*
* ANSI C Example program:
*    MultVoltUpdates-SWTimed.c
*
* Example Category:
*    AO
*
* Description:
*    This example demonstrates how to output multiple Voltage Updates
*    (Samples) to an Analog Output Channel in a software timed loop.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is output on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    Note: Use the Cont Acq Multiple Samples example to verify you
*          are generating the correct output on the DAQ device.
*    3. Set the rate.
*    4. Run the function.
*    5. Stop the function when desired.
*
* Steps:
*    1. Create a task.
*    2. Create an Analog Output Voltage channel.
*    3. Create a sinewave with 1000 points and put the data in an
*       array.
*    4. Call the Start function.
*    5. Write one data point from the array (modulo indexed) until
*       the user hits the stop button or an error occurs. The rate is
*       settable to 1 millisecond.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminal matches the Physical
*    Channel I/O Control. For further connection information, refer
*    to your hardware reference manual.
*
*********************************************************************/

/*********************************************************************
* Microsoft Windows Vista User Account Control
* Running certain applications on Microsoft Windows Vista requires
* administrator privileges, because the application name contains keywords
* such as setup, update, or install. To avoid this problem, you must add an
* additional manifest to the application that specifies the privileges
* required to run the application. Some ANSI-C NI-DAQmx examples include
* these keywords. Therefore, these examples are shipped with an additional
* manifest file that you must embed in the example executable. The manifest
* file is named [ExampleName].exe.manifest, where [ExampleName] is the
* NI-provided example name. For information on how to embed the manifest
* file, refer to http://msdn2.microsoft.com/en-us/library/bb756929.aspx.
*********************************************************************/

#include <stdio.h>
#include <math.h>
#include <NIDAQmx.h>
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <time.h>
#endif

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

#define PI	3.1415926535

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleRate = 1000; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.
// DAQmxCreateAOVoltageChan Options
const char *physicalChannel = "Dev1/ao0"; // The names of the physical channels to use to create virtual channels. You can specify a list or range of physical channels.
const float64 minVal = -10.0; // The minimum value, in units, that you expect to generate.
const float64 maxVal = 10.0; // The maximum value, in units, that you expect to generate.
const int32 units = DAQmx_Val_Volts; // The units in which to generate voltage. Options: DAQmx_Val_Volts, DAQmx_Val_FromCustomScale
// DAQmxWriteAnalogScalarF64 Options
const bool32 autoStart = 1; // Specifies whether or not this function automatically starts the task if you do not start it.
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for this function to write the value. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).

int main(void)
{
	int         error=0;
	TaskHandle  taskHandle=0;
	float64     data[1000];
	char        errBuff[2048]={'\0'};
	uInt32      i=0;

	for(;i<1000;i++)
		data[i] = 9.95*sin((double)i*2.0*PI/1000.0);
	i = 0;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(taskHandle,physicalChannel,"",minVal,maxVal,units,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Generating samples continuously. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		#if defined(WIN32) || defined(_WIN32)
			Sleep(1);
		#elif defined(__APPLE__) || defined(__linux__)
			struct timespec ts = {1, 0};
			nanosleep(&ts, 0);
		#else
			#error - This example requires a platform specific sleep call.
		#endif

		/*********************************************/
		// DAQmx Write Code
		/*********************************************/
		DAQmxErrChk (DAQmxWriteAnalogScalarF64(taskHandle,autoStart,timeout,data[i],NULL));
		if( ++i>=1000 )
			i = 0;
	}

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);
	if( taskHandle!=0 ) {
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		DAQmxStopTask(taskHandle);
		DAQmxClearTask(taskHandle);
	}
	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}
