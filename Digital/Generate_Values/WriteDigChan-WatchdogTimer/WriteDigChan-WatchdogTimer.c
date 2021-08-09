/*********************************************************************
*
* ANSI C Example program:
*    WriteDigChan-WatchdogTimer.c
*
* Example Category:
*    DO
*
* Description:
*    This example demonstrates how to write values to a digital
*    output channel while being monitored by a watchdog timer.
*
* Instructions for Running:
*    1. Select the digital lines on the DAQ device to be written.
*    2. Select a value to write.
*    3. Select the device for the watchdog timer task. This should be
*       the same device used for digital output.
*    4. Select the watchdog timeout.
*    5. Select the expiration states.
*    6. Select the time interval between writes.
*
*    Note: If the watchdog timer expires it will stay in the expired
*          state after execution stops. To clear the expiration call
*          device reset or create a watchdog timer task and use the
*          Control Watchdog Task function with a Clear Expiration
*          action. Starting a watchdog task on the device also clears
*          expiration.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Output channel. Use one channel for all
*       lines. You can alternatively use one channel for each line,
*       but then use a different version of the DAQmx Write function.
*    3. Create a Watchdog Timer task.
*    4. Call the Start function to start the digital output task.
*    5. Call the Start function to start the watchdog timer task.
*    6. Write the digital data continuously until the user hits the
*       stop button or an error occurs.
*    7. Use the Control Watchdog Task function with a Reset Timer
*       action to prevent watchdog timer expiration.
*    8. Wait for the specified amount of time to simulate the extra
*       computation the application being monitored must perform.
*    9. Call the Clear Task function to clear the watchdog timer
*       task.
*    10. Call the Clear Task function to clear the digital output
*        task.
*    11. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal output terminals match the Lines I/O
*    Control. In this case wire the item to receive the signal to the
*    first eight digital lines on your DAQ Device.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

// DAQmxCreateDOChan Options
const char *lines = "Dev1/port0/line0:7"; // The names of the digital lines used to create a virtual channel. Specifying a port and no lines is the equivalent of specifying all the lines of that port in order.
const int32 lineGrouping = DAQmx_Val_ChanForAllLines; // Specifies whether to group digital lines into one or more virtual channels. Options: DAQmx_Val_ChanPerLine, DAQmx_Val_ChanForAllLines

// DAQmxCreateWatchdogTimerTask Options
const char *deviceName = "Dev1"; // Specifies the name to assign to the device. If unspecified, NI-DAQmx chooses the device name.
const char *wdTaskName = "wd"; // The name to assign to the watchdog task.
const float64 wdTimeout = 0.01; // The time, in seconds, until the watchdog timer expires. A value of DAQmx_Val_WaitInfinitely indicates that the internal timer never expires.
const char *channelName = "Dev1/port0/line0:7"; // The digital line or port to modify.
const int32 expState = DAQmx_Val_High; // The state to which to set the digital physical channel when the watchdog timer expires. Options: DAQmx_Val_High, DAQmx_Val_Low, DAQmx_Val_Tristate, DAQmx_Val_NoChange

// DAQmxWriteDigitalLines Options
const int32 numSampsPerChan = 1; // The number of samples, per channel, to write. You must pass in a value of 0 or more in order for the sample to write.
const bool32 autoStart = 1; // Specifies whether or not this function automatically starts the task if you do not start it.
const float64 timeout = 10.0; // The amount of time, in seconds, to wait for this function to write all the samples. To specify an infinite wait, pass -1 (DAQmx_Val_WaitInfinitely). 
const bool32 dataLayout = DAQmx_Val_GroupByChannel; // Specifies how the samples are arranged, either interleaved or noninterleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

// DAQmxControlWatchdogTask Options
const int32 action = DAQmx_Val_ResetTimer; // Specifies how to control the watchdog task. Options: DAQmx_Val_ResetTimer, DAQmx_Val_ClearExpiration

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0,wdTaskHandle=0;
	uInt8       data[8]={1,1,1,1,1,1,1,1};
	char        errBuff[2048]={'\0'};
	int32 		numWritten;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDOChan(taskHandle,lines,"",lineGrouping));
	DAQmxErrChk (DAQmxCreateWatchdogTimerTask(deviceName,wdTaskName,&wdTaskHandle,wdTimeout,channelName,expState,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));
	DAQmxErrChk (DAQmxStartTask(wdTaskHandle));

	printf("Continuously writing. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		/*********************************************/
		// DAQmx Write Code
		/*********************************************/
		DAQmxErrChk (DAQmxWriteDigitalLines(taskHandle,numSampsPerChan,autoStart,timeout,dataLayout,data,&numWritten,NULL));
		DAQmxErrChk (DAQmxControlWatchdogTask(wdTaskHandle,DAQmx_Val_ResetTimer));
   	}

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);

	/*********************************************/
	// DAQmx Stop Code
	/*********************************************/
	DAQmxClearTask(taskHandle);
	DAQmxClearTask(wdTaskHandle);

	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit\n");
	getchar();
	return 0;
}
