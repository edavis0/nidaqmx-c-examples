/*********************************************************************
*
* ANSI C Example program:
*    WriteDigChan-ExtClk.c
*
* Example Category:
*    DO
*
* Description:
*    This example demonstrates how to output a finite digital
*    waveform using an external clock.
*
* Instructions for Running:
*    1. Select the Physical Channel to correspond to where your
*       signal is output on the DAQ device.
*    2. Select the Clock Source for the generation.
*    3. Specify the Rate of the output Waveform
*    4. Enter the Waveform Information.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Output Channel.
*    3. Call the DAQmxCfgSampClkTiming function which sets the sample
*       clock rate. Additionally, set the sample mode to Finite.
*    4. Write the waveform to the output buffer.
*    5. Call the Start function to start the task.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminal matches the Physical
*    Channel I/O Control. Also, make sure your external clock
*    terminal matches the Clock Source Control. For further
*    connection information, refer to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleRate = 1000.0; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.

// DAQmxCreateDOChan Options
const char *lines = "Dev1/port0/line0"; // The names of the digital lines used to create a virtual channel. Specifying a port and no lines is the equivalent of specifying all the lines of that port in order.
const int32 lineGrouping = DAQmx_Val_ChanForAllLines; // Specifies whether to group digital lines into one or more virtual channels. Options: DAQmx_Val_ChanPerLine, DAQmx_Val_ChanForAllLines

// DAQmxCfgSampClkTiming Options
const char *clockSource = "/Dev1/PFI0"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: DAQmx_Val_Rising, DAQmx_Val__Falling
const int32 sampleMode = DAQmx_Val_FiniteSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint

// DAQmxWriteDigitalLines Options
const bool32 autoStart = 0; // Specifies whether or not this function automatically starts the task if you do not start it.
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for this function to write all the samples. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely). 
const bool32 dataLayout = DAQmx_Val_GroupByChannel; // Specifies how the samples are arranged, either interleaved or noninterleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	uInt8       data[sampsPerChan];
	uInt32      i=0;
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandle,lines,"",lineGrouping));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));

	for(;i<1000;i++)
		data[i] = (uInt8)(i%2);

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteDigitalLines(taskHandle,sampsPerChan,autoStart,timeout,dataLayout,data,NULL,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Wait Code
	/*********************************************/
	DAQmxErrChk (DAQmxWaitUntilTaskDone(taskHandle,10.0));

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
