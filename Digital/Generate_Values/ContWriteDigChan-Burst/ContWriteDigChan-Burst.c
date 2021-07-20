/*********************************************************************
*
* ANSI C Example program:
*    ContWriteDigChan-Burst.c
*
* Example Category:
*    DO
*
* Description:
*    This example demonstrates how to output a continuous digital
*    waveform using burst handshaking mode.
*    Note: This example program exports the sample clock from the
*          device. To import the sample clock, call the
*          DAQmxCfgBurstHandshakingTimingImportClock function
*          instead.
*
* Instructions for Running:
*    1. Select the Physical Channels that correspond to where your
*       signal is output on the device.
*    2. Enter the number of Samples per Buffer. This is the number of
*       samples that will be downloaded to the device every time the
*       DAQmx Write function is called.
*    3. Specify the Sample Clock Rate of the output Waveform.
*    4. Specify the Output Terminal for the Exported Sample Clock.
*    5. Specify the Sample Clock Pulse Polarity. When set to Active
*       High, the data lines will toggle on the rising edge of the
*       sample clock.
*    6. Specify the handshaking parameters. The Ready for Transfer
*       Event will be asserted any time this device is ready to
*       transfer data. The Pause Trigger Polarity tells this device
*       when to pause. If the polarity is set to High, then the
*       device will pause when the corresponding PFI line is high.
*
* Steps:
*    1. Create a task.
*    2. Create one Digital Output channel for each Digital Line in
*       the Task.
*    3. Call the DAQmxCfgBurstHandshakingTimingExportClock function
*       which configures the device for Burst Mode Handshaking, sets
*       the sample clock rate, exports the clock on the specified PFI
*       Line, and sets the sample mode to Continuous.
*    4. Call the Start function to start the task.
*    5. Write the data to the output buffer continuously until the
*       user presses the Stop button.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    Connect the Pause Trigger and Ready For Transfer event to the
*    default PFI terminals for the device. The sample clock will be
*    exported to the specified PFI terminal. Make sure your waveform
*    output terminals match the Physical Channel I/O Control.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleClkRate = 1000.0; // Specifies the sampling rate in samples per channel per second.
const uInt64 sampsPerChan = 1000; // The number of samples to acquire from each channel.
// DAQmxCreateDOChan Options
const char *lines = "PXI1Slot3/port0"; // The names of the digital lines used to create a virtual channel. Specifying a port and no lines is the equivalent of specifying all the lines of that port in order.
const int32 lineGrouping = DAQmx_Val_ChanForAllLines; // Specifies whether to group digital lines into one or more virtual channels. Options: DAQmx_Val_ChanPerLine, DAQmx_Val_ChanForAllLines
// DAQmxCfgBurstHandshakingTimingExportClock Options
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. // Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint
const char *sampleClkOutpTerm = "/Dev1/PFI0"; // Specifies the terminal to which to route the Sample Clock.
const int32 sampleClkPulsePolarity = DAQmx_Val_ActiveHigh; // Specifies if the polarity for the exported sample clock is active high or active low.
const int32 pauseWhen = DAQmx_Val_Low; // Specifies whether the task pauses while the signal is high or low.
const int32 readyEventActiveLevel = DAQmx_Val_ActiveHigh; // Specifies the polarity for the Ready for Transfer event.
// DAQmxWriteDigitalU32 Options
const int32 autoStart = 0; // Specifies whether or not this function automatically starts the task if you do not start it.
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for this function to write all the samples. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).
const bool32 dataLayout = DAQmx_Val_GroupByChannel; // Specifies how the samples are arranged, either interleaved or noninterleaved. Options:DAQmx_Val _GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
	int         error=0;
	TaskHandle  taskHandle=0;
	uInt32      i=0;
	uInt32      data[sampsPerChan];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandle,lines,"",lineGrouping)); 
	DAQmxErrChk (DAQmxCfgBurstHandshakingTimingExportClock(taskHandle,sampleMode,sampsPerChan,sampleClkRate,sampleClkOutpTerm,sampleClkPulsePolarity,pauseWhen,readyEventActiveLevel));
	for(;i<sampsPerChan;++i)
		data[i] = i;

	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandle,sampsPerChan,autoStart,timeout,dataLayout,data,NULL,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Generating digital output continuously. Press Enter to interrupt\n");
	getchar();

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

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}
