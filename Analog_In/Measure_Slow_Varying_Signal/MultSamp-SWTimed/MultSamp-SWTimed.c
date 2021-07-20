/*********************************************************************
*
* ANSI C Example program:
*    MultSamp-SWTimed.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to Acquire a Finite amount of data
*    using a software timed acquisition.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the Minimum and Maximum Voltage Ranges.
*    Note: For better accuracy try to match the Input Ranges to the
*          expected voltage level of the measured signal.
*    3. Set the Software Loop Time. This will control the speed of
*       the acquisition.
*    4. Select how many Samples to Acquire on Each Channel.
*
* Steps:
*    1. Create a task.
*    2. Create an Analog Input Voltage channel.
*    3. Call the Start function to begin the acquisition.
*    4. Use the Read function to Measure Single Samples from 1
*       Channel on the Data Acquisition Card. Set a timeout so an
*       error is returned if the samples are not returned in the
*       specified time limit.
*    5. Call the Clear Task function to clear the Task.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O Control.
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

#define NUMSAMPLES	4

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	int         curSample=0;
	float64     value;
	int32		numRead;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(taskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Acquiring samples continuously. Acquisition stops at %d samples\n",NUMSAMPLES);
	while( curSample++<NUMSAMPLES ) {
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
		DAQmxErrChk (DAQmxReadAnalogF64(taskHandle,1,10.0,DAQmx_Val_GroupByScanNumber,&value,1,&numRead,NULL));

		printf("Value: %f\n",value);
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
