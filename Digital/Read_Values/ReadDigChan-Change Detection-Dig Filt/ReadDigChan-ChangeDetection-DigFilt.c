/*********************************************************************
*
* ANSI C Example program:
*    ReadDigChan-ChangeDetection-DigFilt.c
*
* Example Category:
*    DI
*
* Description:
*    This example demonstrates how to read values from one or more
*    digital input channels using change detection and digital
*    filters.
*
* Instructions for Running:
*    1. Select the digital lines on the DAQ device to be read.
*    2. Select the digital lines on which to enable digital
*       filtering.
*    Note: The lines selected for digital filtering must be present
*          in the task.
*    3. Select the minimum pulse width the digital filter should
*       allow to pass.
*    4. Select the rising and falling edge lines on which to perform
*       change detection.
*    5. Select the number of samples per read.
*
* Steps:
*    1. Create a task.
*    2. Create a Digital Input channel. Use one channel for all
*       lines. You can alternatively use one channel for each line,
*       but then use a different version of the DAQmx Read function.
*    3. Setup the Change Detection timing for the acquisition. The
*       timing is set to read continuously. The Rising Edge Lines and
*       Falling Edge Lines specify on which digital lines a change
*       causes a sample to be read.
*    4. Specify which lines have digital filtering enabled and the
*       minimum pulse width of the filter. Since the default value
*       for enable is different across devices, first disable digital
*       filtering on all lines and then enable digital filtering on
*       lines the user chooses.
*    5. Call the Start function to start the task.
*    6. Read the digital data continuously until the user hits the
*       stop button or an error occurs. This read function reads
*       Samples Per Read samples of digital data every time. Also set
*       a timeout so an error is returned if the samples are not
*       returned within the specified time limit. Because this
*       example uses change detection, 10 seconds was chosen as an
*       arbitrarily large number to allow for changes to occur.
*    7. Call the Clear Task function to clear the task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminals match the Lines I/O
*    Control. In this case wire your digital signals to the first
*    eight digital lines on your DAQ Device.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int32       error=0;
	TaskHandle  taskHandle=0;
	uInt8       data[32];
	char        errBuff[2048]={'\0'};
	int32       numRead;
	int32		bytesPerSamp;

   	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDIChan(taskHandle,"Dev1/port0/line0:7","",DAQmx_Val_ChanPerLine));
	DAQmxErrChk (DAQmxCfgChangeDetectionTiming(taskHandle,"Dev1/port0/line0:7","Dev1/port0/line0:7",DAQmx_Val_ContSamps,4));
	DAQmxErrChk (DAQmxSetDIDigFltrEnable(taskHandle, "Dev1/port0/line0,Dev1/port0/line5", 0));
	DAQmxErrChk (DAQmxSetDIDigFltrMinPulseWidth(taskHandle, "Dev1/port0/line0,Dev1/port0/line5", 0.0001));
	DAQmxErrChk (DAQmxSetDIDigFltrEnable(taskHandle, "Dev1/port0/line0,Dev1/port0/line5", 1));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Continuously reading. Press Ctrl+C to interrupt\n");
	while( 1 )
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadDigitalLines(taskHandle,4,10.0,DAQmx_Val_GroupByScanNumber,data,32,&numRead,&bytesPerSamp,NULL));

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
