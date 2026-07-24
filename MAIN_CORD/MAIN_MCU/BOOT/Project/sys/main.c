/*!
    \file    main.c
    \brief   led spark with systick

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
#include <stdio.h>
#include "main.h"
#include "bsp.h"
#include "GUI.h"
#include "dev_modbus.h"
#include "dev_spiflash.h"
#include "dev_key.h"
#include "dev_it7c4337.h"
#include "dev_mbi502x_led.h"
#include "em_menu.h"
#include "dev_lcd.h"
#include "dev_atmos_sk68xx.h"
#include "dev_tca9555.h"

#include "slave_bms.h"
#include "slave_pcs.h"
#include "slave_collect.h"
#include "firmware_upgrate.h"
#include "system_ctrl.h"
#include "timed_polling.h"
#include "atmos_led.h"
#include "sys_info_storage.h"
#include "boot.h"
#include "dev_modbus_rtu.h"

/*
修改了BMS_485 D0/DI---PE3

20240327  新版本BOOT   
20240328  实际测试 BOOT已经OK
20240429  1 修正 升级显示百分比 文件大小需要用32位数据，之前8位有问题。
          2 修改APP起始地址位 0x08010000, GD47x前面4个SECTOR是16K位单位。APP放在0x08010000的位置。
            #define APP_FIRMWARE_INTERNAL_ADDR           0x10000
            #define APP_FIRMWARE_HEAD_INTERNAL_ADDR      0xFF00
20240430  1 修改程序，由于采集器需要先读取modeid 来确认固件，因此加入modbus协议。
20240507  1 加入解压程序，并修改升级程序存储位置，   监控卡从采集器接收到的压缩文件存储位置从 0X1000开始 ----0XC7FFF;
            监控卡收到数据之后，将解压后的数据存储在0XC8000 ----0x1FFFFF;
20240509  1 采集器升级，加入了重发机制，监控卡配合修改完成，已经测试完成
20240510  1 和采集器按照新打包的程序测试升级OK  升级地址 压缩数据放在0x4B0000  解压之后放在 0x5B0000
20240528  1 更换背光脚位
20240606  1 调整上电进入APP时间为1S ，在1S之内按上翻和下翻按键，则停留在BOOT
20240822  1 修改按键灯IO初始化
*/


/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
//0x4000000
//uint8_t test1[4096];
#define OFFSET_VECTORTABLE      ((uint32_t)0x08000000)//0x08010000 
static uint32_t starton_flag = 0;
#include "key_backlight_led.h"


#define UART_BAUD_MAX    7
const  uint32_t  UartBaudData[UART_BAUD_MAX] = {9600,19200,38400,57600,115200,256000,460800};
static  void _CollectUartBaudSet(void)
{
//    static uint16_t old_baud = 0;
//    static uint32_t time_out=0;
//    uint16_t cur_baud;
//    cur_baud = GetCollectBaud();
//    if(GetCollectBaud()!=old_baud)
//    {
//        if(tim_get_systime()- time_out >500)
//        {//延时500ms 需要按照之前波特率回复了之后，再进行切换。
//            time_out = tim_get_systime();
//            old_baud = cur_baud;
//            if(cur_baud >UART_BAUD_MAX)
//            {
//                return;
//            }
//            SetUartRate(&ModbusSlaveCollectorUartHandle,UartBaudData[cur_baud]);
//        }
//    }
//    else
//    {
//        time_out = tim_get_systime();
//        
//    }


    static uint16_t old_baud = 0;
    static uint32_t time_out=0;
    static uint32_t time_out1=0;
    static uint8_t  set_updata_baud_en = 1;
    uint16_t cur_baud;
    uint8_t  updata_baud_status;
    cur_baud = GetCollectBaud();
//延时500ms 再进行设置新的波特率

    updata_baud_status = GetUpdataBaudStatusInfo();
    if(updata_baud_status == 1)
    {
        old_baud = 0xff;
//延时500ms
        if(set_updata_baud_en ==1)
        {
            if(tim_get_systime()- time_out >300)
            {
                set_updata_baud_en = 0;
                SetUartRate(&ModbusSlaveCollectorUartHandle,UartBaudData[4]);    //设置波特率位115200 
            }   
        }
    }
    else
    {
        time_out1 = tim_get_systime();
        set_updata_baud_en = 1;
        if(GetCollectBaud()!=old_baud)
        {
            if(tim_get_systime()- time_out >500)
            {//延时是为了等响应完了之后再修改
                time_out = tim_get_systime();
                old_baud = cur_baud;
                if(cur_baud >UART_BAUD_MAX)
                {
                    return;
                }
                SetUartRate(&ModbusSlaveCollectorUartHandle,UartBaudData[cur_baud]);            
            }

        }
        else
        {
            time_out = tim_get_systime();
            
        }            
    }


}
static  void _CollectSlaveAddrSet(void)
{
    uint16_t cur_slave_addr;
    cur_slave_addr = GetCommAddr();
    if(gCollectParaData.collect_data.slave_comm_addr!=cur_slave_addr)
    {
        SetCommAddr((uint8_t)gCollectParaData.collect_data.slave_comm_addr);
    }
}

int main(void)
{

	__set_PRIMASK(1);   // 关闭总中断
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, OFFSET_VECTORTABLE);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 优先级分组	
    __set_PRIMASK(0);   // 打开总中断 
    systick_config();
    BaseBspInit();
    ExmcBspInit();
    DevSpiFlashInit();
    SysInfoInit();
    DevKeyInit();
	TimedPollingInit();
    DevModbusRTUInit(); 

    DevTca9555DevConfig();       
    CommCollectInit();
    JudgeIfBootMode();
    FirmwareUpgrateInit();
    DevLcdInit();
    while(1) 
    {
        DevModbusDirCtlProcess();
        UpgrateFirmwareProcess();
        SlaveCollectModbusCommProcess();
        _CollectUartBaudSet();
        _CollectSlaveAddrSet();
    }
}







