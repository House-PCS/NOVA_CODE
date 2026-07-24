//------------------------------------------------------------------------------
/*File Name         :   Timer.c
**Original Author   :   lvyb
**Creation Date     :   
**Description       :   TIMER固件库函数

*/
//------------------------------------------------------------------------------
#include "Gpio.h"
#include "Uart.h"
#include "Timer.h"


//------------------------------------------------------------------------------
/*Function Name     :   TimerInit 
**Description       :   Timer0初始化
**Input Param       :   
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void TimerInit()
{
    TIMER0->CTRL = 0;
    TIMER0->VALUE = 0;
    TIMER0->RELOAD = 0;
    TIMER0->INTCLEAR = 0;
    
    TIMER0->VALUE = 50000000;
    TIMER0->RELOAD = 50000000;
    TIMER0->CTRL |= 0x00000001;
    TIMER0->CTRL |= 0x00000008;
    NVIC_EnableIRQ(INT00_IRQn);
}

//------------------------------------------------------------------------------
/*Function Name     :   INT00_Handler 
**Description       :   Timer0中断函数
**Input Param       :   TimeCnt：定时器中断计数次数
**Output Param      :
**Return value      :
*/
//------------------------------------------------------------------------------

void INT00_Handler()
{    
    if(TIMER0->INTSTATUS == 1)
    {
        TIMER0->INTCLEAR = 1;
        ArmLed += 1;
    }
}
