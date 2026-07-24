/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include "bsp.h"
#include "mid_shell.h"
#include "dev_communication.h"
#include "modbus_analysis.h"

/**************************** printf def ***************************************/
#define LOG_STRING(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING, tag,format,##args)
#define LOG_DEBUG(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_DEBUG, tag,format,##args)
#define LOG_DEBUG1(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_DEBUG1, tag,format,##args)
#define LOG_INFO(tag,format,args...)    comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_INFO, tag,format,##args)
#define LOG_ERROR(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_ERROR, tag,format,##args)
#define LOG_DEBUG_HEX(tag, data, len)   comservice_log_info_hex_printf(EN_COMSERVICE_LOG_TYPE_DEBUG, tag,data, len)
#define LOG_INFO_HEX(tag, data, len)   comservice_log_info_hex_printf(EN_COMSERVICE_LOG_TYPE_INFO, tag,data, len)
#define LOG_ERROR_HEX(tag, data, len)   comservice_log_info_hex_printf(EN_COMSERVICE_LOG_TYPE_ERROR, tag,data, len)

/**************************** packet def ***************************************/
#define DEBUG_PACKET_LEN            7     

/**************************** buffer def ***************************************/
#define DEBUG_RX_BUF_LEN    1024
#define COMSERVICE_DEBUG_RX_SIZE    1024
#define COMSERVICE_DEBUG_TX_SIZE    1024

/**************************** Register address *********************************/
/* Starting and ending addresses of each register */
#define UART_DEBUG_REG_BASE_ADDR        (0x0000)
#define UART_DEBUG_REG_END_ADDR         (0x00C8)

/**************************** Register num ************************************/
#define CELL_SOC_ABT_NUM                       2
#define PACK_CURRENT_NUM                       4
#define HEAT_STATUS_NUM                        4
#define CELL_SOC_MIN_RLT_NUM                   2
#define CELL_SOC_MAX_RLT_NUM                   2
#define DEB_MAX_CHARGE_CURRENT_NUM         	   4
#define DEB_MAX_DS_CURRENT_NUM                 4
#define DEB_MAX_DS_CURRENT_30S_NUM             4
#define DEB_MAX_DS_CURRENT_60S_NUM             4

/**************************** Register Addr ***********************************/
#define DEB_CELL_VOLT_0_ADDR                   (0)
#define DEB_CELL_VOLT_1_ADDR                   (DEB_CELL_VOLT_0_ADDR + CELL_VOLT_0_NUM)
#define DEB_CELL_VOLT_2_ADDR                   (DEB_CELL_VOLT_1_ADDR + CELL_VOLT_1_NUM)
#define DEB_CELL_VOLT_3_ADDR                   (DEB_CELL_VOLT_2_ADDR + CELL_VOLT_2_NUM)
#define DEB_CELL_VOLT_4_ADDR                   (DEB_CELL_VOLT_3_ADDR + CELL_VOLT_3_NUM)
#define DEB_CELL_VOLT_5_ADDR                   (DEB_CELL_VOLT_4_ADDR + CELL_VOLT_4_NUM)
#define DEB_CELL_VOLT_6_ADDR                   (DEB_CELL_VOLT_5_ADDR + CELL_VOLT_5_NUM)
#define DEB_CELL_VOLT_7_ADDR                   (DEB_CELL_VOLT_6_ADDR + CELL_VOLT_6_NUM)
#define DEB_CELL_VOLT_8_ADDR                   (DEB_CELL_VOLT_7_ADDR + CELL_VOLT_7_NUM)
#define DEB_CELL_VOLT_9_ADDR                   (DEB_CELL_VOLT_8_ADDR + CELL_VOLT_8_NUM)
#define DEB_CELL_VOLT_10_ADDR                  (DEB_CELL_VOLT_9_ADDR + CELL_VOLT_9_NUM)
#define DEB_CELL_VOLT_11_ADDR                  (DEB_CELL_VOLT_10_ADDR + CELL_VOLT_10_NUM)
#define DEB_CELL_VOLT_12_ADDR                  (DEB_CELL_VOLT_11_ADDR + CELL_VOLT_11_NUM)
#define DEB_CELL_VOLT_13_ADDR                  (DEB_CELL_VOLT_12_ADDR + CELL_VOLT_12_NUM)
#define DEB_CELL_VOLT_14_ADDR                  (DEB_CELL_VOLT_13_ADDR + CELL_VOLT_13_NUM)
#define DEB_CELL_VOLT_15_ADDR                  (DEB_CELL_VOLT_14_ADDR + CELL_VOLT_14_NUM)
#define DEB_CELL_VOLT_16_ADDR                  (DEB_CELL_VOLT_15_ADDR + CELL_VOLT_15_NUM)
#define DEB_CELL_VOLT_17_ADDR                  (DEB_CELL_VOLT_16_ADDR + CELL_VOLT_16_NUM)
#define DEB_CELL_VOLT_18_ADDR                  (DEB_CELL_VOLT_17_ADDR + CELL_VOLT_17_NUM)
#define DEB_CELL_VOLT_19_ADDR                  (DEB_CELL_VOLT_18_ADDR + CELL_VOLT_18_NUM)
#define DEB_CELL_MAX_VOLT_ADDR                 (DEB_CELL_VOLT_19_ADDR + CELL_VOLT_19_NUM)
#define DEB_CELL_MIN_VOLT_ADDR                 (DEB_CELL_MAX_VOLT_ADDR + CELL_MAX_VOLT_NUM)
#define DEB_CELL_SUM_VOLT_ADDR                 (DEB_CELL_MIN_VOLT_ADDR + CELL_MIN_VOLT_NUM)
#define DEB_CELL_TEMP_0_ADDR                   (DEB_CELL_SUM_VOLT_ADDR + CELL_SUM_VOLT_NUM)
#define DEB_CELL_TEMP_1_ADDR                   (DEB_CELL_TEMP_0_ADDR + CELL_TEMP_0_NUM)
#define DEB_CELL_TEMP_2_ADDR                   (DEB_CELL_TEMP_1_ADDR + CELL_TEMP_1_NUM)
#define DEB_CELL_TEMP_3_ADDR                   (DEB_CELL_TEMP_2_ADDR + CELL_TEMP_2_NUM)
#define DEB_CELL_TEMP_4_ADDR                   (DEB_CELL_TEMP_3_ADDR + CELL_TEMP_3_NUM)
#define DEB_CELL_TEMP_5_ADDR                   (DEB_CELL_TEMP_4_ADDR + CELL_TEMP_4_NUM)
#define DEB_CELL_TEMP_6_ADDR                   (DEB_CELL_TEMP_5_ADDR + CELL_TEMP_5_NUM)
#define DEB_CELL_TEMP_7_ADDR                   (DEB_CELL_TEMP_6_ADDR + CELL_TEMP_6_NUM)
#define DEB_CELL_TEMP_8_ADDR                   (DEB_CELL_TEMP_7_ADDR + CELL_TEMP_7_NUM)
#define DEB_CELL_TEMP_9_ADDR                   (DEB_CELL_TEMP_8_ADDR + CELL_TEMP_8_NUM)
#define DEB_CELL_TEMP_10_ADDR                  (DEB_CELL_TEMP_9_ADDR + CELL_TEMP_9_NUM)
#define DEB_CELL_TEMP_11_ADDR                  (DEB_CELL_TEMP_10_ADDR + CELL_TEMP_10_NUM)
#define DEB_CELL_TEMP_12_ADDR                  (DEB_CELL_TEMP_11_ADDR + CELL_TEMP_11_NUM)
#define DEB_CELL_TEMP_13_ADDR                  (DEB_CELL_TEMP_12_ADDR + CELL_TEMP_12_NUM)
#define DEB_CELL_TEMP_14_ADDR                  (DEB_CELL_TEMP_13_ADDR + CELL_TEMP_13_NUM)
#define DEB_CELL_TEMP_15_ADDR                  (DEB_CELL_TEMP_14_ADDR + CELL_TEMP_14_NUM)
#define DEB_CELL_TEMP_16_ADDR                  (DEB_CELL_TEMP_15_ADDR + CELL_TEMP_15_NUM)
#define DEB_CELL_TEMP_17_ADDR                  (DEB_CELL_TEMP_16_ADDR + CELL_TEMP_16_NUM)
#define DEB_CELL_TEMP_18_ADDR                  (DEB_CELL_TEMP_17_ADDR + CELL_TEMP_17_NUM)
#define DEB_CELL_TEMP_19_ADDR                  (DEB_CELL_TEMP_18_ADDR + CELL_TEMP_18_NUM)
#define DEB_CELL_MAX_TEMP_ADDR                 (DEB_CELL_TEMP_19_ADDR + CELL_TEMP_19_NUM)
#define DEB_CELL_MIN_TEMP_ADDR                 (DEB_CELL_MAX_TEMP_ADDR + CELL_MAX_TEMP_NUM)
#define DEB_AFE_TEMP_ADDR                      (DEB_CELL_MIN_TEMP_ADDR + CELL_MIN_TEMP_NUM)
#define DEB_MOS_TEMP_ADDR                      (DEB_AFE_TEMP_ADDR + AFE_TEMP_NUM)
#define DEB_AMBIENT_TEMP_ADDR                  (DEB_MOS_TEMP_ADDR + MOS_TEMP_NUM)
#define DEB_PACK_CURRENT_ADDR                  (DEB_AMBIENT_TEMP_ADDR + AMBIENT_TEMP_NUM)
#define DEB_CURRENT_POWER_ADDR                 (DEB_PACK_CURRENT_ADDR + PACK_CURRENT_NUM)
#define DEB_HEAT_STATUS_ADDR                   (DEB_CURRENT_POWER_ADDR + CURRENT_POWER_NUM)
#define DEB_SOC_ABT_CELL_0_ADDR                (DEB_HEAT_STATUS_ADDR + HEAT_STATUS_NUM)
#define DEB_SOC_ABT_CELL_1_ADDR                (DEB_SOC_ABT_CELL_0_ADDR + CELL_SOC_ABT_NUM)
#define DEB_SOC_ABT_CELL_2_ADDR                (DEB_SOC_ABT_CELL_1_ADDR + CELL_SOC_ABT_NUM)
#define DEB_SOC_ABT_CELL_3_ADDR                (DEB_SOC_ABT_CELL_2_ADDR + CELL_SOC_ABT_NUM)
#define DEB_SOC_ABT_CELL_4_ADDR                (DEB_SOC_ABT_CELL_3_ADDR + CELL_SOC_ABT_NUM)
#define DEB_SOC_ABT_CELL_5_ADDR                (DEB_SOC_ABT_CELL_4_ADDR + CELL_SOC_ABT_NUM)
#define DEB_SOC_ABT_CELL_6_ADDR                (DEB_SOC_ABT_CELL_5_ADDR + CELL_SOC_ABT_NUM)
#define DEB_SOC_ABT_CELL_7_ADDR                (DEB_SOC_ABT_CELL_6_ADDR + CELL_SOC_ABT_NUM)
#define DEB_CELL_SOC_MIN_RLT_ADDR              (DEB_SOC_ABT_CELL_7_ADDR + CELL_SOC_ABT_NUM)
#define DEB_CELL_SOC_MAX_RLT_ADDR              (DEB_CELL_SOC_MIN_RLT_ADDR + CELL_SOC_MIN_RLT_NUM)
#define DEB_PACK_SOC_RLT_ADDR                  (DEB_CELL_SOC_MAX_RLT_ADDR + CELL_SOC_MAX_RLT_NUM)
#define DEB_PACK_SOC_DSP_ADDR                  (DEB_PACK_SOC_RLT_ADDR + SOC_RLT_NUM)
#define DEB_PACK_HEALTH_ADDR                   (DEB_PACK_SOC_DSP_ADDR + SOC_DSP_NUM)
#define DEB_PACK_STATUS_ADDR                   (DEB_PACK_HEALTH_ADDR + PACK_HEALTH_NUM)
#define DEB_RECHARGING_TIME_ADDR               (DEB_PACK_STATUS_ADDR + PACK_STATUS_NUM)
#define DEB_MOS_STATUS_ADDR                    (DEB_RECHARGING_TIME_ADDR + RECHARGING_TIME_NUM)
#define DEB_BALANCE_STATUS_ADDR                (DEB_MOS_STATUS_ADDR + MOS_STATUS_NUM)
#define DEB_FAULT_LEVEL_ADDR                   (DEB_BALANCE_STATUS_ADDR + BALANCE_STATUS_NUM)
#define DEB_FAULT_NUM_CODE_0_ADDR              (DEB_FAULT_LEVEL_ADDR + FAULT_LEVEL_NUM)
#define DEB_FAULT_NUM_CODE_1_ADDR              (DEB_FAULT_NUM_CODE_0_ADDR + FAULT_NUM_CODE_0_NUM)
#define DEB_FAULT_NUM_CODE_2_ADDR              (DEB_FAULT_NUM_CODE_1_ADDR + FAULT_NUM_CODE_1_NUM)
#define DEB_FAULT_NUM_CODE_3_ADDR              (DEB_FAULT_NUM_CODE_2_ADDR + FAULT_NUM_CODE_2_NUM)
#define DEB_FAULT_NUM_CODE_4_ADDR              (DEB_FAULT_NUM_CODE_3_ADDR + FAULT_NUM_CODE_3_NUM)
#define DEB_FAULT_NUM_CODE_5_ADDR              (DEB_FAULT_NUM_CODE_4_ADDR + FAULT_NUM_CODE_4_NUM)
#define DEB_FAULT_NUM_CODE_6_ADDR              (DEB_FAULT_NUM_CODE_5_ADDR + FAULT_NUM_CODE_5_NUM)
#define DEB_FAULT_NUM_CODE_7_ADDR              (DEB_FAULT_NUM_CODE_6_ADDR + FAULT_NUM_CODE_6_NUM)
#define DEB_FAULT_NUM_CODE_8_ADDR              (DEB_FAULT_NUM_CODE_7_ADDR + FAULT_NUM_CODE_7_NUM)
#define DEB_FAULT_NUM_CODE_9_ADDR              (DEB_FAULT_NUM_CODE_8_ADDR + FAULT_NUM_CODE_8_NUM)
#define DEB_FAULT_NUM_CODE_10_ADDR             (DEB_FAULT_NUM_CODE_9_ADDR + FAULT_NUM_CODE_9_NUM)
#define DEB_FAULT_NUM_CODE_11_ADDR             (DEB_FAULT_NUM_CODE_10_ADDR + FAULT_NUM_CODE_10_NUM)
#define DEB_FAULT_NUM_CODE_12_ADDR             (DEB_FAULT_NUM_CODE_11_ADDR + FAULT_NUM_CODE_11_NUM)
#define DEB_FAULT_NUM_CODE_13_ADDR             (DEB_FAULT_NUM_CODE_12_ADDR + FAULT_NUM_CODE_12_NUM)
#define DEB_FAULT_NUM_CODE_14_ADDR             (DEB_FAULT_NUM_CODE_13_ADDR + FAULT_NUM_CODE_13_NUM)
#define DEB_FAULT_NUM_CODE_15_ADDR             (DEB_FAULT_NUM_CODE_14_ADDR + FAULT_NUM_CODE_14_NUM)
#define DEB_FAULT_NUM_CODE_16_ADDR             (DEB_FAULT_NUM_CODE_15_ADDR + FAULT_NUM_CODE_15_NUM)
#define DEB_FAULT_NUM_CODE_17_ADDR             (DEB_FAULT_NUM_CODE_16_ADDR + FAULT_NUM_CODE_16_NUM)
#define DEB_FAULT_NUM_CODE_18_ADDR             (DEB_FAULT_NUM_CODE_17_ADDR + FAULT_NUM_CODE_17_NUM)
#define DEB_FAULT_NUM_CODE_19_ADDR             (DEB_FAULT_NUM_CODE_18_ADDR + FAULT_NUM_CODE_18_NUM)
#define DEB_MAX_CHARGE_CURRENT_ADDR			   (DEB_FAULT_NUM_CODE_19_ADDR + FAULT_NUM_CODE_19_NUM)
#define DEB_MAX_DS_CURRENT_ADDR		           (DEB_MAX_CHARGE_CURRENT_ADDR + DEB_MAX_CHARGE_CURRENT_NUM)
#define DEB_MAX_DS_CURRENT_30S_ADDR		       (DEB_MAX_DS_CURRENT_ADDR + DEB_MAX_DS_CURRENT_NUM)
#define DEB_MAX_DS_CURRENT_60S_ADDR		       (DEB_MAX_DS_CURRENT_30S_ADDR + DEB_MAX_DS_CURRENT_30S_NUM)

/**************************** Register num ************************************/
#define MODE_ID_NUM                            2
#define SN_NUM                                 20
#define HW_VERSION_NUM                         2
#define SW_VERSION_NUM                         2
#define RESERVED_NUM						   2
#define Version_Extend_0_NUM				   2
#define Version_Extend_1_NUM				   2
#define Version_Extend_2_NUM				   2
#define Version_Extend_3_NUM				   2
#define Version_Extend_4_NUM				   2
#define Version_Extend_5_NUM				   2
#define RESERVED_0_NUM                         24
#define CELL_VOLT_0_NUM                        2
#define CELL_VOLT_1_NUM                        2
#define CELL_VOLT_2_NUM                        2
#define CELL_VOLT_3_NUM                        2
#define CELL_VOLT_4_NUM                        2
#define CELL_VOLT_5_NUM                        2
#define CELL_VOLT_6_NUM                        2
#define CELL_VOLT_7_NUM                        2
#define CELL_VOLT_8_NUM                        2
#define CELL_VOLT_9_NUM                        2
#define CELL_VOLT_10_NUM                       2
#define CELL_VOLT_11_NUM                       2
#define CELL_VOLT_12_NUM                       2
#define CELL_VOLT_13_NUM                       2
#define CELL_VOLT_14_NUM                       2
#define CELL_VOLT_15_NUM                       2
#define CELL_VOLT_16_NUM                       2
#define CELL_VOLT_17_NUM                       2
#define CELL_VOLT_18_NUM                       2
#define CELL_VOLT_19_NUM                       2
#define CELL_MAX_VOLT_NUM                      2
#define CELL_MIN_VOLT_NUM                      2
#define CELL_AVE_VOLT_NUM                      2
#define CELL_SUM_VOLT_NUM                      4
#define CELL_CURRENT_NUM                       4
#define CURRENT_POWER_NUM                      2
#define ABS_CELL_CURRENT_NUM                   4
#define SOC_RLT_CELL_MIN_NUM                   2
#define SOC_RLT_CELL_MAX_NUM                   2
#define SOC_RLT_NUM                            2
#define RESERVED_1_NUM                         4
#define SOC_DSP_NUM                            2
#define CAPACITY_STD_AH_NUM                    2
#define CAPACITY_STD_WH_NUM                    2
#define CURRENT_MAX_CAPACITY_MAH_NUM           4
#define PACK_HEALTH_NUM                        2
#define PACK_CYCLES_NUM                        2
#define CHARGING_CAPACITY_NUM                  4
#define DISCHARGING_CAPACITY_NUM               4
#define CURRENT_MAX_CHARGE_CURRENT_NUM         4
#define CURRENT_MAX_DS_CURRENT_NUM             4
#define CURRENT_MAX_DS_CURRENT_30S_NUM         4
#define CURRENT_MAX_DS_CURRENT_60S_NUM         4
#define PACK_SLEEP_TIME_NUM                    4
#define CHARGING_CAPACITY_2_NUM                4
#define DISCHARGING_CAPACITY_2_NUM             4
#define PACK_STATUS_NUM                        2
#define RECHARGING_TIME_NUM                    4
#define MOS_STATUS_NUM                         2
#define EXP_NUM                                2
#define UP_FINISH_FLAG_NUM                     2
#define RESERVED_2_NUM                         6
#define CELL_TEMP_0_NUM                        2
#define CELL_TEMP_1_NUM                        2
#define CELL_TEMP_2_NUM                        2
#define CELL_TEMP_3_NUM                        2
#define CELL_TEMP_4_NUM                        2
#define CELL_TEMP_5_NUM                        2
#define CELL_TEMP_6_NUM                        2
#define CELL_TEMP_7_NUM                        2
#define CELL_TEMP_8_NUM                        2
#define CELL_TEMP_9_NUM                        2
#define CELL_TEMP_10_NUM                       2
#define CELL_TEMP_11_NUM                       2
#define CELL_TEMP_12_NUM                       2
#define CELL_TEMP_13_NUM                       2
#define CELL_TEMP_14_NUM                       2
#define CELL_TEMP_15_NUM                       2
#define CELL_TEMP_16_NUM                       2
#define CELL_TEMP_17_NUM                       2
#define CELL_TEMP_18_NUM                       2
#define CELL_TEMP_19_NUM                       2
#define CELL_MAX_TEMP_NUM                      2
#define CELL_MIN_TEMP_NUM                      2
#define CELL_AVE_TEMP_NUM                      2
#define AFE_TEMP_NUM                           2
#define MOS_TEMP_NUM                           2
#define AMBIENT_TEMP_NUM                       2
#define BALANCE_STATUS_NUM                     2
#define RESERVED_3_NUM                         32
#define FAULT_LEVEL_NUM                        2
#define MOS_CONTROL_ERR_NUM                    2
#define TEMP_SENR_ERR_0_NUM                    2
#define TEMP_SENR_ERR_1_NUM                    2
#define CURR_SENR_ERR_NUM                      2
#define VOLT_SENR_ERR_NUM                      2
#define SOH_ERR_NUM                            2
#define CELL_BAL_ERR_NUM                       2
#define SOC_ERR_NUM                            2
#define HISTORY_SAVE_ERR_NUM                   2
#define RTC_ERR_NUM                            2
#define AFE_CONTROL_ERR_NUM                    2
#define FAULT_NUM_CODE_0_NUM                   2
#define FAULT_NUM_CODE_1_NUM                   2
#define FAULT_NUM_CODE_2_NUM                   2
#define FAULT_NUM_CODE_3_NUM                   2
#define FAULT_NUM_CODE_4_NUM                   2
#define FAULT_NUM_CODE_5_NUM                   2
#define FAULT_NUM_CODE_6_NUM                   2
#define FAULT_NUM_CODE_7_NUM                   2
#define FAULT_NUM_CODE_8_NUM                   2
#define FAULT_NUM_CODE_9_NUM                   2
#define FAULT_NUM_CODE_10_NUM                  2
#define FAULT_NUM_CODE_11_NUM                  2
#define FAULT_NUM_CODE_12_NUM                  2
#define FAULT_NUM_CODE_13_NUM                  2
#define FAULT_NUM_CODE_14_NUM                  2
#define FAULT_NUM_CODE_15_NUM                  2
#define FAULT_NUM_CODE_16_NUM                  2
#define FAULT_NUM_CODE_17_NUM                  2
#define FAULT_NUM_CODE_18_NUM                  2
#define FAULT_NUM_CODE_19_NUM                  2
#define PRECHARGE_STATUS_NUM                   2
#define RESERVED_4_NUM                         20
#define DISCHARGE_MOC_CONTROL_NUM              2
#define CHARGE_MOC_CONTROL_NUM                 2
#define PRECHARGE_MOC_CONTROL_NUM              2
#define LIMIT_CURRENT_CONTROL_NUM              2
#define BMS_SHUT_DOWN_NUM                      2
#define RTC_TIMES_NUM                          4
#define HEAT_CPNTROL_NUM                       2
#define SYS_UPGRADE_STATUS_NUM                 2
#define MCU_RESET_NUM						   2
#define RESERVED_5_NUM                         28
#define CELL_UV_LEVEL_1_NUM                    2
#define CELL_UV_LEVEL_2_NUM                    2
#define CELL_UV_LEVEL_3_NUM                    2
#define CELL_OV_LEVEL_1_NUM                    2
#define CELL_OV_LEVEL_2_NUM                    2
#define CELL_OV_LEVEL_3_NUM                    2
#define PACK_UV_NUM                            4
#define PACK_OV_NUM                            4
#define CELL_OT_LEVEL_1_NUM                    2
#define CELL_OT_LEVEL_2_NUM                    2
#define CELL_OT_LEVEL_3_NUM                    2
#define CELL_UT_LEVEL_NUM                      2
#define MOS_OT_LEVEL_1_NUM                     2
#define MOS_OT_LEVEL_2_NUM                     2
#define MOS_OT_LEVEL_3_NUM                     2
#define DISCHARGE_OC_LEVEL_1_NUM               2
#define DISCHARGE_OC_LEVEL_2_NUM               2
#define DISCHARGE_OC_LEVEL_3_NUM               2
#define CHARGE_OC_LEVEL_1_NUM                  2
#define CHARGE_OC_LEVEL_2_NUM                  2
#define CHARGE_OC_LEVEL_3_NUM                  2
#define CELL_TEMP_DIFF_NUM                     2
#define CELL_VOLTAGE_DIFF_NUM                  2
#define SOH_UNDER_LEVEL_1_NUM                  2
#define SOH_UNDER_LEVEL_2_NUM                  2
#define ERROR_CLEAR_NUM                        2
#define FORCE_OCV_NUM                          2
#define RESET_FACTORY_NUM                      2
#define CURRENT_FACTORY_NUM                    4
#define PRE_DSG_END_NUM                        2
#define DSP_WAKEUPING_NUM                      2
#define UPGRADE_STATUS_NUM                     2
#define RESERVED_6_NUM                         42			 

union Fp32
{
    uint32_t u;
    float f;
};

typedef enum
{
    EN_PROTOCOL_STATE_IDLE = 0,         /**/
    EN_PROTOCOL_STATE_FRAME_COLLECT,    /**/
    EN_PROTOCOL_STATE_DATA_COLLECT,     /**/
    EN_PROTOCOL_STATE_RESPONSE,         /**/
} en_protocol_state_t;

typedef enum
{
    EN_PROTOCOL_CODE_READ = 0,        /**/
    EN_PROTOCOL_CODE_WRITE,           /**/
} en_protocol_code_t;

typedef struct
{
    en_protocol_state_t e_state;         /*协议处理的状态*/
    en_protocol_code_t  e_code;          /*事务代码*/
    uint32_t            n_payload_reg;   /*用户数据寄存器*/
    uint32_t            n_payload_len;   /*用户负载数据长度*/
    uint32_t            n_request_len;   /*用户请求长度*/
    uint32_t            n_pake_len;      /*数据包总长度*/
    uint32_t            n_pake_code;     /*数据包事务码*/
    uint32_t            n_pake_ack;      /*响应的ack码*/
    uint32_t            n_dev_index;     /*通信的索引码*/
    int32_t (*pfun_read_process) (uint8_t *p_data, uint32_t n_reg, uint16_t n_len, uint8_t *p_w_data, uint16_t *pn_w_len);  /*读协议处理函数*/
    int32_t (*pfun_write_process)(uint8_t *p_data, uint32_t n_reg, uint16_t n_len, uint8_t *p_w_data, uint16_t *pn_w_len);  /*写协议处理函数*/
} protocol_obj_t;

typedef struct
{
    uint8_t  *p_buf;       /*数据指针*/
    uint32_t  n_mem_len;   /*内存大小*/
    uint32_t  n_w_cnt;     /*数据读计数*/
    uint32_t  n_r_cnt;     /*数据写计数*/
} transmission_obj_t;

typedef struct
{
    uint8_t  rx_complete_flag;

    protocol_obj_t     protocol_obj;                                          /*协议处理数据对象*/
    int32_t (*pfun_tx)(uint32_t n_index, uint8_t *p_data, uint32_t n_len);    /*发送回调*/
    uint32_t           n_txfun_index;                                         /*对应发送接口的索引*/
    uint32_t           n_rx_index;                                            /*对应接收的接口索引，DMA需要*/
    uint32_t           n_dev_index;                                           /*本机通信的索引码*/
    uint8_t            c_downup_flag;                                         /*转发通道的标志*/
    transmission_obj_t rx_obj;                                                /*接收数据对象*/
    transmission_obj_t tx_obj;                                                /*发送数据对象*/
    uint32_t           n_timeout;
} comm_port_t;

typedef enum
{
    EN_COMSERVICE_LOG_TYPE_STRING,
    EN_COMSERVICE_LOG_TYPE_DEBUG,
    EN_COMSERVICE_LOG_TYPE_DEBUG1,
    EN_COMSERVICE_LOG_TYPE_INFO,
    EN_COMSERVICE_LOG_TYPE_ERROR,
    EN_COMSERVICE_LOG_TYPE_NUM,
} en_comservice_log_type_t;

/**************************************** UART debug register ******************************************/
typedef struct _uart_debug_reg_stru_t
{
    uint16_t cell_vlt[20]                   ;
    uint16_t cell_max_vlt                   ;
    uint16_t cell_min_vlt                   ;
    uint32_t cell_sum_vlt                   ;
    int16_t  cell_temp[20]                  ;
    int16_t  cell_max_temp                  ;
    int16_t  cell_min_temp                  ;
    int16_t  afe_temp                       ;
    int16_t  mos_temp                       ;
    int16_t  ambient_temp                   ;
    int32_t  pack_current                   ;
    uint16_t current_power                  ;
    uint32_t heat_status                    ;
    int16_t  cell_soc_abt[8]                ;
    uint16_t cell_soc_min_rlt               ;
    uint16_t cell_soc_max_rlt               ;
    uint16_t pack_soc_rlt                   ;
    uint16_t pack_soc_dsp                   ;
    uint16_t pack_health                    ;
    uint16_t pack_status                    ;
    uint32_t recharge_time                 	;
    uint16_t mos_status                     ;
    uint16_t balance_status                 ;
    uint16_t flt_lvl                        ;
    uint16_t flt_dtc[20]                	;
    uint32_t max_chg_current                ;
    uint32_t max_dsg_current                ;
    uint32_t max_dsg_30s                    ;
    uint32_t max_dsg_60s                    ;
} UartDebugReg_t;

/**************************************** data point **********************************************/
typedef struct _uart_data_point_t
{
    uint16_t base_addr;
    uint16_t num;
    uint16_t data_type;
    uint16_t attr_rw;
    void *pdata;
} UartDataPoint_t;

extern UartDataPoint_t gUartFpgaDataPoint[];
extern UartDebugReg_t   garrUartDebugReg;
void Uart0Init(void);
void DebugProcess(void);
void comservice_log_info_printf(en_comservice_log_type_t e_type, char *string, char *format, ...);
void comservice_log_info_hex_printf(en_comservice_log_type_t e_type, char *string, uint8_t *p_data, uint32_t n_len);
uint16_t float_cov_uint16(float value);
float uint16_cov_float(uint16_t value);
void DebugInfoProcess(void);
#endif












