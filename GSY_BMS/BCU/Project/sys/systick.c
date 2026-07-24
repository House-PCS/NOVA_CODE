/*!
    \file    systick.c
    \brief   the systick configuration file

    \version 2016-08-15, V1.0.0, firmware for GD32F4xx
    \version 2018-12-12, V2.0.0, firmware for GD32F4xx
    \version 2020-09-30, V2.1.0, firmware for GD32F4xx
    \version 2022-03-09, V3.0.0, firmware for GD32F4xx
*/

/*
    Copyright (c) 2022, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f4xx.h"
#include "systick.h"


volatile static uint32_t delay;
volatile uint32_t SysTiming_ms = 0;
static volatile uint32_t TimingDelay = 0;
/*!
    \brief    configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if(SysTick_Config(SystemCoreClock / 1000U)) {
        /* capture error */
        while(1) {
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/*!
    \brief    delay a time in milliseconds
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t count)
{
    delay = count;

    while(0U != delay) {
    }
}

/*!
    \brief    delay decrement
    \param[in]  none
    \param[out] none
    \retval     none
*/
void delay_decrement(void)
{
    if(0U != delay) {
        delay--;
    }
}
/**
  * @brief  Get system time in milli-second from systick
  * @param  None
  * @return Current system time
  */
uint32_t tim_get_systime(void)
{
    return SysTiming_ms;
}
/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    { 
    TimingDelay--;
    }
}
/**
  * @brief  update SysTiming_ms
  * @param  None
  * @return None
  */
void tim_update_systime(void)
{
    SysTiming_ms++;
    TimingDelay_Decrement();
}
/**
  * @brief  Calculate the difference of two time values.
  * This function will handle overflow problem.
  * @param  time[IN]: First time value to calculate
  * @param  new_time[IN]: 2nd time value to calculate
  * @return Time difference of 2 time values.
  */
uint32_t tim_difftime(uint32_t time, uint32_t new_time)
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
bool SysTimeOut(uint32_t old_time, uint32_t new_time,uint32_t times)
{
    uint32_t diffTime = 0;
    if (new_time >= old_time) /* not overflow */
    {
        diffTime = (new_time - old_time);
    }
    else
    {
        diffTime = new_time + (0xffffffffUL - old_time);
    }
    if(diffTime>=times)
    {
        return true;
    }
    return false;
}
/**
  * @brief  Calculate the time difference from now.
  * @param  time[IN]: Previous recorded time value
  * @return Time difference.
  */
uint32_t tim_difftime_from_now(uint32_t time)
{
    return tim_difftime(time, SysTiming_ms);
}

/**
  * @brief  Check a TimeSlot
  * @param  None
  * @retval bool
  */
bool tim_slot_check(uint32_t *pUserTimeStamp,uint32_t Expect_SlotMs)
{
    if(tim_difftime(*pUserTimeStamp, SysTiming_ms) >= Expect_SlotMs)
    {
    *pUserTimeStamp =SysTiming_ms; 
    return true;
    } 
  
    return false;
}

/**
  * @brief  延时函数，单位1ms.
  * @param  需要延时time_ms毫秒
  * @retval None
  */
void systick_delayms(__IO uint32_t time_ms)
{
    TimingDelay = time_ms;
    while(TimingDelay != 0);
}
/**
  * @brief  延时函数，单位1us.
  * @param  需要延时time_us毫秒
  * @retval None
  * @note 经示波器测试在400M主频及0级优化下延时1us是准确无误的,优化等级3会略快一点
  */
void soft_delayus(const uint32_t usec)      //需要实际测试使其与F2延时公式保持一致 need change
{
    uint32_t count = 0;
    const uint32_t utime = (120 * usec / 7);        
    do
    {
        if(++count > utime)
        {
            return ;
        }
    }while(1);
}
/**
  * @brief  延时函数，单位1ms.
  * @param  需要延时time_ms毫秒
  * @retval None
  * @note 
  */
void soft_delayms(uint32_t msec)
{
    while(msec > 1000)
    {
        soft_delayus(1000 * 1000);
        msec = msec - 1000;
    }
    soft_delayus(msec * 1000);
}
