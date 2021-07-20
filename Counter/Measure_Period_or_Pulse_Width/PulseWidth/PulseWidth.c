/*********************************************************************
*
* ANSI C Example program:
*    PulseWidth.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to measure pulse width on a
*    Counter Input Channel. The Starting Edge, Minimum Value and
*    Maximum Value are all configurable.

*
*    This example shows how to measure pulse width on the counter's
*    default input terminal (refer to the I/O Connections Overview
*    section below for more information), but could easily be
*    expanded to measure pulse width on any PFI, RTSI, or internal
*    signal.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure pulse width on the DAQ device.
*    2. Enter the Measurement Edge to specify what type of pulse you
*       want the counter to measure. Enter the Maximum and Minimum
*       Value to specify the range of your unknown pulse width.
*       Additionally, you can change the input terminal where the
*       pulse width is measured using by setting the appropriate
*       channel attribute.
*    Note: It is important to set the Maximum and Minimum Values of
*          your unknown pulse as accurately as possible so the best
*          internal timebase can be chosen to minimize measurement
*          error. The default values specify a range that can be
*          measured by the counter using the 20MHzTimebase. Use the
*          Gen Dig Pulse example to verify that you are measuring
*          correctly on the DAQ device.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Input channel to measure Pulse Width. The
*       Edge parameter is used to determine if the counter will
*       measure high or low pulses. It is important to set the
*       Maximum and Minimum Values of your unknown pulse as
*       accurately aspossible so the best internal timebase can be
*       chosen to minimize measurement error. The default values
*       specify a range that can be measured by the counter using the
*       20MHzTimebase.
*    3. Call the Start function to arm the counter and begin
*       measuring.
*    4. Call the Read function to return the next pulse width
*       measurement. Set a timeout so an error is returned if a pulse
*       is not returned in the specified time limit.
*    5. Call the Clear Task function to clear the task.
*    6. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure pulses on the input terminal of the
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
	float64     data[1];
	char        errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateCIPulseWidthChan(taskHandle,"Dev1/ctr0","",0.000000100,0.830000000,DAQmx_Val_Seconds,DAQmx_Val_Rising,""));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadCounterScalarF64(taskHandle,10.0,&data[0],0));

	printf("Measured Pulse Width: %.9f sec\n",data[0]);

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
