/*********************************************************************
*
* ANSI C Example program:
*    DigFreq-Buff-Cont-LargeRange2Ctr.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to continuously measure buffered
*    frequency using two counters on a Counter Input Channel. The
*    Divisor, Maximum and Minimum Frequency Values, and the Edge
*    Parameter are configurable.
*
*    This example shows how to measure frequency on the counter's
*    default input terminal (refer to the I/O Connections Overview
*    section below for more information), but could easily be
*    expanded to measure frequency on any PFI, RTSI, or internal
*    signal. Additionally, this example could be extended to measure
*    frequency with other measurement methods for different frequency
*    and quantization error requirements.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure frequency on the DAQ device.
*    2. Enter the Measurement Divisor which specifies how many
*       periods of the unkown signal are used to calculated the
*       frequency. Additionally, you can change the input terminal
*       where the frequency is measured using the DAQmx Set Channel
*       Attribute function.
*    Note: It is important to set the Maximum and Minimum Values of
*          your unknown frequency as accurately as possible so the
*          best internal timebase can be chosen to minimize
*          measurement error. The default values specify a range that
*          can be measured by the counter using the 20MHzTimebase Use
*          the Gen Dig Pulse Train-Continuous example to verify that
*          you are measuring correctly on the DAQ device.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Input channel for Frequency. The Edge
*       parameter is used to determine if the counter will begin
*       measuring on a rising or falling edge. The Divisor specifies
*       how many periods of the unknown signal are used to calculate
*       the frequency. The higher this is, the more accurate your
*       measurement will be, it will also take the measurement
*       longer. It is important to set the Maximum and Minimum Values
*       of your unknown frequency as accurately as possible so the
*       best internal timebase can be chosen to minimize measurement
*       error. The default values specify a range that can be
*       measured by the counter using the 20MHzTimebase
*    3. Call the DAQmx Timing function (Implicit) to configure the
*       Sample Mode and Samples per Channel.

*    Note: For time measurements with counters, the implicit timing
*          function is used because the signal being measured itself
*          determines the sample rate.
*    4. Call the Start function to arm the counter and begin
*       measuring.
*    5. For continuous measurements, the counter will continually
*       read all available data until Stop button is pressed.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure frequency on the input terminal of the
*    counter specified in the Physical Channel I/O control.
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
	DAQmxErrChk (DAQmxCreateCIFreqChan(taskHandle,"Dev1/ctr0","",100000,1000000,DAQmx_Val_Hz,DAQmx_Val_Rising,DAQmx_Val_LargeRng2Ctr,0.001,10,""));
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
