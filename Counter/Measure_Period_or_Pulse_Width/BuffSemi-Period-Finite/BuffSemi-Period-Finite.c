/*********************************************************************
*
* ANSI C Example program:
*    BuffSemi-Period-Finite.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to measure a finite number of
*    semi-periods on a Counter Input Channel. The Minimum Value,
*    Maximum Value, and Samples per Channel are all configurable.
*
*    This example shows how to measure semi-period on the counter's
*    default input terminal (refer to the I/O Connections Overview
*    section below for more information), but could easily be
*    expanded to measure semi-period on any PFI, RTSI, or internal
*    signal.
*
*    Semi-period measurement differs from pulse width measurement in
*    that it measures both the high and the low pulses of a given
*    signal. So for every period, two data points will be returned.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure semi-period on the DAQ device.
*    2. Enter the Maximum and Minimum Value to specify the range of
*       your unknown semi-periods. Additionally, you can change the
*       input terminal where the semi-period is measured by setting
*       the appropriate channel attribute.
*    Note: It is important to set the Maximum and Minimum Values of
*          your unknown semi-period as accurately as possible so the
*          best internal timebase can be chosen to minimize
*          measurement error. The default values specify a range that
*          can be measured by the counter using the 20MHzTimebase.
*          Use the Dig Pulse Train-Continuous example to verify that
*          you are measuring correctly on the DAQ device.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Input channel to measure Semi-Period. It is
*       important to set the Maximum and Minimum Values of your
*       unknown period as accurately as possible so the best internal
*       timebase can be chosen to minimize measurement error. The
*       default values specify a range that can be measured by the
*       counter using the 20MHzTimebase. The Maximum Value will be
*       the largest amount of time between 2 adjacent edges. The
*       Minimum Value will be the smallest amount of time between 2
*       adjacent edges.
*    3. Call the DAQmx Timing function (Implicit) to configure the
*       Sample Mode and Samples per Channel.

*    Note: For time measurements with counters, the implicit timing
*          function is used because the signal being measured itself
*          determines the sample rate.
*    4. Call the Start function to arm the counter and begin
*       measuring.
*    5. For finite measurements, the counter will stop reading data
*       when the Samples per Channel have been received. For
*       continuous measurements, the counter will continually read
*       all available data until the Stop button is pressed.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure semi-period on the input terminal of
*    the counter specified in the Physical Channel I/O control.
*
*    This example uses the default source (or gate) terminal for the
*    counter of your device. To determine what the default counter
*    pins for your device are or to set a different source (or gate)
*    pin, refer to the Connecting Counter Signals topic in the
*    NI-DAQmx Help (search for "Connecting Counter Signals").
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int         error=0;
	TaskHandle  taskHandle=0;
	int32       read;
	float64     data[1000];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateCISemiPeriodChan(taskHandle,"Dev1/ctr0","",0.000000100,0.838860750,DAQmx_Val_Seconds,""));
	DAQmxErrChk (DAQmxCfgImplicitTiming(taskHandle,DAQmx_Val_FiniteSamps,1000));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadCounterF64(taskHandle,1000,10.0,data,1000,&read,0));

	printf("Acquired %d samples\n",(int)read);

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
