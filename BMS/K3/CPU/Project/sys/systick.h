/*!
    \file    systick.h
    \brief   the header file of systick

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

#ifndef SYS_TICK_H
#define SYS_TICK_H

#include <stdbool.h>
//#if defined GD32F450
//    #include "gd32f4xx.h"
//#elif defined GD32F30X_HD || GD32F30X_CL
//    #include "gd32f30x.h"
//#endif
#include "gd32f30x.h"
typedef void(*time5ms_callback_fun)(void);
typedef void(*time_beep_callback_fun)(void);
void  RegisterBeepCallBackFun(time_beep_callback_fun fun);
void  Register5msCallBackFun(time5ms_callback_fun  fun);
/* configure systick */
void systick_config(void);
/* delay a time in milliseconds */
void delay_1ms(uint32_t count);
/* delay decrement */
void delay_decrement(void);
void     systick_delayms(__IO uint32_t time_ms);
void     soft_delayus(const uint32_t usec);
void     soft_delayms(const uint32_t msec);
void     tim_update_systime(void);
bool     tim_slot_check(uint32_t *pUserTimeStamp, uint32_t Expect_SlotMs);
uint32_t TimGetSystime(void);
uint32_t TimDifftime  (uint32_t time, uint32_t new_time);
uint32_t TimeDiffFromNow(uint32_t time);

#define NOVA_GET_BIT(data, i)      ((data >> i) & 0x01)
#define NOVA_SET_BIT(data, i)      ((data) |= 1<<(i))
#endif /* SYS_TICK_H */
