/*********************************************************************
*
* ANSI C Example program:
*    WriteDigPort.c
*
* Example Category:
*    DO
*
* Description:
*    This example demonstrates how to write values to a digital
*    output port.
*
* Instructions for Running:
*    1. Select the digital port on the DAQ device to be written.
*    2. Select a value to write.
*    Note: The Data to Write control is in hexadecimal.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Output channel. Use one channel for all
*       lines. In this case, the port itself acts as an individual
*       channel.
*    3. Call the Start function to start the task.
*    4. Write digital port data. This write function writes a single
*       sample of digital data on demand, so no timeout is necessary.
*    5. Call the Clear Task function to clear the Task.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminals match the Port I/O
*    Control. In this case wire the item to receive the signal to the
*    first N digital lines on your DAQ Device.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// DAQmxCreateDOChan Options
const char *lines = "Dev1/port0"; // The names of the digital lines used to create a virtual channel. Specifying a port and no lines is the equivalent of specifying all the lines of that port in order.
const int32 lineGrouping = DAQmx_Val_ChanForAllLines; // Specifies whether to group digital lines into one or more virtual channels. Options: DAQmx_Val_ChanPerLine, DAQmx_Val_ChanForAllLines

// DAQmxWriteDigitalU32 Options
const int32 numSampsPerChan = 1; // The number of samples, per channel, to write.
const int32 autoStart = 1; // Specifies whether or not this function automatically starts the task if you do not start it.
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for this function to write all the samples. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).
const bool32 dataLayout = DAQmx_Val_GroupByChannel; // Specifies how the samples are arranged, either interleaved or noninterleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int         error=0;
	TaskHandle	taskHandle=0;
	uInt32      data=0xffffffff;
	char        errBuff[2048]={'\0'};
	int32		written;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandle,lines,"",lineGrouping));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandle,numSampsPerChan,autoStart,timeout,dataLayout,&data,&written,NULL));

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
