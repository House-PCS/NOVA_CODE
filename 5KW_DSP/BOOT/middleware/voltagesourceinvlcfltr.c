/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        voltagesourceinvlcfltr.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#include "voltagesourceinvlcfltr.h"

/************************User Define**********************************/
volatile uint16_t InvDutyTemp = 0;

volatile uint16_t LLCDuty = 0;
volatile uint16_t LLCDutyLimit = (uint16_t)((float32)LLC_PWM_PERIOD / 2 * LLC_PWM_LIMIT);
volatile uint16_t LLCDutyMax = LLC_PWM_PERIOD / 2;

uint8_t SicaDataTemp;
/*********************************************************************/
//TODO Inverter Control Variables
// Inverter Variables
//NOTE: Io & Ig are used interchangeably,
//NOTE: Vo and Vg are used Interchangeably
// Measurement Variables
// Inverter Side Inductor Current Measurement
volatile float invIiInst,invIiInstADC,invIiInstSD;
// Output Voltage measurement
volatile float invVoInst,invVoInstADC,invVoInstSD;
// Inverter Side Inductor Current Measurement Offset
volatile float invIiOffset;
// Cap Current SD measurement
volatile float invIfInstSD;
// Cap Current SD measurement offset
volatile float invIfSDOffset;
// Output current measurement
volatile float invIoInst,invIoInstSD;
// Output current measurement
volatile float invIoSDOffset;
// Output Voltage measurement Offset
// Vo Offset May not be applicable in real application as Vo may be present always
volatile float invVoOffset,invVoSDOffset;
// Inv DC Bus Voltage Measurement
volatile float invVbusInst,invVbusInstADC,invVbusInstSD;
// Inv DC Bus Voltage Measurement Offset
volatile float invVbusOffset,invVbusSDOffset;
// Inv Filtered DC bus measurement
volatile float invVbusAvg;
// Inv DC Bus measurement by SD Offset
// Reference variables
// Inverter modulation index, used for open loop check
volatile float invModIndex;
// Peak value of the Ii, Io current set point
volatile float invIiRef, invIoRef;
// Peak value of the Vo  set point
volatile float invVoRef;
// Instantaneous Ii or Io reference = invIiRef* Angle
volatile float invIiRefInst,invIoRefInst;
// Instantaneous Vo reference = invVoRef* Angle
volatile float invVoRefInst;
// DC bus reference
volatile float invVbusRef;

// Display Values
float guiVbus;
float guiACFreq;
float guiPrms;
float guiIrms;
volatile float guiVrms;
float guiVavg;
float guiVo;
float guiIi;
float guiIo;
float guiPowerFactor;
float guiVA;
float guiFreqAvg;
float guiVema;

// Variables used to calibrate measurement offsets
//Offset filter coefficient K1: 0.05/(T+0.05);
float k1 = 0.998;
//Offset filter coefficient K2: T/(T+0.05)
float k2 = 0.001999;
int16_t offsetCalCounter;
float offset165;

// Duty variables for inverter (per unit and compare register values)
// per unit duty value
float32 invDutyPU,invDutyPUPrev;
// duty value written into the PWM register
int16_t invDuty,invDutyCompl;
// PWM TBPRD Half
volatile float halfPWMPeriod;

// Flags for clearing trips and closing the loops and the Relay
int16_t closeILoopInv,closeVLoopInv, clearInvTrip, rlyConnect;

// Flags for detecting ZCD
float32 invSine, invSinePrev;
int16_t zeroCrossDetectFlag;

volatile int16 tempChange=0;
volatile int16 period;

//*********************** USER Variables *************************************//
// Global variables used in this system specific to this control method
// common variables are kpet in <system_name>_board.c
//****************************************************************************//

// RAMP to generate forced angle when grid is not present
RAMPGEN_F rgen1;

uint16_t guiInvStart;
uint16_t guiInvStop;
int16_t slewState;

enum enum_boardState
{
    boardState_InverterOFF = 0,
    boardState_CheckDCBus = 1,
    boardState_RlyConnect = 2,
    boardState_InverterON=3,
    boardState_TripCondition = 4,
};
enum enum_boardState boardState = boardState_InverterOFF;

// Stand Alone Flash Image Instrumentation
// GPIO toggles for different states


//extern to access tables in ROM
extern long FPUsinTable[];


float adc_noise;
float H_calc;
float error_bound;
float inj_calculated;




//GUI support variables
// sets a limit on the amount of external GUI controls - increase as necessary



//------------------------------------------------------------------------------------
enum enum_BuildLevel
{
    BuildLevel1_OpenLoop_AC = 0,
    BuildLevel1_OpenLoop_DC=1,
    BuildLevel2_CurrentLoop_AC = 2,
    BuildLevel2_CurrentLoop_DC = 3,
    BuildLevel3_VoltageLoop_AC = 4,
    BuildLevel3_VoltageLoop_DC = 4,
};
enum enum_BuildLevel BuildInfo = BuildLevel1_OpenLoop_AC ;

volatile long sdfm32;

sineAnalyzerWithPowerMeas sine_mains;

#if 0
//TODO device_setup()
void setupDevice(void)
{
#ifdef FLASH
// Copy time critical code and Flash setup code to RAM
// The  RamfuncsLoadStart, RamfuncsLoadEnd, and RamfuncsRunStart
// symbols are created by the linker. Refer to the linker files.
    memcpy(&RamfuncsRunStart, &RamfuncsLoadStart, (Uint32)&RamfuncsLoadSize);
#endif

    //DeviceInit(); // Device Life support & GPIO
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F28M3Xx_SysCtrl.c file.
    InitSysCtrl();

#ifdef FLASH
// Call Flash Initialization to setup flash waitstates
// This function must reside in RAM
    InitFlash();    // Call the flash wrapper init function
#endif //(FLASH)

    // Step 2. Initialize GPIO:
    // This example function is found in the F28M3Xx_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    //InitGpio(); // Skipped for this example
    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F28M3Xx_PieCtrl.c file.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F28M3Xx_DefaultIsr.c.
    // This function is found in F28M3Xx_PieVect.c.
    InitPieVectTable();

    // initialize CPU timers
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 200, 10000);
    ConfigCpuTimer(&CpuTimer1, 200, 20000);
    CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
    CpuTimer1Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0

}
//TODO Main
void main(void)
{
    // This routine sets up the basic device ocnfiguration such as initializing PLL
    // copying code from FLASH to RAM,
    setupDevice();
    ConfigCpuTimer(&CpuTimer2, 200, 80000);
    CpuTimer2Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0

    // Stop all PWM mode clock
    DisablePWMCLKCounting();

    SetupInverterPWM(PWM_NO,INV_PWM_PERIOD,INV_DEADBAND_PWM_COUNT,INV_DEADBAND_PWM_COUNT);

    InitEPwm1Gpio();
    InitEPwm2Gpio();


    //Control Variable specific to the voltage source inverter Initialization

    //RAMPGEN
    RAMPGEN_F_init(&rgen1);
    rgen1.Freq = (float32)(AC_FREQ);
    rgen1.StepAngleMax = (float32)(1.0/INV_PWM_SWITCHING_FREQUENCY);

    // Initialize global variables generic to the board like ones used to read current values etc
    InitGlobalVariables();

    // Enable PWM Clocks
    EnablePWMCLKCounting();

    // ISR Mapping
    setupInterrupt();

// IDLE loop. Just sit and loop forever, periodically will branch into A0-A3, B0-B3, C0-C3 tasks
// Frequency of this brnaching is set in setupDevice routine:
    for(;;)  //infinite loop
    {
        // State machine entry & exit point
    }
} //END MAIN CODE
#endif



void EpwmInit(void)
{
    // Stop all PWM mode clock
    DisablePWMCLKCounting();

    SetupInverterPWM(PWM_NO, INV_PWM_PERIOD, INV_DEADBAND_PWM_COUNT, INV_DEADBAND_PWM_COUNT);
    InitEPwm2Gpio();
    InitEPwm3Gpio();
    InvDutyTemp = (*ePWM[PWM_NO]).TBPRD - PWM_DUTY_CUT;

//  SetupPVPWM(11, INV_PWM_PERIOD, 0, 0);
//  InitEPwm11Gpio();

    SetupBusPWM(6, LLC_PWM_PERIOD, 0);
    InitEPwm6Gpio();
    InitEPwm7Gpio();

    SetupLLCPWM(8, LLC_PWM_PERIOD, 0, 180);
    InitEPwm8Gpio();
    InitEPwm9Gpio();




    //Control Variable specific to the voltage source inverter Initialization

    //RAMPGEN
    RAMPGEN_F_init(&rgen1);
    rgen1.Freq = (float32)(AC_FREQ);
    rgen1.StepAngleMax = (float32)(1.0/INV_PWM_SWITCHING_FREQUENCY);

    // Initialize global variables generic to the board like ones used to read current values etc
    InitGlobalVariables();

    // Enable PWM Clocks
    EnablePWMCLKCounting();

    // ISR Mapping
    setupInterrupt();
//  setupLLCInterrupt();
}


//TODO Inverter ISR Code
/**
* @brief: InverterISR
* @param: void                        
* @return: 
* @note: 
*
*/
interrupt void InverterISR(void)
{
    SciaRxDataRecv();
    readCurrVolADCSignals();
    // Generate the RAMP Angle
    RAMPGEN_F_MACRO(rgen1);
    // Use the angle value to compute the sine value
    invSine = _IQsinPU(rgen1.Out);

    invDutyPU = SFRA_F_INJECT(_IQ24mpy(invSine,invModIndex));
    updateInverterPWM(PWM_NO);
    UpdateLLCPWM(6);
    clearInterrupt();
}

//TODO InitGlobalVariables()
/**
* @brief: InitGlobalVariables
* @param: void                        
* @note: 
*
*/
void InitGlobalVariables(void)
{
    // Gui Variables
//  guiVbus=0;
//  guiACFreq=0;
//  guiPrms=0;
//  guiIrms=0;
//  guiVrms=0;
//  guiVavg=0;
//  guiFreqAvg=0;
//  guiVema=0;

//  dLog1.input_ptr1 = &dVal1;
//  dLog1.input_ptr2 = &dVal2;
//  dLog1.input_ptr3 = &dVal3;  // spll.Out logged in buff3
//  dLog1.input_ptr4 = &dVal4;  // ramp_sin logged in buff4
//  dLog1.output_ptr1 = dBuff1;
//  dLog1.output_ptr2 = dBuff2;
//  dLog1.output_ptr3 = dBuff3;
//  dLog1.output_ptr4 = dBuff4;
//  dLog1.prev_value = (float32)(0);
//  dLog1.trig_value = (float32)(0.1);
//  dLog1.status = 1;
//  dLog1.pre_scalar = 5;
//  dLog1.skip_count = 0;
//  dLog1.size = 200;
//  dLog1.count = 0;

    //Variable initialization
    invIiInst=0;
    invIiOffset=0;
    invIiInstSD=0;

    invIfInstSD=0;
    invIfSDOffset=0;
    invIoInstSD=0;
    invIoSDOffset=0;
    invVoInst=0;
    invVoOffset=0.5;
    invVoInstSD=0;
    invVoSDOffset=0;
    invVbusInst=0;
    invVbusOffset=0;
    invVbusAvg=0;
    invVbusInstSD=0;
    invVbusSDOffset=0;
    invModIndex=0.5;
    invIiRef=0;
    invIoRef=0;
    invIoRefInst=0;
    invVoRef=0;
    invIiRefInst=0;
    invVoRefInst=0;
    invVbusRef=0;
    offsetCalCounter=0;
    invDutyPU=0;
    invDuty=0;
    halfPWMPeriod=0;


    closeILoopInv=0;
    closeVLoopInv=0;
    clearInvTrip=0;
    rlyConnect=0;
    invSine=0;
    invSinePrev=0;
    zeroCrossDetectFlag=0;


}
//===========================================================================
// No more.
//===========================================================================
