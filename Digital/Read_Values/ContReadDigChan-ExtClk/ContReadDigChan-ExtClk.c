/*********************************************************************
*
* ANSI C Example program:
*    ContReadDigChan-ExtClk.c
*
* Example Category:
*    DI
*
* Description:
*    This example demonstrates how to input a continuous digital
*    waveform using an external clock.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Select the Clock Source for the acquistion.
*    3. Set the approximate Rate of the external clock. This allows
*       the internal characteristics of the acquisition to be as
*       efficient as possible. Also set the Samples to Read control.
*       This will determine how many samples are read each time. This
*       also determines how many points are plotted on the graph each
*       iteration.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Input channel.
*    3. Define the parameters for an External Clock Source.
*       Additionally, set the sample mode to be continuous.
*    4. Call the Start function to start the acquisition.
*    5. Read the waveform data continuously until the user hits the
*       stop button or an error occurs.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. Also, make sure your external clock
*    terminal matches the Physical Channel I/O Control. For further
*    connection information, refer to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleRate = 10000.0; // The sampling rate in samples per second per channel
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.
// DAQmxCreateDIChan Options
const char *lines = "Dev1/port0/line0"; // The names of the digital lines used to create a virtual channel. You can specify a list or range of lines.
const int32 lineGrouping = DAQmx_Val_ChanPerLine; // Specifies whether to group digital lines into one or more virtual channels. Options: DAQmx_Val_ChanPerLine, DAQmx_Val_ChanForAllLines
// DAQmxCfgSampClkTiming Options
const char *clockSource = "/Dev1/PFI0"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint
// DAQmxReadDigitalU32 Options
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for the function to read the sample(s). To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).
const float64 fillMode = DAQmx_Val_GroupByChannel; // Specifies whether or not the samples are interleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int32		error=0;
	TaskHandle	taskHandle=0;
	uInt32		data[sampsPerChan];
	int32		sampsRead,totalRead=0;
	char		errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/

	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDIChan(taskHandle,lines,"",lineGrouping));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,clockSource,sampleRate,activeEdge,sampleMode,sampsPerChan));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Acquiring samples continuously. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadDigitalU32(taskHandle,sampsPerChan,timeout,fillMode,data,sampsPerChan,&sampsRead,NULL));

		if( sampsRead>0 ) {
			totalRead += sampsRead;
			printf("Acquired %d samples. Total %d\n",(int)sampsRead,(int)totalRead);
			for (int i = 0; i < sampsPerChan; i++)
			{
			printf("0x%X\n", data[i]);
			}
			fflush(stdout);
		}
	}
	printf("\nAcquired %d total samples.\n",(int)totalRead);

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
