/*********************************************************************
*
* ANSI C Example program:
*    ContAccelSamps-IntClk-AnlgStart.c
*
* Example Category:
*    AI
*
* Description:
*    This example demonstrates how to create an analog input
*    acceleration task and perform a continuous acquisition using
*    option IEPE excitation, analog triggering, and overload
*    detection.
*
* Instructions for Running:
*    1. Select the physical channel to correspond to where your
*       signal is input on the device.
*    2. Enter the minimum and maximum expected acceleration values.
*    Note: To optimize gain selection, try to match the Input Ranges
*          to the expected level of the measured signal.
*    3. Program the analog input terminal configuration and IEPE
*       excitation settings for your device.
*    4. If your device supports overload detection, check the
*       Overload Detection checkbox. Refer to your device
*       documentation to see if overload protection is supported.
*    5. Set the rate of the acquisition. Also set the Samples to Read
*       control. This will determine how many samples are read at a
*       time. This also determines how many points are plotted on the
*       graph each time.
*    Note: The rate should be at least twice as fast as the maximum
*          frequency component of the signal being acquired.
*    6. Set the source of the Analog Edge Start Trigger. By default
*       this is Dev1/ai0.
*    7. Set the slope and level of desired analog edge condition.
*    8. Set the Hysteresis Level.
*    9. Input the sensitivity and units for your accelerometer.
*
* Steps:
*    1. Create a task.
*    2. Create an analog input acceleration channel. This step
*       defines accelerometer sensitivity, desired range, and IEPE
*       excitation.
*    3. Set the sample rate and define a continuous acquisition.
*    4. Define the trigger channel, trigger level, rising/falling
*       edge, and hysteresis window for an analog start trigger.
*    5. Call the Start function to start the acquisition.
*    6. Read the waveform data in the EveryNCallback function until
*       the user hits the stop button or an error occurs.
*    7. Check for overloaded channels.
*    8. Call the Clear Task function to clear the Task.
*    9. Display an error if any.
*
* I/O Connections Overview:
*    Make sure your signal input terminal matches the Physical
*    Channel I/O control. Also, make sure your analog trigger
*    terminal matches the Trigger Source Control. For further
*    connection information, refer to your hardware reference manual.
*
*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <NIDAQmx.h>

#define DAQmxErrChk(functionCall)            \
    if (DAQmxFailed(error = (functionCall))) \
        goto Error;                          \
    else

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData);
int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData);

/*********************************************/
// DAQmx Configuration Options
/*********************************************/
// Sampling Options
const float64 sampleRate = 1000.0; // The sampling rate in samples per second per channel.
const uInt64 sampsPerChan = 1000; // The number of samples to acquire or generate for each channel in the task.
// DAQmxCreateAIAccelChan Options
const char* physicalChannel = "Dev1/ai0"; // The names of the physical channels to use to create virtual channels. You can specify a list or range of physical channels.
const int32 terminalConfig = DAQmx_Val_PseudoDiff; // The input terminal configuration for the channel. Options: DAQmx_Val_Cfg_Default, DAQmx_Val_RSE, DAQmx_Val_NRSE, DAQmx_Val_Diff, DAQmx_Val_PseudoDiff
const float64 minVal = -50.0; // The minimum value, in units, that you expect to measure.
const float64 maxVal = 50.0; // The maximum value, in units, that you expect to measure.
const int32 units = DAQmx_Val_AccelUnit_g; // The units to use to return acceleration measurements from the channel. Options: DAQmx_Val_AccelUnit_g, DAQmx_Val_FromCustomScale
const float64 sensitivity = 175; // The sensitivity of the sensor. This value is in the units you specify with the sensitivityUnits input.
const int32 sensitivityUnits = DAQmx_Val_mVoltsPerG; // The units of sensitivity. Options: DAQmx_Val_mVoltsPerG, DAQmx_Val_VoltsPerG
const int32 currentExcitSource = DAQmx_Val_Internal; // The source of excitation. Options: DAQmx_Val_Internal,DAQmx_Val _External, DAQmx_Val_None
const float64 currentExcitVal = 0.004; // The amount of excitation, in amperes, that the sensor requires.
// DAQmxCfgSampClkTiming Options
const char* clockSource = "OnboardClock"; // The source terminal of the Sample Clock. To use the internal clock of the device, use NULL or use OnboardClock.
const int32 activeEdge = DAQmx_Val_Rising; // Specifies on which edge of the clock to acquire or generate samples. Options: DAQmx_Val_Rising, DAQmx_Val_Falling
const int32 sampleMode = DAQmx_Val_ContSamps; // Specifies whether the task acquires or generates samples continuously or if it acquires or generates a finite number of samples. Options: DAQmx_Val_FiniteSamps, DAQmx_Val_ContSamps, DAQmx_Val_HWTimedSinglePoint
// DAQmxCfgAnlgEdgeStartTrig Options
const char* startTriggerSource = "/Dev1/APFI0"; // The name of a channel or terminal where there is an analog signal to use as the source of the trigger.
const int32 startTriggerSlope = DAQmx_Val_RisingSlope; // Specifies on which slope of the signal to start acquiring. Options: DAQmx_Val_RisingSlope, DAQmx_Val_FallingSlope
const float64 startTriggerLevel = 30.0; // The threshold at which to start acquiring samples. Specify this value in the units of the measurement.
// DAQmxSetAnlgEdgeStartTrigHyst Options
const float64 hystLevel = 10.0; // Specifies a hysteresis level in the units of the measurement.
// DAQmxRegisterEveryNSamplesEvent Options
const int32 everyNsamplesEventType = DAQmx_Val_Acquired_Into_Buffer; // The type of event you want to receive. Options: DAQmx_Val_Acquired_Into_Buffer, DAQmx_Val_Transferred_From_Buffer
const uInt32 options = 0; // Use this parameter to set certain options. Pass a value of zero if no options need to be set. Options: 0, DAQmx_Val_SynchronousEventCallbacks
// DAQmxReadAnalogF64 Options
const float64 timeout = 10; // The amount of time, in seconds, to wait for the function to read the sample(s).
const bool32 fillMode = DAQmx_Val_GroupByScanNumber; // Specifies whether or not the samples are interleaved. Options: DAQmx_Val_GroupByChannel, DAQmx_Val_GroupByScanNumber

int main(void)
{
    int32 error = 0;
    TaskHandle taskHandle = 0;
    char errBuff[2048] = { '\0' };

    /*********************************************/
    // DAQmx Configure Code
    /*********************************************/
    DAQmxErrChk(DAQmxCreateTask("", &taskHandle));
    DAQmxErrChk(DAQmxCreateAIAccelChan(taskHandle, physicalChannel, "", terminalConfig, minVal, maxVal, units, sensitivity, sensitivityUnits, currentExcitSource, currentExcitVal, NULL));
    DAQmxErrChk(DAQmxCfgSampClkTiming(taskHandle, clockSource, sampleRate, activeEdge, sampleMode, sampsPerChan));
    DAQmxErrChk(DAQmxCfgAnlgEdgeStartTrig(taskHandle, startTriggerSource, startTriggerSlope, startTriggerLevel));
    DAQmxErrChk(DAQmxSetAnlgEdgeStartTrigHyst(taskHandle, hystLevel));

    DAQmxErrChk(DAQmxRegisterEveryNSamplesEvent(taskHandle, everyNsamplesEventType, sampsPerChan, options, EveryNCallback, NULL));
    DAQmxErrChk(DAQmxRegisterDoneEvent(taskHandle, 0, DoneCallback, NULL));

    /*********************************************/
    // DAQmx Start Code
    /*********************************************/
    DAQmxErrChk(DAQmxStartTask(taskHandle));

    printf("Acquiring samples continuously. Press Enter to interrupt\n");
    getchar();

Error:
    if (DAQmxFailed(error))
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
    if (taskHandle != 0)
    {
        /*********************************************/
        // DAQmx Stop Code
        /*********************************************/
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
    }
    if (DAQmxFailed(error))
        printf("DAQmx Error: %s\n", errBuff);
    printf("End of program, press Enter key to quit\n");
    getchar();
    return 0;
}

int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void* callbackData)
{
    int32 error = 0;
    char errBuff[2048] = { '\0' };
    static int totalRead = 0;
    int32 read = 0;
    float64 data[sampsPerChan];
    /* Change this variable to 1 if you are using a DSA device and want to check for Overloads. */
    int32 overloadDetectionEnabled = 0;
    bool32 overloaded = 0;
    char overloadedChannels[1000];

    /*********************************************/
    // DAQmx Read Code
    /*********************************************/
    DAQmxErrChk(DAQmxReadAnalogF64(taskHandle, sampsPerChan, timeout, fillMode, data, sampsPerChan, &read, NULL));
    if (overloadDetectionEnabled)
    {
        DAQmxErrChk(DAQmxGetReadOverloadedChansExist(taskHandle, &overloaded));
    }

    if (read > 0)
        printf("Acquired %d samples. Total %d\n", (int)read, (int)(totalRead += read));
    for (int i = 0; i < sampsPerChan; i++)
    {
        printf("%.2f\n", data[i]);
    }
    if (overloaded)
    {
        DAQmxErrChk(DAQmxGetReadOverloadedChans(taskHandle, overloadedChannels, 1000));
        printf("Overloaded channels: %s\n", overloadedChannels);
    }
    fflush(stdout);

Error:
    if (DAQmxFailed(error))
    {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        /*********************************************/
        // DAQmx Stop Code
        /*********************************************/
        DAQmxStopTask(taskHandle);
        DAQmxClearTask(taskHandle);
        printf("DAQmx Error: %s\n", errBuff);
    }
    return 0;
}

int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void* callbackData)
{
    int32 error = 0;
    char errBuff[2048] = { '\0' };

    // Check to see if an error stopped the task.
    DAQmxErrChk(status);

Error:
    if (DAQmxFailed(error))
    {
        DAQmxGetExtendedErrorInfo(errBuff, 2048);
        DAQmxClearTask(taskHandle);
        printf("DAQmx Error: %s\n", errBuff);
    }
    return 0;
}
