/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file main.c
 * @author Nations
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include <stdio.h>
#include "main.h"
#include "systick.h"
#include "bsp.h"

#include "dev_modbus_h.h"
#include "dev_atmos_sk68xx.h"
#include "dev_atmos_sy_adbic.h"

#include "slave_ledctl.h"
#include "timed_polling.h"
#include "atmos_side_led.h"
#include "firmware_upgrate.h"
#include "boot.h"

/*
20251225    1 新增上位机升级代码


*/

#define OFFSET_VECTORTABLE      ((uint32_t)0x08000000)
/** 
 * @brief  Main program
 */
 
uint8_t timer_test = 0;
uint8_t flag = 0;
 
RCC_ClocksType RCC_ClocksStatus;
int main(void)
{
    __set_PRIMASK(1);   // 关闭总中断+
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, OFFSET_VECTORTABLE);
    systick_config();
    
    RCC_GetClocksFreqValue(&RCC_ClocksStatus);
    BaseBspInit();
    
//    TimedPollingInit();   //定时器初始化
//    DevSyAdbicAtmosInit(); //初始化新固件氛围灯ok
//    DevAtmosInit();    //初始化氛围灯ok   sk68xx灯珠
    
    DevModbusHostRTUInit();
    __set_PRIMASK(0);   // 打开总中断
    SlaveLedCommInit();
    FirmwareUpgrateInit();
    JudgeIfBootMode();   
    
    while(1)
    {   
        SlaveLedCtlModbusCommProcess();
//        DevSideLedProcess();
        UpgrateFirmwareProcess();
    }
}



