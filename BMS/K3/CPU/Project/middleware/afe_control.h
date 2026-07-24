/**
******************************************************************************
* @copyright (C) COPYRIGHT  Nova
* @file        afe_control.h
* @author      Xi'an NovaStar Tech Co.,LTD.
* @version     V1.0.0
* @date       2024-08-20
* @brief
******************************************************************************
* @attention
* @relation
* @par :
* <table>
* <tr><th>Date          <th>Version   <th>Author      <th>Description
* <tr><td>2024-08-20    <td>V1.0.0    <td>Xingwen Xu  <td>InitialVersion
* <tr><td>2024-08-28    <td>V1.1.0    <td>Xingwen Xu  <td>re_implementate
* </table>
******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AFE_CONTROL_H
#define __AFE_CONTROL_H

/* Private Inculdes ------------------------------------------------------------------*/
#include "dev_bq76952.h"
#include "debug.h"
#include "bsp.h"
#include <stdio.h>
#include "sox.h"
/* Private define ------------------------------------------------------------------*/

#define CELL (16) 				    // numbe of tot cell by afe.
#define PACK_CELL (8)			    // cell number of pack
#define TEMPERATURE_SAMPLE_EN (0)   // 1: enable afe temperature sampling
#define HIGH_VOLTAGE_SAMPLE_EN (0)  // 1: enable high voltage sampling
#define AFE_CONTROL_MAX_REQUEST_SINGLE_LEN (1)
#define AFE_CONTROL_MAX_REQUEST_LOOP_LEN (10)
#define AFE_CONTROL_MAX_FILTER_LEN (30)
#define AFE_CONTROL_MAX_FILTER_EN  (0)


/* Exported typedef -----------------------------------------------------------*/

/* AFE state handle */

/*AFE module states*/
typedef enum
{
    EN_MODULE_UNINIT = 0u, /*! uninit */
    EN_MODULE_IDLE,
    EN_MODULE_WAKING_UP,
    EN_MODULE_INITIALIZING,
    EN_MODULE_CONTINOUS_MEASURE, /* ! Include cell and gpio voltage.*/
    EN_MODULE_READ_CELL_VOLTAGES,
    EN_MODULE_READ_GPIO_VOLTAGES,
    EN_MODULE_READ_STACK_VOLTAGES,
    EN_MODULE_READ_CURRENT,
    EN_MODULE_SET_CELL_BALANCING,
    EN_MODULE_DETECT_CELL_OPEN_WIRE,
    EN_MODULE_READ_CELL_BALANCE_STATE, /* ! The interval time between set and read the CBL must greater than 2ms.*/
    EN_MODULE_PERFORM_SOFT_RESET,
    EN_MODULE_LIVE_CONT_MEASURE,
    EN_MODULE_FAULT_HANDLE,
} AFE_EN_MODULE;

/* AFE request */
typedef enum
{
    EN_REQUEST_WAKEUP = 0u, /*! wakeup */
    EN_REQUEST_NO_REQ,      /*! no request */
    EN_REQUEST_INITIALIZE,  /*! initialize */
    EN_REQUEST_CONT_MEASURE,
    EN_REQUEST_READ_CELL_VOLTAGES,
    EN_REQUEST_READ_GPIO_VOLTAGES,
    EN_REQUEST_READ_STACK_VOLTAGES,
    EN_REQUEST_READ_CURR,
    EN_REQUEST_SET_CELL_BALANCING,
    EN_REQUEST_DETECT_CELL_OPEN_WIRE,
    EN_REQUEST_READ_CELL_BALANCE_STATE,
    EN_REQUEST_SOFT_RESET, /*! Soft Reset */
    EN_REQUEST_LIVE_CONT_MEASURE,
    EN_REQUEST_FAULT_HANDLE,
    EN_REQUEST_REQUEST_LENGTH,
} AFE_EN_REQUEST;

typedef enum
{
    STATUS_HANDLE_NO_ERR = 0u, /*! no error*/
    STATE_HANDLE_ERROR,        /*! error*/
} AFE_EN_REQUEST_ERROR;

typedef struct _Afe_drive_stru_t
{
    AFE_EN_MODULE eAfeModule;
    AFE_EN_REQUEST eAfeRequest;
    AFE_EN_REQUEST_ERROR eStsHandleStmcErr;
    bool StartupSequenceComplete;
    bool FullScanSts; /* true : scan complete. */
    uint8_t RequestQueueStartupCounter;
    uint8_t RequestQueueLoopCounter;
    uint16_t AfeLiveCnt;
} AfeCtrolDrv_t;

/* afe control cfg structure ---------------------------------------------------*/
typedef struct _afe_control_cfg
{
    /*! Array of requests executed once, during initialization */
    AFE_EN_REQUEST arrRequestQueueSingle[AFE_CONTROL_MAX_REQUEST_SINGLE_LEN];
    /*! Number of requests executed once, during initialization */
    uint8_t RequestQueueSingleLen;
    /*! Array of requests executed cyclically */
    AFE_EN_REQUEST arrRequestQueueLoop[AFE_CONTROL_MAX_REQUEST_LOOP_LEN];
    /*! Number of requests executed cyclically */
    uint8_t RequestQueueLoopLen;
} AfeCtrolCfg_t;

/* cell info structure define---------------------------------------------------*/
typedef struct _Cell_handle_stru_t
{
    uint32_t    total_cell_voltage;
    uint32_t    StackVol;
    uint32_t    LinkVol; 
    uint32_t    LDVol;    
    uint16_t    cell_voltage[16];
    uint16_t    cell_max_voltage;
    uint16_t    cell_min_voltage;
    uint16_t    cell_ave_voltage;
    uint16_t    cell_diff_voltage;
    uint8_t     CommFltCnt;    
    int32_t     cell_current;
    int16_t     cell_temperature[16];
    int16_t     cell_max_temperature;           /* 0.1c */
    int16_t     cell_min_temperature;
    int16_t     cell_ave_temperature;
    int16_t     afe_net_temperature; 
    int16_t     dmos_temperature;
    int16_t     cmos_temperature;
    int16_t     emos_temperature;
    int16_t     mos_max_temperature;
    int16_t     mos_min_temperature;
    int16_t     mos_ave_temperature;
    int16_t     pack_max_temperature;
    int16_t     pack_min_temperature;
    int16_t     pack_ave_temperature;    
    uint8_t     current_status;                 /* 0-idle 1:dsg 2:chg */
    uint8_t     balance_open_status[16];        /* 均衡开启状态标志 0-close 1:open */
    uint8_t     cell_bal_flag[16];              // 均衡触发标记 0=未触发 1=已触发
    uint32_t    cell_bal_timer[16];             // 均衡独立计时器(ms)
    uint32_t    cell_bal_max_time;              // 单节电芯最大均衡时间(ms)    
    uint32_t    bat_init_vol;
    uint16_t    cur_ref;
    int16_t     max_temperature_diff;    
    bool        arrCellBalSts[16];          /* 实际均衡开启状态0-close, 1:open */
	bool        arrCellBalCmd[16];          /* 0-close, 1:open */
    bool        CellOpenWire[16];               /* 0-normal, 1:open */ 
	bool        PowKeySts;    
} CellInfo_t;

typedef enum
{
    EN_AFE_IDLE = 0,    /*空闲*/
    EN_AFE_SET_B,
    EN_AFE_READ_B,    
    EN_AFE_CLEAR_B,
    EN_AFE_GET_C,       /*采集电流*/    
} EN_AFE_STATE;

typedef struct {
    float b0, b1, b2;
    float a0, a1, a2;
    float x[3]; // x(n), x(n-1), x(n-2)
    float y[3]; // y(n), y(n-1), y(n-2)
} LPF2_Float_T;

extern LPF2_Float_T lpf_float;
extern uint8_t gFirst_AFE_Coved_Flag;
extern CellInfo_t gstCellInfo;
void AFEInit(void);
void AFEProcess(void);
void TwosComponentS32(uint32_t input, int32_t* output);
void TwosComponentS16(uint16_t input, int32_t* output);
bool AFEFirstSample(void);
bool CurrentGet(void);
bool VotageGet(void);
bool SetBalance(void);
bool ClearBalance(void);
bool TemperatureGet_AFE(void);
void StopAFESample(void);
void StartAFESample(void);
int32_t sliding_window_filter(int32_t *buf, uint16_t size);
void VoltageSample(void);
void BalanceSample(void);
void OpenWireCheck(void);
void BalanceProcess(void);
#endif

