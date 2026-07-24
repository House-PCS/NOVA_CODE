#include    "system_ctrl.h"
#include    "dev_it7c4337.h"
#include    "dev_spiflash.h"
#include    "storage_cap.h"
#include    "sys_info_storage.h"
teSysStatus         SystemStatus;

void SystemCtrlInit(void)
{
    SysInfoInit();
    CapInitl();   //  
                                            
}
static  void _CollectUartBaudSet(void)
{
    static uint16_t old_baud = 0;
    uint16_t cur_baud;
    cur_baud = GetCollectBaud();
    if(GetCollectBaud()!=old_baud)
    {
        old_baud = cur_baud;
        switch(cur_baud)
        {
            case 0:
            {
                SetUartRate(&ModbusSlaveCollectorUartHandle,115200);
                break;
            }
            case 1:
            {
                SetUartRate(&ModbusSlaveCollectorUartHandle,9600);
                break;
            }
            default:
                break;
        }
    }
}
void SystemCtrlProc(void)
{
 
    CapCtrl();    //
    _CollectUartBaudSet();
    if(GetCorrectRtcStatus() == 1)
    {
        CleanCorrectRtcFlag();
        CalibrationRtcTime((gCollectParaData.collect_data.rtc_time[0]<<16) + gCollectParaData.collect_data.rtc_time[1]); 
    }
    else
    {
        gCollectParaData.collect_data.rtc_time[0] = (uint16_t)(GetRtcTimeStamp()>>16);
        gCollectParaData.collect_data.rtc_time[1] = (uint16_t)(GetRtcTimeStamp());
    }
    switch(SystemStatus)
    {
        case SYS_INIT_STATUS:
        {
            break;
        }
        case SYS_WAKEUP_STATUS:
        {
            break;
        }
        case SYS_IDLE_STATUS:
        {
            break;
        }
        case SYS_WORKING_STATUS:
        {
            break;
        }
        case SYS_ERROR_STATUS:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}
teChgAdjustStatus   ChgAdjustStatus;

void ChgAdjustProc(void)
{
    switch(ChgAdjustStatus)
    {
        case CHG_INIT:
        {
            break;
        }
        case CHG_SET_MAX_VOLT_STATUS:
        {
            break;
        }
        case CHG_SET_MAX_CURR_STATUS:
        {
            break;
        }
        case CHG_SET_POWER_STATUS:
        {
            break;
        }
        case CHG_WORKING_STATUS:
        {
            break;
        }
        case CHG_OVP_STATUS:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}

void ErrCheckFun(void)
{
    
}
bool  CheckTimeData(uint32_t   start_tamp,uint32_t end_tamp,uint32_t cur_tamp)
{

    DevRtc_t    start_time;
    DevRtc_t    end_time;
    DevRtc_t    cur_time;
    uint16_t    start_sec;
    uint16_t    end_sec;
    uint16_t    cur_sec;
    bool        status;

    start_time.time_stamp = start_tamp;
    RtcSecConvert2Date(&start_time);
    start_sec  = start_time.hour*3600ul + start_time.minute*60ul + start_time.second;

    end_time.time_stamp   = end_tamp;
    RtcSecConvert2Date(&end_time);
    end_sec  = end_time.hour*3600ul + end_time.minute*60ul + end_time.second;

    cur_time.time_stamp = cur_tamp;
    RtcSecConvert2Date(&cur_time);    
    cur_sec  = cur_time.hour*3600ul + cur_time.minute*60ul + cur_time.second;    


    if(start_sec > end_sec)
    {
        status = false ; 
    }
    else
    {
        if((cur_sec >= start_sec) && (cur_sec <= end_sec))
        {
            status = true ;
        }
        else
        {
            status = false ;            
        }
    }
    return status;

}

//消峰填谷
void  PeakLoadShiftFun(tsPeakLoadShiftData peak_load_shift_time)
{
//cur_time;
//需要转为时间戳
    bool    status;
    bool    status1;
    status  = CheckTimeData(peak_load_shift_time.chg_start_time1,peak_load_shift_time.chg_end_time1,RtcTime.time_stamp);
    status1 = CheckTimeData(peak_load_shift_time.chg_start_time2,peak_load_shift_time.chg_end_time2,RtcTime.time_stamp);
    if((status == true) ||(status1 == true))
    {
        ;// 充电使能
    }else
    {
        ;//充电截至
    }

    status  = CheckTimeData(peak_load_shift_time.dsg_start_time1,peak_load_shift_time.dsg_end_time1,RtcTime.time_stamp);
    status1 = CheckTimeData(peak_load_shift_time.dsg_start_time2,peak_load_shift_time.dsg_end_time2,RtcTime.time_stamp);
    if((status == true) ||(status1 == true))
    {
        ;// 放电使能
    }else
    {
        ;// 放电截至
    }

}

