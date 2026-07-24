/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __MODBUS_ANALYSIS_H_
#define __MODBUS_ANALYSIS_H_
#include "mid_comservice.h"

#define SLAVE_COMM_REV_MAX_TIME         300
#define SLAVE_COMM_INTERVAL_TIME        150
#define SLAVE_COMM_POLLING_TIME         150
#define SLAVE_COMM_BMS_INTERVAL_TIME    150

#define READ_INPUT_TYPE                 0x04
#define READ_HOLD_TYPE                  0x03
#define WRITE_HOLD_TYPE                 0x10


#define ARC_MODBUS_READ_START           0x0000
#define ARC_MODBUS_READ_END             0x00FF
#define ARC_MODBUS_HOLD_START           0X0100
#define ARC_MODBUS_HOLD_END             0X011F

#define MODBUS_WRITE_QUEUE_DEPTH        3

/*********************Host*************************/
typedef enum
{
    EN_ARC_READ_INPUT_EVENT,
    EN_ARC_READ_HOLD_EVENT,
    EN_READ_EVENT_NUM,
} EN_READ_EVENT;

typedef enum
{
    EN_MODBUS_SLAVE_COMM_STATUS_OK,
    EN_MODBUS_SLAVE_COMM_STATUS_NOACK,
    EN_MODBUS_SLAVE_COMM_STATUS_ACK_ERROR,
    EN_MODBUS_SLAVE_COMM_STATUS_DISORDER
} EN_MODBUS_SLAVE_COMM_STATUS;

typedef struct _modbus_rev_03_stru_t
{
    uint8_t             data_num;
    uint8_t             *pRev;
} ModbusRev03_t;

typedef struct _modbus_rev_10_stru_t
{
    uint16_t            reg_start_addr;
    uint16_t            reg_num;
} ModbusRev10_t;

typedef struct _modbus_rev_error_stru_t
{
    uint8_t             err_code;
} ModbusRevError_t;

typedef struct _modbus_rev_data_stru_t
{
    uint8_t             slave_addr;
    uint8_t             func_code;
    ModbusRev03_t       rev_03;
    ModbusRev10_t       rev_10;
    ModbusRevError_t    rev_error;
} ModbusRevData_t;

typedef enum
{
    EN_SLAVE_NO_ERROR = 0,
    EN_SLAVE_ERROR_ILLEGAL_FUNC_CODE = 1,
    EN_SLAVE_ERROR_ILLEGAL_REG_ADDR = 2,
    EN_SLAVE_ERROR_ILLEGAL_WRITE_DATA = 3,
    EN_SLAVE_ERROR_HITCH = 4
} EN_SLAVE_ERROR_CODE;

typedef struct _modbus_read_reg_stru_t    //小端数据
{
    uint16_t dsp_wake_up_over               ;
} ModbusInputReg_t;

typedef struct _arc_only_read_para_stru_t
{
    uint16_t    mode_id;                    //X0000            
    uint16_t    sn[10];                 
    uint16_t    hw_version;                  
    uint16_t    soft_version;  //
    uint16_t    arc_res_err_code;//
    uint16_t    reserve[1];
    uint16_t    channel_num;
    uint16_t    channle_curr_value[8];
    uint16_t    arc_alarm_info;     //0x0018  
    uint16_t    real_time_arc_alarm_info; //0x0019
    uint16_t    channel_arc_strength[8];
    uint16_t    channel_arc_history_max_value[8];
    uint16_t    arc_selftest_result;
    uint16_t    reserve1[5];
    uint16_t    channel_input_volt[8];
    uint16_t    channel_rx[8];
    uint16_t    channel_ry[8];
    uint16_t    impedance_alarm_info;           //0x0048
    uint16_t    inpedance_breakdown_info;       //0x0049 
} ArcOnlyReadPara_t;

typedef struct _arc_hold_para_stru_t
{
    uint16_t    slave_addr;                             //0X100
    uint16_t    comm_baud;
    uint16_t    arc_selftest_cmd;
    uint16_t    arc_alarm_threshold[8];
    uint16_t    arc_sensitivity;                        //0X10B
    uint16_t    arc_insulation_detect_en;               //0X10C
    uint16_t    sys_upgrade_status;                     //0X10D
    uint16_t    reserved[2];
    uint16_t    impedance_alarm_threshold_rx[8];
    uint16_t    impedance_alarm_threshold_ry[8];          
} ArcHoldPara_t;

typedef struct _arc_para_stru_t
{
    ArcOnlyReadPara_t    arc_only_read_para;
    ArcHoldPara_t        arc_hold_para;
} ArcPara_t;

typedef struct _slave_read_stru_t
{
    EN_READ_EVENT         e_event_index;
    EN_COMSERVICE_INDEX   e_sci_index;
    uint16_t              slave_addr;
    uint16_t              func_code;
    uint16_t              reg_addr;
    uint16_t              reg_num;
    uint16_t              data[30];
} SlaveRead_t;

typedef struct _slave_write_stru_t
{
    EN_COMSERVICE_INDEX e_sci_index;
    uint8_t             slave_addr;
    uint8_t             func_code;
    uint16_t            reg_addr;
    uint8_t             reg_num;
    uint8_t             data_num;
    uint16_t            data[256];
} SlaveWrite_t;

typedef struct _modbus_para_stru_t
{
    ArcPara_t                        arc_para;
    uint16_t                         pcs_comm_err_cnt;
    uint16_t                         arc_comm_err_cnt;
    uint16_t                         pcs_comm_time_out;
    uint16_t                         arc_comm_time_out;
} ModBusPara_t;

typedef struct  _deal_fram_stru_t
{
    EN_COMSERVICE_INDEX                      e_sci_index;
    uint16_t                                 slave_addr;
    EN_READ_EVENT                            envent;
    uint16_t                                 func_code;
    uint16_t                                 reg_addr;
    uint16_t                                 *pdata;
    uint16_t                                 reg_num;
    uint16_t                                 *ack_result;    //0xFF:无效 0x00:正确 0x01：错误
} ReadDealFram_t;

typedef enum
{
    EN_SLAVE_POLLING_STATUS_IDLE = 0,
    EN_SLAVE_POLLING_STATUS_WAITING_REV,
    EN_SLAVE_POLLING_STATUS_REV_OVERTIME,
    EN_SLAVE_POLLING_STATUS_WAIT_WRITE_ACK,
} EN_SLAVE_POLLING_STATUS;

typedef struct _modbus_ack_stru_t
{
    uint16_t                 read_arc_only_read;      // 0xFF 无效 0x00 成功      0x01 错误                        
    uint16_t                 read_arc_hold;           // 0xFF 无效 0x00 成功      0x01 错误   
    uint16_t                 read_memter_current;     // 0xFF 无效 0x00 成功      0x01 错误
    uint16_t                 write_arc_hold;          // 0xFF 无效 0x00 成功      0x01 错误   
} ModbusAckStatus_t;

void ModbusRegInit(void);
void BmsModBusHostWrite10(uint8_t slave_addr, uint16_t reg_start_addr, uint16_t reg_num, uint8_t data_num, uint16_t *pReg_value);
void ModbusProcess(void);
void ModbusHostAnalysis(SciRevData_t *p_com_cmd);
bool SetArcSysReset(void);

/*********************Slave*************************/
#define MOD_FUN_ERROR       (1)
#define MOD_REG_ADDR_ERROR  (2)
#define MOD_REG_DATA_ERROR  (3)
#define MOD_COMM_ERROR      (4)


typedef enum
{
    EN_MODBUS_RTU,
    EN_MODBUS_TCP
} EN_MODBUS_TYPE;

typedef enum
{
    EN_MODBUS_REV_STATUS_IDLE,
    EN_MODBUS_REV_STATUS_ANALYSIS,
    EN_MODBUS_REV_STATUS_ACK,
    EN_MODBUS_REV_STATUS_ACK_SEND_WAIT,
    EN_MODBUS_REV_STATUS_ACK_WAIT,
    EN_MODBUS_READ_MPPT,
    EN_COM_REV_STATUS_ACK_WAIT,
    EN_COM_SEND,
} EN_MODBUS_REV_STATUS;



typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
} EN_MODBUS_FUNC_CODE;

typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
} EN_MODBUS_ERROR_CODE;

uint8_t ModbusCmdAnalysis(SciRevData_t *p_com_cmd);
#endif

