
#include "bms_protocol.h"
#include "commonpack_analysis.h"
#include "dev_led.h"
#include "tooling.h"
#include "dev_key.h"
#include "dev_mos.h"
#include "can_main_card.h"
#include "modbus_analysis.h"
#include "dev_spiflash.h"
#include "rtc.h"

#define LOG_TAG     "bms_protocol"

#define HOLD_NOW_STATUS         (0) //保持当前状态
#define ENTER_OFF_STATUS        (1) //进入关机状态
#define ENTER_RESTART_STATUS    (2) //进入重启状态


/* 整机下电处理 */
uint8_t _RunStepOffProcess(void);


EN_BMS_RUNSTATE _UpLoadStatus = EN_BMS_RUNSTATE_DISABLE;

BMSSysInfo_t   _gBMSSysInfo;
uint8_t _garrErrorSaveData[MAX_ERROR_SAVE_ADDR_NUM][5] = {0xAA, 0, 0, 0, 0};
uint8_t _garrErrorSaveFlag[MAX_ERROR_SAVE_ADDR_NUM];

uint8_t GetSystemErrorLevel(void)
{
    uint8_t bms_sysfailsta;
    if((gBmsErrorInfo.mos_error_code.value & 0x003F) || (gBmsErrorInfo.curr_error_code.value & 0X0024) || \
            (gBmsErrorInfo.voltage1_error_code.value & 0x01E4) || (gBmsErrorInfo.voltage2_error_code.value & 0x0000) || \
            (gBmsErrorInfo.temper_error_code.value & 0x0008) || (gBmsErrorInfo.state_error_code.value & 0x0004) || (gBmsErrorInfo.other1_error_code.value & 0x03FF))
    {
        _gBMSSysInfo.bms_sys_fail_sta = 3;
    }
    else if((gBmsErrorInfo.mos_error_code.value & 0x0040) || (gBmsErrorInfo.curr_error_code.value & 0x0012) || \
            (gBmsErrorInfo.voltage1_error_code.value & 0x0012) || (gBmsErrorInfo.voltage2_error_code.value & 0x0000) || \
            (gBmsErrorInfo.temper_error_code.value & 0x0024) || (gBmsErrorInfo.state_error_code.value & 0x0002) || (gBmsErrorInfo.other1_error_code.value & 0x0000))
    {
        _gBMSSysInfo.bms_sys_fail_sta = 2;
    }
    else if((gBmsErrorInfo.mos_error_code.value & 0x0080) || (gBmsErrorInfo.curr_error_code.value & 0x0009) || \
            (gBmsErrorInfo.voltage1_error_code.value & 0x0009) || (gBmsErrorInfo.voltage2_error_code.value & 0x0000) || \
            (gBmsErrorInfo.temper_error_code.value & 0x0013) || (gBmsErrorInfo.state_error_code.value & 0x0001) || (gBmsErrorInfo.other1_error_code.value & 0x0000))
    {
        _gBMSSysInfo.bms_sys_fail_sta = 1;
    }
    else
    {
        _gBMSSysInfo.bms_sys_fail_sta = 0;
    }

    return _gBMSSysInfo.bms_sys_fail_sta;
}

void RecoverErrorCode(void)
{
    /* 读取锁存故障码 */
    uint8_t            success_flag[2] = {0, 0};
    BMSErrorLockInfo_t st_rev_error_info[2];
    memset(&st_rev_error_info, 0, sizeof(BMSErrorLockInfo_t) * 2);
    
    //读取
    for(int i=0; i<5; i++)
    {
        //读取保存的数据
        spi_flash_read_bytes(ERROR_INFO_1_FLASH_ADDR, (uint8_t*)&st_rev_error_info, sizeof(BMSErrorLockInfo_t) * 2);
        
        //读取成功
        if((st_rev_error_info[0].start_flag == 0X5555) && (st_rev_error_info[0].end_flag   == 0XAAAA))
        {
            memcpy((uint8_t*)&gBMSLockErrorInfo[0], (uint8_t*)&st_rev_error_info[0], sizeof(BMSErrorLockInfo_t));
            gBMSLockErrorInfo[0].lock_error_code.b.flash_read_success = 1;
            garrModbusOnlyReadReg[0].once_run_error_code_1_high = gBMSLockErrorInfo[0].lock_error_code.value >> 16;
            garrModbusOnlyReadReg[0].once_run_error_code_1_low  = gBMSLockErrorInfo[0].lock_error_code.value;
            success_flag[0] = 1;
        }
        
        //读取成功
        if((st_rev_error_info[1].start_flag == 0X5555) && (st_rev_error_info[1].end_flag   == 0XAAAA))
        {
            memcpy((uint8_t*)&gBMSLockErrorInfo[1], (uint8_t*)&st_rev_error_info[1], sizeof(BMSErrorLockInfo_t));
            gBMSLockErrorInfo[1].lock_error_code.b.flash_read_success = 1;
            garrModbusOnlyReadReg[0].once_run_error_code_2_high = gBMSLockErrorInfo[1].lock_error_code.value >> 16;
            garrModbusOnlyReadReg[0].once_run_error_code_2_low  = gBMSLockErrorInfo[1].lock_error_code.value;
            success_flag[1] = 1;
        }
        
        //全部读取正确，跳出
        if((success_flag[0] == 1) && (success_flag[1] == 1))
        {
            break;
        }
        
        soft_delayms(5);
    }
}

/**
 * @brief  : Flash故障保存函数
 * @para   : None
 * @note   : None
 * @return : None
 */
void _FlashSaveErrorCode(void)
{
    BMSErrorLockInfo_t *p_lock = NULL; 
    BMSErrorLockInfo_t *p_other_lock = NULL; 
    BMSErrorLockInfo_t st_error_save_check[2]; 
    volatile uint32_t save_addr = 0;
    volatile uint32_t save_id = 0;
    
    if((gBMSLockErrorInfo[0].lock_error_code.b.number == 0) && (gBMSLockErrorInfo[1].lock_error_code.b.number== 7))
    {
        p_lock = &gBMSLockErrorInfo[1];
        p_other_lock = &gBMSLockErrorInfo[0];
        save_addr = ERROR_INFO_1_FLASH_ADDR;
        save_id = 1;
    }
    else if(gBMSLockErrorInfo[0].lock_error_code.b.number < gBMSLockErrorInfo[1].lock_error_code.b.number)
    {
        p_lock = &gBMSLockErrorInfo[0];
        p_other_lock = &gBMSLockErrorInfo[1];
        save_addr = ERROR_INFO_1_FLASH_ADDR;
        save_id = 0;
    }
    else
    {
        p_lock = &gBMSLockErrorInfo[1];
        p_other_lock = &gBMSLockErrorInfo[0];
        save_addr = ERROR_INFO_1_FLASH_ADDR;
        save_id = 1;
    }
    
    //起始标志
    p_lock->start_flag = 0X5555;
    //故障信息
    p_lock->lock_error_code.b.cell_under_vol3        = gBmsErrorInfo.voltage1_error_code.b.cell_under_vol3      ;   /* 单体欠压3级          */
    p_lock->lock_error_code.b.cell_over_vol3         = gBmsErrorInfo.voltage1_error_code.b.cell_over_vol3       ;   /* 单体过压3级          */
    p_lock->lock_error_code.b.pack_under_vol1        = gBmsErrorInfo.voltage1_error_code.b.pack_under_vol1      ;   /* 电池包总电压欠压故障 */
    p_lock->lock_error_code.b.pack_over_vol1         = gBmsErrorInfo.voltage1_error_code.b.pack_over_vol1       ;   /* 电池包总电压过压故障 */
    p_lock->lock_error_code.b.cell_voltage_diff1     = gBmsErrorInfo.voltage1_error_code.b.cell_voltage_diff1   ;   /* 电芯压差大           */
    p_lock->lock_error_code.b.dsg_overcurr3          = gBmsErrorInfo.curr_error_code.b.discharge_overcurr3      ;   /* 放电过流3级          */
    p_lock->lock_error_code.b.chg_overcurr3          = gBmsErrorInfo.curr_error_code.b.charge_overcurr3         ;   /* 充电过流3级          */
    p_lock->lock_error_code.b.cell_low_temper1       = gBmsErrorInfo.temper_error_code.b.cell_low_temper1       ;   /* 电池温度过低故障     */
    p_lock->lock_error_code.b.cell_high_temper3      = gBmsErrorInfo.temper_error_code.b.cell_high_temper3      ;   /* 电池温度过高3级故障  */
    p_lock->lock_error_code.b.ntc_high_diff2         = gBmsErrorInfo.temper_error_code.b.ntc_high_diff2         ;   /* ntc温差大故障2级     */
    p_lock->lock_error_code.b.soh_low_error3         = gBmsErrorInfo.state_error_code.b.soh_low_error3          ;   /* SOH过低3级故障       */
    p_lock->lock_error_code.b.vol_sensor_error       = gBmsErrorInfo.other1_error_code.b.vol_sensor_error       ;   /* 电压传感器故障       */
    p_lock->lock_error_code.b.cell_ntc_error         = gBmsErrorInfo.other1_error_code.b.cell_ntc_error         ;   /* 电池温度传感器故障   */
    p_lock->lock_error_code.b.spi_flash_error        = gBmsErrorInfo.other1_error_code.b.spi_flash_error        ;   /* spi fash故障         */
    p_lock->lock_error_code.b.e2prom_error           = gBmsErrorInfo.other1_error_code.b.e2prom_error           ;   /* e2prom故障           */
    p_lock->lock_error_code.b.rtc_error              = gBmsErrorInfo.other1_error_code.b.rtc_error              ;   /* rtc故障              */
    p_lock->lock_error_code.b.chg_short_protection   = gBmsErrorInfo.other1_error_code.b.chg_short_protection   ;   /* 充电短路保护         */
    p_lock->lock_error_code.b.dsg_short_protection   = gBmsErrorInfo.other1_error_code.b.dsg_short_protection   ;   /* 放电短路保护         */
    p_lock->lock_error_code.b.pre_charge_timeout     = gBmsErrorInfo.other1_error_code.b.pre_charge_timeout     ;   /* 预充超时故障         */
    p_lock->lock_error_code.b.heat_error             = gBmsErrorInfo.other1_error_code.b.heat_error             ;   /* 加热膜短路故障       */
    p_lock->lock_error_code.b.afe_error              = gBmsErrorInfo.other1_error_code.b.afe_com_error          ;   /* AFE断线检测故障      */
    p_lock->lock_error_code.b.mos_adhesion           = gBmsErrorInfo.mos_error_code.b.mos_adhesion              ;   /* 放电或充电MOS粘连故障*/
    p_lock->lock_error_code.b.pmos_fail_close        = gBmsErrorInfo.mos_error_code.b.pmos_fail_close           ;   /* 预充MOS无法闭合故障  */
    p_lock->lock_error_code.b.cmos_fail_close        = gBmsErrorInfo.mos_error_code.b.cmos_fail_close           ;   /* 充电MOS无法闭合故障  */
    p_lock->lock_error_code.b.dmos_fail_close        = gBmsErrorInfo.mos_error_code.b.dmos_fail_close           ;   /* 放电MOS无法闭合故障  */
    p_lock->lock_error_code.b.mos_ntc_error          = gBmsErrorInfo.mos_error_code.b.mos_ntc_error             ;   /* MOS传感器故障        */
    p_lock->lock_error_code.b.mos_high_temper3       = gBmsErrorInfo.mos_error_code.b.mos_high_temper3          ;   /* MOS过温3级           */
    //流水号
    p_lock->lock_error_code.b.number = (p_other_lock->lock_error_code.b.number + 1) % 8;  //number的最大值是8
    //结束标志
    p_lock->end_flag = 0XAAAA;
    
    //保存
    for(int i=0; i<5; i++)
    {
        spi_flash_write_bytes_with_erase(save_addr, (uint8_t*)gBMSLockErrorInfo, sizeof(BMSErrorLockInfo_t) * 2);
        
        soft_delayms(5);

        //读取保存的数据
        spi_flash_read_bytes(save_addr, (uint8_t*)st_error_save_check, sizeof(BMSErrorLockInfo_t) * 2);
        
        //确认备份成功
        if((st_error_save_check[save_id].start_flag == 0X5555) && (st_error_save_check[save_id].end_flag   == 0XAAAA))
        {
            //停止重复备份
            break;
        }
        soft_delayms(5);
    }
    
}


void SaveErrorCode(void)
{
    uint32_t rtc_time = 0;
    uint8_t error_save_data[5] = {0xAA, 0, 0, 0, 0};
    
    if(gBmsErrorInfo.state_error_code.b.soh_low_error1)//电池SOH过低1级故障
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR3_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.state_error_code.b.soh_low_error2)//电池SOH过低2级故障
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR4_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.state_error_code.b.soh_low_error3)//电池SOH过低3级故障
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR5_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.mos_error_code.b.mos_adhesion)//放电或预充或充电mos粘连故障
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR6_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.other1_error_code.b.chg_short_protection)//充电短路
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR9_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.other1_error_code.b.dsg_short_protection)//放电短路
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR10_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.other1_error_code.b.heat_error)//加热膜短路
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR11_SAVE_ADDR, 5);
    }

    if(gBmsErrorInfo.other1_error_code.b.afe_com_error)//加热膜短路
    {
        rtc_time = RtcGetTime();
        memcpy(&error_save_data[1], (uint8_t*)&rtc_time, sizeof(rtc_time));
        EepromWrite(error_save_data, ERROR13_SAVE_ADDR, 5);
    }
    
    //故障锁存
    _FlashSaveErrorCode();
    
}

static bool _EmergencyErrorCheck(void)
{
    //工装模式，该函数不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 1)
    {
        return false;
    }
    
    uint8_t error_level = 0;
    uint32_t rtc_time = 0;
    uint8_t error_save_data[5] = {0xAA, 0, 0, 0, 0};
    static uint8_t over_current_flag = 0;
    static uint32_t over_current_times = 0;
    static bool is_first_error_out = false;

    error_level = GetSystemErrorLevel();
            
    if(error_level == 3)
    {
        is_first_error_out = true;
        return true;
    }
    else if((error_level == 2) || (error_level == 1))
    {
        is_first_error_out = true;
        /* MOS故障 */
        //mos过温一级功率降50%
        if(gBmsErrorInfo.mos_error_code.b.mos_high_temper1 && gPackStatus.mode == EN_PACK_CURRENT_DISCHARGE_MODE)
        {
            //放电状态下，关闭放电MOS，不允许继续放电
            CloseDisChargeMos();
        }
        else
        {
            //充电或空闲状态下，降低功率
            gPackStatus.current_power_percentage = 0.5;
        }
        
        //mos过温2级功率降到0
        if(gBmsErrorInfo.mos_error_code.b.mos_high_temper2)
        {
            gPackStatus.current_power_percentage = 0;
            CCEnOff();
            CloseChargeMos();
        }
        
        /* 电芯电压故障 */
        //电池二级过压，关闭充电MOS，打开放电mos
        if(gBmsErrorInfo.voltage1_error_code.b.cell_over_vol2)
        {
            CCEnOff();
            CloseChargeMos();
        }
        
        //电池二级欠压，关闭放电MOS，打开充电MOS
        if(gBmsErrorInfo.voltage1_error_code.b.cell_under_vol2)
        {
            //强充状态下，不关放电MOS
            if((_gBMSSysInfo.force_chg_allow_flag == 2) && 
                (_gBMSSysInfo.uv_hold_flag == 2))
            {
                CloseDisChargeMos();
            }
        }
        
        /* 电芯温度故障 */
        if(gBmsErrorInfo.temper_error_code.b.cell_high_temper1)//充放电温度高功率降50%
        {
            gPackStatus.current_power_percentage = 0.5;
        }

        if(gBmsErrorInfo.temper_error_code.b.cell_high_temper2)//充放电温度高功率降到0%
        {
            CCEnOff();
            CloseChargeMos();
            CloseDisChargeMos();
            gPackStatus.current_power_percentage = 0;
        }

        if(gBmsErrorInfo.temper_error_code.b.cell_low_temper1)//充放电温度高功率降到0%
        {
            gPackStatus.current_power_percentage = 0;
        }
        
        //老化测试模式，不判断电流故障
        if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 2)
        {
            return false;
        }
        
        /* 电流故障 */
        //二级充电过流，关闭mos
        if(gBmsErrorInfo.curr_error_code.b.charge_overcurr2)//降功率为0
        {
            CCEnOff();
            CloseChargeMos();
            CloseDisChargeMos();
            gPackStatus.current_power_percentage = 0;
        }
        
        //二级放电过流，关闭mos
        if(gBmsErrorInfo.curr_error_code.b.discharge_overcurr2)//放电过流
        {
            CCEnOff();
            CloseChargeMos();
            CloseDisChargeMos();
            gPackStatus.current_power_percentage = 0;//降功率为0
        }
        return false;
    }
    else
    {
        if(is_first_error_out == true)
        {
            is_first_error_out = false;
            OpenDisChargeMos();
        }

        gPackStatus.current_power_percentage = 1.0f;
        return false;
    }
}

static void _Update_gBMSSys_info(void)
{
    _gBMSSysInfo.precharge_state = garrModbusWriteReadReg[0].pre_mos_ctl;
    _gBMSSysInfo.charge_state = garrModbusWriteReadReg[0].chg_mos_ctl;
    _gBMSSysInfo.discharge_state = garrModbusWriteReadReg[0].dsg_mos_ctl;
}

/* 上电开MOS处理 */
bool _StartRunMosProcess()
{
    static uint32_t interval_time;  //开MOS间隔时间
    static bool first_run_flag = true;
    
    if(first_run_flag)
    {
        OpenDisChargeMos();//开放电MOS
        
        //老化测试模式下，开启充放电MOS,直接进入运行状态
        if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl  == 2)
        {
            OpenChargeMos();
            CCEnOff();
        }
        else
        {
            CloseChargeMos();
            CCEnOn();
        }
        interval_time = tim_get_systime();  //获取系统时间
        first_run_flag = false;
    }
    
    if(tim_difftime_from_now(interval_time) < 20)
    {
        return false;
    }
    
    ClosePreChargeMos();
    first_run_flag = true;  //刷新标志，保证下次进入时正常允许
    
    return true;
}

/**
 * @brief  : 重启初始化函数（下电过程中使用）
 * @para   : None
 * @note   : None
 * @return : None
 */
void _ReStartInit()
{
    memset(&gBmsErrorInfo, 0, sizeof(BMSErrorInfo_t));
    RstClearShutDownReg();  //设置开机标志，清除关机命令
    FaultRegInit(); //故障初始化
    DevLedInit();   //LED闪烁方式初始化
    _gBMSSysInfo.first_enter_off_flag = true;
    gUartPackInfo.final_com_time = tim_get_systime();   //重新上电前刷新通讯超时时间，但不清除com_out_flag
}


/**
 * @brief  : 耐久度测试模式判断
 * @para   : None
 * @note   : None
 * @return : None
 */

void _EnduranceModeJudgeFunc(void)
{
    static uint32_t check_time = 0;
    static uint16_t key_check_cnt = 0;
    
    int x = GetKeyStatus();
    //确定进入耐久度测试模式、正常模式、老化测试模式后不再判断
    //测试标志为 0 时，判断按键是否一直被按下，持续按下2分钟进入耐久度测试模式，标志位改为1
    //一旦判断按键有松开，立即进入正常运行模式，且不允许再进入耐久度测试模式，标志位改为2
    if((_gBMSSysInfo.endurance_mode_flag != 0) || 
       (gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 2)
      )
    {
        return;
    }
    
    //每1s检测一次
    if(tim_difftime_from_now(check_time) > 1000)
    {
        check_time = tim_get_systime();
        if(GetKeyStatus() == 0)
        {
            if(key_check_cnt++ >= 180)
            {   //进入耐久度测试模式
                _gBMSSysInfo.endurance_mode_flag = 1;
                LedCtlBlink(LED_SRC_SYS, LED_BLINK_TIME_NORMAL);
                _UpLoadStatus = EN_BMS_RUNSTATE_DISABLE;
            }
        }
        else
        {
            //进入正常运行模式
            key_check_cnt = 0;
            _gBMSSysInfo.endurance_mode_flag = 2;
        }
    }
}



/**
 * @brief  : 距离上次通讯超过一定时间后，代表通讯失败，BMS关机
 * @para   :
 * @note   : None
 * @return : None
 */
bool _MainCardCommonFailProcess(EN_BMS_RUNSTATE* current_run_status)
{
    //老化模式下不判断无通讯关机
    if(_gBMSSysInfo.endurance_mode_flag == 1)
    {
        return true;
    }
    //关机状态下不再判断
    if( *current_run_status == EN_BMS_RUNSTATE_OFF ||
        *current_run_status == EN_BMS_RUNSTATE_OFF_OVER
      )
    {
        return true;
    }
    //距离上次通讯超过一定时间后，直接关机
    if(tim_difftime_from_now(gUartPackInfo.final_com_time) > gUartPackInfo.com_out_time)
    {
        gUartPackInfo.com_out_flag = 1;
        *current_run_status = EN_BMS_RUNSTATE_OFF;
        return false;
    }
    return true;
}



/**
 * @brief  : 进入正常运行状态判断
 * @para   : None
 * @note   : None
 * @return : 
 */
void _StartRunSignalJudge(EN_BMS_RUNSTATE* current_run_status)
{
    static KeyMsg_t    *p_msg;
    static bool        first_judge_flag;          //该函数首次运行标志
    static uint32_t    wait_start_signal_time;    //等待开机信号时长

    if(first_judge_flag == 0)
    {
        //记录时间
        first_judge_flag = 1;
        wait_start_signal_time = tim_get_systime();
    }
    
    //120s内未接收到开机信号，停止运行
    if(tim_difftime_from_now(wait_start_signal_time) > 120000)
    {
        _gBMSSysInfo.start_out_flag = 1;
        first_judge_flag = 0;       //跳转后清除
        *current_run_status = EN_BMS_RUNSTATE_OFF_PRE_PROCESS;
        return;
    }
    
    //按键开机
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        if(p_msg->press_value == (EN_KEY_PRESSED_MSG)EN_KEY_SWITCH_LONG_SINGLE_PRESS)
        {
            //第一次检测到长按信号，为开机信号
            if(_gBMSSysInfo.long_key_start_flag == 0)
            {
                _gBMSSysInfo.long_key_start_flag = 1;
            }
        }
    }
    
    //检测到开机信号(长按按键、DSP重启完成标志、BMS重启测试标志)，进入待机状态
    if((_gBMSSysInfo.long_key_start_flag == 1) || (_gBMSSysInfo.dsp_status_flag == EN_DSP_STATUS_WAKEUP_FINISH) ||\
        NOVA_GET_BIT(garrModbusWriteReadReg[0].sys_up_status, 2) != 0)
    {
        //DSP重启完成标志置起(此时为PV启机)
        if(_gBMSSysInfo.long_key_start_flag == 0 && NOVA_GET_BIT(garrModbusWriteReadReg[0].sys_up_status, 2) == 0)
        {
            _gBMSSysInfo.bms_uv_pv_start_flag = true;
        }
        //按键启机时，不运行进行强充
        if(_gBMSSysInfo.long_key_start_flag == 1)
        {
            _gBMSSysInfo.force_chg_allow_flag = 2;
        }
        
        *current_run_status = EN_BMS_RUNSTATE_STANDBY;
        first_judge_flag = 0;       //跳转后清除
    }
}

/**
 * @brief  : 重启信号判断（关机状态使用）
 * @para   : None
 * @note   : None
 * @return : 
 */
bool _ReStartSignalJudge()
{
    static KeyMsg_t    *p_msg;
    
    //按键开机
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        if(p_msg->press_value == (EN_KEY_PRESSED_MSG)EN_KEY_SWITCH_LONG_SINGLE_PRESS)
        {
            //关机状态下检测到的长按信号均视为开机信号
            _gBMSSysInfo.long_key_start_flag = 1;
            return true;
        }
    }
    return false;
}

/**
 * @brief  : 结束运行状态判断
 * @para   : None
 * @note   : None
 * @return : 
 */
void _EndKeySignalJudge(EN_BMS_RUNSTATE* current_run_status)
{
    static KeyMsg_t    *p_msg;
    
    //按键启机时，没有长按信号，不允许有关机信号
//    if((gUartPackInfo.key_start_flag == 1) && (_gBMSSysInfo.long_key_start_flag == 0))
//    {
//        return;
//    }
    //未进入开机运行模式，不检测按键
    if(*current_run_status <= EN_BMS_RUNSTATE_RUN_SIGNAL_WAIT)
    {
        return;
    }
    
    //已经进入关机模式时，不再检测关机按键
    if((*current_run_status == EN_BMS_RUNSTATE_OFF_PRE_PROCESS) ||
       (*current_run_status == EN_BMS_RUNSTATE_OFF) ||
       (*current_run_status == EN_BMS_RUNSTATE_OFF_OVER) ||
       (*current_run_status == EN_BMS_RUNSTATE_RESTART))
    {
        return;
    }
    
    //按键关机检测
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        if(p_msg->press_value == (EN_KEY_PRESSED_MSG)EN_KEY_SWITCH_LONG_SINGLE_PRESS)
        {
            //检测到长按信号，为关机信号
//            _gBMSSysInfo.long_key_end_flag = 1;
            *current_run_status = EN_BMS_RUNSTATE_OFF_PRE_PROCESS;
        }
    }
}

/**
 * @brief  : 低压情况下，太长时间没有充电关机
 * @para   :
 * @note   : None
 * @return : None
 */
bool _UvEndRunProcess(EN_BMS_RUNSTATE* current_run_status)
{
    static uint32_t end_time = 0;
    
    //只允许第一个Pack运行
    if(gUartPackInfo.ModbusAddr != MODBUS_ADDR_PACK0)
    {
        _gBMSSysInfo.uv_hold_flag = 2;
        return true;
    }
    
    //老化模式下不判断无通讯关机
    if(_gBMSSysInfo.endurance_mode_flag == 1)
    {
        return true;
    }
    //关机状态下不再判断
    if( *current_run_status == EN_BMS_RUNSTATE_OFF ||
        *current_run_status == EN_BMS_RUNSTATE_OFF_OVER
      )
    {
        return true;
    }
    
    //一上电检测
    if(_gBMSSysInfo.uv_hold_flag == 0)
    {
        //如果检测到电芯最低电压<2.9V，置起欠压保持标志
        if(gCell_Info.cell_min_voltage <= DISCHARGE_MIN_VOLTAGE + 200)
        {
            //记录时间
            _gBMSSysInfo.uv_hold_flag = 1;
            end_time = tim_get_systime();
        }
        else
        {
            //否则不允许再置起欠压保持标志
            _gBMSSysInfo.uv_hold_flag = 2;
        }
    }
    
    //欠压保持状态
    else if(_gBMSSysInfo.uv_hold_flag == 1)
    {
        //最低节电芯小于最低电压，且不在充电模式,一直计时
        if((gCell_Info.cell_min_voltage > DISCHARGE_MIN_VOLTAGE) ||
        (gPackStatus.mode == EN_PACK_CURRENT_CHARGE_MODE))
        {
            //刷新时间
            end_time = tim_get_systime();
        }
        //低压且没有充电的情况下，5分钟后关机
        if(tim_difftime_from_now(end_time) > 300000)
        {
            _gBMSSysInfo.uv_hold_flag = 2;
            *current_run_status = EN_BMS_RUNSTATE_OFF;
            return false;
        }
        
        //如果检测到电芯最低电压>2.9V，不允许再置起欠压保持标志
        if(gCell_Info.cell_min_voltage > DISCHARGE_MIN_VOLTAGE + 200)
        {
            //记录时间
            _gBMSSysInfo.uv_hold_flag = 2;
        }
    }
    return true;
}

/**
 * @brief  : 关机寄存器判断判断
 * @para   : None
 * @note   : None
 * @return : 
 */
void _ShutDownRegEndJudge(EN_BMS_RUNSTATE* current_run_status)
{
    //已经进入关机模式时，不再检测关机按键
    if((*current_run_status == EN_BMS_RUNSTATE_OFF_PRE_PROCESS) ||
       (*current_run_status == EN_BMS_RUNSTATE_OFF) ||
       (*current_run_status == EN_BMS_RUNSTATE_OFF_OVER) ||
       (*current_run_status == EN_BMS_RUNSTATE_RESTART))
    {
        return;
    }

    //检测到关机寄存器
    if(ShutdownRegGetMonitor() == MONITOR_OFF_CMD)
    {
        *current_run_status = EN_BMS_RUNSTATE_OFF_PRE_PROCESS;
    }
    
    //有多个Pack且是第一个Pack
    if((gUartPackInfo.ModbusAddr == MODBUS_ADDR_PACK0) && (gUartPackInfo.bms_num > 1))
    {
        //检测到后续有一个包不是开机状态，整机关机
        for(int i=1; i<gUartPackInfo.bms_num; i++)
        {
            if((garrModbusWriteReadReg[i].bms_shutdown & 0X0F) != BMS_ON_STATUS)
            {
                *current_run_status = EN_BMS_RUNSTATE_OFF_PRE_PROCESS;
            }
        }
    }
}


/**
 * @brief  : 关机信号判断
 * @para   : None
 * @note   : None
 * @return : None
 */
void _ShutDownSignal(EN_BMS_RUNSTATE* current_run_status)
{
    //关机按键信号判断
    if(_gBMSSysInfo.endurance_mode_flag != 1)
    {
        _EndKeySignalJudge(current_run_status);
    }
    //没有与监控卡通讯，自动关机（耐久度测试下取消通讯超时关机）
    _MainCardCommonFailProcess(current_run_status);
    //欠压且长时间没有充电时，自动关机(5min)
    _UvEndRunProcess(current_run_status);
    //后续关机寄存器判断
    _ShutDownRegEndJudge(current_run_status);
    
}

/**
 * @brief  : 第一个包查询后续Pack的关机状态
 * @para   : None
 * @note   : None
 * @return : 
 */
bool _FirstPackCheckShutDownFunc()
{
    static uint32_t send_time = 0;
    static uint16_t send_addr = MODBUS_ADDR_PACK0;
           uint16_t pack_cnt = 0;
    
    
    if(tim_difftime_from_now(send_time) < 4000)
    {
        return false;
    }
    send_time = tim_get_systime();
    
    //检测
    for(pack_cnt=1; pack_cnt<gUartPackInfo.bms_num; pack_cnt++)
    {
        if((garrModbusWriteReadReg[pack_cnt].bms_shutdown & 0X00FF) != BMS_OFF_STATUS)
        {
            //有bms没有关机，发送关机命令
            ShutDownCmdSend();
            break;
        }
    }
    //所有Pack均进入关机状态
    if(pack_cnt == gUartPackInfo.bms_num)
    {
        return true;
    }
    return false;
}

/**
 * @brief  : 运行状态预下电处理
 * @para   : None
 * @note   : None
 * @return : 
 */
void _RunStepPreOffProcess(EN_BMS_RUNSTATE* current_run_status)
{
    static uint32_t shutdown_wait_time = 0;
       
    //设置关机寄存器
    if(ShutdownRegGetSelf() != BMS_OFF_ING_STATUS)
    {
        shutdown_wait_time = tim_get_systime();
        ShutdownRegSet(BMS_OFF_ING_STATUS);
    }
    
    //等待关机指令或20s
    if((ShutdownRegGetMonitor() == MONITOR_OFF_CMD) ||
       (tim_difftime_from_now(shutdown_wait_time) > 20000)
      )
    {
        *current_run_status = EN_BMS_RUNSTATE_OFF;
    }

      

}    

/**
 * @brief  : BMS运行状态处理（上电、下电、正常运行）
 * @para   : None
 * @note   : None
 * @return : None
 */

void BMSRunStateProcess(void)
{
    static uint32_t precharge_times = 0;
    static uint32_t dsg_chg_opened_close_premos_times = 0;
    static uint32_t chg_open_dsgmos_times = 0;
    static uint32_t sn_time = 0;
    static uint32_t precharge_finsh_count = 0;
    static uint32_t packsamp_time = 0;
    static uint32_t pack_vol_ref = 0;
    static uint32_t precharge_timeout_count = 0;
    
    
    static bool     is_key_long_start_run = false;  //是否为按键长按开机
    static bool     is_error_flag         = false;  //是否出现故障
    
    //BMS信息更新
    _Update_gBMSSys_info();
    //关机信号
    _ShutDownSignal(&_UpLoadStatus);

    //耐久度测试模式判断
    _EnduranceModeJudgeFunc();
    //获取当前系统状态
    _gBMSSysInfo.bms_run_status = _UpLoadStatus;
    
    //上电2s后进入运行状态
    if(tim_difftime_from_now(sn_time) < 2000)
    {
        return;
    }
    switch(_UpLoadStatus)
    {
        
        /* 无效状态 */
        case EN_BMS_RUNSTATE_DISABLE:
        {
            _EmergencyErrorCheck();
            if((_gBMSSysInfo.bms_shutdown == 0) && (is_error_flag == 0) && (gUartPackInfo.com_out_flag == 0)
                && (_gBMSSysInfo.start_out_flag == 0))//开机
            {
                memset(&gBmsErrorInfo, 0, sizeof(BMSErrorInfo_t));
                FaultRegInit();
                RecoverErrorCode();
//                LOG_DEBUG(LOG_TAG, "BMS Init start!!!\r\n");
                _UpLoadStatus = EN_BMS_RUNSTATE_RUN_SIGNAL_WAIT;
            }
            //耐久度测试模式，直接跳过开机环节，进入准备状态
            if(_gBMSSysInfo.endurance_mode_flag == 1)
            {     
                _UpLoadStatus = EN_BMS_RUNSTATE_STANDBY;
            }

            break;
        }
        
        /* 等待允许运行信号 */
        case EN_BMS_RUNSTATE_RUN_SIGNAL_WAIT:
        {
            //开机信号判断
            _StartRunSignalJudge(&_UpLoadStatus);
            break;
        }
        
        /* 待机 */
        case EN_BMS_RUNSTATE_STANDBY:
        {
            if(_EmergencyErrorCheck())
            {
                _UpLoadStatus = EN_BMS_RUNSTATE_ERROR;
                break;
            }
            ReleaseOff();               //关闭泄放电路
            PowerHoldOn();
            CloseDisChargeMos();
            OpenPreChargeMos();         //打开预充mos
            OVCEnOpen();                //OVC使能
            _gBMSSysInfo.bms_run_flag = 0;
            _gBMSSysInfo.first_enter_off_flag = true;
            ShutdownRegSet(BMS_ON_STATUS);              //寄存器状态改为开机
            precharge_times = tim_get_systime();
            _UpLoadStatus = EN_BMS_RUNSTATE_POWER_HOLD;
            break;
        }
        /******* 保持 *******/
        case EN_BMS_RUNSTATE_POWER_HOLD:
        {
            if(_EmergencyErrorCheck())
            {
                ClosePreChargeMos();
                _UpLoadStatus = EN_BMS_RUNSTATE_ERROR;
                break;
            }
            
            //老化测试模式下，开启充放电MOS,直接进入运行状态
            if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl  == 2)
            {
                _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS;
                break;
            }
            if(NOVA_GET_BIT(garrModbusWriteReadReg[0].sys_up_status, 2) != 0)
            {
                _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS;
                break;
            }
            //正常模式及工装测试模式下，进行正常预充判断
            if(tim_difftime_from_now(precharge_times) > 20000)
            {
                precharge_times = tim_get_systime();
                ClosePreChargeMos();
                _UpLoadStatus = EN_BMS_RUNSTATE_ERROR;
            }
            if(gFaultAnalysisFlag.dischargemos_enable_flag) //放电mos已经打开
            {
                if(tim_difftime_from_now(dsg_chg_opened_close_premos_times) > 200)
                {
                    if(!gFaultAnalysisFlag.chargemos_enable_flag)
                    {
                        OpenChargeMos();
                    }
                    ClosePreChargeMos();
                    _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS;
                }
            }
            else
            {

                pack_vol_ref = gCell_Info.total_cell_voltage - 7000;      //电池电压 - 7V
                
                if(tim_difftime_from_now(packsamp_time) > 200)
                {
                    packsamp_time = tim_get_systime();
                    int32_t pre_vol =  OVCVoltageGet();
                    if(PreMosCloseErrorJudge(pre_vol) == false)
                    {
                        gBmsErrorInfo.mos_error_code.b.pmos_fail_close = 1;         //pmos故障
                    }
                    
                    if(_gBMSSysInfo.precharge_state && (pre_vol > pack_vol_ref))    //放电
                    {
                        bool pre_success_flag = false;
                        precharge_finsh_count ++;
                        
                        if(precharge_finsh_count > 5)
                        {
                            pre_success_flag = true;
                        }
                        
                        if(pre_success_flag == true)
                        {
                          
                            dsg_chg_opened_close_premos_times = tim_get_systime();
                            _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS;
                        }
                    }
                    else
                    {
                        precharge_timeout_count ++;
                        //预充10s以上，故障
                        if(precharge_timeout_count > 50)
                        {
                            precharge_timeout_count = 0;
                            ClosePreChargeMos();
                            gBmsErrorInfo.other1_error_code.b.pre_charge_timeout = 1;       //预充超时
                            _UpLoadStatus = EN_BMS_RUNSTATE_ERROR;//故障模式
                        }
                    }
                }
            }
            break;
        }
        
        /* 正常运行预处理 */
        case EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS:
        {
            static uint8_t cmd_send_count = 0;
            static uint32_t cmd_send_time = 0;
            static bool first_run_flag = true;

            if(_EmergencyErrorCheck())
            {
                ClosePreChargeMos();
                _UpLoadStatus = EN_BMS_RUNSTATE_ERROR;
                break;
            }
            
            //第一次进入该状态，开启MOS
            if(first_run_flag)
            {
                //开启MOS
                if(_StartRunMosProcess() == false)
                {
                    break;
                }
                first_run_flag = false;
            }
            
            if(tim_difftime_from_now(cmd_send_time) < 500)
            {
                break;
            }
            
            //发送开机指令
            SendShutDownSignal(2); 
            cmd_send_time = tim_get_systime();            
            cmd_send_count++;
            
            //收到开机指令，或重复发送次数，大于5次，直接开机
            if((cmd_send_count >= 10) || (CanRevData.bms_shut_down_signal == 2))
            {
                cmd_send_count = 0;
                gUartPackInfo.p_modbus_only_read_reg[0]->bms_pre_chg_status = 1;        //预充完成标志
                first_run_flag = true;                                                  //刷新flag，保证下次进入时正常运行
                _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL;
            }
            break;
        }
        
        /* 正常运行 */
        case EN_BMS_RUNSTATE_NORMAL:
        {
            HeatProcess();          //加热膜
            if(_EmergencyErrorCheck())
            {
                _UpLoadStatus = EN_BMS_RUNSTATE_ERROR;
            }

            break;
        }
        
        /* 下电前预处理 */
        case EN_BMS_RUNSTATE_OFF_PRE_PROCESS:
        {
            _RunStepPreOffProcess(&_UpLoadStatus);
            break;
        }
        
        /* 下电 */
        case EN_BMS_RUNSTATE_OFF:
        {
            _gBMSSysInfo.bms_run_flag = 1;
            uint8_t status = _RunStepOffProcess();
            //下电条件判断
            if(status == ENTER_OFF_STATUS)
            {   //符合下电条件，关机
                _UpLoadStatus = EN_BMS_RUNSTATE_OFF_OVER;
            }
            else if(status == ENTER_RESTART_STATUS)
            {   //符合重新上电条件（P端电压较高且稳定），进行再次开机
                _UpLoadStatus = EN_BMS_RUNSTATE_RESTART;
            }
            break;
        }
        /* 重新启动（关机过程中使用） */
        case EN_BMS_RUNSTATE_RESTART:
        {
            _ReStartInit();
//            LOG_DEBUG(LOG_TAG, "BMS ReStart!!!\r\n");
            //重新进入开机流程
            _UpLoadStatus = EN_BMS_RUNSTATE_STANDBY;
            break;
        }
        
        /* 下电完成 */
        case EN_BMS_RUNSTATE_OFF_OVER:
        {
//            LOG_DEBUG(LOG_TAG, "BMS All MOS Off!!!\r\n");
            OVCEnClose();       //OVC失能
            PowerHoldOff(); 
            break;
        }
        /* 故障 */
        case EN_BMS_RUNSTATE_ERROR:
        {
            is_error_flag = true;
            CloseDisChargeMos();
            ClosePreChargeMos();
            CloseChargeMos();
            CCEnOff();
            OVCEnClose();       //OVC失能
            _gBMSSysInfo.bms_run_flag = 2;
            gPackStatus.cc_status = EN_CC_STATUS_CLOSE;
            break;
        }
        default:
            break;
    }
}


EN_BMS_RUNSTATE GetBMSState(void)
{
    return _UpLoadStatus;
}


/**
 * @brief  : 关机命令处理
 * @para   : None
 * @note   : None
 * @return : 
 */
void _ShutdownCmdProcess()
{
    //取高八位，寄存器设置的数值
    _gBMSSysInfo.bms_shutdown &= 0X00FF;
    _gBMSSysInfo.bms_shutdown |= (garrModbusWriteReadReg[0].bms_shutdown & 0XFF00);
    
}

/**
 * @brief  : 根据写读寄存器的更新进行相应操作
 * @para   : None
 * @note   : None
 * @return : 
 */
void ModbusWriteReadRegProcess(uint16_t target_addr)
{
    //--------MOS开关----------
    switch(target_addr)
    {
        case MODBUS_WRITE_READ_REG_DSG_MOS_CTRL:
        {
            if(garrModbusWriteReadReg[0].dsg_mos_ctl == 0)
            {
                CloseDisChargeMos();  
            }
            else
            {
                if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl == 2)
                {
                    OpenPreChargeMos();
                    soft_delayms(1);
                    OpenDisChargeMos();//开放电MOS
                    soft_delayms(1);
                    ClosePreChargeMos();
//                    soft_delayms(10);
//                    OpenChargeMos();//开充电MOS
                }
                else
                {
                    OpenDisChargeMos();
                }
            }
            break;
        }
        
        case MODBUS_WRITE_READ_REG_CHG_MOS_CTRL:
        {
            if(garrModbusWriteReadReg[0].chg_mos_ctl == 0)
            {
                CloseChargeMos();
            }
            else
            {
                OpenChargeMos();
            }
            break;
        }
        
        case MODBUS_WRITE_READ_REG_PRE_MOS_CTRL:
        {
            if(garrModbusWriteReadReg[0].pre_mos_ctl == 0)
            {
                ClosePreChargeMos();
            }
            else
            {
                OpenPreChargeMos();
            }
            break;
        }
        //--------限流测试----------
        case MODBUS_TEST_MODE_WRITE_READ_REG_CC_CTRL:
        {
            if(garrModbusWriteReadReg[0].limit_curr_ctl == 0)
            {
                CCEnOff();
            }
            else
            {
                CCEnOn();
            }
            break;
        }
        
        //--------开关机处理----------
        case MODBUS_TEST_MODE_WRITE_READ_REG_SHUTDOWN_CTRL:
        {
            _ShutdownCmdProcess();
            break;
        }
        
        //--------进入测试模式----------
        case MODBUS_TEST_MODE_WRITE_READ_REG_ENTER_TEST_MODE:
        {
            ToolingInit();                              //测试模式标志位

            OpenDisChargeMos();//开放电MOS
            CCEnOff();
            ClosePreChargeMos();
            soft_delayms(15);
            OpenChargeMos();//开充电MOS
//            _UpLoadStatus = EN_BMS_RUNSTATE_NORMAL;
            _UpLoadStatus = EN_BMS_RUNSTATE_STANDBY;
            break;
        }
        //--------RTC时钟设置----------
        case MODBUS_WRITE_READ_REG_RTC_TIMES_HIGH:
        {
            uint32_t rtc_time = (uint32_t)(garrModbusWriteReadReg[0].rtc_times_high << 16) | garrModbusWriteReadReg[0].rtc_times_low;
            RtcSetTime(rtc_time);
            break;
        }
        //--------电流校准----------
        case MODBUS_WRITE_READ_REG_I_CALIBRATION:
        {
            int32_t current_calibration_value = 0;
            // 电流值接收
            current_calibration_value = ((uint32_t)garrModbusWriteReadReg[0].current_calibrat_high << 16) | garrModbusWriteReadReg[0].current_calibrat_low; /* 单位0.1V */
            current_calibration_value *= 100;          /* 单位1mV */
            //电流校准系数计算
            if(current_calibration_value > PACK_CURRENT_MIN_THRESHOLD)
            {
                gPackStatus.dsg_pack_current_calibration_factor = (float)current_calibration_value / gPackStatus.pack_current_real;
            }
            else
            {
                gPackStatus.chg_pack_current_calibration_factor = (float)current_calibration_value / gPackStatus.pack_current_real;
                
            }
            //上下限限制
            if(gPackStatus.dsg_pack_current_calibration_factor < 0.5f || gPackStatus.dsg_pack_current_calibration_factor > 1.5f)
            {
                gPackStatus.dsg_pack_current_calibration_factor = 1;
            }
            if(gPackStatus.chg_pack_current_calibration_factor < 0.5f || gPackStatus.chg_pack_current_calibration_factor > 1.5f)
            {
                gPackStatus.chg_pack_current_calibration_factor = 1;
            }
            //电流校准因子Flash保存
            CurrentCalibrationFactorSave();
            break;
        }
        //------------------加热膜------------------
        case MODBUS_WRITE_READ_REG_HEAT_CTRL:
        {
            if(garrModbusWriteReadReg[0].heat_ctl == 0)
            {
                gstModbusToolTestOnlyReadReg.heat_status = HeatClose();
            }
            else
            {
                gstModbusToolTestOnlyReadReg.heat_status = HeatOpen();
            }
            //将当前加热膜使能引脚状态放入寄存器中
            garrModbusWriteReadReg[0].heat_ctl = GpioBitGet((&HeatEnHandle));
            break;
        }
        //------------------DSP唤醒状态------------------
        case MODBUS_WRITE_READ_REG_DSP_WAKEUP_STATUS:
        {
            _gBMSSysInfo.dsp_status_flag = garrModbusWriteReadReg[0].dsp_wakeup_status;
            break;
        }
        //------------------升级状态------------------
        case MODBUS_WRITE_READ_REG_UPGRADE_STATUS:
        {
            //重启标志
            static uint8_t reset_flag = 0;
            
            //重启标志立起后不允许消除，只重置重发标志，允许再次设置重启命令
            if(garrModbusWriteReadReg[0].sys_up_status == 0)
            {
                reset_flag = 0;
            }
            if(reset_flag)
            {
                NOVA_SET_BIT(garrModbusWriteReadReg[0].sys_up_status, 1);
            }
            
            //复位标志（BIT1）已置起
            if(NOVA_GET_BIT(garrModbusWriteReadReg[0].sys_up_status, 1) != 0)
            {
                //设置重启命令（只执行一次）(除最后一个Pack外均进行)
                if((gUartPackInfo.is_final_bms == false) && (reset_flag == 0))
                {
                    ResetCmdSet();
                }
                //记录重启时间点，从此刻开始重启延时（只有最后一个Pack执行）
                if((gUartPackInfo.is_final_bms == true) && (reset_flag == 0))
                {
                    ResetFlagSet();
                }
                
                reset_flag = 1;
                //不允许写测试位（BIT2）
                garrModbusWriteReadReg[0].sys_up_status &= (~(0X01 << 2));
            }
            break;
        }
        default:
            break;
    }
}


/**
 * @brief  : 关机状态P端电压检测
 * @para   : None
 * @note   : None
 * @return : 
 */
uint8_t HoldOffEnCheck()
{
    static int32_t      arr_vol[20] = {50000,50000,50000,50000,50000};  //P端电压数组(赋初值，防止误判)
    static int32_t      arr_e_vol[20];  //P端电压微分数组
    static  uint32_t    time            = 0;
    int32_t             e_vol           = 50000;
    
    //250ms检测一次
    if(tim_difftime_from_now(time) < 250)
    {
        return HOLD_NOW_STATUS;
    }
    time = tim_get_systime();
    
    //整体前移一项
    memcpy(arr_vol, &arr_vol[1], 19*4);
    //将实时电压加入数组末尾;
    arr_vol[19] = OVCVoltageGet(); //P端时实电压
    
    //通讯超时的情况下，直接关机
    if(gUartPackInfo.com_out_flag == 1)
    {
        //没有通讯，关机
        return ENTER_OFF_STATUS;
    }
    
    //5S内检测到的P端电压全低于35V，允许掉电
    for(int i=0; i<20; i++)
    {
        if(arr_vol[i] > 35000)
        {
            break;
        }
        if(i == 19)
        {
            return ENTER_OFF_STATUS;
        }
    }
    
    //连续4次检测到P端电压低于20V，允许关机
    for(int i=0; i < 4; i++)
    {
        if(arr_vol[19 - i] > 20000)
        {
            break;
        }
        if(i == 3)
        {
            return ENTER_OFF_STATUS;
        }
    }
    
    return HOLD_NOW_STATUS;
}

/**
 * @brief  : 运行状态下电处理
 * @para   : None
 * @note   : None
 * @return : 
 */
uint8_t _RunStepOffProcess()
{
    static uint32_t delay_time;
    //第一次进入下电流程，关闭MOS
    if(_gBMSSysInfo.first_enter_off_flag == true)
    {
        _gBMSSysInfo.first_enter_off_flag = false;
        delay_time = tim_get_systime();
        /*错误数据保存*/
        SaveErrorCode();
        /* 保存当前SOC数据 */
        SoxInfoSave(&gPackStatus);
        ClosePreChargeMos();
        CloseDisChargeMos();
        CloseChargeMos();
        CCEnOff();
        gPackStatus.cc_status = EN_CC_STATUS_CLOSE;
        ReleaseOn();        //打开泄放电路
        LedCtlBlink(LED_SRC_SYS, LED_BLINK_TIME_CLOSE);
        ShutdownRegSet(BMS_OFF_STATUS); //设置关机标志
    }
    
    //不止一个Pack且是第一个Pack
    if((gUartPackInfo.ModbusAddr == MODBUS_ADDR_PACK0) && (gUartPackInfo.bms_num > 1))
    {   //循环查询并发送关机指令
        _FirstPackCheckShutDownFunc();
    }
    
    
    if(tim_difftime_from_now(delay_time) > 10000)
    {
        //检测P端电压，电压泄放至一定阈值后再关机
        if(HoldOffEnCheck() == ENTER_OFF_STATUS)
        {
            return ENTER_OFF_STATUS;
        }
        else
        {
            //不符合下电条件，判断是否有按键进行再次开机
            if(_ReStartSignalJudge() == true)
            {
                return ENTER_RESTART_STATUS;
            }
        } 
    }
    return HOLD_NOW_STATUS;
}    




/**
 * @brief  : 加热膜处理
 * @para   :
 * @note   : None
 * @return : None
 */
void HeatProcess()
{
    //工装测试模式下，该函数不运行
    if(gstModbusToolTestWriteReadReg.test_mode_enter_ctrl != 0)
    {
        return;
    }
    //小于5度打开，大于15度关闭
    if(gPackStatus.pack_min_temperature < 50)
    {
        gstModbusToolTestOnlyReadReg.heat_status = HeatOpen();
    }
    else if((gPackStatus.pack_min_temperature > 150) && (GpioBitGet((&HeatCheckHandle))  == RESET))
    {
        gstModbusToolTestOnlyReadReg.heat_status = HeatClose();
    } 
}


/**
* @brief  : 重启标志设置
* @para    : None
 * @note   : None
 * @return : None
 */
void ResetFlagSet()
{
    //设置重启标志及重启时间
    _gBMSSysInfo.bms_reset_flag = true;
     _gBMSSysInfo.sys_reset_delay_times = tim_get_systime();
}

/**
 * @brief  : 自动重启处理
 * @para   :
 * @note   : None
 * @return : None
 */
void SysResetProcess()
{
    static bool process_flag = 1;       //重启处理标志
    
    //一个电池包不允许重启
    if(((gUartPackInfo.ModbusAddr == MODBUS_ADDR_PACK0) || (gUartPackInfo.ModbusAddr == MODBUS_ADDR_DEFAULT_VALUE))\
        && (gUartPackInfo.bms_num == 1))
    {
        return;
    }
    
    //重启标志未置起，返回
    if(_gBMSSysInfo.bms_reset_flag == false)
    {
        return;
    }
    
    //重启预处理
    if(process_flag)
    {
        /*错误数据保存*/
        SaveErrorCode();
        /* 保存当前SOC数据 */
        SoxInfoSave(&gPackStatus);
        ClosePreChargeMos();
        CloseDisChargeMos();
        CloseChargeMos();
        CCEnOff();
        process_flag = 0;
    }    
    
    if(tim_difftime_from_now(_gBMSSysInfo.sys_reset_delay_times) > 5000)
    {
        __set_PRIMASK(1);   // 关闭总中断
        NVIC_SystemReset();
    }

}



/* 欠压启动处理 */
void BmsUvStartProcess()
{
    _UpLoadStatus = EN_BMS_RUNSTATE_STANDBY;
}









