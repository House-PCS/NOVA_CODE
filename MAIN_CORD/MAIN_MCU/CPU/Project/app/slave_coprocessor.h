#ifndef __SLAVE_COPROCESSOR_H_
#define __SLAVE_COPROCESSOR_H_

#include "dev_modbus.h"
#include "bsp.h"
#include <stdio.h>

#define SLAVE_COPROCESSOR_ADDR                  (0x01)

#define READ_COPRO_INPUT_TYPE                   (0x04)
#define READ_COPRO_HOLD_TYPE                    (0x03)
#define WRITE_COPRO_HOLD_TYPE                   (0x10)

#define CORPRO_WRITE_QUEUE_DEPTH                (4)

#define START_ATMOS_LED_REG_ADDR                (0xA000)
#define SYS_ATMOS_LED_REG_ADDR                  (0xA001)
#define COLLECTOR_ATMOS_LED_REG_ADDR            (0xA002)
#define PCS_ATMOS_LED_REG_ADDR                  (0xA003)
#define CONFIRM_KEY_ATMOS_LED_REG_ADDR          (0xA004)
#define ESC_KEY_ATMOS_LED_REG_ADDR              (0xA005)
#define UP_KEY_ATMOS_LED_REG_ADDR               (0xA006)
#define DOEN_KEY_ATMOS_LED_REG_ADDR             (0xA007)
#define MOOD_LED_REG_ADDR                       (0xA008)    


#define MAIN_COP_BASE_INFO_REG_ADDR             (0x5800)
#define MAIN_COP_RUNSTAT_INFO_REG_ADDR          (0x5820)
#define MAIN_COP_UPGRADE_REG_ADDR               (0x5830)

#define MAIN_COP_CONFIRM_KEY_REG_ADDR           (0x5835)
#define MAIN_COP_ESC_KEY_REG_ADDR               (0x5836)
#define MAIN_COP_UP_KEY_REG_ADDR                (0x5837)
#define MAIN_COP_DOWN_KEY_REG_ADDR              (0x5838)

#define MAIN_COP_LED_MODE_REG_ADDR              (0x5839)
#define MAIN_COP_DISP_SOC_REG_ADDR              (0x583D)

typedef enum
{
    EN_READ_COPRO_BASE_INFO_EVENT,
    EN_READ_COPRO_RUN_STAT_EVENT,
    EN_READ_COPRO_CTRL_INFO_EVENT,
    
//    EN_READ_COPRO_START_LED_EVENT,      //开机灯效
//    EN_READ_COPRO_SYS_LED_EVENT,        //系统灯效
//    EN_READ_COPRO_COLLECTOR_LED_EVENT,  //采集器灯效
//    EN_READ_COPRO_PCS_LED_EVENT,        //PCS灯效
//    EN_READ_COPRO_CONFIRM_KEY_LED_EVENT,   //确认按键灯效
//    EN_READ_COPRO_ESC_KEY_LED_EVENT,       //返回按键灯效
//    EN_READ_COPRO_UP_KEY_LED_EVENT,     //上按键器灯效
//    EN_READ_COPRO_DOWN_KEY_LED_EVENT,   //下按键器灯效
    EN_READ_COPRO_EVENT_NUM,
} EN_COPRO_PARA_READ_EVENT;



typedef enum
{
    EN_COPRO_POLLING_STATUS_IDLE = 0,
    EN_COPRO_POLLING_STATUS_WAITING_REV,
    EN_COPRO_POLLING_STATUS_REV_OVERTIME,
    EN_COPRO_POLLING_STATUS_REV_INTERVAL_WAIT,
} EN_COPRO_POLLING_STATUS;

#pragma pack(2)

typedef struct _copro_hold_reg_para_stru_t
{
    uint16_t start_staus;               //A000
    uint16_t sys_status;                //A001
    uint16_t collector_status;
    uint16_t pcs_status;
    uint16_t confirm_key_status;
    uint16_t esc_key_status;
    uint16_t up_key_status;
    uint16_t down_key_status;           //A007
} CoproHoldPara_t;


/*********************监控卡协处理器信息************************/
/*----监控卡协处理器基础信息---*/
typedef struct
{
    uint16_t    model_id;               //5800
    uint16_t    sn[10];                 //5801~580A
    uint16_t    hw_ver;                 //580B
    uint16_t    sw_ver[2];              //580C~580D
    uint16_t    sw_ver_ext[12];         //580E~5819
    uint16_t    reserve[6];             //581A~581F
} MainCardCopBaseInfo_t;

/*----监控卡协处理器状态信息---*/
typedef struct
{
    uint16_t    run_status;             //5820
    uint16_t    reserve[15];            //5821~582F
} MainCardCopStatusInfo_t;

/*----监控卡协处理器控制信息---*/
typedef struct
{
    uint16_t    upgrade_status;         //5830
    uint16_t    start_staus;            //5831
    uint16_t    sys_status;             //5832
    uint16_t    collector_status;       //5833
    uint16_t    pcs_status;             //5834
    uint16_t    confirm_key_status;     //5835
    uint16_t    esc_key_status;         //5836
    uint16_t    up_key_status;          //5837
    uint16_t    down_key_status;        //5838
    uint16_t    led_dir_r;              //5839
    uint16_t    led_g_b;                //583A
    uint16_t    led_mode;               //583B
    uint16_t    led_period;             //583C
    uint16_t    pack_disp_soc;          //583D
    uint16_t    reserve[18];            //583E~584F
} MainCardCopCtrlInfo_t;

/*----监控卡协处理器信息---*/
typedef struct
{
    MainCardCopBaseInfo_t    base_info;   
    MainCardCopStatusInfo_t  status_info; 
    MainCardCopCtrlInfo_t    ctrl_info;   
} MainCardCopInfo_t;


typedef struct _copro_para_stru_t
{
    CoproHoldPara_t CoproHoldData;
    uint8_t         copro_comm_err_cnt;
    uint8_t         copro_comm_time_out;
    uint8_t         copro_busy_flag;            //协处理器串口忙碌标志
} CoprocessorPara_t;

#pragma pack()

extern MainCardCopInfo_t   gstMainCardCopInfo;

void SlaveCoproModbusCommProcess(void);
void SlaveCorproCommInit(void);

bool SetStartLedStaInfo(uint16_t status);
bool SetSysLedStaInfo(uint16_t status);
bool SetCollectorLedStaInfo(uint16_t status);
bool SetPCSLedStaInfo(uint16_t status);
bool SetConfirmLedStaInfo(uint16_t status);
bool SetESCLedStaInfo(uint16_t status);
bool SetUpKeyLedStaInfo(uint16_t status);
bool SetDownKeyLedStaInfo(uint16_t status);
bool SetMoodLedStaInfo(uint32_t clour_value,uint16_t mode,uint16_t period);
bool SetMainCopSysUpgradeStatus(uint16_t status);
void KeyLedProcess(void);
void SendMainCopUpgradeData(uint16_t pack_id, uint16_t data_len, uint8_t *pdata);

bool SetPackSocInfo(uint16_t pack_soc);
#endif


