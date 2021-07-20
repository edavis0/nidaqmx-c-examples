/*********************************************************************
*
* ANSI C Example program:
*    DigPeriods-Buff-Cont-HighFreq2Ctr.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to continuously measure buffered
*    periods using two counters on a Counter Input Channel.
*
*    This example shows how to measure period on the counter's
*    default input terminal (refer to the I/O Connections Overview
*    section below for more information), but could easily be
*    expanded to measure period on any PFI, RTSI, or internal signal.
*    Additionally, this example could be extended to measure period
*    with other measurement methods for different frequency and
*    quantization error requirements.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure period on the DAQ device.
*    2. Enter the Measurement Time which specifies how often a period
*       is calculated by counting the number of edges that have
*       passed in the elapsed time. Additionally, you can change the
*       input terminal where the period is measured by setting the
*       appropriate channel attribute.
*    Note: Use the Gen Dig Pulse Train-Continuous example to verify
*          that you are measuring correctly on the DAQ device.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Input channel to measure Period. The Edge
*       parameter is used to determine if the counter will begin
*       measuring on a rising or falling edge. The Measurement Time
*       specifies how often a period is calculated by counting the
*       number of edges that have passed in the elapsed time.
*    Note: The Maximum and Minimum Values are not used when measuring
*          period using the High Frequency 2 Ctr Method.
*    3. Call the DAQmx Timing function (Implicit) to configure the
*       Sample Mode and Samples per Channel.

*    Note: For time measurements with counters, the implicit timing
*          function is used because the signal being measured itself
*          determines the sample rate.
*    4. Call the Start function to arm the counter and begin
*       measuring.
*    5. For continuous measurements, the counter will continually
*       read all available data until the stop button is pressed.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure period on input terminal of the counter
*    specified in the Physical Channel I/O control.
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
	DAQmxErrChk (DAQmxCreateCIPeriodChan(taskHandle,"Dev1/ctr0","",0.000001,0.100000,DAQmx_Val_Seconds,DAQmx_Val_Rising,DAQmx_Val_HighFreq2Ctr,0.000100,4,""));
	DAQmxErrChk (DAQmxCfgImplicitTiming(taskHandle,DAQmx_Val_ContSamps,1000));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Continuously reading. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadCounterF64(taskHandle,1000,10.0,data,1000,&read,0));

		printf("Acquired %d samples\n",(int)read);
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
