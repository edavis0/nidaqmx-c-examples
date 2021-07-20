/*********************************************************************
*
* ANSI C Example program:
*    DigFreq-Buff-Cont-ExtClk-ArmStart.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to continually measure the
*    frquency on a Counter Input Channel with a sample clock and arm
*    start trigger.

*
*    This example shows how to measure frequency with a counter on
*    any PFI, RTSI, or internal signal.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the counter
*       you want to measure frequency on the DAQ device.
*    2. Enter the Maximum and Minimum Value to specify the range or
*       your unknown pulse frequency.

*    Note: It is important to set the Maximum and Minimum Values of
*          your unknown pulse width as accurately as possible so the
*          best internal timebase can be chosen to minimize
*          measurement error. Use the Gen Dig Pulse Train-Continuous
*          example to verify that you are measuring correctly on the
*          DAQ device.
*    3. Select the frequency input channel.
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
*    3. Call the DAQmx Timing function (Sample Clock) to configure
*       the external sample clock timing parameters such as Sample
*       Mode and Sample Clock Source. The Sample Clock Source
*       determines when a sample will be inserted into the buffer.
*       The Edge parameter can be used to determine when a sample is
*       taken.
*    Note: If the sample clock frequency is not half or less of the
*          frequency measured you may encounter a over run.
*    4. Enable the Arm Start Trigger and set the source to be the
*       sample clock. This ensures that for a free running sample
*       clock the counter arms on a sample clock edge and then
*       recognizes the next sample clock. This keeps the counter arm
*       from happening close enough to the first sample clock to
*       cause an over run condition. You can also start the clock
*       after the measurement start.
*    5. Call the Start function to arm the counter and begin
*       measuring.
*    6. For continuous measurements, the counter will continually
*       read all available data until Stop button is pressed.
*    7. Call the Clear Task function to clear the task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    The counter will measure frequency on the input terminal of the
*    counter specified in the Physical Channel I/O control.
*
*    In this example the frequency will be measured on PFI0 with
*    ctr0.

*
*    To determine what the default counter pins for your device are
*    or to set a different source (or gate) pin, refer to the
*    Connecting Counter Signals topic in the NI-DAQmx Help (search
*    for "Connecting Counter Signals").
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
	DAQmxErrChk (DAQmxCreateCIFreqChan(taskHandle,"Dev1/ctr0","",200,1000000,DAQmx_Val_Hz,DAQmx_Val_Rising,DAQmx_Val_LowFreq1Ctr,0.001,10,""));
	DAQmxErrChk (DAQmxSetCIFreqTerm(taskHandle,"Dev1/ctr0","/Dev1/PFI0"));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"/Dev1/PFI1",100,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));
	DAQmxErrChk (DAQmxSetArmStartTrigType(taskHandle,DAQmx_Val_DigEdge));
	DAQmxErrChk (DAQmxSetDigEdgeArmStartTrigSrc(taskHandle,"/Dev1/PFI1"));
	DAQmxErrChk (DAQmxSetDigEdgeArmStartTrigEdge(taskHandle,DAQmx_Val_Rising));

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
