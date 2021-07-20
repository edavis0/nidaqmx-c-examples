/*********************************************************************
*
* ANSI C Example program:
*    GPSTimestamp-Finite-Buff.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to use a finite buffered task to
*    measure time using a GPS Timestamp Channel. The Synchronization
*    Method, Synchronization Source, Sample Clock Source, and Samples
*    per Channel are all configurable.

*
*    Note: For buffered GPS Timestamp measurements, an external
*          sample clock is necessary to signal when the counter
*          should generate a sample. This is set by the Sample Clock
*          Source control.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the GPS
*       counter you want to count edges on the DAQ device.
*    2. Enter the Synchronization Method and Synchronization Source
*       to specify which type of GPS synchronization signal you want
*       to use. Set the Sample Clock Source and Samples per Channel
*       to configure timing for the measurement.

*    Note: An external sample clock must be used. Counters do not
*          have an internal sample clock available. You can use the
*          Gen Dig Pulse Train-Continuous example to generate a pulse
*          train on another counter and connect it to the Sample
*          Clock Source you are using in this example.
*
* Steps:
*    1. Create a task.
*    2. Create a GPS Timestamp channel.
*    3. Specify the source of your GPS synchronization signal.
*    4. Call the DAQmx Timing function (Sample Clock) to configure
*       the external sample clock timing parameters such as Sample
*       Mode, Samples per Channel, and Sample Clock Source. The Edge
*       parameter can be used to determine when a sample is taken.
*    5. Call the Start function to arm the timestamp counter.
*    6. For finite measurements, the counter will stop reading data
*       when the Samples per Channel have been received.
*    7. Use the Get Time from Seconds utility function to view full
*       timestamp information.
*    8. Call the Clear Task function to clear the task.
*    9. Display an error if any.
*
* I/O Connections Overview:
*    The GPS counter will synchronize with the signal specified in
*    the Synchronization Source I/O control.
*
*    In this example the GPS Timestamp counter will take measurements
*    on valid edges of the external Sample Clock Source which is PFI9
*    and synchronize with the GPS signal on PFI7.
*
*    This example uses the default source (or gate) terminal for the
*    counter of your device. To determine what the default counter
*    pins for your device are or to set a different source (or gate)
*    pin, refer to the Connecting Counter Signals topic in the
*    NI-DAQmx Help (search for "Connecting Counter Signals").
*
*********************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static uInt16	year;
static char *monthstr[]={"<invalid>","January","February","March","April","May","June","July","August","September","October","November","December"};

static void GetTimeFromGPSSeconds(float64 secondsSinceJan1, float64 *seconds, uInt8 *minutes, uInt8 *hours, uInt8 *date, uInt8 *month);

int main(void)
{
	int32       error=0;
	char        errBuff[2048]={0};
	TaskHandle  taskHandle=0;
	int         i=0;
	int32       read;
	uInt8       month,day,hours,minutes;
	float64     seconds,data[100];
	time_t      calTime;
	struct tm   *locTime;

	calTime = time(NULL);
	if( calTime==-1 ) {
		printf("Library Error: Failed to get the system time.\nAborting.\n");
		return 0;
	}
	locTime = localtime(&calTime);
	year = locTime->tm_year + 1900;

	/*********************************************/
	// DAQmx Configure Code
	/*********************************************/
	DAQmxErrChk (DAQmxCreateTask("",&taskHandle));
	DAQmxErrChk (DAQmxCreateCIGPSTimestampChan(taskHandle,"Dev1/gpsTimestampCtr0","",DAQmx_Val_Seconds,DAQmx_Val_IRIGB,""));
	DAQmxErrChk (DAQmxSetCIGPSSyncSrc(taskHandle, "", "/Dev1/PFI7"));
	DAQmxErrChk (DAQmxCfgSampClkTiming(taskHandle,"/Dev1/PFI9",1000,DAQmx_Val_Rising,DAQmx_Val_FiniteSamps,100));


	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	/*********************************************/
	// DAQmx Read Code
	/*********************************************/
	DAQmxErrChk (DAQmxReadCounterF64(taskHandle,100,-1,data,100,&read,NULL));

	printf("GPS Data:\n");
	for(;i<100;++i) {
		printf("[%02d]\t(GPS Seconds: %17.6f) ",i,data[i]);
		GetTimeFromGPSSeconds(data[i],&seconds,&minutes,&hours,&day,&month);
		printf("%2u:%02u:%05.2f (h:m:s) %s %u, %u\n",hours,minutes,seconds,monthstr[month],day+1,year);
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

static void GetTimeFromGPSSeconds(float64 secondsSinceJan1, float64 *seconds, uInt8 *minutes, uInt8 *hours, uInt8 *date, uInt8 *month)
{
	float64 fminutes,fhours,fdays,febDays;

	// Convert seconds to raw days
	fdays = secondsSinceJan1/86400.0;
	// Account for leap year
	febDays = year%4==0 && (year%100!=0 || year%400==0) ? 29.0 : 28.0;
	*month = 0;
	// Get month from days
	if( fdays<=31.0 ) *month = 1;
	else if( fdays<=31.0+febDays ) { *month = 2; fdays -= 31.0; }
	else if( fdays<=62.0+febDays ) { *month = 3; fdays -= (31.0  + febDays); }
	else if( fdays<=92.0+febDays) { *month = 4; fdays -= 62.0+febDays; }
	else if( fdays<=123.0+febDays ) { *month = 5; fdays -= 92.0+febDays; }
	else if( fdays<=153.0+febDays ) { *month = 6; fdays -= 123.0+febDays; }
	else if( fdays<=184.0+febDays ) { *month = 7; fdays -= 153.0+febDays; }
	else if( fdays<=215.0+febDays ) { *month = 8; fdays -= 184.0+febDays; }
	else if( fdays<=245.0+febDays ) { *month = 9; fdays -= 215.0+febDays; }
	else if( fdays<=276.0+febDays ) { *month = 10; fdays -= 245.0+febDays; }
	else if( fdays<=306.0+febDays ) { *month = 11; fdays -= 276.0+febDays; }
	else { *month = 12; fdays -= 306.0+febDays; }
	// Get hours from remainder of days
	fhours = 24.0 * modf(fdays,&fdays);
	*date = (uInt8)fdays + 1; // Date begins count from 1, not 0
	// Get minutes from remainder of hours
	fminutes = 60.0 * modf(fhours,&fhours);
	*hours = (uInt8)fhours;
	// Get seconds from remainder of minutes
	*minutes = (uInt8)fminutes;
	*seconds = 60.0 * modf(fminutes,&fminutes);
}
