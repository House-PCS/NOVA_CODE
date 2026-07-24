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

#define SLAVE_COMM_REV_MAX_TIME         350
#define SLAVE_COMM_INTERVAL_TIME        150
#define SLAVE_COMM_POLLING_TIME         350
#define SLAVE_COMM_BMS_INTERVAL_TIME    150

#define READ_INPUT_TYPE                 0x04
#define READ_HOLD_TYPE                  0x03
#define WRITE_HOLD_TYPE                 0x10


#define ARC_MODBUS_READ_START           0x6400
#define ARC_MODBUS_READ_END             0x644F
#define ARC_MODBUS_HOLD_START           0X6450
#define ARC_MODBUS_HOLD_END             0X647F

#define COPRO_MODBUS_READ_START         0x6480
#define COPRO_MODBUS_READ_END           0x64CF
#define COPRO_MODBUS_HOLD_START         0X64D0
#define COPRO_MODBUS_HOLD_END           0X64FF

#define MODBUS_WRITE_QUEUE_DEPTH        3

/*********************Host*************************/
typedef enum
{
    EN_ARC_READ_INPUT_EVENT,
    EN_ARC_READ_HOLD_EVENT,
    EN_COPRO_READ_INPUT_EVENT,
    EN_COPRO_READ_HOLD_EVENT,
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
    uint16_t    arc_model_id;                               //0x6400 
    uint16_t    arc_sn[10];                                 //0x6401-0x640A SN号
    uint16_t    arc_hw_version;                             //0x640B 硬件版本号
    uint16_t    arc_sw_version[2];                          //0x640C~0x640D 固件版本号
    uint16_t    arc_sw_version_extend[12];                   //0x640E~0x6419 固件版本号扩展
    uint16_t    arc_reserve[6];                             //0x641A~0x641F 预留
    
    /*ARC 检测状态信息*/
    uint16_t    arc_occur;                           //0x6420 各通道电弧是否触发阈值，每个bit代表一个通道 0代表未发生，1代表发生 
    uint16_t    ins_res_low_occur;                     //0x6421 各通道绝缘阻抗低触发阈值，每个bit代表一个通道 0代表未发生，1代表发生
    uint16_t    arc_detect_sensitivity;               //0x6422 各通道拉弧检测灵敏度
    uint16_t    ins_res_threshold;                    //0x6423 绝缘检测阻抗阈值
    
    uint16_t    pv1_channel_vol;
    uint16_t    pv2_channel_vol;
    uint16_t    pv3_channel_vol;
    uint16_t    pv4_channel_vol;

    uint16_t    pv1_channel_Rp;
    uint16_t    pv2_channel_Rp;
    uint16_t    pv3_channel_Rp;
    uint16_t    pv4_channel_Rp;

    uint16_t    pv1_channel_Rn;
    uint16_t    pv2_channel_Rn;
    uint16_t    pv3_channel_Rn;
    uint16_t    pv4_channel_Rn;

    uint16_t    pv1_arc_threshold_num;
    uint16_t    pv2_arc_threshold_num;
    uint16_t    pv3_arc_threshold_num;
    uint16_t    pv4_arc_threshold_num;

    uint16_t    rescheckoverflag;                 //0x6434 绝缘自检完成标志位，0代表未完成，1代表完成   

    uint16_t    arc_status_reserve[27];             //0x6435~0x644F
} ArcOnlyReadPara_t;

typedef struct _arc_hold_para_stru_t
{
    uint16_t    arc_detect_enable;                            //0x6450 拉弧检测使能
    uint16_t    ins_res_enable;                               //0x6451 绝缘检测使能
    uint16_t    arc_ctrlstate_reserve1[14];                 //0x6452~0x645F 预留
    uint16_t    sys_upgrade_status;                           //0x6460 系统升级状态
    uint16_t    arc_ctrlstate_reserve2[31];                 //0x6461~0x647F 预留
        
} ArcHoldPara_t;

typedef struct _arc_para_stru_t
{
    ArcOnlyReadPara_t    arc_only_read_para;
    ArcHoldPara_t        arc_hold_para;
} ArcPara_t;


typedef struct _copro_only_read_para_stru_t
{
    uint16_t    pcs_copro_mode_id;                                
    uint16_t    sn[10];                 
    uint16_t    hw_version;                  
    uint16_t    soft_version[2];  
    //uint16_t    Copro_res_err_code;//
    uint16_t    soft_version_reserve[12];
    uint16_t    reserve[6];

    uint16_t    pcs_copro_run_status;                       //0x64A0 PCS协处理器运行状态
    uint16_t    pcs_copro_state_reserve[47];                //0x64A1- 0x64CF

    // uint16_t    channel_num;
    // uint16_t    channle_curr_value[8];
    // uint16_t    arc_alarm_info;       
    // uint16_t    real_time_arc_alarm_info; 
    // uint16_t    channel_arc_strength[8];
    // uint16_t    channel_arc_history_max_value[8];
    // uint16_t    arc_selftest_result;
    // uint16_t    reserve1[5];
    // uint16_t    channel_input_volt[8];
    // uint16_t    channel_rx[8];
    // uint16_t    channel_ry[8];
    // uint16_t    impedance_alarm_info;           
    // uint16_t    inpedance_breakdown_info;       
    // uint16_t    version_extend[6];
} CoproOnlyReadPara_t;

typedef struct _copro_hold_para_stru_t
{
    // uint16_t    PcsCopRunStatus;
    // uint16_t    reserve[47];  

    uint16_t    sys_upgrade_status;
    uint16_t    dsp_heart_beat;
    uint16_t    mcu_rly_cntl;
    uint16_t    mcu_rly_state;
    
    // uint16_t    arc_alarm_threshold[8];
    // uint16_t    arc_sensitivity;                        
    // uint16_t    arc_insulation_detect_en;               
    // uint16_t    sys_upgrade_status;                     
    // uint16_t    reserved[2];
    // uint16_t    impedance_alarm_threshold_rx[8];
    // uint16_t    impedance_alarm_threshold_ry[8];          
} CoproHoldPara_t;


typedef struct _copro_para_stru_t
{
    CoproOnlyReadPara_t    copro_only_read_para;
    CoproHoldPara_t        copro_hold_para;
} CoproPara_t;




typedef struct _meter_info_stru_t
{
    uint16_t reset_sci_count ;
    int16_t r_phase_meter_curr;
    int16_t s_phase_meter_curr;
    int16_t t_phase_meter_curr;
    int32_t r_phase_meter_pwr;
} meter_info_t;




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
    CoproPara_t                      copro_para;
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

    uint16_t                 read_copro_only_read;      // 0xFF 无效 0x00 成功      0x01 错误                        
    uint16_t                 read_copro_hold;           // 0xFF 无效 0x00 成功      0x01 错误   
    uint16_t                 write_copro_hold;          // 0xFF 无效 0x00 成功      0x01 错误
} ModbusAckStatus_t;

void ModbusRegInit(void);
void BmsModBusHostWrite10(uint8_t slave_addr, uint16_t reg_start_addr, uint16_t reg_num, uint8_t data_num, uint16_t *pReg_value);
void ModbusProcess(void);
void ModbusHostAnalysis(SciRevData_t *p_com_cmd);
bool SetArcSysReset(void);
bool SetCoproSysReset(void);
bool WriteArcHoldReg(uint16_t reg_addr, uint16_t value);

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


extern ModBusPara_t   gstModBusPara;

uint8_t ModbusCmdAnalysis(SciRevData_t *p_com_cmd);
int16_t GetMeterCurInfo(void);
int32_t GetMeterPowerInfo();
uint16_t GetMeterConnectState(void);
#endif

