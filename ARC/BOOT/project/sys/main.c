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
#include "bsp.h"
#include "peri_process.h"
//#include "boot.h"
#include "firmware_iap.h"
#include "proto_uniform.h"
#include "modbus_analysis.h"

#define OFFSET_VECTORTABLE      ((uint32_t)0x08000000)
/** 
 * @brief  Main program
 */
             
static ModbusRTURevData_t *p_rtu_cmd;

int main(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, OFFSET_VECTORTABLE);
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    systick_config();
    BaseBspInit();
	DevModbusRTUInit();
    ModbusRegInit();
//    JudgeIfBootMode();   
//    EnterBootModeJudge();
	uint8_t Power_on_test = 0;
     while(1)
    { 
        SysFirmwareIap();
		DevModbusDirCtlProcess();         //modbus通信发送、接收设置
		ModbusCmdProcess();	              //modbus通信轮询
		if(com_revupgradedata.is_update_data == true)
		{						
			if((tim_get_systime() - com_revupgradedata.overtime) > 100)
			{
				ProtoUnpackJudge(&com_revupgradedata.com_revdata);
				com_revupgradedata.overtime = tim_get_systime();
				com_revupgradedata.is_update_data = false;
			}							
		}
        UpgradeCompletehandle();
	}
}




