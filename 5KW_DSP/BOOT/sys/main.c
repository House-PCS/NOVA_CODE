/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        main.c
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


//
// Included Files
//
#include "F28x_Project.h"
#include "cpu_timers.h"
#include "dev_flash.h"
#include "sci.h"
//#include "mid_comservice.h"
#include "modbus_analysis.h"
#include "flash.h"
#include "firmware_iap.h"
//
// Defines
//
#define BLINKY_LED_GPIO    75
#define PWMOUTEN_PCS    83

void main(void)
{
//
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the F2837xD_SysCtrl.c file.
//
    InitSysCtrl();

    // FALSH…’–¥≈‰÷√
#ifdef _FLASH
    InitFlash();
#endif
//
// Step 2. Initialize GPIO:
// This example function is found in the F2837xD_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
//
    InitGpio();
    GPIO_SetupPinMux(BLINKY_LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLINKY_LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    //  PWM  ‰≥ˆ245OE πƒ‹
    GPIO_SetupPinMux(PWMOUTEN_PCS, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(PWMOUTEN_PCS, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(PWMOUTEN_PCS, 1);

//
// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
//
    DINT;

//
// Initialize the PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the F2837xD_PieCtrl.c file.
//
    InitPieCtrl();

//
// Disable CPU interrupts and clear all CPU interrupt flags:
//
    IER = 0x0000;
    IFR = 0x0000;

//
// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2837xD_DefaultIsr.c.
// This function is found in F2837xD_PieVect.c.
//
    InitPieVectTable();

    CpuTimersInit();
    SciaFifoInit();
    SpiFlashInit();		//4M(32M-bit)


//    SciaMsg("Dsp 28375D Boot Loader!\r\n");
    MidComserviceInit();
//    InFlashInit();

//      uint8_t data[256];
//      uint16_t j = 0;
//      for(j = 0; j < 256; j ++)
//      {
//      data[j] = j;
//      }
//      data[0] = WdRegs.WDCR.all & 0x00FF;
//      data[1] = (WdRegs.WDCR.all >> 8);
//      SciaSend(data, 2);

//    JumpToAppProcess();
//    EnableDog();


//    EnterBootModeJudge();
// Enable global Interrupts and higher priority real-time debug events:
//

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM
//    APP_JUMP;
//    AdcInit();
//
// Step 6. IDLE loop. Just sit and loop forever (optional):
//
    static uint32_t sn_time = 0;
    ModbusRegInit();
    for(;;)
    {
        SysFirmwareIap();

        if(TimDifftimeFromNow(sn_time) > 100)
        {
//          ServiceDog();
            GPIO_WritePin(BLINKY_LED_GPIO, GPIO_TOGGLE);
            sn_time = TimGetSystime();
        }
        //
        // Turn on LED
        //
//        ServiceDog();
    }
}


//
// End of file
//
