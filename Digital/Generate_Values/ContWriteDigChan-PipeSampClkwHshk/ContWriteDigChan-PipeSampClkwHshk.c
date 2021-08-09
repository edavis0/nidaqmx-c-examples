/*********************************************************************
*
* ANSI C Example program:
*    ContWriteDigChan-PipeSampClkwHshk.c
*
* Example Category:
*    DO
*
* Description:
*    This examples demostrates how to interface the NI 6536/7 to a
*    synchonous FIFO.
*
* Instructions for Running:
*    1. Select the Physical Channels that correspond to where your
*       signal is output on the device.
*    2. Enter the number of Samples per Buffer. This is the number of
*       samples that will be downloaded to the device every time the
*       DAQmx Write function is called.
*    3. Specify the Sample Clock Rate of the output waveform.
*    4. Specify the Output Terminal for the Exported Sample Clock.
*    5. Specify the Sample Clock Pulse Polarity. When set to Active
*       High, the data lines will toggle on the rising edge of the
*       sample clock.
*    6. Specify the handshaking parameters. The Data Active Event
*       will be asserted when a valid sample is clocked out.The Pause
*       Trigger Polarity tells this device when to pause. If the
*       polarity is set to High, then the device will pause when the
*       corresponding PFI line is high. Note, that the device will
*       not pause on the next sample clock edge because of
*       pipelining.
*
* Steps:
*    1. Create a task.
*    2. Create one Digital Output channel for each Digital Line in
*       the Task.
*    3. Call the DAQmxCfgPipelinedSampClkTiming function which
*       configures the device for Pipelined Sample Clock.
*    4. Configure the pause trigger.
*    5. Configure the exported sample clock and data active event.
*    6. Configure the hardware to pause if the onboard memory becomes
*       empty.
*    7. Disallow Regeneration. When regeneration is disallowed, the
*       data transfer between the device and the DAQmx buffer will
*       pause when the device has emptied this buffer. It will resume
*       when more data has been written into the buffer.
*    8. Write the waveform to the output buffer.
*    9. Call the Start function to start the task.
*    10. Write the data to the output buffer continuously.
*    11. Call the Clear Task function to clear the Task.
*    12. Display an error if any.
*
* I/O Connections Overview:
*    Connect the FIFO's Almost Full Flag to the Pause Trigger.
*    Connect the FIFO's Write Enable signal to the Data Active Event.
*    Connect the FIFO's Writw Clock to the exported sample clock
*    terminal. Connect the data lines from the NI 6536/7 to the data
*    lines of the FIFO.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int32 DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// DAQmxCreateDOChan Options
const char *lines = "PXI1Slot3/port0"; // The names of the digital lines used to create a virtual channel. Specifying a port and no lines is the equivalent of specifying all the lines of that port in order.
const int32 lineGrouping = DAQmx_Val_ChanForAllLines; // Specifies whether to group digital lines into one or more virtual channels. Options: _ChanPerLine, _ChanForAllLines

// DAQmxCfgPipelinedSampClkTiming Options
const char *clockSource = ""; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const float64 rate = 100000.0; // The sampling rate in samples per second per channel.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: _Rising, _Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: _FiniteSamps, _ContSamps, _HWTimedSinglePoint
const uInt64 sampsPerChanToAcquire = 1000; // The number of samples to acquire or generate for each channel in the task.

// DAQmxSetPauseTrigType Options
const int32 pauseTrigType = DAQmx_Val_DigLvl; // Specifies the type of trigger to use to pause a task. Options: _AnlgLvl, _AnlgWin, _DigLvl, _DigPattern, _None

// DAQmxSetDigLvlPauseTrigSrc Options
const char *pauseTrigSource = "/Dev1/PFI1"; // Specifies the name of a terminal where there is a digital signal to use as the source of the Pause Trigger.

// DAQmxSetDigLvlPauseTrigWhen Options
const int32 pauseTrigWhen = DAQmx_Val_High; // Specifies whether the task pauses while the signal is high or low. Options: _High, _Low

// DAQmxSetExportedSampClkOutputTerm Options
const char *clockOutputTerm = "/Dev1/PFI4"; // Specifies the terminal to which to route the Sample Clock.

// DAQmxSetExportedSampClkPulsePolarity Options
const int32 clockPulsePolarity = DAQmx_Val_ActiveHigh; // Specifies the polarity of the exported Sample Clock if Output Behavior is DAQmx_Val_Pulse. Options: _ActiveHigh, _ActiveLow

// DAQmxSetExportedDataActiveEventLvlActiveLvl Options
const int32 dataActiveEventLevel = DAQmx_Val_ActiveLow; // Specifies the polarity of the exported Data Active Event. Options: _ActiveHigh, _ActiveLow

// DAQmxSetExportedDataActiveEventOutputTerm Options
const char *dataActiveEventTerminal = "/Dev1/PFI0"; // Specifies the terminal to which to export the Data Active Event.

// DAQmxSetSampClkUnderflowBehavior Options
const int32 underflowBehavior = DAQmx_Val_PauseUntilDataAvailable; // Specifies the action to take when the onboard memory of the device becomes empty. Options: _HaltOutputAndError, _PauseUntilDataAvailable

// DAQmxSetWriteRegenMode Options
const int32 regenMode = DAQmx_Val_DoNotAllowRegen; // Specifies whether to allow NI-DAQmx to generate the same data multiple times. Options: _AllowRegen, _DoNotAllowRegen

// DAQmxWriteDigitalU32 Options
const int32 numSampsPerChan = 1000; // The number of samples, per channel, to write.
const int32 autoStart = 0; // Specifies whether or not this function automatically starts the task if you do not start it.
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for this function to write all the samples. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely).
const bool32 dataLayout = DAQmx_Val_GroupByChannel; // Specifies how the samples are arranged, either interleaved or noninterleaved. Options: _GroupByChannel, _GroupByScanNumber


int main(void)
{
	int         error=0;
	TaskHandle  taskHandle=0;
	uInt32      i=0;
	uInt32      data[sampsPerChanToAcquire];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandle,lines,"",lineGrouping));
	DAQmxErrChk (DAQmxCfgPipelinedSampClkTiming(taskHandle,clockSource,rate,activeEdge,sampleMode,sampsPerChanToAcquire));
	DAQmxErrChk (DAQmxSetPauseTrigType(taskHandle,pauseTrigType));
	DAQmxErrChk (DAQmxSetDigLvlPauseTrigSrc(taskHandle,pauseTrigSource));
	DAQmxErrChk (DAQmxSetDigLvlPauseTrigWhen(taskHandle,pauseTrigWhen));
	DAQmxErrChk (DAQmxSetExportedSampClkOutputTerm(taskHandle,clockOutputTerm));
	DAQmxErrChk (DAQmxSetExportedSampClkPulsePolarity(taskHandle,clockPulsePolarity));
	DAQmxErrChk (DAQmxSetExportedDataActiveEventLvlActiveLvl(taskHandle,dataActiveEventLevel));
	DAQmxErrChk (DAQmxSetExportedDataActiveEventOutputTerm(taskHandle,dataActiveEventTerminal));
	DAQmxErrChk (DAQmxSetSampClkUnderflowBehavior(taskHandle,underflowBehavior));
	DAQmxErrChk (DAQmxSetWriteRegenMode(taskHandle,DAQmx_Val_DoNotAllowRegen));

	for(;i<sampsPerChanToAcquire;++i)
		data[i] = i;

	DAQmxErrChk (DAQmxRegisterDoneEvent(taskHandle,0,DoneCallback,NULL));

	DAQmxErrChk (DAQmxWriteDigitalU32(taskHandle,numSampsPerChan,autoStart,timeout,dataLayout,data,NULL,NULL));

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

int32 DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
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
