/*********************************************************************
*
* ANSI C Example program:
*    PulseWidth-Buff-SampClk-Cont.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to continually measure pulse
*    widths on a Counter Input Channel using an external sample
*    clock. The Maximum and Minimum Values, Sample Clock Source, and
*    Samples per Channel are all configurable.
*
*    This example shows how to measure pulse width on the counter's
*    default input terminal (refer to section IV, I/O Connections
*    Overview, below for more information), but could easily be
*    expanded to measure pulse width on any PFI, RTSI, or internal
*    signal.
*
*    Note: For sample clock measurements, an external sample clock is
*          necessary to signal when the counter should measure a
*          sample. This is set by the Sample Clock Source control.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure pulse width on the DAQ device.
*    2. Enter the Maximum and Minimum Value to specify the range or
*       your unknown pulse width.

*    Note: It is important to set the Maximum and Minimum Values of
*          your unknown pulse width as accurately as possible so the
*          best internal timebase can be chosen to minimize
*          measurement error. The default values specify a range that
*          can be measured by the counter using the 20MhzTimebase.
*          Use the Gen Dig Pulse Train-Continuous example to verify
*          that you are measuring correctly on the DAQ device.
*    3. Set the Sample Clock Source and Samples per Channel to
*       configure timing for the measurement.

*    Note: An external sample clock must be used. Counters do not
*          have an internal sample clock available. You can use the
*          Gen Dig Pulse Train-Continuous example to generate a pulse
*          train on another counter and connect it to the Sample
*          Clock Source you are using in this example.
*
* Steps:
*    1. Create a task.
*    2. Create a Counter Input channel to Pulse Width. It is
*       important to set the Maximum and Minimum Values of your
*       unknown pulse width as accurately as possible so the best
*       internal timebase can be chosen to minimize measurement
*       error. The default values specify a range that can be
*       measured by the counter using the 20MhzTimebase.
*    3. Call the DAQmx Timing function (Sample Clock) to configure
*       the external sample clock timing parameters such as Sample
*       Mode and Sample Clock Source. The Sample Clock Source
*       determines when a sample will be inserted into the buffer.
*       The Edge parameter can be used to determine when a sample is
*       taken.
*    4. Call the Start function to arm the counter and begin
*       measuring.
*    5. Call the Read function continuously to read the pulse width
*       measurements.
*    6. Call the Clear Task function to clear the task.
*    7. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure pulses on the input terminal of the
*    counter specified in the Physical Channel I/O control.
*
*    In this example the pulse width will be measured on the default
*    input terminal on ctr0. The counter will take measurements on
*    valid edges of the external Sample Clock Source.
*
*    For more information on the default counter input and output
*    terminals for your device, open the NI-DAQmx Help, and refer to
*    Counter Signal Connections found under the Device Considerations
*    book in the table of contents.
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
	DAQmxErrChk (DAQmxCreateCIPulseWidthChan(taskHandle,"Dev1/ctr0","",0.000000100,0.838860750,DAQmx_Val_Seconds,DAQmx_Val_Rising,""));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"/Dev1/PFI0",1000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

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
