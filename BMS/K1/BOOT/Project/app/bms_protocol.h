
#ifndef __BMS_PROTOCOL_H_
#define __BMS_PROTOCOL_H_

#include "bsp.h"
#include "systick.h"
#include "fault_analysis.h"
//#include "dev_charge_control.h"
#include "modbus_analysis.h"
//#include "dev_74hc595.h"
#include "sensor_control.h"
#include "dev_spiflash.h"
#include "rtc.h"
#include "dev_at24cxx.h"
//#include "can_protocol.h"
//#include "slave_ctl.h"
//#include "dev_beep.h"
#include "dev_mos.h"
#include "sox.h"

#define FWDGT_CNT_HEADER        (0X5A)
/***** 看门狗存储地址 *****/
#define FWDGT_SPI_FLASH_ADDR    (0x00130000)

typedef enum
{
    EN_BMS_RUNSTATE_DISABLE = 0,            //无效状态
    EN_BMS_RUNSTATE_RUN_SIGNAL_WAIT,        //等待允许运行信号
    EN_BMS_RUNSTATE_STANDBY,                //待机
    EN_BMS_RUNSTATE_POWER_HOLD,             //高压
    EN_BMS_RUNSTATE_NORMAL_PRE_PROCESS,     //正常运行前预处理
    EN_BMS_RUNSTATE_NORMAL,                 //正常运行
    EN_BMS_RUNSTATE_OFF_PRE_PROCESS,        //下电前预处理
    EN_BMS_RUNSTATE_OFF,                    //下电
    EN_BMS_RUNSTATE_ERROR,                  //错误
    EN_BMS_RUNSTATE_OFF_OVER,               //下电完成
    EN_BMS_RUNSTATE_RESTART,                //重启（下电过程中使用）
    EN_BMS_RUNSTATE_NUM = 0xff,
} EN_BMS_RUNSTATE;

#pragma pack(1)
typedef struct _bms_system_info_stru_t
{
    uint8_t                 precharge_state;            //0-断开 1-打开
    uint8_t                 charge_state;               //0-断开 1-打开
    uint8_t                 discharge_state;            //0-断开 1-打开
    uint8_t                 bms_shutdown;
    uint8_t                 bms_sys_fail_sta;           //故障状态等级 1-故障1级 2-故障2级 3故障3级
    uint32_t                bms_bat2_drv_train_enb;
    uint32_t                shutdown_delay_times;
    
    bool                    bms_uv_pv_start_flag;       //PV启机
    uint8_t                 force_chg_allow_flag;       //强充标志
    
    bool                    bms_upgrade_ing_flag;        //bms升级中标志
    uint8_t                 bms_reset_flag;              //bms重启标志
    uint32_t                sys_reset_delay_times;       //系统重启延迟时间
    
    uint32_t                start_out_time;              // 开机超时时间 
    uint8_t                 start_out_flag;              // 开机超时标志 
     
    EN_BMS_RUNSTATE         bms_run_status;              //bms运行状态
    uint16_t                bms_run_flag;                //bms运行标志  0：正常运行，1：关机，2：故障
    uint8_t                 long_key_start_flag;         //长按开机信号
    uint8_t                 long_key_end_flag;           //长按关机信号
    uint8_t                 dsp_status_flag;             //DSP状态信号
    
    bool                    first_enter_off_flag;        //首次进入关机状态标志
} BMSSysInfo_t;

typedef struct
{
    uint8_t                 header;             //数据头
    uint8_t                 fwdgt_cnt;          //看门狗触发次数计数
    uint8_t                 sumcheck;           //和校验
} FwdgtInfo_t;
#pragma pack()

extern BMSSysInfo_t   _gBMSSysInfo; //BMS当前状态

void ResetFlagSet(void);
void SysResetProcess(void);
uint8_t FwdgtFlagProcess(void);
void ClearFwdgtCnt(void);
#endif














