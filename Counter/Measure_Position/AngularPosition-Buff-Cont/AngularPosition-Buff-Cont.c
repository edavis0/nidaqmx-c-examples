/*********************************************************************
*
* ANSI C Example program:
*    AngularPosition-Buff-Cont.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to measure angular position using
*    a quadrature encoder on a Counter Input Channel. The Decoding
*    Type, Pulses Per Revolution, Z Index Enable, Z Index Phase, Z
*    Index Value, and Sample Clock Source are all configurable.

*
*    Position is measured on the counter's default A, B, and Z input
*    terminals (refer to the I/O Connections Overview section below
*    for more information).
*
*    Note: For buffered position measurement, an external sample
*          clock is necessary to signal when a sample should be
*          inserted into the buffer. This is set by the Sample Clock
*          Source control.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure position on the DAQ device.
*    2. Enter the Decoding Type, Pulses Per Revolution, Z Index
*       Enable, Z Index Phase, Z Index Value to specify how you want
*       the counter to measure position. Set the Sample Clock Source.

*    Note: An external sample clock must be used. Counters do not
*          have an internal sample clock available. You can use the
*          Dig Pulse Train-Continuous example to generate a pulse
*          train on another counter and connect it to the Sample
*          Clock Source you are using in this example.
*    3. Set the Samples to Read control. This will determine how many
*       samples are read each time.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Input channel for Angular Encoder. The
*       Decoding Type, Pulses Per Revolution, Z Index Enable, Z Index
*       Phase, Z Index Value parameters are used to determine how the
*       counter should measure position.
*    3. Call the DAQmx Timing function (Sample Clock) to configure
*       the external sample clock timing parameters such as Sample
*       Mode and Sample Clock Source. The Sample Clock Source
*       determines when a sample will be inserted into the buffer.
*       The 100kHz, 20MHz, and 80MHz timebases cannot be used as the
*       Sample Clock Source. The Edge parameter can be used to
*       determine when a sample is taken.
*    4. Call the Start function to arm the counter and begin
*       measuring position. The counter will be preloaded with the
*       Initial Angle.
*    5. For continuous measurements, the counter will continually
*       read all available data until the Stop button is pressed.
*    6. Call the Clear Task function to clear the Task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure position on the A, B, and Z Input
*    Terminals of the counter specified in the Physical Channel I/O
*    control.
*
*    Position measurement only works with TIO counters.

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
	DAQmxErrChk (DAQmxCreateCIAngEncoderChan(taskHandle,"Dev1/ctr0","",DAQmx_Val_X4,0,0.0,DAQmx_Val_AHighBHigh,DAQmx_Val_Degrees,24,0.0,""));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"/Dev1/PFI9",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

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
