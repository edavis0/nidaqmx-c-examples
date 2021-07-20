/*********************************************************************
*
* ANSI C Example program:
*    ContReadDigChan-PipeSampClkwHshk.c
*
* Example Category:
*    DI
*
* Description:
*    This examples demostrates how to interface the NI 6536/7 to a
*    synchonous FIFO.
*
* Instructions for Running:
*    1. Select the Physical Channels that correspond to where your
*       signal is input on the device.
*    2. Enter the number of Samples Clock Rate. This is the number of
*       samples that will be read every time the DAQmx Read function
*       is called.
*    3. Specify whether the ready for transfer signal is active high
*       or active low.
*    4. Specify the Ready for Transfer Output Terminal.
*    5. Specify the Ready for Transfer Deassert Condition Threshold.
*       This specifies in samples the threshold below which the Ready
*       for Transfer Event deasserts.
*    6. Specify the Pause Trigger Polarity. This parameter tells this
*       device when to pause. If the polarity is set to High, then
*       the device will pause when the corresponding PFI line is
*       high. Note, that the device will not pause on the next sample
*       clock edge because of pipelining.
*    7. Specify the Pause Trigger Source Terminal.
*
* Steps:
*    1. Create a task.
*    2. Create one Digital Input channel for each Digital Line in the
*       Task.
*    3. Call the DAQmxCfgPipelinedSampClkTiming function which
*       configures the device for Pipelined Sample Clock.
*    4. Configure the pause trigger.
*    5. Configure the ready for transfer event. You need to configure
*       the ready for transfer deassert threshold to correspond to
*       how many samples it takes for the device connected to the NI
*       6536/7 to pause the data transfer.
*    6. Disallow Overwrites. When overwrites are disallowed, the data
*       transfer between the device and the DAQmx buffer will pause
*       when the DAQmx buffer is full. It will resume when more space
*       is available in the buffer.
*    7. Call the Start function.
*    8. Read the data from the DAQmx buffer continuously. This will
*       free space in the buffer.
*    9. Call the Clear Task function to clear the Task.
*    10. Display an error if any.
*
* I/O Connections Overview:
*    Connect the FIFO's Not Empty Flag to the Pause Trigger. Connect
*    the FIFO's Read Enable signal to the Ready for transfer Event.
*    Connect the FIFO's read clock to the sample clock terminal.
*    Connect the data lines from the NI 6536/7 to the data lines of
*    the FIFO.
*
*********************************************************************/

#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int main(void)
{
	int32		error=0;
	TaskHandle	taskHandle=0;
	uInt32		data[1000];
	int32		sampsRead,totalRead=0;
	char		errBuff[2048]={'\0'};

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateDIChan(taskHandle,"Dev1/port0/line0:7","",DAQmx_Val_ChanPerLine));
	DAQmxErrChk (DAQmxCfgPipelinedSampClkTiming(taskHandle,"",100000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	DAQmxErrChk (DAQmxSetPauseTrigType(taskHandle,DAQmx_Val_DigLvl));
	DAQmxErrChk (DAQmxSetExportedRdyForXferEventOutputTerm(taskHandle,"/Dev1/PFI0"));
	DAQmxErrChk (DAQmxSetExportedRdyForXferEventLvlActiveLvl(taskHandle,DAQmx_Val_ActiveLow));
	DAQmxErrChk (DAQmxSetExportedRdyForXferEventDeassertCond(taskHandle,DAQmx_Val_OnbrdMemCustomThreshold));
	DAQmxErrChk (DAQmxSetExportedRdyForXferEventDeassertCondCustomThreshold(taskHandle,256));
	DAQmxErrChk (DAQmxSetDigLvlPauseTrigSrc(taskHandle,"/Dev1/PFI11"));
	DAQmxErrChk (DAQmxSetDigLvlPauseTrigWhen(taskHandle,DAQmx_Val_High));
	DAQmxErrChk (DAQmxSetReadOverWrite(taskHandle,DAQmx_Val_DoNotOverwriteUnreadSamps));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Acquiring samples continuously. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadDigitalU32(taskHandle,1000,10.0,DAQmx_Val_GroupByChannel,data,1000,&sampsRead,NULL));

		if( sampsRead>0 ) {
			totalRead += sampsRead;
			printf("Acquired %d samples. Total %d\r",(int)sampsRead,(int)totalRead);
			fflush(stdout);
		}
	}
	printf("\nAcquired %d total samples.\n",(int)totalRead);

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
