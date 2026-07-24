
#ifndef __BMS_PROTOCOL_H_
#define __BMS_PROTOCOL_H_

#include "bsp.h"



#define BMS_ON_STATUS         (0) //开机
#define BMS_OFF_ING_STATUS    (1) //关机中
#define BMS_OFF_STATUS        (2) //关机
#define MONITOR_OFF_CMD       (1) //关机命令

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

typedef enum
{
    EN_MPPT_RUNSTATE_CHECK = 0,      //MPPT启动方式检查
    EN_MPPT_RUNSTATE_EXTERNAL_START, //外部启动
    EN_MPPT_RUNSTATE_INTERNAL_START, //内部启动
    EN_MPPT_RUNSTATE_EXTERNAL_BUSY, //外部启动中
    EN_MPPT_RUNSTATE_NORMAL,        //待机
    EN_MPPT_RUNSTATE_SET_FIRST_POWER, //设置充电电压  功率 电流
    EN_MPPT_RUNSTATE_SET_FIRST_POWER_ACK,
    EN_MPPT_RUNSTATE_IDLE,
    EN_MPPT_RUNSTATE_FULL_CHARGE,//满充
    EN_MPPT_RUNSTATE_ERROR,

    EN_MPPT_RUNSTATE_NUM = 0xff,
} EN_MPPT_RUNSTATE;

/* BMS 故障类别 */
typedef enum
{
    EN_TINY_FAULT = 0,                  //系统无故障或者仅有轻微故障，只记录故障码，系统性能不受影响。
    EN_RESERVED1,                       //
    EN_RESERVED2,                       //
    EN_PERFORMANCE_LIMITED1,            //按电芯不同SOC与温度进行限制功率
    EN_RESERVED3,                       //
    EN_PERFORMANCE_LIMITED2,            //放电：1min内当前功率线性降至200W;充电：限制到当前查表电流75%
    EN_OPEN_REQUEST1,                   //1. 1min内当前功率线性降至0kw，BMS等待MCU shutdown命令 ，收到指令或故障产生后90s ，BMS走正常下电流程（主回路电流降到5A以下，再切断MOS）。
    //2. 充电的时候立即将功率限到0kw，BMS等待MCU shutdown命令 ，收到指令或故障产生后90s ，BMS需要走正常下电流程（主回路电流降到5A以下，再切MOS）。
    EN_OPEN_REQUEST2,                   //立即将功率限到0kw，BMS等待MOS shutdown ，收到指令或1s后BMS走正常下电流程（主回路电流降到5A以下，再切断MOS）。
    EN_IMMEDIATE_OPEN_REQUEST,          //立即将功率限到0kw并自行立即断开MOS
    EN_ERROR_CATEGORY = 0XFF,
} EN_BMS_ERROR_CATEGORY;

/* BMS 系统故障等级 */
typedef enum
{
    EN_SYSTEM_ERROR_LEVEL0 = 0,
    EN_SYSTEM_ERROR_LEVEL1,
    EN_SYSTEM_ERROR_LEVEL2,
    EN_SYSTEM_ERROR_LEVEL3,
    EN_SYSTEM_ERROR = 0XFF,
} EN_BMS_SYSTEM_ERROR;

typedef enum
{
    EN_DSP_STATUS_NONE = 0,
    EN_DSP_STATUS_WAKEUP_ING,
    EN_DSP_STATUS_WAKEUP_FINISH,
} EN_DSP_STATUS;

#pragma pack(1)
typedef struct _bms_system_info_stru_t
{
    uint8_t                 precharge_state;            //0-断开 1-打开
    uint8_t                 charge_state;               //0-断开 1-打开
    uint8_t                 discharge_state;            //0-断开 1-打开
    uint16_t                bms_shutdown;
    uint8_t                 bms_sys_fail_sta;           //故障状态等级 1-故障1级 2-故障2级 3故障3级
    uint32_t                bms_bat2_drv_train_enb;
    uint32_t                shutdown_delay_times;
    
    bool                    bms_uv_pv_start_flag;       //PV启机
    uint8_t                 force_chg_allow_flag;       //强充标志
    uint8_t                 uv_hold_flag;               //欠压保持标志
    
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
    uint8_t                 endurance_mode_flag;         //耐久度测试标志（单板老化） 0：等待，1：进入，2：禁止进入
    
    bool                    first_enter_off_flag;        //首次进入关机状态标志
} BMSSysInfo_t;

#pragma pack()

extern BMSSysInfo_t   _gBMSSysInfo; //BMS当前状态

void BMSRunStateProcess(void);
void RecoverErrorCode(void);


uint8_t GetSystemErrorLevel(void);
void BMSBeepAlarm(void);
void BMSMPPTBeepAlarmClear(void);
void SaveErrorCode(void);
void ModbusWriteReadRegProcess(uint16_t target_addr);

/* bms状态信息初始化 */
void BmsStatusInfoInit(void);
/* 开始按键处理函数 */
void StartKeyProcess(void);
/* 加热膜处理 */
void HeatProcess(void);
/* 系统重启标志设置 */
void ResetFlagSet(void);
/* 系统重启处理 */
void SysResetProcess(void);
/* 获取系统运行状态 */
EN_BMS_RUNSTATE GetBMSState(void);
/* 欠压启动处理 */
void BmsUvStartProcess(void);
#endif














