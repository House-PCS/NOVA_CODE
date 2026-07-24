/**
 ******************************************************************************
 * @file    shell_netif.h
 * @author  NovaStar R&D Team
 * @version V1.0.0
 * @date    2017.09.28
 * @brief
 ******************************************************************************
 * @attention
 *
 * History<br>
 * version     date        auther      description<br>
 * V1.0.0      2017.11.13  Huyuxin     Initial Version<br>
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AFE_CONTROL_H
#define __AFE_CONTROL_H

/* Inculdes ------------------------------------------------------------------*/

#include "dev_adbms6830.h"
#include "dev_sq52201.h"
#include "dev_ntc.h"
#include "debug.h"
#include "sox.h"
#include "filter_calc.h"


#ifndef AFE_NUM
#define AFE_NUM                     (1)      //单板芯片数量
#endif
#ifndef AFE_CELL_NUM
#define AFE_CELL_NUM                (16)      //单芯片电芯串数
#endif
#ifndef AFE_CELL_TEMP_NUM
#define AFE_CELL_TEMP_NUM           (13)      //单芯片电芯温度数
#endif
#ifndef AFE_BALANCE_TEMP_NUM
#define AFE_BALANCE_TEMP_NUM        (4)      //单芯片均衡温度数
#endif

#define CELL_VOLT_SAMPLE_MAX        (AFE_NUM * AFE_CELL_NUM)
//#define AFE_TEMP_NUM                (AFE_CELL_TEMP_NUM + AFE_BALANCE_TEMP_NUM)      //单芯片温度数
#define PACK_TEMP_SAMPLE_MAX        (AFE_CELL_TEMP_NUM * AFE_NUM)
#define BALANCE_TEMP_SAMPLE_MAX     (AFE_BALANCE_TEMP_NUM * AFE_NUM)

#define BALANCE_OPEN_V_DIFF_VAL         (20)

//#define BALANCE_CNT                     (36000)      //均衡时间3小时
#define BALANCE_CNT                     (21600)      //均衡时间3小时

#define CELL_VOLT_FILTER_WIN            20//电压滤波窗口

/* Exported typedef -----------------------------------------------------------*/
/******AFE检测状态机******/
typedef enum
{
    EN_AFE_CTL_IDLE,
    EN_AFE_CTL_V_GET,//电压
    EN_AFE_CTL_T_GET,//温度
    EN_AFE_CTL_BT_GET,//均衡温度
    EN_AFE_CTL_B_SET,//均衡
    EN_AFE_CTL_B_STOP,//停止均衡
    EN_AFE_CTL_OW_DET,//断线检测
    EN_AFE_CTL_GAP,//AFE控制空档期
    EN_AFE_CTL_NEED_RESET,//芯片需要复位
    EN_AFE_CTL_NUM
} EN_AFE_CTL_FUNC;

typedef enum
{
    EN_AFE_STATUS_IDLE,
    EN_AFE_STATUS_V_GET,//电压
    EN_AFE_STATUS_T_GET,//温度
    EN_AFE_STATUS_BT_GET,//均衡温度
    EN_AFE_STATUS_B_SET,//均衡
    EN_AFE_STATUS_B_STOP,//停止均衡
    EN_AFE_STATUS_OW_DET,//断线检测
    EN_AFE_STATUS_GAP,//AFE控制空档期
    EN_AFE_STATUS_NEED_RESET,//芯片需要复位
    EN_AFE_STATUS_NUM
} EN_AFE_STATUS;

/******检测结果******/
typedef enum
{
    EN_AFE_OP_RES_ING,
    EN_AFE_OP_RES_COMPLETE,
    EN_AFE_OP_RES_ERROR,
} EN_AFE_OP_RES;

/******奇偶断线检测状态******/
//typedef enum
//{
//    EN_AFE_OW_STA_CMD_EVEN,
//    EN_AFE_OW_STA_WAIT_EVEN,
//    EN_AFE_OW_STA_CMD_ODD,
//    EN_AFE_OW_STA_WAIT_ODD,
//} EN_AFE_OW_STA;

typedef enum
{
    EN_AFE_OW_STA_CMD_REF,
    EN_AFE_OW_STA_WAIT_REF,
    EN_AFE_OW_STA_CMD_EVEN,
    EN_AFE_OW_STA_WAIT_EVEN,
    EN_AFE_OW_STA_CMD_ODD,
    EN_AFE_OW_STA_WAIT_ODD,
    EN_AFE_OW_STA_RESULT,
} EN_AFE_OW_STA;

/******电压检测状态******/
typedef enum
{
    EN_AFE_V_STA_CMD,
    EN_AFE_V_STA_WAIT,
    EN_AFE_V_STA_COMPUTE,
    EN_AFE_V_STA_END,
} EN_AFE_V_STA;

/******电压检测状态******/
typedef enum
{
    EN_AFE_T_STA_T_1,
    EN_AFE_T_STA_T_2,
    EN_AFE_T_STA_T_3,
    EN_AFE_T_STA_T_4,
    EN_AFE_T_STA_T_COMPUTE,
} EN_AFE_T_STA;

/******奇偶断线检测状态******/
typedef enum
{
    EN_AFE_BALANCE_STA_CHECK,
    EN_AFE_BALANCE_STA_OPEN,
    EN_AFE_BALANCE_STA_WAIT,
    EN_AFE_BALANCE_STA_CLOSE,
    EN_AFE_BALANCE_STA_END
} EN_AFE_BALANCE_STA;

typedef struct _Cell_handle_stru_t
{
    uint32_t    total_cell_voltage;
    uint16_t    cell_voltage[20];
    uint16_t    cell_max_voltage;
    uint16_t    cell_min_voltage;
    uint16_t    cell_ave_voltage;
    uint16_t    cell_diff_voltage;
    int32_t     cell_current;
    int16_t     arr_cell_temperature[20];
    int16_t     cell_max_temperature;       /* 0.1c */
    int16_t     cell_min_temperature;
    int16_t     cell_ave_temperature;
    int16_t     afe_ntc_temperature;
    int16_t     afe_ambient_temperature;
    int16_t     mos_temperature;
    uint8_t     current_status;     /* 0-idle 1:dsg 2:chg */
    uint8_t     balance_open_status;    /* 0-close 1:open */
    uint32_t    bat_init_vol;
    uint16_t    cur_ref;
    uint16_t    cell_max_vol_id;   /* 最高节电芯编号 */
    uint16_t    cell_min_vol_id;   /* 最低节电芯编号 */
} Cell_Info;

typedef struct _afe_sample_info_stru_t
{
    uint16_t        cell_voltage[CELL_VOLT_FILTER_WIN][AFE_NUM][AFE_CELL_NUM];//滤波前的电压值，包含历史数据
    uint16_t        cell_voltage_filter[AFE_NUM][AFE_CELL_NUM];//滤波后的电压值
    float           cell_temp[AFE_NUM][AFE_CELL_TEMP_NUM];
    float           balance_temp[AFE_NUM][AFE_BALANCE_TEMP_NUM];
    
    bool            balance_open_condition;//均衡开启条件，只有充电模式下才允许均衡
    uint8_t         balance_type;//0-本地均衡，1-主控控制均衡，2-不均衡
    uint16_t        balance_bcu_ctl[AFE_NUM];//在主控控制均衡模式下，由主控下发均衡通道，在本地均衡模式下，自行根据电压情况计算
    bool            balance_bcu_open;
    uint16_t        balance_stat_flag[AFE_NUM];//均衡状态，每个bit对应一个通道，1表示对应通道均衡开启
    uint16_t        cell_ow_val[AFE_NUM][AFE_CELL_NUM];//电芯断线数值(0.01)
    uint16_t        cell_ow_stat[AFE_NUM];//电芯断线状态，每个bit对应一个通道，1表示对应通道发生断线
    //uint16_t        cell_so_stat[AFE_NUM];//电芯短路状态，每个bit对应一个通道，1表示对应通道发生短路
    uint16_t        temp_ow_stat[AFE_NUM];//温度断线状态，每个bit对应一个通道，1表示对应通道发生断线
    //uint16_t        temp_so_stat[AFE_NUM];//温度短路状态，每个bit对应一个通道，1表示对应通道发生短路
    uint8_t         balance_temp_ow_stat[AFE_NUM];//均衡温度断线状态，每个bit对应一个通道，1表示对应通道发生断线
    
    //下面记录是否进行过至少一次的采样
    bool            volt_sample_flag;
    bool            temp_sample_flag;
    bool            balance_temp_sample_flag;
    bool            ow_sample_flag;
    
    //下面用于调试
    uint32_t        cell_v_error_num;
    uint32_t        cell_t_error_num;
    uint32_t        bt_error_num;
    uint32_t        balance_error_num;
    uint32_t        ow_error_num;
    //下面同样用于调试，用于记录不同功能的AFE操作所需时长
    uint32_t        cell_v_duration_time;
    uint32_t        cell_t_duration_time;
    uint32_t        bt_duration_time;
    uint32_t        balance_duration_time;
    uint32_t        ow_duration_time;
    
} AFESampleInfo_t;

typedef struct _afe_ctl_sche_stru_t
{
    EN_AFE_CTL_FUNC     ctl_type;
    uint32_t            min_interval;//最小间隔，单位ms
    uint32_t            inverval_count;//间隔计数
} AFECtlSche_t;

typedef struct
{
    uint16_t            last_b_status;              //记录的均衡状态，last_b_status = now_b_status；
    uint16_t            now_b_status;               //电芯的均衡状态，未分组1
    uint16_t            real_b_status;              //真实的电芯的均衡状态     real_b_status = (now_b_status * 均衡分组)
    uint16_t            disp_b_status;              //上传电芯的均衡状态
    uint32_t            stop_time;                  //记录采集电压时，停止均衡的时间点
    uint32_t            balance_cnt[AFE_CELL_NUM];  //均衡间隔计数
    uint32_t            b_accu_count[AFE_CELL_NUM]; //电压达到开启标准时，需要多次计数后才开启
} AFEBalanceStatus_t;

extern Cell_Info gCell_Info;
extern uint8_t gFirst_AFE_Coved_Flag;

void AFEProcess(void);
bool AFEFirstSample(void);
uint16_t GetNowBalanceStatus(void);
uint16_t GetAllBalanceStatus(void);
void GetBalanceCnt(uint16_t* p);
uint16_t GetAfeOwStatus(void);
uint16_t GetAfeOwMinVal(void);
#endif /*__MID_SHELL_H */

