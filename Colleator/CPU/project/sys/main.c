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
#include "config.h"
#include "main.h"

#include "bsp.h"
#include "sys_info.h"
#include "peri_process.h"
#include "dev_fc41d.h"
#include "proto_nova_uniform.h"
#include "collector.h"
#include "dev_board_id.h"
#include "n32g4fr_iwdg.h"
#include "dev_dbg.h"
#include "dev_modbus_h.h"

#define PCS_BAUD_RATE		9600
#ifndef PCS_BAUD_RATE
#error "Please define PCS_BAUD_RATE"
#endif 

#define OFFSET_VECTORTABLE      ((uint32_t)0x08008000)

/*
20251107  1 修改上报一次版本号问题导致升级成功后 APP无法获取新版本信息    版本升级1.6.4
20251121  1 修改五分钟上报数据一致问题 2 在队列获取中添加中断的开关 
          3 扩大FC41D_CTX_BASE_SIZE的大小 4 修改采集器向下获取信息的时间
20260114  1 新增AI模式寄存器 2 新增并机所用的寄存器   版本升级1.6.5
20260407  1 修改UART串口驱动程序bug    版本升级->1.6.6
20260409  1 修改UART中导致模块不启动的bug  版本转测
20260427  1 新增蓝牙通过AT指令设置UUID时的无响应方式写入，指示，通知功能打开 版本升级->1.6.7
20260516  1 新增远程关机寄存器，APP通过下发关机指令进行整机关机操作 版本升级->1.6.8
20260519  1 新增CSIP功率平均值信息回读
20260528  1 新增CSIP中PCS端实时的功率值回读
20260623  1 修改CSIP中24个队列只下发16个队列的问题

*/

__IO uint32_t LsiFreq     = 40000;

int iwdg_init(void)
{
    /* Check if the system has resumed from IWDG reset */
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
       /* IWDGRST flag set */
       /* Clear reset flags */
       RCC_ClrFlag();
    }
    else
    {
       /* IWDGRST flag is not set */
    }
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteConfig(IWDG_WRITE_ENABLE);
    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescalerDiv(IWDG_PRESCALER_DIV256);
    /* Set counter reload value to obtain 250ms IWDG TimeOut.
       Counter Reload Value = 250ms/IWDG counter clock period
                            = 250ms / (LSI/32)
                            = 0.25s / (LsiFreq/32)
                            =/(32 * 4)
                            = LsiFreq/128 */
    IWDG_CntReload(LsiFreq * 21 / 256);
    /* Reload IWDG counter */
    IWDG_ReloadKey();
    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();
    return 0;
}

#if 0
int test_flash_read()
{
	{
		typedef struct _LocalUpgrade_t
		{
			int32_t recv_size;
			uint32_t pkg_num;
			uint32_t last_pkg_size;		// 最后一次写数据大小
			uint32_t last_flash_addr;	// 最后一次写flash地址
			uint8_t firmware[64];
			uint8_t recv_md5[16];		// 接收到下发的md5
		} LocalUpgrade_t;

		uint32_t start_addr = 0x3E8000;
		LocalUpgrade_t upgrade_ctx = {0};
		upgrade_ctx.recv_size = 100;
		upgrade_ctx.pkg_num = 101;
		upgrade_ctx.last_pkg_size = 102;
		upgrade_ctx.last_flash_addr = 103;
		SpiFlashWriteBytesWithErase(start_addr, (uint8_t *)&upgrade_ctx, sizeof(LocalUpgrade_t));

		memset(&upgrade_ctx, 0, sizeof(upgrade_ctx));
		SpiFlashReadData(start_addr, (uint8_t *)&upgrade_ctx, sizeof(LocalUpgrade_t));

		memset(&upgrade_ctx, 0, sizeof(upgrade_ctx));

	}
	while (1)
	{
		uint8_t test[1024] = {0};
		uint32_t start_addr = 0x200000;
		SpiFlashReadData(start_addr, test, sizeof(test));
	}
}
#endif 


/** 
 * @brief  Main program
 */
int main(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, OFFSET_VECTORTABLE);
    __set_PRIMASK(1);   // 打开总中断
    systick_config();
    BaseBspInit();
    DevSpiFlashInit();
    DevModbusHostRTUInit(PCS_BAUD_RATE);
    DevLedInit();
    DevKeyInit(); 
    DevBoardIDInit();
    DevDbgInit();
#if 0
	test_flash_read(); }
#endif 
    
    SysInfoInit();

	DevOpInit();

    PeriStatusInit();

    CollectorInit();
    //printf("Test123\r\n");
#if COLLECTOR_IWDG
	iwdg_init();
#endif 
    __set_PRIMASK(0);   // 打开总中断

    while(1)
    {
#if COLLECTOR_IWDG
		IWDG_ReloadKey();
#endif 
        DevModbusDirCtlProcess();
		DevOpProcess();
        DevLedProcess();
        DevKeyDetectProcess();
        PeriProcess();
		CollectorProcess();
    }
}



