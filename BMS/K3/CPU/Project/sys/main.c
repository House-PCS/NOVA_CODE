/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        main.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-6-1
  * @brief
  ******************************************************************************
  * @relation
  * @par :
  * <table>
  * <tr><<th>Date          <<th>Version   <<th>Author      <<th>Description
  * <tr><td>2025-6-1        <td>V1.0.0    <td>              <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "main.h"
#define DEBUG   0
#define LOG_TAG     "main"
#define OFFSET_VECTORTABLE      ((uint32_t)0x08008000)
int main(void)
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, OFFSET_VECTORTABLE);
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);  // 优先级分组
    __set_PRIMASK(0);   // 打开总中断
    systick_config();   // 滴答定时器初始化
    soft_delayms(20);
    BspBaseInit();
    BspInit();
    BMSBoardPowerOn();
    DevFeedExtWdg();
    rcu_periph_clock_enable(RCU_PMU);
    UART1ToFPGASoftcoreInit();
    Uart0Init();//串口打印
    LedInit();
    KeyInit();
    SpiFlashInit();
    RtcInit();
    DevEepromCtlInit();
//    cm_backtrace_init_u16("GD32F303", gSysInfo.hw_version, gSysInfo.sw_version);
    AFEInit();
    ModbusRegInit();
    SysInfoInit();
    if(DEBUG)
    {
        gSoxDebugParam.sox_debug_mode = 1;
    }
    else
    {
        gSoxDebugParam.sox_debug_mode = 0;
    }
    while(1)
    {
        FaultAnalysisProcess();
        AFEProcess();
        LedProcess();
        UpdateModbusReg();
        ModbusCmdProcess();
        DebugProcess();
        ChargeDischargeControl();
        KeyDetectProcess();
        KeyValueProcess();
        DevFeedExtWdg();
    }
}
