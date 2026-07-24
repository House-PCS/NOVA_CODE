/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        timed_polling.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-1-25
  * @brief       定时器轮询，需要较为准确的定时轮询业务放在定时器中断中，防止主
                 循环部分业务占用时间过长导致其他业务无法定时执行
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-1-25     <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "timed_polling.h"
#include "dev_mbi502x_led.h"
#include "dev_key.h"
#include "slave_bms.h"
#include "slave_pcs.h"
#include "slave_collect.h"
#include "firmware_upgrate.h"
#include "system_ctrl.h"
#include "slave_meter.h"

//#include "slave_dsp.h"
#include "slave_coprocessor.h"

static void _TimerUpIrqProcess(void);
static void _CommTimerUpIrqProcess(void);
/**
  * @brief   定时器轮询初始化
  * @param   None
  * @return  None
  * @note    None
  */
void TimedPollingInit(void)
{
//    TimerPollingHandle.timer_cfg->period = 5 * 1000;//目前轮询周期是10ms
    TimerPollingHandle.irq_process = _TimerUpIrqProcess;
    TimerPollingBspInit();
}


/**
* @brief   定时器轮询处理函数，很多对时间要求高的轮询业务放在此处
* @param   None
* @return  None
* @note    这里只放轮询业务函数，具体业务如果轮询时间更长，则在其轮询内部进行计数再处理，时间是该轮询的倍数关系
*/

static void _TimerUpIrqProcess(void)
{
    if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode != 1)
    {
        if(gDeviceVersionType == DEVICE_TYPE_LCD)
        {
        
            if((BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over == 1)&&(gstFirmwareUpgrateInfo.upgrading_status == 0))
            {
                KeyLedProcess();
                DevKeyDetectProcess();
                DrmStatusDectctProc();
            }

        }
        DevModbusDirCtlProcess();
    }
}






void CommTimedPollingInit(void)
{
    CommTimerPollingHandle.irq_process = _CommTimerUpIrqProcess;
    CommTimerPollingBspInit();
}
/**
* @brief   LED控制函数
* @param   None
* @return  None
* @note    
*/void MoodLedCtrl(uint32_t clour_value,uint16_t mode,uint16_t period)
{
    if((BmsCtrlInfo.bms_ctrl_info_value.bits.f_addring_over == 1)&&(gstFirmwareUpgrateInfo.upgrading_status == 0))
    {
        SetMoodLedStaInfo(clour_value,mode,period);
        BmsMoodLedCtrl(clour_value,mode,period);
    }
}

/**
* @brief   定时器轮询处理函数，专门处理通讯模块
* @param   None
* @return  None
* @note    50ms轮询一次
*/

static void _CommTimerUpIrqProcess(void)
{
    uint32_t clour_value    = 0;
    uint32_t heat_beat_time = 0;
    static uint8_t status   = 0;
    volatile static uint8_t MoodLedCtrlHeatBeatTime = 0;
    
    heat_beat_time = gstCollectInfo.maincard_data.device_info.led_aux_period>>1;
    if(heat_beat_time < 200)
    {
        heat_beat_time = 200;
    }
    if(MoodLedCtrlHeatBeatTime++>=(heat_beat_time/50))
    {
        MoodLedCtrlHeatBeatTime = 0;
        //节拍
        status = !status;
        //颜色
        clour_value = gstCollectInfo.maincard_data.device_info.led_aux_b;
        clour_value<<=8;
        clour_value |= gstCollectInfo.maincard_data.device_info.led_aux_g;
        clour_value<<=8;
        clour_value |= gstCollectInfo.maincard_data.device_info.led_aux_r;
        clour_value <<=8;
        clour_value |= status;
        
        //开机进入正常运行状态前与收到关机信号后，关闭侧灯板颜色
        if(((gstPackSlavePara.pack_para[0].bms_info.ctrl_info.shutdown & 0X0F )!= 0) ||
            (GetSysRunStatus() != EN_SYS_WORKING_STATUS)
          )
        {
            clour_value &= 0X000F;
        }
        MoodLedCtrl(clour_value,gstCollectInfo.maincard_data.device_info.led_aux_mode,gstCollectInfo.maincard_data.device_info.led_aux_period);
    }

    if(gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode != 1)
    {
        DevModbusDirCtlProcess();
        UpgrateFirmwareProcess();

        switch(gstFirmwareUpgrateInfo.updata_slave_type)
        {
            //升级分发固件不影响另外的通讯。
            case UPDATA_PCS_TYPE:
            case UPDATA_ARC_TYPE:
            case UPDATA_PCS_COP_TYPE:
            {     
                SlaveCoproModbusCommProcess();
                BmsCommProc();
                break;
            }
            case UPDATA_BMS_TYPE:
            case UPDATA_FPGA_TYPE:
            {
                SlaveCoproModbusCommProcess();
                DspCommProc();
                break;
            }
            case UPDATA_MAIN_COP_TYPE:
            {
                DspCommProc();
                BmsCommProc();
                break;
            }
            default:
            {
                DspCommProc();
                BmsCommProc();
                SlaveCoproModbusCommProcess();
                break;
            }
        }

        BmsAddrFun();
        SlaveCollectModbusCommProcess();
        SystemTurnOffCtrl();//开关机控制
//        SlaveMeterModbusCommProcess();
//        JudgeDRM0ConnectStatus();
        
        
    }
}









