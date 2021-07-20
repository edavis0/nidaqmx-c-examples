/*********************************************************************
*
* ANSI C Example program:
*    DSASharedTimebaseAndTrig-AIandAO.c
*
* Example Category:
*    Sync
*
* Description:
*    This example configures a total of four tasks on two PXI Dynamic
*    Signal Acquistion (DSA) devices. Each device runs a task for
*    analog input and another for analog output. This VI illustrates
*    continuous analog input and ouptut operations.
*
* Instructions for Running:
*    1. Select which type of Syncrhonization method you want to use.
*    2. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    3. Enter the minimum and maximum expected voltage for the input
*       and output operations on each device.
*    Note: For optimal accuracy, match the input range to the
*          expected voltage level of the measured signal (for input)
*          and generated signal (for output).
*    4. Set the number of samples to acquire per channel. This
*       parameter determines how many points will be read and
*       generated with each iteration.
*    5. Set the rate of the acquisition and generation. The same
*       sampling rate is employed for input and output operations on
*       each device.
*    Note: The sampling rate should be at least 2.2 times the maximum
*          frequency component of the signal being acquired on analog
*          input and the signal being generated on analog output.
*          Frequency components beyond about 0.47 times the sampling
*          rate will be eliminated by the alias and imaging
*          protection on the DSA device.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input and output channel for both the master
*       and slave devices.
*    3. Set timing parameters for a continuous acquisition. The
*       sample rate and block size are set to the same values for
*       each device.
*    4. There are two types of synchronization available on DSA
*       devices. The first method shares the Sample Clock Timebase
*       across the PXI_Star bus. It also uses the Sync Pulse which is
*       shared across the PXI_Trig / RTSI bus. The second method uses
*       the Sync Pulse in conjunction with the PXI Reference clock 10
*       on the PXI backplane.
*    5. Call the Get Terminal Name with Device Prefix utility
*       function. This will take a Task and a terminal and create a
*       properly formatted device + terminal name. This signal is
*       then used as an output generation trigger on the master as
*       well as and acquisition start trigger and generation start
*       trigger on the slave. The signal is shared along the PXI_Trig
*       / RTSI bus.
*    6. Synthesize a standard waveform (sine, square, or triangle)
*       and load this data into the output RAM buffer for each
*       device.
*    7. Call the Start function to start the acquisition.
*    8. Read all of the data continuously. The 'Samples per Channel'
*       control will specify how many samples per channel are read
*       each time. If either device reports an error or the user
*       presses the 'Stop' button, the acquisition will stop.
*    9. Call the Clear Task function to clear the task.
*    10. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input and output terminals matches the
*    Physical Channel I/O controls.
*
*    It is important to ensure that your PXI chassis has been
*    properly configured in MAX. If your chassis has not been
*    configured in software before running the VI, your devices may
*    fail to synchronize properly.
*
*********************************************************************/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <NIDAQmx.h>

static TaskHandle gMasterAItaskHandle=0,gMasterAOtaskHandle=0,gSlaveAItaskHandle=0,gSlaveAOtaskHandle=0;


#define PI	3.1415926535

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

int GenSineWave(int numElements, double amplitude, double frequency, double *phase, double sineWave[]);
static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[]);

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);

int main(void)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};
	char    timebaseSrc[1024],syncPulseSrc[1024],startTrigger[1024];
	float64 masterWriteData[250],slaveWriteData[250];
	int     synchronizationType=0;	/* 0 = Sample Clock; 1 = Reference Clock */
	double  phase=0.0;
	int32   numMasterWritten,numSlaveWritten;

	/*********************************************/
	// DAQmx Configure Tasks Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("Master input task",&gMasterAItaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(gMasterAItaskHandle,"Dev1/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,0));
	DAQmxErrChk (DAQmxCfgSampClkTiming(gMasterAItaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	DAQmxErrChk (DAQmxCreateTask("Master output task",&gMasterAOtaskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(gMasterAOtaskHandle,"Dev1/ao0","",-10.0,10.0,DAQmx_Val_Volts,0));
	DAQmxErrChk (DAQmxCfgSampClkTiming(gMasterAOtaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	DAQmxErrChk (DAQmxCreateTask("Slave input task",&gSlaveAItaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(gSlaveAItaskHandle,"Dev2/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,0));
	DAQmxErrChk (DAQmxCfgSampClkTiming(gSlaveAItaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	DAQmxErrChk (DAQmxCreateTask("Slave output task",&gSlaveAOtaskHandle));
	DAQmxErrChk (DAQmxCreateAOVoltageChan(gSlaveAOtaskHandle,"Dev2/ao0","",-10.0,10.0,DAQmx_Val_Volts,0));
	DAQmxErrChk (DAQmxCfgSampClkTiming(gSlaveAOtaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_ContSamps,1000));

	/*********************************************/
	// DAQmx Clock Synchronization Code
	/*********************************************/
	if( synchronizationType ) {
		/* 
			Note: Not all DSA devices support reference clock synchronization. Refer to
			your hardware device manual for further information on whether this method
			of synchronization is supported for your particular device.
		 */
		DAQmxErrChk (DAQmxSetRefClkSrc(gMasterAItaskHandle,"PXI_Clk10"));
		DAQmxErrChk (DAQmxSetRefClkSrc(gMasterAOtaskHandle,"PXI_Clk10"));
		DAQmxErrChk (DAQmxSetRefClkSrc(gSlaveAItaskHandle,"PXI_Clk10"));
		DAQmxErrChk (DAQmxSetRefClkSrc(gSlaveAOtaskHandle,"PXI_Clk10"));
	} else {
		/*  Note: If you are using PXI DSA devices, the master device must reside in PXI Slot 2. */
		DAQmxErrChk (GetTerminalNameWithDevPrefix(gMasterAItaskHandle,"SampleClockTimebase",timebaseSrc));
		DAQmxErrChk (DAQmxSetSampClkTimebaseSrc(gMasterAOtaskHandle,timebaseSrc));
		DAQmxErrChk (DAQmxSetSampClkTimebaseSrc(gSlaveAItaskHandle,timebaseSrc));
		DAQmxErrChk (DAQmxSetSampClkTimebaseSrc(gSlaveAOtaskHandle,timebaseSrc));
	}

	DAQmxErrChk (GetTerminalNameWithDevPrefix(gMasterAItaskHandle,"SyncPulse",syncPulseSrc));
	DAQmxErrChk (DAQmxSetSyncPulseSrc(gMasterAOtaskHandle,syncPulseSrc));
	DAQmxErrChk (DAQmxSetSyncPulseSrc(gSlaveAItaskHandle,syncPulseSrc));
	DAQmxErrChk (DAQmxSetSyncPulseSrc(gSlaveAOtaskHandle,syncPulseSrc));

	/*********************************************/
	// DAQmx Configure Start Trigger Code
	/*********************************************/
	DAQmxErrChk (GetTerminalNameWithDevPrefix(gMasterAItaskHandle,"ai/StartTrigger",startTrigger));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(gMasterAOtaskHandle,startTrigger,DAQmx_Val_Rising));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(gSlaveAItaskHandle,startTrigger,DAQmx_Val_Rising));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(gSlaveAOtaskHandle,startTrigger,DAQmx_Val_Rising));

	GenSineWave(250,1.0,0.02,&phase,masterWriteData);
	GenSineWave(250,1.0,0.02,&phase,slaveWriteData);

	/*********************************************/
	// DAQmx Write Code
	/*********************************************/
	DAQmxErrChk (DAQmxWriteAnalogF64(gMasterAOtaskHandle,250,0,10.0,DAQmx_Val_GroupByChannel,masterWriteData,&numMasterWritten,0));
	DAQmxErrChk (DAQmxWriteAnalogF64(gSlaveAOtaskHandle,250,0,10.0,DAQmx_Val_GroupByChannel,slaveWriteData,&numSlaveWritten,0));

	DAQmxErrChk (DAQmxRegisterEveryNSamplesEvent(gMasterAItaskHandle,DAQmx_Val_Acquired_Into_Buffer,1000,0,EveryNCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(gMasterAItaskHandle,0,DoneCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(gMasterAOtaskHandle,0,DoneCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(gSlaveAItaskHandle,0,DoneCallback,NULL));
	DAQmxErrChk (DAQmxRegisterDoneEvent(gSlaveAOtaskHandle,0,DoneCallback,NULL));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(gMasterAOtaskHandle));
	DAQmxErrChk (DAQmxStartTask(gSlaveAItaskHandle));
	DAQmxErrChk (DAQmxStartTask(gSlaveAOtaskHandle));
	DAQmxErrChk (DAQmxStartTask(gMasterAItaskHandle)); /* trigger task must be started last */

	printf("Acquiring samples continuously. Press Enter to interrupt\n");
	printf("\nRead:\tMaster\tSlave\tTotal:\tMaster\tSlave\n");
	getchar();

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);

	/*********************************************/
	// DAQmx Cleanup Code
	/*********************************************/
	if( gMasterAItaskHandle ) {
		DAQmxStopTask(gMasterAItaskHandle);
		DAQmxClearTask(gMasterAItaskHandle);
		gMasterAItaskHandle = 0;
	}
	if( gMasterAOtaskHandle ) {
		DAQmxStopTask(gMasterAOtaskHandle);
		DAQmxClearTask(gMasterAOtaskHandle);
		gMasterAOtaskHandle = 0;
	}
	if( gSlaveAItaskHandle ) {
		DAQmxStopTask(gSlaveAItaskHandle);
		DAQmxClearTask(gSlaveAItaskHandle);
		gSlaveAItaskHandle = 0;
	}
	if( gSlaveAOtaskHandle ) {
		DAQmxStopTask(gSlaveAOtaskHandle);
		DAQmxClearTask(gSlaveAOtaskHandle);
		gSlaveAOtaskHandle = 0;
	}
	
	if (DAQmxFailed(error))
		printf("DAQmx Error: %s\n", errBuff);
	printf("End of program, press Enter key to quit");
	getchar();
	return 0;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData)
{
	int32           error=0;
	char            errBuff[2048]={'\0'};
	static int32    masterTotal=0,slaveTotal=0;
	int32           masterRead,slaveRead;
	float64         masterData[2000],slaveData[2000];


	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(gMasterAItaskHandle,2000,10.0,DAQmx_Val_GroupByChannel,masterData,2000,&masterRead,NULL));
	DAQmxErrChk (DAQmxReadAnalogF64(gSlaveAItaskHandle,2000,10.0,DAQmx_Val_GroupByChannel,slaveData,2000,&slaveRead,NULL));
	
	if( masterRead>0 )
		masterTotal += masterRead;
	if( slaveRead>0 )
		slaveTotal += slaveRead;
	printf("\t%d\t%d\t\t%d\t%d\r",(int)masterRead,(int)slaveRead,(int)masterTotal,(int)slaveTotal);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		/*********************************************/
		// DAQmx Stop Code
		/*********************************************/
		if( gMasterAItaskHandle ) {
			DAQmxStopTask(gMasterAItaskHandle);
			DAQmxClearTask(gMasterAItaskHandle);
			gMasterAItaskHandle = 0;
		}
		if( gMasterAOtaskHandle ) {
			DAQmxStopTask(gMasterAOtaskHandle);
			DAQmxClearTask(gMasterAOtaskHandle);
			gMasterAOtaskHandle = 0;
		}
		if( gSlaveAItaskHandle ) {
			DAQmxStopTask(gSlaveAItaskHandle);
			DAQmxClearTask(gSlaveAItaskHandle);
			gSlaveAItaskHandle = 0;
		}
		if( gSlaveAOtaskHandle ) {
			DAQmxStopTask(gSlaveAOtaskHandle);
			DAQmxClearTask(gSlaveAOtaskHandle);
			gSlaveAOtaskHandle = 0;
		}
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData)
{
	int32   error=0;
	char    errBuff[2048]={'\0'};

	// Check to see if an error stopped the task.
	DAQmxErrChk (status);

Error:
	if( DAQmxFailed(error) ) {
		DAQmxGetExtendedErrorInfo(errBuff,2048);
		DAQmxClearTask(taskHandle);
		if( gMasterAItaskHandle ) {
			DAQmxStopTask(gMasterAItaskHandle);
			DAQmxClearTask(gMasterAItaskHandle);
			gMasterAItaskHandle = 0;
		}
		if( gMasterAOtaskHandle ) {
			DAQmxStopTask(gMasterAOtaskHandle);
			DAQmxClearTask(gMasterAOtaskHandle);
			gMasterAOtaskHandle = 0;
		}
		if( gSlaveAItaskHandle ) {
			DAQmxStopTask(gSlaveAItaskHandle);
			DAQmxClearTask(gSlaveAItaskHandle);
			gSlaveAItaskHandle = 0;
		}
		if( gSlaveAOtaskHandle ) {
			DAQmxStopTask(gSlaveAOtaskHandle);
			DAQmxClearTask(gSlaveAOtaskHandle);
			gSlaveAOtaskHandle = 0;
		}
		printf("DAQmx Error: %s\n",errBuff);
	}
	return 0;
}

int GenSineWave(int numElements, double amplitude, double frequency, double *phase, double sineWave[])
{
	int i=0;

	for(;i<numElements;++i)
		sineWave[i] = amplitude*sin(PI/180.0*(*phase+360.0*frequency*i));
	*phase = fmod(*phase+frequency*360.0*numElements,360.0);
	return 0;
}

static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[])
{
	int32	error=0;
	char	device[256];
	int32	productCategory;
	uInt32	numDevices,i=1;

	DAQmxErrChk (DAQmxGetTaskNumDevices(taskHandle,&numDevices));
	while( i<=numDevices ) {
		DAQmxErrChk (DAQmxGetNthTaskDevice(taskHandle,i++,device,256));
		DAQmxErrChk (DAQmxGetDevProductCategory(device,&productCategory));
		if( productCategory!=DAQmx_Val_CSeriesModule && productCategory!=DAQmx_Val_SCXIModule ) {
			*triggerName++ = '/';
			strcat(strcat(strcpy(triggerName,device),"/"),terminalName);
			break;
		}
	}

Error:
	return error;
}
