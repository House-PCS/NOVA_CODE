#include "alarm.h"
#include "dev_led.h"
#include "modbus_analysis.h"
#include "arc_detect_threshold.h"
#include "dev_chan_sw.h"
#include "arc_detect_process.h"
#include "arc_detect_info.h"
#include "dev_cur_detect.h"
#include "main.h"
#include "res_detect_process.h"
#include "systick.h"
#include "bsp.h"

#include "flash.h"


static uint8_t  s_res_retry_active = 0;    
static uint8_t  s_res_retry_count = 0;     
static uint32_t s_res_retry_last_time = 0; 


static void Res_AlarmIoSet(void)
{
    GpioBitSet(&LedSysHandle);
    GpioBitSet(&ToMintorHandle);
}


static void Res_AlarmIoRelease(void)
{
    GpioBitReset(&LedSysHandle);
    GpioBitReset(&ToMintorHandle);
}


static uint16_t Res_GetBreakdownNow(void)
{
    uint16_t breakdown = 0;
    uint8_t num = dev_chan_detect();
    uint8_t i;

    for(i = 0; i < num && i < 2; i++)
    {
        float th_rx = (float)thershold_info.impedance_thersholdrx1 * 0.1f + Flash_DateReg.res_hysteresis_increment;
        float th_ry = (float)thershold_info.impedance_thersholdry1 * 0.1f;

        if(i == 1)
        {
            th_rx = (float)thershold_info.impedance_thersholdrx2 * 0.1f + Flash_DateReg.res_hysteresis_increment;
            th_ry = (float)thershold_info.impedance_thersholdry2 * 0.1f;
        }
        if(res_judge(gBmsSampleRes[i].res_p1, gBmsSampleRes[i].res_n1, th_rx, th_ry) != 0)
        {
            breakdown |= (0x01U << i);
        }
    }
    return breakdown;
}


static uint32_t Res_RetryGetIntervalMs(void)
{
    if(s_res_retry_count >= RES_RETRY_COUNT_TO_2H)
    {
        return RES_RETRY_INTERVAL_2H_MS;
    }
    return RES_RETRY_INTERVAL_5MIN_MS;
}



void Res_Detect_Init(void)
{
    uint8_t flash_buff[4] = {0};
    InternalFlashRead(RES_BREAKED_HYSTERESIS_ADDR, flash_buff, 4);
    if(flash_buff[0] == 0x5A)
    {
        Flash_DateReg.res_hysteresis_increment = RES_HYSTERSIS_INCREMENT;
        Flash_DateReg.trigger_alarm_flag = 0x0;
        InternalFlashWrite(RES_BREAKED_HYSTERESIS_ADDR, (uint32_t*)&Flash_DateReg, 1);
    }
    else
    {
        Flash_DateReg.res_hysteresis_increment = 0;
    }
}



uint8_t Res_Detect_Alarm(void)
{
    uint16_t res_breakdown = 0;
    uint8_t channal_num = dev_chan_detect();
    uint8_t i;

    res_breakdown = gResRiskState(0.1f);  
    for(i = 0; i < channal_num; i++)
    {
        if((res_breakdown & (0x01U << i)) == (0x01U << i))
        {
            Res_AlarmIoSet();  
            Flash_DateReg.trigger_alarm_flag = 0x5A;
            InternalFlashWrite(RES_BREAKED_HYSTERESIS_ADDR, &Flash_DateReg.trigger_alarm_flag, 1);
            return 1;
        }
    }
    return 0;
}


void Res_Retry_Start(void)
{
    s_res_retry_active = 1;
    s_res_retry_count = 0;
    s_res_retry_last_time = tim_get_systime();
}


uint8_t Res_Retry_IsActive(void)
{
    return s_res_retry_active;
}


uint8_t Res_Retry_IsBusy(void)
{
    if(s_res_retry_active == 0)
    {
        return 0;
    }
    return (channal_switch < dev_chan_detect()) ? 1 : 0;
}


void Res_Retry_OnDetectComplete(uint8_t channal_num)
{
    if(s_res_retry_active == 0)
    {
        return;
    }
    if(channal_switch < channal_num)
    {
        return;
    }
    if(Res_GetBreakdownNow() == 0)
    {
        
        s_res_retry_active = 0;
        s_res_retry_count = 0;
        Res_AlarmIoRelease();
        return;
    }
    
    s_res_retry_last_time = tim_get_systime();
}


void Res_Retry_Process(void)
{
    if(s_res_retry_active == 0)
    {
        return;
    }
    if(Res_Retry_IsBusy())
    {
        return;
    }
    if(tim_slot_check(&s_res_retry_last_time, Res_RetryGetIntervalMs()) == false)
    {
        return;
    }
    s_res_retry_count++;
    Res_Detect_PrepareRetry();
}


void Arc_Alarm(void)
{
    static uint8_t error_flag = 0;
    uint8_t channal_num = dev_chan_detect();
    uint16_t arc_alarm_flag = gArcState();//或者等于全局变量
    if(!error_flag)//只要拉低一次就不再检测
    {
        for(uint8_t i = 0; i < channal_num; i++)
        {
            if((arc_alarm_flag & (0x0001 << i)) == (0x0001 << i))
            {
                GpioBitSet(&LedSysHandle);
                GpioBitSet(&ToMintorHandle);
                error_flag = 1;
                break;
            }
        }
    }
}


//    uint8_t Arc_Warninig = alarm_flag;
//  if((alarm_flag & (0x01<<0)) == (0x01<<0))
//  {
//        GpioBitSet(&LedSysHandle);
//        GpioBitSet(&ToMintorHandle);
//    //此处可通过指针数组的形式进行处理，避免冗余
//  }
//  else if((alarm_flag & (0x01<<1)) == (0x01<<1))
//  {
//        GpioBitSet(&LedSysHandle);
//        GpioBitSet(&ToMintorHandle);
//  }
//  else if((alarm_flag & (0x01<<2)) == (0x01<<2))
//  {
//        GpioBitSet(&LedSysHandle);
//        GpioBitSet(&ToMintorHandle);
//  }
//  else if((alarm_flag & (0x01<<3)) == (0x01<<3))
//  {
//        GpioBitSet(&LedSysHandle);
//        GpioBitSet(&ToMintorHandle);
//  }
//  else
//    {
//        if(res_detect_ok)
//        {

//            GpioBitReset(&LedSysHandle);
//            GpioBitReset(&ToMintorHandle);
//        }
//    }

//    if(!error_flag)
//    {
//        if(Arcjudgment(DetectProcessHandle.FFT1_OutputMag_f32,DetectThreshold.Threshold1,thershold_info.arc_threshold1) !=0)
//        {
//            error_flag = 1;
//            GpioBitSet(&LedSysHandle);
//        }
//         else if(Arcjudgment(DetectProcessHandle.FFT2_OutputMag_f32,DetectThreshold.Threshold2,thershold_info.arc_threshold2) !=0)
//        {
//            error_flag = 1;
//            GpioBitSet(&LedSysHandle);
//        }
//        else
//        {
//            error_flag = 0;
//            GpioBitReset(&LedSysHandle);
//        }
//    }
//}
//void Alarm(uint8_t value)
//{
//  uint8_t Sw_Flag = 0;
//  Sw_Flag = dev_chan_sw();
//  switch(Sw_Flag)
//  {
//      case EN_SW_CHANNAL1:
//      {
//          if(value != 0)
//          {
//              GpioBitSet(&LedSysHandle);
//          }
//          else
//          {
//              GpioBitReset(&LedSysHandle);
//          }
//          break;
//      }
//      case EN_SW_CHANNAL12:           //代码未完善，存在一路报警、一路不报警情况，后续根据报警动作进行调整
//      {
//          if(value != 0)
//          {
//              GpioBitSet(&LedSysHandle);
//          }
//          else
//          {
//              GpioBitReset(&LedSysHandle);
//          }
//          break;
//      }
//      case EN_SW_CHANNAL123:
//      {
//          if(value != 0)
//          {
//              GpioBitSet(&LedSysHandle);
//          }
//          else
//          {
//              GpioBitReset(&LedSysHandle);
//          }
//          break;
//      }
//      case EN_SW_CHANNAL1234:
//      {
//          if(value != 0)
//          {
//              GpioBitSet(&LedSysHandle);
//          }
//          else
//          {
//              GpioBitReset(&LedSysHandle);
//          }
//          break;
//      }
//  }
//}





