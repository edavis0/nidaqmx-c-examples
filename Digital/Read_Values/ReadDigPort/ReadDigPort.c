/*********************************************************************
*
* ANSI C Example program:
*    ReadDigPort.c
*
* Example Category:
*    DI
*
* Description:
*    This example demonstrates how to read values from a digital
*    input port.
*
* Instructions for Running:
*    1. Select the digital port on the DAQ device to be read.
*    Note: The Data Read indicator is in hexadecimal.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Input channel. Use one channel for all
*       lines. In this case, the port itself acts as an individual
*       channel.
*    3. Call the Start function to start the task.
*    4. Read the digital data. This read function reads a single
*       sample of digital data on demand, so no timeout is necessary.
*    5. Call the Clear Task function to clear the Task.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminals match the Port I/O
*    Control. In this case wire your digital signals to the first N
*    digital lines on your DAQ Device.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const uInt64 sampsPerChan = 1; // The number of samples to acquire or generate for each channel in the task.

// DAQmxCreateDIChan Options
const char *lines = "Dev1/port0"; // The names of the digital lines used to create a virtual channel. You can specify a list or range of lines.
const int32 lineGrouping = DAQmx_Val_ChanForAllLines; // Specifies whether to group digital lines into one or more virtual channels. Options: DAQmx_Val_ChanPerLine, DAQmx_Val_ChanForAllLines

// DAQmxReadDigitalU32 Options
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for the function to read the sample(s). To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).
const float64 fillMode = DAQmx_Val_GroupByChannel; // Specifies whether or not the samples are interleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int32		error=0;
	TaskHandle	taskHandle=0;
	uInt32		data;
	char		errBuff[2048]={'\0'};
	int32		read;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDIChan(taskHandle,lines,"",lineGrouping));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadDigitalU32(taskHandle,sampsPerChan,timeout,fillMode,&data,sampsPerChan,&read,NULL));

	printf("Data acquired: 0x%X\n",(unsigned)data);
	
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
