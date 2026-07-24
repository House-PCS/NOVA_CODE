/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
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
  * <tr><td>2024-2-4    <td>V1.0.1    <td>xingxiao    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "systick.h"

static __IO uint32_t TimingDelay = 0;
static __IO uint32_t SysTimingMs = 0;

void SystickInit(void)
{
    uint32_t temp = SystemCoreClock / 1000;             //1ms interrupt
    SysTick->LOAD = temp;
    SysTick->VAL = 0;                                       //Reset current counter's value

    //select clock source, enable interrupt, enable counter
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
    NVIC_EnableIRQ(SysTick_IRQn);
}

/**
  * @param nTime*ms
  * @return none
  * @brief delay ms
  */
void DelayMs(__IO uint32_t nTime)
{
    TimingDelay = nTime;

    //enable systick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    while(TimingDelay != 0);
}

/**
  * @param none
  * @return none
  * @brief get systick,add into SysTick_Handler
  */
void TimingDelay_Decrement(void)
{
    if(TimingDelay != 0x00)
    {
        TimingDelay--;
    }
}

void _TimUpdateSystime(void)
{
    SysTimingMs++;
    TimingDelay_Decrement();
}

void SysTick_Handler(void)
{
    _TimUpdateSystime();
}

uint32_t TimGetSystime(void)
{
    return SysTimingMs;
}

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

uint32_t TimDifftimeFromNow(uint32_t time)
{
    return TimDifftime(time, SysTimingMs);
}

bool TimSlotCheck(uint32_t *pUserTimeStamp, uint32_t Expect_SlotMs)
{
    if(TimDifftime(*pUserTimeStamp, SysTimingMs) >= Expect_SlotMs)
    {
        *pUserTimeStamp = SysTimingMs;
        return true;
    }

    return false;
}

void SoftDelayus(const uint32_t usec)      //需要实际测试使其与F2延时公式保持一致 need change
{
    uint32_t count = 0;
    const uint32_t utime = (50 * usec / 5);
    do
    {
        if(++count > utime)
        {
            return ;
        }
    }
    while(1);
}

void SoftDelayms(uint32_t msec)
{
    while(msec > 1000)
    {
        SoftDelayus(1000 * 1000);
        msec = msec - 1000;
    }
    SoftDelayus(msec * 1000);
}
