#include    "system_ctrl.h"
#include    "dev_it7c4337.h"
#include    "dev_spiflash.h"
#include    "storage_cap.h"
#include    "sys_info_storage.h"
#include    "dev_lcd.h"
#include    "dev_led.h"
#include    "error_proc.h"
#include    "firmware_upgrate.h"
#include    "em_menu.h"
#include    "atmos_led.h"
#include    "can_proto_can.h"
#include    "can_tx_rx_process.h"
#include    "dev_mbi502x_led.h"
#include    "slave_meter.h"
#include    "em_menu.h"
#include    "error_proc.h"
#define     OIL_INPUT           1
#define     AC_INPUT            0

#define     FULL_SOC                            1000
#define     SELF_DSG_SOC_LIMIT_L1               950
#define     SELF_DSG_SOC_LIMIT_L2               900
#define     CHG_LIMIT_POWER_VALUE               100

#define     MAIN_TO_MAIN_COP_SOC_TIME           30 * 1000

const  uint32_t  garrUartBaudData[UART_BAUD_MAX] = {9600, 19200, 38400, 57600, 115200, 256000, 460800};
EN_SYS_STATUS    eSystemStatus;
SysCtrlInfo_t   _gstSysCtrlInfo;
static uint16_t _gBackLightTimeCnt;
uint8_t gSystemPowerStatus = 0;
static uint8_t _gCapReceiveTurnOff = 0;  ////关机前存储标志

typedef struct _on_grid_time_para_stru_t
{
    uint8_t   on_grid_flag;
    DevRtc_t  on_grid_time;
    DevRtc_t  off_grid_timre;
    DevRtc_t  running_time;
    DevRtc_t  total_on_grid_time;
} OnGridTimePara_t;
static OnGridTimePara_t _gstGridTimePara;
bool  CheckTimeData(uint32_t start_tamp, uint32_t end_tamp, uint32_t cur_tamp)
{

    DevRtc_t    start_time;
    DevRtc_t    end_time;
    DevRtc_t    cur_time;
    uint32_t    start_sec;
    uint32_t    end_sec;
    uint32_t    cur_sec;
    bool        status;

    start_time.time_stamp = start_tamp;
    RtcSecConvert2Date(&start_time);
    start_sec  = start_time.hour * 3600ul + start_time.minute * 60ul + start_time.second;

    end_time.time_stamp   = end_tamp;
    RtcSecConvert2Date(&end_time);
    end_sec  = end_time.hour * 3600ul + end_time.minute * 60ul + end_time.second;

    cur_time.time_stamp = cur_tamp;
    RtcSecConvert2Date(&cur_time);
    cur_sec  = cur_time.hour * 3600ul + cur_time.minute * 60ul + cur_time.second;

    if(start_sec > end_sec)
    {
        //??
        if((cur_sec >= end_sec) && (cur_sec <= start_sec))
        {
            status = false ;
        }
        else
        {
            status = true ;
        }
    }
    else if(start_sec < end_sec)
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
    else
    {
        status = false ;
    }
    return status;

}

/**
  * @brief  通知BMS，所有Pack电压的平均值，用于计算DCDC的电压补偿（包间均衡微调使用）
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _AllPackAveVoltSetProc(void)
{
    uint32_t volt_sum = 0;
    uint16_t volt_ave = 0;
    static uint32_t   send_delay_time_buf = 0;
    
    //每隔12S进行一次更新
    if(tim_get_systime() - send_delay_time_buf < 12000)
    {
        return;
    }
    
    send_delay_time_buf = tim_get_systime();
    
    for(int i=0; i<gstCollectInfo.maincard_data.total_pack_info.pack_num; i++)
    {
        volt_sum += gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol;
    }
    
    volt_ave = volt_sum / gstCollectInfo.maincard_data.total_pack_info.pack_num;
    
    SetBmsAllPackAveVolt(volt_ave);
}

/**
  * @brief  均衡控制（粗调）
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _BalanceCtrlProc(void)
{
    //均衡功能
//先查出最低SOC位置和最高SOC位置。并且长时间不变
//最高和最低SOC差值大于5%
    uint8_t   batt_num = 0;
    uint8_t   min_soc_id = 0;
    uint8_t   max_soc_id = 0;
    uint16_t  max_soc = 0;
    uint16_t  min_soc = 0;
    uint16_t  pack_total_v[BMS_MAX_NUM];        
    uint16_t  pack_max_cell_v[BMS_MAX_NUM];     //Pack最高节电芯电压
    uint16_t  balance_level = 1;                //均衡等级
    
    static uint8_t  old_min_soc_id =0xff;
    static uint8_t  old_max_soc_id =0xff;
    static uint32_t   send_delay_time_buf = 0;
    
    memset(pack_total_v, 0, (2*BMS_MAX_NUM));
    memset(pack_max_cell_v, 0, (2*BMS_MAX_NUM));
    batt_num = gstCollectInfo.maincard_data.total_pack_info.pack_num;

    if(BmsCtrlInfo.bms_ctrl_info_value.bits.polling_bms_data == 0)
    {
        return;
    }
    
    
    max_soc =  gstPackSlavePara.pack_para[0].bms_info.status_info.soc_disp;
    min_soc =  gstPackSlavePara.pack_para[0].bms_info.status_info.soc_disp;
    
    //查找SOC最高和最低的Pack，记录对应的ID
    for(uint8_t i = 0; i < batt_num; i++)
    {
        if(gstPackSlavePara.pack_para[i].bms_info.status_info.soc_disp < min_soc)
        {
            min_soc = gstPackSlavePara.pack_para[i].bms_info.status_info.soc_disp;
            min_soc_id = i;
        }
        if(gstPackSlavePara.pack_para[i].bms_info.status_info.soc_disp > max_soc)
        {
            max_soc = gstPackSlavePara.pack_para[i].bms_info.status_info.soc_disp;
            max_soc_id = i;
        }
        pack_total_v[i] = gstPackSlavePara.pack_para[i].bms_info.status_info.total_vol;
        pack_max_cell_v[i] = gstPackSlavePara.pack_para[i].bms_info.status_info.cell_max_vol;
    }
    
    //每隔15S进行一次更新
    if(tim_get_systime() - send_delay_time_buf > 15000)
    {
        send_delay_time_buf = tim_get_systime();
        //>5%开均衡，<3%关均衡，电池总电压低关均衡，SOC最低的Pack单节电芯电压高关均衡
        if(((max_soc - min_soc) <= 30) || (pack_total_v[max_soc_id] < 2400) || (pack_max_cell_v[min_soc_id] > 3550))
        {
            old_min_soc_id = 0xff;
            old_max_soc_id = 0xff;
            SetBmsBlanceDisable();
        }
        else
        {   //SOC相差5%以上，或者有最高/最低SOC的ID有变更
//            if(((max_soc - min_soc) >= 50) || (old_min_soc_id != min_soc_id) || (old_max_soc_id != max_soc_id))
            if((max_soc - min_soc) >= 50)
            {
                //设置最高节放电均衡， 最低节为充电均衡。
                SetBmsChgBlanceCtrl(min_soc_id);
                SetBmsDsgBlanceCtrl(max_soc_id);
                //清除旧的标志
                if(old_min_soc_id != min_soc_id)
                {
                    ResetBmsChgBlanceCtrl(old_min_soc_id);
                    old_min_soc_id = min_soc_id;
                }
                if(old_max_soc_id != max_soc_id)
                {
                    ResetBmsDsgBlanceCtrl(old_max_soc_id);
                    old_max_soc_id = max_soc_id;
                }
            }  
        }    
    }
}

/**
  * @brief  峰谷充放电切换函数
  * @param   PeakLoadShiftData_t
  * @return  None
  * @note    None
  */

void  PeakLoadShiftFun()
{

    bool    status;
    bool    status1;
    if(gstCollectInfo.power_data.power_set_info.work_mode != PEAK_SHAVING)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_chg_en = 0;
        _gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_dsg_en = 0;
        return;
    }
    status  = CheckTimeData(gstCollectInfo.maincard_data.peak_time_info.chg_start_time1, gstCollectInfo.maincard_data.peak_time_info.chg_end_time1, gstRtcTime.time_stamp);
    status1 = CheckTimeData(gstCollectInfo.maincard_data.peak_time_info.chg_start_time2, gstCollectInfo.maincard_data.peak_time_info.chg_end_time2, gstRtcTime.time_stamp);
    if((status == true) || (status1 == true))
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_chg_en = 1; //
    }
    else
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_chg_en = 0; //
    }

    status  = CheckTimeData(gstCollectInfo.maincard_data.peak_time_info.dsg_start_time1, gstCollectInfo.maincard_data.peak_time_info.dsg_end_time1, gstRtcTime.time_stamp);
    status1 = CheckTimeData(gstCollectInfo.maincard_data.peak_time_info.dsg_start_time2, gstCollectInfo.maincard_data.peak_time_info.dsg_end_time2, gstRtcTime.time_stamp);
    if((status == true) || (status1 == true))
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_dsg_en = 1;//
    }
    else
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_dsg_en = 0;//
    }
}

void SystemCtrlInit(void)
{
    SysInfoInit();
    CapInitl();   //
}
static  void _CollectUartBaudSet(void)
{
    static uint16_t old_baud = 0;
    static uint32_t time_out = 0;
    static uint8_t  set_updata_baud_en = 1;
    uint16_t cur_baud;
    uint8_t  updata_baud_status;
    cur_baud = GetCollectBaud();


    updata_baud_status = GetUpdataBaudStatusInfo();
    if(updata_baud_status == 1)
    {
        old_baud = 0xff;
//??500ms
        if(set_updata_baud_en == 1)
        {
            if(tim_get_systime() - time_out > 1000)
            {
                set_updata_baud_en = 0;
                SetUartRate(&ModbusSlaveCollectorUartHandle, garrUartBaudData[4]);
            }
        }
        else
        {
            time_out = tim_get_systime();
        }
    }
    else
    {
        set_updata_baud_en = 1;
        if(GetCollectBaud() != old_baud)
        {
            if(tim_get_systime() - time_out > 1000)
            {

                time_out = tim_get_systime();
                old_baud = cur_baud;
                if(cur_baud > UART_BAUD_MAX)
                {
                    return;
                }
                SetUartRate(&ModbusSlaveCollectorUartHandle, garrUartBaudData[cur_baud]);
            }

        }
        else
        {
            time_out = tim_get_systime();

        }
    }


}
void    LcdBackLightCtrl(void)
{
    static uint32_t back_light_time;
    uint8_t upgrate_status;
    uint8_t back_light_status = 0; //
    upgrate_status = GetUpdataStatus();
    if(GetBackLightUpdataFlag() == 1)
    {
        CleanBackLightUpdataFlag();
        _gBackLightTimeCnt = 0;
    }
    if(upgrate_status == UPGRATING)
    {
        _gBackLightTimeCnt = 0;
    }

    if(gSystemPowerStatus == BMS_TRUN_OFF)
    {
        DevLcdBLClose();
        _gBackLightTimeCnt = 0;
        back_light_status = 0;
        SetLcdBackLigthStatus(back_light_status);
    }
    else
    {
        if(tim_get_systime() - back_light_time > 1000)
        {
            back_light_time = tim_get_systime();
            if(gstCollectInfo.maincard_data.device_info.lcd_bl_time == 0)
            {
                DevLcdBLOpen();
                back_light_status = 1;
                SetLcdBackLigthStatus(back_light_status);
            }
            else
            {
                if(++_gBackLightTimeCnt > gstCollectInfo.maincard_data.device_info.lcd_bl_time * 60)
                {
                    _gBackLightTimeCnt = gstCollectInfo.maincard_data.device_info.lcd_bl_time * 60;
                    if(gShutDownPara.shut_down_status == 1)
                    {
                        DevLcdBLOpen();
                        back_light_status = 1;
                        SetLcdBackLigthStatus(back_light_status);
                    }
                    else
                    {
                        DevLcdBLClose();
                        back_light_status = 0;
                        SetLcdBackLigthStatus(back_light_status);
                    }
                }
                else
                {
                    DevLcdBLOpen();
                    back_light_status = 1;
                    SetLcdBackLigthStatus(back_light_status);
                }
            }
        }
    }



}
void CleanBackLightTimeCnt(void)
{
    _gBackLightTimeCnt = 0;
}

/**
  * @brief  BMS关机控制
  * @param   None
  * @return  None
  * @note    None
  */
void _BmsSwitchCtrlFunc(void)
{
    static uint32_t time;

    //500ms执行一次
    if(tim_difftime_from_now(time) < 500)
    {
        return;
    }
    time = tim_get_systime();

    //检测到关机信号
    if((gstPackSlavePara.pack_para[0].bms_info.ctrl_info.shutdown & 0X03) == 2)
    {
        BMSShutDownSend();
        PCSShutDownSend();
    }
}
/**
* @brief  发送PCS类型给DCDC
  * @param   None
  * @return  None
  * @note    None
  */
void _SendPcsTypeToDcdcFunc(void)
{
    static uint32_t time;
    static uint16_t type_old;
    uint16_t type = 0;

    //1s执行一次
    if(tim_difftime_from_now(time) < 1000)
    {
        return;
    }
    time = tim_get_systime();

    type = gstPackSlavePara.pack_para[0].dcdc_info.status_info.pcs_type;
    //检测到第一个Pack识别到的PCS类型，且不予之前的相同
    if((type != type_old) && (type != 0))
    {
        SetDcdcPcsType(type);
        type_old = type;
    }
}

/**
  * @brief  设置PCS充放电允许功率
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void SetCtrlInfoToPcs(void)
{
//    uint16_t  stop_chg_flag = 0;
    static uint32_t  set_power_time_out;
    uint8_t  batt_num;
    uint8_t  set_updata = 0;
    uint8_t  soc_low_flag = 0;

    uint16_t full_chg_flag = 0;
    uint16_t batt_max_chg_curr = 0;
    uint32_t batt_max_chg_power = 0;
    uint16_t batt_max_dsg_curr = 0;
    uint32_t batt_max_dsg_power = 0;
    float max_chg_temp = 0.0f;

    uint16_t max_soc_rpt = 0;
    static uint8_t fbat_soc_threshold = 0;

    bool close_dsg_chg_flag = false;
    bool close_dsg_flag = false;
    bool close_chg_flag = false;

    static  uint16_t pv_max_power = 16000;//
    uint16_t max_inv_power = 0;
    batt_num = gstCollectInfo.maincard_data.total_pack_info.pack_num;


    for(int i = 0; i < batt_num; i++)
    {
        //查找最大SOC的Pack
        if(max_soc_rpt < gstCollectInfo.pack_data[i].state_info.soc_disp)
        {
            max_soc_rpt = gstCollectInfo.pack_data[i].state_info.soc_disp;
        }
    }

    //soc = 100时，停止充电
    if(gstCollectInfo.maincard_data.total_pack_info.soc >= 1000)
    {
        gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_power = 0;
    }

    if(max_soc_rpt > 900)
    {
        fbat_soc_threshold = 1;
    }
    else if(max_soc_rpt < 880)
    {
        fbat_soc_threshold = 0;
    }

    if(batt_num < 3)
    {
        pv_max_power = 6000;
    }
    else
    {

        if(fbat_soc_threshold == 1)
        {
            pv_max_power = 6000;
        }
        else
        {
            pv_max_power = 10000;
        }
    }

    if((gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 2) ||
        (gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 3))
    {
        max_inv_power = 6000;
    }
    else
    {
        if(batt_num <= 1)
        {
            max_inv_power = 2500;
        }
        else if(batt_num == 2)
        {
            max_inv_power = 5000;
        }
        else if(batt_num == 3)
        {
            max_inv_power = 6000;
        }
        else
        {
            max_inv_power = 6000;
        }
    }


//    for(int i = 0; i < batt_num; i++)
//    {
//        if(gstCollectInfo.pack_data[i].state_info.max_chg_current == 0x00)
//        {
//            stop_chg_flag = 1;
//        }
//    }

        
    //最终允许电流（Pack允许电流与设置允许电流取小）
    batt_max_chg_curr = 
        (gstCollectInfo.maincard_data.sys_info.max_chg_curr_set < gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_current) ?
         gstCollectInfo.maincard_data.sys_info.max_chg_curr_set : gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_current  ;
    
    batt_max_dsg_curr = 
        (gstCollectInfo.maincard_data.sys_info.max_dsg_curr_set < gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_current) ?
         gstCollectInfo.maincard_data.sys_info.max_dsg_curr_set : gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_current  ;

    //最终允许功率（Pack允许功率与设置允许功率取小）
    batt_max_chg_power = 
        (gstCollectInfo.maincard_data.sys_info.max_chg_power_set < gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_power) ?
         gstCollectInfo.maincard_data.sys_info.max_chg_power_set : gstCollectInfo.maincard_data.total_pack_info.allow_max_chg_power  ;

    batt_max_dsg_power =
        (gstCollectInfo.maincard_data.sys_info.max_dsg_power_set < gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_power) ?
         gstCollectInfo.maincard_data.sys_info.max_dsg_power_set : gstCollectInfo.maincard_data.total_pack_info.allow_max_dsg_power  ;

    if(tim_get_systime() - set_power_time_out > 1500)
    {
        set_power_time_out = tim_get_systime();

        if(/*(stop_chg_flag == 1) || */gstCollectInfo.maincard_data.total_pack_info.soc == FULL_SOC)
        {
            full_chg_flag = 1;
            batt_max_chg_power = 0;
        }

        if((full_chg_flag == 1) || full_chg_flag == 2)
        {
            if(gstCollectInfo.maincard_data.total_pack_info.soc > SELF_DSG_SOC_LIMIT_L1)
            {
                if(full_chg_flag == 1)
                {
                    batt_max_chg_power = 0;
                }
                else if(full_chg_flag == 2)
                {
                    batt_max_chg_power = 100;
                }
            }
            else if(gstCollectInfo.maincard_data.total_pack_info.soc > SELF_DSG_SOC_LIMIT_L2)
            {
                full_chg_flag = 2;
                max_chg_temp = CHG_LIMIT_POWER_VALUE - (((CHG_LIMIT_POWER_VALUE - (float)batt_max_chg_power) * (SELF_DSG_SOC_LIMIT_L1 -
                                                        (float)gstCollectInfo.maincard_data.total_pack_info.soc)) / (SELF_DSG_SOC_LIMIT_L1 - SELF_DSG_SOC_LIMIT_L2));//线性
                if(max_chg_temp > batt_max_chg_power)
                {
                    max_chg_temp = batt_max_chg_power;
                }
                if(max_chg_temp < 0)
                {
                    max_chg_temp = 0;
                }
                batt_max_chg_power = max_chg_temp;
            }
            else
            {
                full_chg_flag = 0;
            }
        }
#if 0
        close_dsg_chg_flag = GetCloseChgDsgFun();
        close_dsg_flag = GetCloseDsgFun();
        close_chg_flag = GetCloseChgFun();
        
        if(close_chg_flag)
        {
            batt_max_chg_curr = 0;
            batt_max_chg_power = 0;
        }

        if(close_dsg_flag)
        {
            batt_max_dsg_curr = 0;
            batt_max_dsg_power = 0;
        }

        if(close_dsg_chg_flag)
        {
            batt_max_chg_curr = 0;
            batt_max_chg_power = 0;
            batt_max_dsg_curr = 0;
            batt_max_dsg_power = 0;
        }
#endif    
        if((_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en == 0) && (_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en == 0))
        {
            batt_max_chg_curr = 0;
            batt_max_chg_power = 0;
            batt_max_dsg_curr = 0;
            batt_max_dsg_power = 0;
        }
        
        if(_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en == 0)
        {
            batt_max_chg_curr = 0;
            batt_max_chg_power = 0;
        }    
        
        if(_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en == 0)
        {
            batt_max_dsg_curr = 0;
            batt_max_dsg_power = 0;
        }
        
        
        
        //chg下发充电电流功率
        if(batt_max_chg_curr != gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_current)
        {
            set_updata = 1;
        }

        if(batt_max_chg_power != gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power)
        {
            set_updata = 1;
        }

        if((set_updata == 1) && (GetPcsCommStatus() == 0))
        {
//            batt_max_chg_power = test_Ldata[1];
//            SetPcsChgVoltCurPowerInfo(test_Ldata[0], batt_max_chg_power);
            SetPcsChgVoltCurPowerInfo(batt_max_chg_curr, batt_max_chg_power);
        }

        //dsg下发放电电流功率
        set_updata = 0;
        if(batt_max_dsg_curr != gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current)
        {
            set_updata = 1;
        }

        if(batt_max_dsg_power != gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power)
        {
            set_updata = 1;
        }

        if((set_updata == 1) && (GetPcsCommStatus() == 0))
        {
            SetPcsDsgVoltCurPowerInfo(batt_max_dsg_curr, batt_max_dsg_power);
        }
//        if((gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_current != gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_chg_current) ||
//                (gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current != gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_dsg_current) ||
//                (gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power   != (gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_chg_power[0] << 16 | gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_chg_power[1])) ||
//                (gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power   != (gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_dsg_power[0] << 16 | gstUdpPcsSlavePara.pcs_info.set_info.inv_bat_max_dsg_power[1])))
//        {
//            set_updata = 1;
//        }

//        if((set_updata == 1) && (GetPcsCommStatus() == 0))
//        {
//            SetPcsChgDsgCurPowerInfo(gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_current,
//                                     gstCollectInfo.power_data.power_set_info.inv_bat_max_chg_power,
//                                     gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_current,
//                                     gstCollectInfo.power_data.power_set_info.inv_bat_max_dsg_power
//                                    );
//        }
        if(BmsCtrlInfo.bms_ctrl_info_value.bits.polling_bms_data == 1)
        {
            //soc低于5%时关闭backup使能
            if((gstCollectInfo.maincard_data.total_pack_info.soc < 50) && (soc_low_flag == 0))
            {
                SetPcsBackEnCtrl(0);
                soc_low_flag = 1;
            }

            //soc高于8%时开启backup使能
            if((gstCollectInfo.maincard_data.total_pack_info.soc > 80) && (soc_low_flag == 1))
            {
                SetPcsBackEnCtrl(1);
                soc_low_flag = 0;
            } 
        }
        //PCS设置最大功率
        if(GetPcsCommStatus() == 0)
        {
            if((gstCollectInfo.power_data.power_set_info.inv_pv_max_power != pv_max_power) || \
                    (gstCollectInfo.power_data.power_set_info.inv_on_grid_out_max_power != max_inv_power)) 
            {
                SetPcsMaxPvPower(pv_max_power, max_inv_power);
            }

        }
    }

}
uint16_t CheckCurDircetFun(int16_t cur_data, int16_t min_threshold, int16_t max_threshold)
{
    uint16_t cur_direct = 0;
    if((cur_data > min_threshold) && (cur_data < max_threshold))
    {
        cur_direct = CUR_DIRECT_STANDBY;
    }
    else
    {
        if(cur_data < min_threshold)
        {
            cur_direct = CUR_DIRECT_NEGATIVE;
        }
        else
        {
            cur_direct = CUR_DIRECT_POSITIVE;
        }
    }
    return cur_direct;
}

void GetSystemCurDirectFun()
{

    uint8_t i = 0;
    int16_t pv_cur = 0;
    int16_t bat_cur = 0;
    int16_t grid_cur = 0;
    int16_t backup_cur = 0;

    pv_cur     = gstCollectInfo.power_data.power_value_info.pv_1_current;           //?????为什么只有PV1和PV2
    pv_cur    += gstCollectInfo.power_data.power_value_info.pv_2_current;

    bat_cur    = gstCollectInfo.power_data.power_value_info.inv_bat_current;
    grid_cur   = gstCollectInfo.power_data.power_value_info.r_meter_current;
    backup_cur = gstCollectInfo.power_data.power_value_info.r_backup_current;



    gstCollectInfo.power_data.inside_info.cur_direct_status.bits.pv_cur_direct     = CheckCurDircetFun(pv_cur,     -10, 10);
    gstCollectInfo.power_data.inside_info.cur_direct_status.bits.batt_cur_direct   = CheckCurDircetFun(bat_cur,    -10, 10);
    gstCollectInfo.power_data.inside_info.cur_direct_status.bits.grid_cur_direct   = CheckCurDircetFun(grid_cur,   -10, 10);

    gstCollectInfo.power_data.inside_info.cur_direct_status.bits.backup_cur_direct = CheckCurDircetFun(backup_cur, -10, 10);
//所有的正负值  是以PCS作为基准。  流入PCS为正，流出PCS为负.
//backup 电流只能从PCS流出，因为只有两种状态，一种是待机，一种是负。
    if(gstCollectInfo.power_data.inside_info.cur_direct_status.bits.backup_cur_direct == CUR_DIRECT_POSITIVE)
    {
        gstCollectInfo.power_data.inside_info.cur_direct_status.bits.backup_cur_direct = CUR_DIRECT_NEGATIVE;
    }
}

#define INSULATION_CHECK_TIME_INTERVAL      (60UL*60*24*7)    //WEEK     
void    TimeSetArcInsulationFun(void)
{
//100H 下发一次绝缘检测使能
    static uint32_t set_arc_insulation_time = 0;
    if(gstCollectInfo.maincard_data.sys_info.run_time - set_arc_insulation_time > INSULATION_CHECK_TIME_INTERVAL)
    {
        set_arc_insulation_time = gstCollectInfo.maincard_data.sys_info.run_time;

        SetArcInsulationDetect(1);       //send insulation en
    }
}




void SetUserTypeToPcs(void)
{
    static  uint32_t pcs_write_time_buf = 0;
    static  uint16_t old_user_type;
    if((old_user_type != gstCollectInfo.maincard_data.base_info.user_type) && (GetPcsCommStatus() == 0))
    {
        //每隔1S写入
        if(tim_get_systime() - pcs_write_time_buf > 1000)
        {
            pcs_write_time_buf = tim_get_systime();
            SetPcsUserType(gstCollectInfo.maincard_data.base_info.user_type, 4);
            old_user_type = gstCollectInfo.maincard_data.base_info.user_type;
        }
    }
    else
    {
        pcs_write_time_buf = (tim_get_systime() + 1001);
    }
}


void RemotLoadPowerCal(void)
{
    int16_t cur_data = 0;
    int32_t power_data = 0;
    if(gstCollectInfo.power_data.power_set_info.ct_or_meter == 0)
    {
        //
        cur_data = gstCollectInfo.power_data.power_value_info.r_meter_current;
    }
    else
    {
        cur_data = GetMeterCurrValue();
    }
    cur_data = (cur_data - gstCollectInfo.power_data.power_value_info.r_inv_current);  //uint 0.1A
    power_data = cur_data * gstCollectInfo.power_data.power_value_info.r_grid_vol;
    //gstCollectInfo.power_data.inside_info.nload_act_power = power_data/100;         //?????
}

/**
  * @brief   发送DRM控制信息
  * @param   None
  * @return  None
  * @note    None
  */
void _SendDrmCtrlInfo(void)
{
    static uint32_t set_drm_time_out = 0;
    uint16_t drm_status = GetDrmStatus();
    if(gstCollectInfo.power_data.power_set_info.drm_enable == 1)   //drm总开关 2025年12月1日
    {
        drm_status = 0;
    }

    //先不做处理 2025年12月2日  
    if((drm_status != gstUdpPcsSlavePara.pcs_info.set_info.drm_enable) && (GetPcsCommStatus() == 0))
    {
        if(tim_get_systime() - set_drm_time_out > 4000)
        {
            set_drm_time_out = tim_get_systime();
            SetDspDrmStatus(drm_status);
        }
    }
    else
    {
        set_drm_time_out = tim_get_systime();
    }

}

/**
  * @brief   发送LLC开启
  * @param   None
  * @return  None
  * @note    None
  */
void _SendLLCEnalbeInfo(void)
{
    static uint32_t set_drm_time_out = 0;
    uint16_t pack_num = gstCollectInfo.maincard_data.total_pack_info.pack_num;
    //没有pack，或者PSC的LLC已开启，不发送
    if(pack_num == 0 || gstUdpPcsSlavePara.pcs_info.set_info.llc_enable == 1)
    {
        return;
    }
    //所有Pack的DCDC必须都工作在正常运行状态下
    for(int i=0; i<pack_num; i++)
    {
        if(gstPackSlavePara.pack_para[i].dcdc_info.status_info.dcdc_work_status != 3)
        {
            return;
        }
    }
    //每两秒发一次
    if(tim_get_systime() - set_drm_time_out <= 2000)
    {
        return;
    }
    set_drm_time_out = tim_get_systime();
    SetDspLLCEnable();

}

/**
  * @brief   SN锁
  * @param   None
  * @return  None
  * @note    加入SN锁机制，如果没有开锁，SN不能更改
  */
void _SnLockFunc()
{
    static uint32_t unlock_time;    //解锁时间
    static char sn[20];
    static uint8_t flag;
    static uint16_t lock;

    //记录当前SN号
    if(flag == 0)
    {
        flag = 1;
        memcpy(sn, gstCollectInfo.maincard_data.base_info.sn, 20);
    }
    //未解锁，不允许改变SN
    if(lock == 0)
    {
        if(memcmp(sn, gstCollectInfo.maincard_data.base_info.sn, 20) != 0)
        {
            memcpy(gstCollectInfo.maincard_data.base_info.sn, sn, 20);
        }
        unlock_time = tim_get_systime();
        return;
    }
    //解锁时间为10s，10s后自动落锁
    if(tim_difftime_from_now(unlock_time) > 10000)
    {
        lock = 0;
    }
}

void SetElecMeterTypeToPcs(void)  //
{
    static uint32_t set_time_out = 0;
    static uint16_t elec_meter_type = 0;
    elec_meter_type = GetMeterConnectSta();
    if( (elec_meter_type != gstCollectInfo.power_data.power_set_info.meter_brand ) && (GetPcsCommStatus() == 0))
    {
        if(tim_get_systime() - set_time_out > 2000)
        {
            set_time_out = tim_get_systime();
            SetPcsElecMeterType(elec_meter_type);
        }
    }
    else
    {
        set_time_out = tim_get_systime();
    }
}

//void JudgeDRM0ConnectStatus(void)
//{
////    uint8_t connect_io;
////    static uint8_t stable_cnt;
////    static uint8_t stable_cnt1;
////    connect_io = GpioBitGet((&DRM0ConnectIOHandle));
////    if(connect_io == 0) //IO消抖
////    {
////        stable_cnt1 = 0;
////        if(stable_cnt++ > 5)
////        {
////            connect_io = 0;
////        }
////    }
////    else   //IO为高电平
////    {
////        stable_cnt = 0;
////        if(stable_cnt1++ > 5)
////        {
////            connect_io = 1;
////        }
////    }
//
////    if((connect_io == 0) && (gstCollectParaData.collect_data.drm_enable == 1))
////    {
////        //置标志位 通过此标志位关闭PCS
////        _gDRM0ConnectStatus = 1;
////    }
////    else
////    {
////        _gDRM0ConnectStatus = 0;
////    }
//}


//
uint32_t GetOnGridTime(void)
{
#if 0
    if((gstCollectParaData.pcs_input_front_data.invs_status == 2) && (!_gstGridTimePara.on_grid_flag)) //??
    {
        _gstGridTimePara.on_grid_time.time_stamp = GetRtcTimeStamp();
        _gstGridTimePara.on_grid_flag = 1;
    }
    else if((gstCollectParaData.pcs_input_front_data.invs_status == 1) && _gstGridTimePara.on_grid_flag)
    {
        _gstGridTimePara.on_grid_flag = 0;
        _gstGridTimePara.off_grid_timre.time_stamp = GetRtcTimeStamp();
        _gstGridTimePara.running_time.time_stamp = _gstGridTimePara.off_grid_timre.time_stamp - _gstGridTimePara.on_grid_time.time_stamp;
        _gstGridTimePara.total_on_grid_time.time_stamp += _gstGridTimePara.running_time.time_stamp;
    }
#endif
    static uint8_t on_grid_flag = 0;
    static uint32_t on_grid_time[5] = {0};
    static uint32_t running_time[3] = {0};
    uint32_t running_time_tmp = 0;
    int32_t running_time_tmp2 = 0;
    uint32_t on_grid_time_tmp = 0;
    int32_t on_grid_time_tmp2 = 0;

    running_time_tmp = GetRtcTimeStamp();

    // 统计系统上电时间
    if (running_time[1] == 0)
    {
        running_time[1] = running_time_tmp;
        running_time[2] = running_time_tmp;
    }
    else
    {
        running_time_tmp2 = (int32_t)(running_time_tmp - running_time[2]);
        running_time_tmp2 = abs(running_time_tmp2);
        if (running_time_tmp2 > 10 * 60)
        {
            //由于云端自动校时导致时间跳变，只在上电进行一次优化
            running_time_tmp2 = (int32_t)(running_time[2] - running_time[1]);
            if (running_time_tmp2 < 0)
            {
                running_time_tmp2 = 0;
            }
            running_time[1] = running_time_tmp - running_time_tmp2;
        }
        running_time[2] = running_time_tmp;
    }
    running_time[0] = running_time[2] - running_time[1];

    //系统运行时间实时赋值
    gstCollectInfo.maincard_data.sys_info.run_time = running_time[0];


    // 并网时间
    on_grid_time_tmp = GetRtcTimeStamp();
    if (gstCollectInfo.power_data.model_info.on_grid_state == 1)
    {
        if (on_grid_flag == 1)
        {
            on_grid_time[3] += on_grid_time[0];
            on_grid_flag = 0;
        }
    }
    else if (gstCollectInfo.power_data.model_info.on_grid_state == 2)
    {
        if (on_grid_flag == 0)
        {
            on_grid_time[1] = on_grid_time_tmp;
            on_grid_time[2] = on_grid_time_tmp;
            on_grid_flag = 1;
        }

        on_grid_time_tmp2 = (int32_t)(on_grid_time_tmp - on_grid_time[2]);
        on_grid_time_tmp2 = abs(on_grid_time_tmp2);
        if(on_grid_time_tmp2 > 10 * 60)
        {
            //由于云端自动校时导致时间跳变，进行时间优化
            on_grid_time_tmp2 = (int32_t)(on_grid_time[2] - on_grid_time[1]);
            if(on_grid_time_tmp2 < 0)
            {
                on_grid_time_tmp2 = 0;
            }
            on_grid_time[1] = on_grid_time_tmp - on_grid_time_tmp2;
        }

        on_grid_time[2] = on_grid_time_tmp;
        on_grid_time[0] = (on_grid_time[2] - on_grid_time[1]);
        on_grid_time[4] = on_grid_time[3] + on_grid_time[0];
    }

    // 系统运行时间和并网时间实时赋值
    _gstGridTimePara.total_on_grid_time.time_stamp = on_grid_time[4];
    return _gstGridTimePara.total_on_grid_time.time_stamp;
}

//存储并离网切换次数 等采集器回读
//void GetOnGridNumber(void)
//{
//    gstCollectInfo.maincard_data.sys_info.on_grid_num = gstCollectInfo.maincard_data.sys_info.on_grid_num;
//}
//采集器下发清除用电量信息指令
void CleanCapHistoryValue(void)
{
    if(gstCollectInfo.maincard_data.sys_info.clear_statistics == 1)
    {
        SpiFlashAutoEraseSectors(0x258000, (0x2000 * 12 * 4)); //0X1000 ->4k 半个月数据 *2=一个月 *12=一年数据 *4=4年数据   2024开始-2027年底       0x1000
        gstCollectInfo.maincard_data.sys_info.clear_statistics = 0;
    }
}


#define   BMS_WAKE_UP_START             0
#define   BMS_WAKE_UP_CONFIRM           1


#define     DSP_WAKE_UP_START               3
#define     DSP_WAIT_WAKE_UP_EN             4
#define     DSP_WAKE_UP_BMS_ADRESSING       5
#define     DSP_WAIT_WAKE_UP_ADRESS         6
#define     DSP_SET_WAKE_UP_BAT_VOLT        7
#define     DSP_WAIT_WAKE_UP_BAT_VOLT       8
#define     DSP_SET_EXIT_WAKE_UP            9
#define     DSP_WAIT_EXIT_WAKE_UP           10
#define     DSP_WAKE_UP_ERR                 11
#define     DSP_TEST_MODE                   12

#define     DSP_WAKUPING                    1
#define     DSP_WAKUP_OVER                  2
#define     DSP_WAKUP_EXIT                  3


#define     BMS_DSP_WAKUPING                1
#define     BMS_DSP_WAKUP_EXIT              2
#define     TEST_BMS                        0
#define     TEST_CHG_DC_SOURCE              1



void CheckTestModeFun(void)
{
    static uint32_t  time_out = 0;
    if((gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 2) || 
        (gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 3))
    {
        gstFirmwareUpgrateInfo.upgrad_err_code = 0;

        if(tim_get_systime() - time_out > 2000)
        {
            time_out = tim_get_systime();
            if(gstCollectInfo.power_data.inside_info.dsp_wake_en != DSP_WAKUP_EXIT)
            {
                SetBattVoltToDsp((uint16_t)400, DSP_WAKUP_EXIT);
            }
            else if(gstCollectInfo.power_data.inside_info.ignor_ct_err != 1)
            {
                SetDspEnterCtTestMode(1);
            }
        }
    }
    else
    {
        time_out =  (tim_get_systime());
    }
}

/**
  * @brief  允许充放电标志设置函数
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void _ChgDsgAllowFlagSetFunc(void)
{
    static uint32_t time_out = 0;
    uint8_t  set_pre_stop_en = 0;
    EN_ERR_WROK_ACTION e_err_work_action;
//    static uint8_t flag_bat_depth = 0;
//    static uint8_t flag_bat_min_soc_depth = 0;
//    static uint8_t soc_retern_diff = 0;
//    static uint8_t chg_end_retern_diff = 0;     //充电末端充电回差标志
    
    _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_ON;
    _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ON;
#if 1    
    //2026年4月22日：修改削峰填谷模式下买卖电时间段内都可以进行充放电
    //2026年4月23日：与DSP之间的配合出现问题重新修改方案，使用功率是否为0来控制是否充放电
    //峰谷模式，充电时间段不允许放电，放电时间段不允许充电
    if(gstCollectInfo.power_data.power_set_info.work_mode == PEAK_SHAVING)
    {
        if(_gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_chg_en != 1)
        {
            _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_OFF;
        }
        if((gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 2) || 
            (gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 3))
        {

        }
        else
        {
            if(_gstSysCtrlInfo.sys_ctrl_status.bits.peak_shift_dsg_en != 1)
            {
                _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_OFF;
            }
        }
    }
#endif   

    if((gstCollectInfo.maincard_data.total_pack_info.on_grid_dsg_stop_soc < 10) ||
       (gstCollectInfo.maincard_data.total_pack_info.on_grid_dsg_stop_soc > 90))
    {
        gstCollectInfo.maincard_data.total_pack_info.on_grid_dsg_stop_soc = 10;
    }
    //达到停止放电深度，停止放电
    if(BmsCtrlInfo.bms_ctrl_info_value.bits.polling_bms_data == 1)
    {
        if(gstCollectInfo.maincard_data.total_pack_info.soc <= (gstCollectInfo.maincard_data.total_pack_info.on_grid_dsg_stop_soc * 10))
        {
            _gstSysCtrlInfo.sys_ctrl_status.bits.flag_bat_depth = 1;
        }
    }
    
    if(gstCollectInfo.maincard_data.total_pack_info.soc >= (gstCollectInfo.maincard_data.total_pack_info.on_grid_dsg_stop_soc * 10) + 30)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.flag_bat_depth = 0;
    }

    //单体SOC过低，停止放电
    if(GetBattMinCellSoc() < 11)  //50
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.flag_bat_min_soc_depth = 1;
    }
    if(GetBattMinCellSoc() > 39) //80
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.flag_bat_min_soc_depth = 0;
    }

    if((_gstSysCtrlInfo.sys_ctrl_status.bits.flag_bat_depth == 1) || (_gstSysCtrlInfo.sys_ctrl_status.bits.flag_bat_min_soc_depth == 1))
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_ERR;
    }

    //soh过低，停止充放电
    if(gstCollectInfo.maincard_data.total_pack_info.soh < MIN_SOH) //
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_ERR;
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ERR;
    }

    //发电机充电，soc低于89.5之后允许再次充电（回差）
    if(gstCollectInfo.power_data.power_set_info.gen_mode == OIL_INPUT)
    {
        if(gstCollectInfo.maincard_data.total_pack_info.soc > 905)
        {
            //soc>90.5%
            _gstSysCtrlInfo.sys_ctrl_status.bits.soc_retern_diff = 1;
        }
        if(gstCollectInfo.maincard_data.total_pack_info.soc < 895)
        {
            _gstSysCtrlInfo.sys_ctrl_status.bits.soc_retern_diff = 0;
        }
        if(_gstSysCtrlInfo.sys_ctrl_status.bits.soc_retern_diff == 1)
        {
            _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ERR;
        }
    }
    
    //充电末端，soc低于98之后允许再次充电（回差）
    if(gstCollectInfo.maincard_data.total_pack_info.soc >= 1000)
    {
        //soc>90.5%
        _gstSysCtrlInfo.sys_ctrl_status.bits.chg_end_retern_diff = 1;
    }
    if(gstCollectInfo.maincard_data.total_pack_info.soc < 980)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.chg_end_retern_diff = 0;
    }
    if(_gstSysCtrlInfo.sys_ctrl_status.bits.chg_end_retern_diff == 1)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ERR;
    }
    
    //获取系统故障状态
    e_err_work_action = GetSysError();
    if(e_err_work_action == EN_ERR_CLOSE_CHG_DSG)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_ERR;
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ERR;
    }
    else if(e_err_work_action == EN_ERR_CLOSE_CHG)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ERR;
    }
    else if(e_err_work_action == EN_ERR_CLOSE_DSG)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_ERR;
    }
    //升级中
    if(gstFirmwareUpgrateInfo.upgrad_resting == 1)
    {
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en = PCS_DSG_CTRL_ERR;
        _gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en = PCS_CHG_CTRL_ERR;
    }
    //PCS设置
    SetCtrlInfoToPcs();
    if(tim_get_systime() - time_out > 1500)
    {
        time_out = tim_get_systime();
        if(GetPcsCommStatus() == 0)
        {

            if(_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en != gstCollectInfo.power_data.inside_info.pcs_chg_ctrl)
            {
                SetPcsChgCtrlStatus((uint8_t)_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_chg_ctrl_en);
            }

            if(_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en != gstCollectInfo.power_data.inside_info.pcs_dsg_ctrl)
            {
                SetPcsDsgCtrlStatus((uint8_t)_gstSysCtrlInfo.sys_ctrl_status.bits.pcs_dsg_ctrl_en);
            }
        }
    }
}

/**
  * @brief  允许充放电标志设置函数
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
 static void _SetPackSOCToMainCop()
 {
    #if 0
    static uint16_t soc_tmp = 0;

    #endif
    static uint32_t time_out;
    if(tim_get_systime() - time_out > MAIN_TO_MAIN_COP_SOC_TIME)
    {
        time_out = tim_get_systime();
        SetPackSocInfo(gstCollectInfo.maincard_data.total_pack_info.soc);
//        SetPackSocInfo(soc_tmp);
    }
 }

/**
  * @brief  系统控制函数(重要)
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
void SystemCtrlProc(void)
{

    static uint32_t time_out = 0;
    static uint32_t opt_time = 0;
    static uint8_t old_upgrading_status = 0xff;
    static uint32_t first_power_on_delay = 0;
    static uint8_t  first_power_on_delay_cnt = 0;
    static uint8_t  wake_dptr = 0;
    static uint8_t  wakeup_repeat_cnt = 0;
    uint32_t batt_volt = 0;
    uint32_t pcs_batt_volt = 0;
    uint8_t  set_pre_stop_en = 0;
    EN_ERR_WROK_ACTION e_err_work_action;
    static uint8_t flag_bat_depth = 0;
    static uint8_t flag_bat_min_soc_depth = 0;
    static uint8_t soc_retern_diff = 0;
//    if(tim_get_systime() - opt_time > 1000)
//    {
//        opt_time = tim_get_systime();
//        gstCollectInfo.maincard_data.sys_info.run_time++;  //OTP time
//    }
    if(old_upgrading_status != gstFirmwareUpgrateInfo.upgrading_status)
    {
        old_upgrading_status = gstFirmwareUpgrateInfo.upgrading_status;
        ShowEmUpgrading(gstFirmwareUpgrateInfo.upgrading_status);
    }
    SysErrorProcess();
    CapCtrl();    //
//    TimeSetArcInsulationFun();
    LcdBackLightCtrl();
    _CollectUartBaudSet();
    GetSystemCurDirectFun();
    PeakLoadShiftFun();
    CtrlInfoStorageProcess();
    GetOnGridTime();  //
//    GetOnGridNumber(); //
    CleanCapHistoryValue();  //
    _BmsSwitchCtrlFunc();   //BMS开关机控制
    _SendPcsTypeToDcdcFunc();//通知DCDC当前PCS类型
    _SnLockFunc();          //SN锁

    if(GetCorrectRtcStatus() == 1)
    {
        CleanCorrectRtcFlag();
        uint32_t time = 0;
        time = gstCollectInfo.maincard_data.sys_info.time_stamp;
        CalibrationRtcTime(time);
        UpdataFlashDataEn();//
//BAT
//        BMS_RtcTimeSet(time);
    }
    else
    {
        gstCollectInfo.maincard_data.sys_info.time_stamp = GetRtcTimeStamp();
    }
    switch(eSystemStatus)
    {
        case EN_SYS_BMS_ADRESS_STATUS:
        {
//
            if(gstFirmwareUpgrateInfo.updata_slave_type == UPDATA_NO_TYPE)
            {

                if(tim_get_systime() - time_out > 2000)
                {
                    time_out = tim_get_systime();
                    eSystemStatus = EN_SYS_INIT_STATUS;
                }
            }
            else
            {
                time_out = tim_get_systime();
            }
            break;
        }
        case EN_SYS_INIT_STATUS:
        {

#if(TEST_BMS)
            eSystemStatus = EN_SYS_WAKEUP_STATUS;
            SetBattVoltToDsp((uint16_t)batt_volt, BMS_DSP_WAKUP_EXIT);
            wake_dptr = BMS_WAKE_UP_START;


#else

            eSystemStatus = EN_SYS_WORKING_STATUS;
            break;

//            time_out = tim_get_systime();
//            if(GetAdressStatus() == 1)
//            {// //ADRESS ERR       NO BMS
//                wake_dptr = DSP_WAKE_UP_START;
//                eSystemStatus = EN_SYS_WAKEUP_STATUS;
//            }
//            else
//            {//adress OK
//                eSystemStatus = EN_SYS_WAKEUP_STATUS;
//                SetBattVoltToDsp((uint16_t)batt_volt, BMS_DSP_WAKUP_EXIT);
//                wake_dptr = BMS_WAKE_UP_START;
//            }



            time_out = tim_get_systime();
            if(GetAdressStatus() == 1)
//
//            if(1)
            {
                // //ADRESS ERR       NO BMS
                wake_dptr = DSP_WAKE_UP_START;
                eSystemStatus = EN_SYS_WAKEUP_STATUS;
            }
            else
            {
                //adress OK
                eSystemStatus = EN_SYS_WAKEUP_STATUS;
                wake_dptr = BMS_WAKE_UP_START;
                wakeup_repeat_cnt = 0;

            }

#endif


            break;
        }

        case EN_SYS_WAKEUP_STATUS:
        {
            gstCollectInfo.maincard_data.sys_info.dev_run_state = 1;
            if((gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 2) || 
                (gstCollectInfo.maincard_data.inside_info.tool_info.ctrl_info.enter_tool_mode == 3))
            {
                eSystemStatus = EN_SYS_WORKING_STATUS;
                break;
            }
            switch(wake_dptr)
            {
                case BMS_WAKE_UP_START:
                {
                    batt_volt = 400;
                    SetBattVoltToDsp((uint16_t)batt_volt, DSP_WAKUP_EXIT);
                    SetDspWakeUpStatusToBms(BMS_DSP_WAKUP_EXIT);
                    wake_dptr = BMS_WAKE_UP_CONFIRM;
                    wakeup_repeat_cnt++;
                    time_out = tim_get_systime();

                    break;
                }
                case BMS_WAKE_UP_CONFIRM:
                {

                    if(tim_get_systime() - time_out > 2000)
                    {
                        time_out = tim_get_systime();

                        if(wakeup_repeat_cnt >= 3)
                        {
                            wakeup_repeat_cnt = 0;
                            eSystemStatus = EN_SYS_WORKING_STATUS;
                        }
                        else
                        {
                            wake_dptr = BMS_WAKE_UP_START;
                        }
                    }
                    else
                    {
                        if(gstCollectInfo.power_data.inside_info.dsp_wake_en == DSP_WAKUP_EXIT)
                        {
                            eSystemStatus = EN_SYS_WORKING_STATUS;
                        }

                    }
                    break;
                }

                case DSP_WAKE_UP_START:
                {
                    //first tell dsp start
                    batt_volt = 400;  //默认40V
                    SetBattVoltToDsp((uint16_t)batt_volt, DSP_WAKUPING);
                    wake_dptr = DSP_WAIT_WAKE_UP_EN;
                    break;
                }
                case DSP_WAIT_WAKE_UP_EN:
                {
                    if(tim_get_systime() - time_out > 3000)
                    {
                        time_out = tim_get_systime();
                        if(++wakeup_repeat_cnt > 10)
                        {
                            wakeup_repeat_cnt = 0;
                            wake_dptr = DSP_WAKE_UP_ERR;      //写入失败
                        }
                        else
                        {
                            wake_dptr = DSP_WAKE_UP_START;
                        }
                    }
                    if(gstCollectInfo.power_data.inside_info.dsp_wake_up_status == 3)//激活就绪 ---是发送激活中 返回就绪。
                    {
                        wakeup_repeat_cnt = 0;// set dsp ok
                        wake_dptr = DSP_WAKE_UP_BMS_ADRESSING;
                    }
//                    if(gstCollectParaData.pcs_data.udp_pcs_hold_para.dsp_wake_en == DSP_WAKUPING)
//                    {
//                        wakeup_repeat_cnt = 0;// set dsp ok
//                        wake_dptr = DSP_WAKE_UP_BMS_ADRESSING;
//                    }
                    break;
                }

                case DSP_WAKE_UP_BMS_ADRESSING:
                {
//                    SetBmsAdressEn();
                    wake_dptr = DSP_WAIT_WAKE_UP_ADRESS;
                    break;
                }
                case DSP_WAIT_WAKE_UP_ADRESS:
                {
                    if(gstFirmwareUpgrateInfo.updata_slave_type == UPDATA_NO_TYPE)
                    {
                        //adress over
                        wake_dptr = DSP_SET_WAKE_UP_BAT_VOLT;
                    }
                    wakeup_repeat_cnt = 0;
                    break;
                }
                case DSP_SET_WAKE_UP_BAT_VOLT:
                {
                    if(gstPackSlavePara.pack_para[0].bms_info.base_info.model_id != 0)
                    {
                        batt_volt = gstPackSlavePara.pack_para[0].bms_info.status_info.total_vol;
                    }
                    else
                    {
                        batt_volt = 400;
                    }
                    SetDspWakeUpStatusToBms(BMS_DSP_WAKUPING);
//                    SetBattVoltToDsp((uint16_t)batt_volt, DSP_WAKUPING);
                    time_out = tim_get_systime();
                    wake_dptr = DSP_WAIT_WAKE_UP_BAT_VOLT;
                    break;
                }
                case DSP_WAIT_WAKE_UP_BAT_VOLT:
                {
                    if(tim_get_systime() - time_out > 3000)
                    {
                        time_out = tim_get_systime();
                        if(++wakeup_repeat_cnt > 10)
                        {
                            wakeup_repeat_cnt = 0;
                            wake_dptr = DSP_WAKE_UP_ERR;      //写入失败
                        }
                        else
                        {
                            wake_dptr = DSP_SET_WAKE_UP_BAT_VOLT;
                        }

                    }
                    else
                    {
//                        uint16_t  dlt_data = 0;
//                        batt_volt = (gstBmsSlavePara.bms_para[0].bms_front_para.cell_total_voltage[0] << 16) + \
//                                    gstBmsSlavePara.bms_para[0].bms_front_para.cell_total_voltage[1];
                        if(gstCollectInfo.power_data.inside_info.dsp_wake_up_status == BMS_DSP_WAKUPING)

//                        if((gstCollectParaData.bms_data[0].bms_hold_para.dsp_wakeup_satus == BMS_DSP_WAKUPING) && (gstCollectParaData.pcs_data.udp_pcs_input_para.dsp_wake_up_status == 4))
                        {
//                            if(gstCollectInfo.power_data.power_value_info.inv_bat_vol >= batt_volt)
//                            {
//                                dlt_data = gstCollectInfo.power_data.power_value_info.inv_bat_vol - batt_volt;
//                            }
//                            else
//                            {
//                                dlt_data = batt_volt - gstCollectInfo.power_data.power_value_info.inv_bat_vol;
//                            }
//                            if((dlt_data <= 55)&&(gstCollectInfo.power_data.power_value_info.inv_bat_vol > 400))
//                            {
//                                wake_dptr = DSP_SET_EXIT_WAKE_UP;  //wake up OK
//                                wakeup_repeat_cnt = 0;
//                            }
                            wake_dptr = DSP_SET_EXIT_WAKE_UP;  //wake up OK
                            wakeup_repeat_cnt = 0;


                        }
                    }
                    break;
                }
                /* DSP退出唤醒 */
                case DSP_SET_EXIT_WAKE_UP:
                {
                    SetDspWakeUpStatusToBms(BMS_DSP_WAKUP_EXIT);
                    SetBattVoltToDsp((uint16_t)batt_volt, DSP_WAKUP_OVER);
                    time_out = tim_get_systime();
                    wake_dptr = DSP_WAIT_EXIT_WAKE_UP;
                    break;
                }
                /* DSP等待退出唤醒ACK */
                case DSP_WAIT_EXIT_WAKE_UP:
                {
                    if(tim_get_systime() - time_out > 3000)
                    {
                        time_out = tim_get_systime();

                        if(++wakeup_repeat_cnt > 10)
                        {
                            wakeup_repeat_cnt = 0;
                            wake_dptr = DSP_WAKE_UP_ERR;      //写入失败
                        }
                        else
                        {
                            wake_dptr = DSP_SET_EXIT_WAKE_UP;
                        }
                    }
                    else
                    {
                        if((gstCollectInfo.power_data.inside_info.dsp_wake_up_status == BMS_DSP_WAKUP_EXIT) &&        /*?????*/
                                (gstCollectInfo.power_data.inside_info.dsp_wake_en == DSP_WAKUP_OVER) && \
                                (gstCollectInfo.power_data.inside_info.dsp_wake_up_status == 4))//发送退出  返回激活完成
                        {
                            eSystemStatus = EN_SYS_WORKING_STATUS;
                            wake_dptr = 0;
                        }
                    }
                    break;
                }
                case DSP_WAKE_UP_ERR:
                {
                    break;
                }
                case DSP_TEST_MODE:
                {

                }
                default:
                    break;
            }
            break;
        }
        case EN_SYS_IDLE_STATUS:
        {


            break;
        }
        case EN_SYS_WORKING_STATUS:
        {
            gstCollectInfo.maincard_data.sys_info.dev_run_state = 2;
            CheckTestModeFun();
            RemotLoadPowerCal();
            SetUserTypeToPcs();
            _BalanceCtrlProc();         //均衡控制
            _AllPackAveVoltSetProc();   //通知每个Pack，监控卡计算的所有Pack的平均电压
//            SetElecMeterTypeToPcs();
            _SendDrmCtrlInfo();         //DRM处理
            _SendLLCEnalbeInfo();       //LLC开启
            _ChgDsgAllowFlagSetFunc();  //充放电标志设置
            _SetPackSOCToMainCop();     //给协处理器下发SOC值  
            break;
        }
        case EN_SYS_ERROR_STATUS:
        {

            break;
        }
        default:
        {
            break;
        }
    }
}


void SystemTurnOffCtrl(void)
{

    uint8_t power_status = 0;
    static  uint8_t remote_pcs_dptr = 0;
    static  uint32_t time_out = 0;
    static  uint8_t repet_send_cnt = 0;
//    power_status = GetSystemPowerStatus();
    switch(power_status)
    {
        case BMS_NORMAL:
        {
            repet_send_cnt = 0;
            remote_pcs_dptr = 0;
            time_out = tim_get_systime();
            break;
        }
        case BMS_TRUN_OFF:
        case BMS_TURN_ON:
        {
            //关机
            if(power_status == BMS_TRUN_OFF)
            {
                //关灯
                SwitchLedOff();
            }
            else
            {
                //开灯
                SwitchLedOn();
            }
            switch(remote_pcs_dptr)
            {
                case 0:
                {
                    time_out = tim_get_systime();
                    remote_pcs_dptr = 1;
                    CleanPcsRmoteAck();
//                    SetPcsRemoteShutdown(power_status);//发送关机命令
                    break;
                }
                case 1:
                {
                    //延时200ms 判断是否读取到，否则继续发送 连续发送3次 则不在进行
                    if(GetPcsRomoteAck() == 1)
//                    if(Ack_flag == 1)
                    {
                        remote_pcs_dptr = 0;
//                        AckBmsRomotShutDownCtrl(power_status);//已经应答成功，需要通过CAN 返回给BMS
                        gSystemPowerStatus = power_status;
//                        CleanPowerStatus();
                        _gCapReceiveTurnOff = 1;
                    }
                    else
                    {

                        if(tim_get_systime() - time_out > 200)
                        {
                            time_out = tim_get_systime();
//                        gSystemPowerStatus = power_status;
//AckBmsRomotShutDownCtrl(power_status);//已经应答成功，需要通过CAN 返回给BMS
                            repet_send_cnt++;
                            if(repet_send_cnt > 3)
                            {
//                                CleanPowerStatus();
                            }
                            else
                            {
                                remote_pcs_dptr = 0;
                            }

                        }
                    }
                }
                default:
                    break;

            }

            break;
        }
        default:
        {
            break;
        }

    }
}


uint8_t GetTurnOffStatus()
{
    return _gCapReceiveTurnOff;
}

/**
  * @brief  获取系统运行状态
  * @param  NULL
  * @return NULL
  * @note   NULL
  */
EN_SYS_STATUS GetSysRunStatus()
{
    return eSystemStatus;
}
