/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        afe_ctl.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-20
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-8-20     <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __AFE_CTL_H_
#define __AFE_CTL_H_

#include "dev_adbms6830.h"

#ifndef AFE_NUM
#define AFE_NUM     2      //单板芯片数量
#endif
#ifndef AFE_CELL_NUM
#define AFE_CELL_NUM     16      //单芯片电芯串数
#endif
#ifndef AFE_CELL_TEMP_NUM
#define AFE_CELL_TEMP_NUM     13      //单芯片电芯温度数
#endif
#ifndef AFE_BALANCE_TEMP_NUM
#define AFE_BALANCE_TEMP_NUM    2      //单芯片均衡温度数
#endif

#define CELL_VOLT_SAMPLE_MAX        (AFE_NUM * AFE_CELL_NUM)
//#define AFE_TEMP_NUM                (AFE_CELL_TEMP_NUM + AFE_BALANCE_TEMP_NUM)      //单芯片温度数
#define PACK_TEMP_SAMPLE_MAX        (AFE_CELL_TEMP_NUM * AFE_NUM)
#define BALANCE_TEMP_SAMPLE_MAX     (AFE_BALANCE_TEMP_NUM * AFE_NUM)



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
    EN_AFE_V_STA_CMD,
    EN_AFE_V_STA_WAIT,
} EN_AFE_V_STA;


typedef enum
{
    EN_AFE_T_STA_CH_SEL,
    EN_AFE_T_STA_CMD_1,
    EN_AFE_T_STA_WAIT_1,
    EN_AFE_T_STA_CMD_2,
    EN_AFE_T_STA_WAIT_2,
} EN_AFE_T_STA;


typedef enum
{
    EN_AFE_BT_STA_CMD_1,
    EN_AFE_BT_STA_WAIT_1,
    EN_AFE_BT_STA_CMD_2,
    EN_AFE_BT_STA_WAIT_2,
} EN_AFE_BT_STA;


typedef enum
{
    EN_AFE_BALANCE_STA_COND_JUDGE,
    EN_AFE_BALANCE_STA_START_CMD,
    EN_AFE_BALANCE_STA_STOP_CMD
} EN_AFE_BALANCE_STA;


typedef enum
{
    EN_AFE_OW_STA_CMD_EVEN,
    EN_AFE_OW_STA_WAIT_EVEN,
    EN_AFE_OW_STA_CMD_ODD,
    EN_AFE_OW_STA_WAIT_ODD,
} EN_AFE_OW_STA;




typedef enum
{
    EN_AFE_OP_RES_ING,
    EN_AFE_OP_RES_COMPLETE,
    EN_AFE_OP_RES_ERROR,
} EN_AFE_OP_RES;


typedef struct _afe_sample_info_stru_t
{
    uint16_t        cell_voltage[AFE_NUM][AFE_CELL_NUM];
    float           cell_temp[AFE_NUM][AFE_CELL_TEMP_NUM];
    float           balance_temp[AFE_NUM][AFE_BALANCE_TEMP_NUM];
    
    bool            balance_open_condition;//均衡开启条件，只有充电模式下才允许均衡
    uint8_t         balance_type;//0-本地均衡，1-主控控制均衡，2-不均衡
    uint16_t        balance_bcu_ctl[AFE_NUM];//在主控控制均衡模式下，由主控下发均衡通道，在本地均衡模式下，自行根据电压情况计算
    bool            balance_bcu_open;
    uint16_t        balance_stat_flag[AFE_NUM];//均衡状态，每个bit对应一个通道，1表示对应通道均衡开启
    
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



extern AFESampleInfo_t gAFESampleInfo;

void AFECtlProcess(void);

#endif








