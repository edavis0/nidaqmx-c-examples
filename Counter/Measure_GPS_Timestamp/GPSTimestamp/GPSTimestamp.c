/*********************************************************************
*
* ANSI C Example program:
*    GPSTimestamp.c
*
* Example Category:
*    CI
*
* Description:
*    This example demonstrates how to use a single point task to
*    measure time using a GPS Timestamp Channel. The Synchronization
*    Method and Synchronization Source are configurable.
*
* Instructions for Running:
*    1. Select the Physical Channel which corresponds to the GPS
*       counter you want to count edges on the DAQ device.
*    2. Enter the Synchronization Method and Synchronization Source
*       to specify which type of GPS synchronization signal you want
*       to use.
*
* Steps:
*    1. Create a task.
*    2. Create a GPS Timestamp Input channel to read. The
*       synchronization method parameter should be set based on the
*       GPS receiver synchronization signal you are using.
*    3. Specify the source of your GPS synchronization signal.
*    4. Call the Start function to arm the counter and begin reading.
*    5. The timestamp counter will be continually polled until the
*       Stop button is pressed.
*    6. Use the Get Time from Seconds utility function to view full
*       timestamp information.
*    7. Call the Clear Task function to clear the task.
*    8. Display an error if any.
*
* I/O Connections Overview:
*    The GPS counter will synchronize with the signal specified in
*    the Synchronization Source I/O control.
*
*    In this example the GPS Timestamp counter will synchronize with
*    the signal on PFI7.
*
*    This example uses the default source (or gate) terminal for the
*    counter of your device. To determine what the default counter
*    pins for your device are or to set a different source (or gate)
*    pin, refer to the Connecting Counter Signals topic in the
*    NI-DAQmx Help (search for "Connecting Counter Signals").
*
*********************************************************************/

#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall) if( DAQmxFailed(error=(functionCall)) ) goto Error; else

static uInt16 year;
static char *monthStr[]={"<invalid>","January","February","March","April","May","June","July","August","September","October","November","December"};

static void GetTimeFromGPSSeconds(float64 secondsSinceJan1, float64 *seconds, uInt8 *minutes, uInt8 *hours, uInt8 *date, uInt8 *month);

int main(void)
{
	int32       error=0;
	char        errBuff[2048]="";
	TaskHandle  taskHandle=0;
	uInt8       month,day,hours,minutes;
	float64     seconds,GPSsecs;
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


	/*********************************************/
	// DAQmx Start Code
	/*********************************************/
	DAQmxErrChk (DAQmxStartTask(taskHandle));

	printf("Continuously polling. Press Ctrl+C to interrupt\n");
	while( 1 ) {
		/*********************************************/
		// DAQmx Read Code
		/*********************************************/
		DAQmxErrChk (DAQmxReadCounterScalarF64(taskHandle,-1,&GPSsecs,NULL));

		printf("GPS Seconds: %15.6f  ",GPSsecs);

		GetTimeFromGPSSeconds(GPSsecs,&seconds,&minutes,&hours,&day,&month);

		printf("The time is %2u:%02u:%05.2f %s %u, %u\n",hours,minutes,seconds,monthStr[month],day+1,year);
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
