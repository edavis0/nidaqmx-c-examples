/*********************************************************************
*
* ANSI C Example program:
*    ContAcqSamp-SWTimed.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to continuously acquire data using
*    software timing (rate is governed by a UI timer).
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage ranges.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel.
*    3. Call the Start function to start the task.
*    4. Read the waveform data in the TimerCallback function until
*       the user hits the stop button or an error occurs.
*    5. Call the Clear Task function to clear the Task.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
#include <time.h>
#endif

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else


int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	float64     value;
	char        errBuff[2048]={'\0'};
	int32		read;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Acquiring samples continuously. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		#if defined(WIN32) || defined(_WIN32)
			Sleep(250);
		#elif defined(__APPLE__) || defined(__linux__)
			struct timespec ts = {0, 250000000};
			nanosleep(&ts, 0);
		#else
			#error - This example requires a platform specific sleep call.
		#endif

		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,1,10.0,DAQmx_Val_GroupByScanNumber,&value,1,&read,NULL));

		printf("Value: %f\r",value);
		fflush(stdout);
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
