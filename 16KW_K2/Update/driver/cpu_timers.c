/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_timers.c
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
#include "cpu_timers.h"
#include "F28x_Project.h"
//
// Function Prototypes
//
__interrupt void cpu_timer0_isr(void);
__interrupt void cpu_timer1_isr(void);
__interrupt void cpu_timer2_isr(void);

static TimerState_t _gTimer;
//
// cpu_timer0_isr - CPU Timer0 ISR with interrupt counter
//
__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;

    //
    // Acknowledge this interrupt to receive more interrupts from group 1
    //
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

//
// cpu_timer1_isr - CPU Timer1 ISR
//
#ifdef EN_CLA_RUN_TIME_MEASURE

#else
__interrupt void cpu_timer1_isr(void)
{
    CpuTimer1.InterruptCount++;
}
#endif

//
// cpu_timer2_isr CPU Timer2 ISR
//
__interrupt void cpu_timer2_isr(void)
{
    CpuTimer2.InterruptCount++;
//    ServiceDog();
}

uint32_t TimGetSystime(void)
{
    return CpuTimer2.InterruptCount;
}

/**
  * @brief  Calculate the difference of two time values.
  * This function will handle overflow problem.
  * @param  time[IN]: First time value to calculate
  * @param  new_time[IN]: 2nd time value to calculate
  * @return Time difference of 2 time values.
  */
uint32_t TimDifftime(uint32_t time, uint32_t new_time)
{
    uint32_t diffTime = 0;
    if (new_time >= time) /* not overflow */
    {
        diffTime = (new_time - time);
    }
    else
    {
        diffTime = new_time + (0xffffffffUL - time);
    }
    return (uint32_t)(diffTime);
}

/**
  * @brief  Calculate the time difference from now.
  * @param  time[IN]: Previous recorded time value
  * @return Time difference.
  */
uint32_t TimDifftimeFromNow(uint32_t time)
{
    return TimDifftime(time, CpuTimer2.InterruptCount);
}

void CpuTimerFlag(void)
{
    if(TimDifftimeFromNow(_gTimer.Time) >= 1) //1ms¶¨Ê±Æ÷
    {
        _gTimer.Timer_1ms_Count++;
        _gTimer.Timer_5ms_Count++;
        _gTimer.Timer_10ms_Count++;
        _gTimer.Timer_20ms_Count++;
        _gTimer.Timer_100ms_Count++;
        _gTimer.Timer_1000ms_Count++;

        if(1)
        {
            _gTimer.Timer_1ms_Flag = 1;
            _gTimer.Timer_1ms_Count = 0;
        }
        if(_gTimer.Timer_5ms_Count >= 5)
        {
            _gTimer.Timer_5ms_Flag = 1;
            _gTimer.Timer_5ms_Count = 0;
        }
        if(_gTimer.Timer_10ms_Count >= 10)
        {
            _gTimer.Timer_10ms_Flag = 1;
            _gTimer.Timer_10ms_Count = 0;
        }
        if(_gTimer.Timer_20ms_Count >= 20)
        {
            _gTimer.Timer_20ms_Flag = 1;
            _gTimer.Timer_20ms_Count = 0;
        }
        if(_gTimer.Timer_100ms_Count >= 100)
        {
            _gTimer.Timer_100ms_Flag = 1;
            _gTimer.Timer_100ms_Count = 0;
        }
        if(_gTimer.Timer_1000ms_Count >= 1000)
        {
            _gTimer.Timer_1000ms_Flag = 1;
            _gTimer.Timer_1000ms_Count = 0;
        }

        _gTimer.Time = TimGetSystime();
    }
}
uint16_t Get_1ms_Flag(void)
{
    return (_gTimer.Timer_1ms_Flag);
}
uint16_t Get_5ms_Flag(void)
{
    return (_gTimer.Timer_5ms_Flag);
}
uint16_t Get_10ms_Flag(void)
{
    return (_gTimer.Timer_10ms_Flag);
}
uint16_t Get_20ms_Flag(void)
{
    return (_gTimer.Timer_20ms_Flag);
}
uint16_t Get_100ms_Flag(void)
{
    return (_gTimer.Timer_100ms_Flag);
}
uint16_t Get_1s_Flag(void)
{
    return (_gTimer.Timer_1000ms_Flag);
}
void Timers_1ms(void)
{
    _gTimer.Timer_1ms_Flag = 0;
}
void Timers_5ms(void)
{
    _gTimer.Timer_5ms_Flag = 0;
}
void Timers_10ms(void)
{
    _gTimer.Timer_10ms_Flag = 0;
}
void Timers_20ms(void)
{
    _gTimer.Timer_20ms_Flag = 0;
}
void Timers_100ms(void)
{
    _gTimer.Timer_100ms_Flag = 0;
}
void Timers_1s(void)
{
    _gTimer.Timer_1000ms_Flag = 0;
}

void CpuTimer_Init(void)
{
//
// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
//
    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    #ifdef EN_CLA_RUN_TIME_MEASURE
#else
    PieVectTable.TIMER1_INT = &cpu_timer1_isr;
#endif

    PieVectTable.TIMER2_INT = &cpu_timer2_isr;
    EDIS;    // This is needed to disable write to EALLOW protected registers

//
// Step 4. Initialize the Device Peripheral. This function can be
//         found in F2837xD_CpuTimers.c
//
    InitCpuTimers();   // For this example, only initialize the Cpu Timers

//
// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
// 200MHz CPU Freq, 1 second Period (in uSeconds)
//
    ConfigCpuTimer(&CpuTimer0, 200, 1000000);
   #ifdef EN_CLA_RUN_TIME_MEASURE
    ConfigCpuTimer(&CpuTimer1, 200, 0xFFFFFFFF);
#else
    ConfigCpuTimer(&CpuTimer1, 200, 1000000);
#endif

    ConfigCpuTimer(&CpuTimer2, 200, 1000);

//
// To ensure precise timing, use write-only instructions to write to the
// entire register. Therefore, if any of the configuration bits are changed in
// ConfigCpuTimer and InitCpuTimers (in F2837xD_cputimervars.h), the below
// settings must also be updated.
//
    CpuTimer0Regs.TCR.all = 0x4000;
    #ifdef EN_CLA_RUN_TIME_MEASURE
    CpuTimer1Regs.TCR.all = 0x0000;
#else
    CpuTimer1Regs.TCR.all = 0x4000;
#endif

    CpuTimer2Regs.TCR.all = 0x4000;

//
// Step 5. User specific code, enable interrupts:
//

//
// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
// which is connected to CPU-Timer 1, and CPU int 14, which is connected
// to CPU-Timer 2:
//
    IER |= M_INT1;
    IER |= M_INT13;
    IER |= M_INT14;

//
// Enable TINT0 in the PIE: Group 1 interrupt 7
//
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}
