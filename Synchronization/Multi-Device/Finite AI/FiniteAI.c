/*********************************************************************
*
* ANSI C Example program:
*    FiniteAI.c
*
* Example Category:
*    Sync
*
* Description:
*    This example demonstrates how to acquire a finite amount of data
*    using the DAQ device's internal clock. It also shows how to
*    synchronize two devices for different device families (E Series,
*    S Series, M Series, and DSA), to simultaneously acquire the
*    data.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the DAQ device.
*    2. Enter the minimum and maximum voltage range.
*    Note: For better accuracy try to match the input range to the
*          expected voltage level of the measured signal.
*    3. Set the number of samples to acquire per channel.
*    4. Set the rate of the acquisition
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*    5. Choose which type of devices you are trying to synchronize.
*       This will select the correct synchronization method to use.
*    Note: If you choose the incorrect device an error is returned
*          specifying the attribute is not supported by the device or
*          is not applicable to the task.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input voltage channel for both the Master
*       and Slave devices.
*    3. Set timing parameters. Note that sample mode is set to Finite
*       Samples. In this example, the Rate and the Samples per
*       Channel are set the same for both devices, you can however
*       use different values for each device.
*    4. The synchronization method chosen depends on what type of
*       device you are using.
*    5. Call the Get Terminal Name with Device Prefix utility
*       function. This will take a Task and a terminal and create a
*       properly formatted device + terminal name to use as the
*       source of the Slaves Trigger. For the Slave, set the Source
*       for the trigger to the ai/StartTrigger of the Master Device.
*       This will ensure both devices start sampling at the same
*       time. (Note: The trigger is automatically routed through the
*       RTSI cable.)
*    6. Call the Start function to start the acquisition.
*    7. Read all of the waveform data from both devices.
*    8. Call the Clear Task function to clear the task.
*    9. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control.
*
*    If you have a PXI chassis, ensure it has been properly
*    identified in MAX. If you have devices with a RTSI bus, ensure
*    they are connected with a RTSI cable and that the RTSI cable is
*    registered in MAX.
*
*********************************************************************/

#include <string.h>
#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static int32 GetTerminalNameWithDevPrefix(TaskHandle taskHandle, const char terminalName[], char triggerName[]);

int main(void)
{
	int32       error=0;
	TaskHandle  masterTaskHandle=0,slaveTaskHandle=0;
	int32       numMasterRead,numSlaveRead;
	float64     masterData[1000];
	float64     slaveData[1000];
	char        errBuff[2048]={'\0'};
	char	    str1[256],str2[256],trigName[256];
	float64     clkRate;
	// synchType indicates what device family the devices you are synching belong to:
	// 0 : E series
	// 1 : M series (PCI)
	// 2 : M series (PXI)
	// 3 : DSA Sample Clock Timebase
	// 4 : DSA Reference Clock
	uInt32 synchType=0;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&masterTaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(masterTaskHandle,"PXI1Slot2/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(masterTaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,1000));
	DAQmxErrChk (DAQmxCreateTask("",&slaveTaskHandle));
	DAQmxErrChk (DAQmxCreateAIVoltageChan(slaveTaskHandle,"PXI1Slot3/ai0","",DAQmx_Val_Cfg_Default,-10.0,10.0,DAQmx_Val_Volts,NULL));
	DAQmxErrChk (DAQmxCfgSampClkTiming(slaveTaskHandle,"",10000.0,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,1000));
	switch (synchType)
	{
		case 0: // E & S Series Sharing Master Timebase
			// Note:  PXI 6115 and 6120 (S Series) devices don't require sharing of master timebase, 
			// because they auto-lock to Clock 10.  For those devices sharing a start trigger is adequate.
			// For the PCI-6154 S Series device use the M Series (PCI) synchronization type to synchronize 
			// using the reference clock. 
			DAQmxErrChk (DAQmxGetMasterTimebaseSrc(masterTaskHandle,str1,256));
			DAQmxErrChk (DAQmxGetMasterTimebaseRate(masterTaskHandle,&clkRate));
			DAQmxErrChk (DAQmxSetMasterTimebaseSrc(slaveTaskHandle,str1));
			DAQmxErrChk (DAQmxSetMasterTimebaseRate(slaveTaskHandle,clkRate));
			break;
		case 1: // M Series Sharing Reference Clock for PCI Devices
			DAQmxErrChk (DAQmxSetRefClkSrc(masterTaskHandle,"OnboardClock"));
			DAQmxErrChk (DAQmxGetRefClkSrc(masterTaskHandle,str1,256));
			DAQmxErrChk (DAQmxGetRefClkRate(masterTaskHandle,&clkRate));
			DAQmxErrChk (DAQmxSetRefClkSrc(slaveTaskHandle,str1));
			DAQmxErrChk (DAQmxSetRefClkRate(slaveTaskHandle,clkRate));
			break;
		case 2: // M Series Sharing Reference Clock for PXI Devices
			DAQmxErrChk (DAQmxSetRefClkSrc(masterTaskHandle,"PXI_Clk10"));
			DAQmxErrChk (DAQmxSetRefClkRate(masterTaskHandle,10000000.0));
			DAQmxErrChk (DAQmxSetRefClkSrc(slaveTaskHandle,"PXI_Clk10"));
			DAQmxErrChk (DAQmxSetRefClkRate(slaveTaskHandle,10000000.0));
			break;
		case 3: // DSA Sharing Sample Clock
			// Note:  If you are using PXI DSA Devices, the master device must reside in PXI Slot 2.
			DAQmxErrChk (GetTerminalNameWithDevPrefix(masterTaskHandle,"SampleClockTimebase",str1));
			DAQmxErrChk (GetTerminalNameWithDevPrefix(masterTaskHandle,"SyncPulse",str2));
			DAQmxErrChk (DAQmxSetSampClkTimebaseSrc(slaveTaskHandle,str1));
			DAQmxErrChk (DAQmxSetSyncPulseSrc(slaveTaskHandle,str2));
			break;
		case 4: // Reference clock 10 synchronization for DSA devices.
			// Note: Not all DSA devices support reference clock synchronization. Refer to your hardware 
			// device manual for further information on whether this method of synchronization is supported
			// for your particular device
			DAQmxErrChk (DAQmxSetRefClkSrc(masterTaskHandle, "PXI_Clk10"));
			DAQmxErrChk (GetTerminalNameWithDevPrefix(masterTaskHandle,"SyncPulse",str1));
			DAQmxErrChk (DAQmxSetSyncPulseSrc(slaveTaskHandle, str1));
			DAQmxErrChk (DAQmxSetRefClkSrc(slaveTaskHandle, "PXI_Clk10"));
			break;
		default:
			break;
	}
	DAQmxErrChk (GetTerminalNameWithDevPrefix(masterTaskHandle,"ai/StartTrigger",trigName));
	DAQmxErrChk (DAQmxCfgDigEdgeStartTrig(slaveTaskHandle,trigName,DAQmx_Val_Rising));

	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	// The slave device is armed before the master so that the slave device does
	// not miss the trigger.
	DAQmxErrChk (DAQmxStartTask(slaveTaskHandle));
	DAQmxErrChk (DAQmxStartTask(masterTaskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadAnalogF64(masterTaskHandle,-1,10.0,DAQmx_Val_GroupByChannel,masterData,1000,&numMasterRead,NULL));
	DAQmxErrChk (DAQmxReadAnalogF64(slaveTaskHandle,-1,10.0,DAQmx_Val_GroupByChannel,slaveData,1000,&numSlaveRead,NULL));

	if( numMasterRead>0 )
		printf("Acquired %d master samples\n",(int)numMasterRead);
	if( numSlaveRead>0 )
		printf("Acquired %d slave samples\n",(int)numSlaveRead);

Error:
	if( DAQmxFailed(error) )
		DAQmxGetExtendedErrorInfo(errBuff,2048);

	/*********************************************/
	// DAQmx Stop Code
	/*********************************************/
	if( masterTaskHandle!=0 ) {
		DAQmxStopTask(masterTaskHandle);
		DAQmxClearTask(masterTaskHandle);
	}
	if( slaveTaskHandle!=0 ) {
		DAQmxStopTask(slaveTaskHandle);
		DAQmxClearTask(slaveTaskHandle);
	}

	if( DAQmxFailed(error) )
		printf("DAQmx Error: %s\n",errBuff);
	printf("End of program, press Enter key to quit");
	getchar();
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
