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





/***** 2024.2.29 *****/
/*修改内容:1.电弧触发实时状态、历史状态、绝缘阻抗报警、故障状态等函数的参数改为结构体成员变量（thershold.xxx）
           2.增加ModeId\SN\阈值信息写入FLASH、从FLASH读取函数
           3.增加读、写策略，具体策略实现在modbus.c文件中
           4.将dev_modbus.c文件移动至dev层
*/

/****** 2024.3.6 ******/
/*修改内容:1.电弧采样不准问题排查，产生原因：外部FLASH的引脚与ADC采样引脚冲突
           2.去直流分量由平均值改为2048，也发现，减平均值时，FFT刚开始的直流分量很小
           3.增加面积阈值法

*/
/*******2024.4.12******/
/*修改内容:1.去除绝缘阻抗检测case0-case4的四通道检测模式，采取一开始就选择通道，只用一路来进行绝缘检测
           2.去掉拉弧检测检测case0-case4的四通道检测模式，策略为一律认为四通道检测

*/
/*******2024.7.1******/
/*修改内容:1.第一版测试问题修复，增加中位数+差值绝对值和判断阈值方法，效果一般，仍采用平均值+数点
           2.绝缘阻抗bug修复。内容：（1）一短一断情况，已规避，但仍需解决。（2）无电压通道最大值显示
                                    （3）一断一电阻（6MΩ），ok    (4)一短路一电阻，只能判断短路，不能检测出电阻。

*/
/*******2024.7.3******/
/*软件版本：2.0.0*/
/*修改内容:升级重发机制完善，之前是将两个字节seqid和N字节（可调节长度）改为2+可调节长度=N; 初步验证ok


*/
/*
升级ok,通讯ok,软件版本等初始化，掉电保存
*/


/*
 * @file main.c
 * @author Nations
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#include <stdio.h>
#include "main.h"
#include "bsp.h"
#include "dev_led.h"
#include "dev_cur_detect.h"
#include "dev_modbus_s.h"
#include "math.h"
#include "arm_math.h"
#include "uart.h"
#include "dma.h"
#include "arc_detect_process.h"
#include "arc_detect_threshold.h"
#include "alarm.h"
#include "modbus_analysis.h"
#include "arc_detect_info.h"
#include "dev_mco.h"
#include "dev_chan_sw.h"
#include "res_detect_process.h"
#include "dev_ht7017.h"
#include "proto_uniform.h"
#include "dev_spiflash.h"

#define OFFSET_VECTORTABLE      ((uint32_t)0x08008000)

static ModbusRTURevData_t *p_rtu_cmd;
/**
 * @brief  Main program
 */
uint8_t channal_num = 0;
uint8_t res_detect_ok = 0;

int main(void)
{
	
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, OFFSET_VECTORTABLE);
    
    systick_config();
    BaseBspInit();
    DO_BspInit();
    DevAdcCurDetectInit();
    DevModbusRTUInit();
    DevLedInit();
    DevMcoInit();       //函数可封装在7010初始化中
    DetectIdBspInit();
    DetectChannelSwitchBspInit();
    Res_Detect_Init();
    DevHt7010Init();
//  DeviceBaseInfoWrite();
    SysInfoRead();
    ModbusRegInit();
    channal_num = dev_chan_detect();
	__set_PRIMASK(0);   //打开总中断
    while(1)
    {
//        ArcHandleClearProcess();
        DevModbusDirCtlProcess();
        ModbusCmdProcess();

        if(Modbus_writeReg.resdetectenable == 1)
        {
//            channal_switch = 0;
        }
        if((channal_switch < channal_num) || (Modbus_writeReg.resdetectenable == 1) || Res_Retry_IsBusy())   ////上电检测一次，检测有效通道
        {
            SampleRes_Main(channal_switch);
            if(gBmsSampleRes[channal_switch].sample_over == 1)
            {
                Modbus_ReadReg.arcselftestresult = Modbus_ReadReg.arcselftestresult | (1 << channal_switch);
                channal_switch++;
            }
            if(channal_switch >= channal_num)
            {
                res_detect_ok = 1;
                if(Res_Retry_IsActive())
                {
                    Res_Retry_OnDetectComplete(channal_num);
                }
                else
                {
                    if(Res_Detect_Alarm() && (gArcState() == 0))
                    {
                        Res_Retry_Start();
                    }
                }
            }
            Modbus_writeReg.resdetectenable = 0;
        }
        else
        {
            Res_Retry_Process();
            if(Modbus_writeReg.arcdetectlevel == 1 || enter_test)
            {
                ArcDectectProcess();              //电弧检测过程
                Arc_Alarm();                      //电弧检测报警处理
            }
//            else
//            {
//				AdcStopConv(&Adc1ArcCurHandle);
//                ArcStateInit();
//            }
        }
        /*升级处理*/
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










