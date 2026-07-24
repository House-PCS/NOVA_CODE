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

#define DEBUG_RX_BUF_LEN    256
#define COMSERVICE_DEBUG_RX_SIZE    1024
#define COMSERVICE_DEBUG_TX_SIZE    1024

typedef enum
{
    EN_PROTOCOL_STATE_IDLE = 0,         /**/
    EN_PROTOCOL_STATE_FRAME_COLLECT,    /**/
    EN_PROTOCOL_STATE_DATA_COLLECT,     /**/
    EN_PROTOCOL_STATE_RESPONSE,         /**/
}en_protocol_state_t;

typedef enum
{
    EN_PROTOCOL_CODE_READ = 0,        /**/
    EN_PROTOCOL_CODE_WRITE,           /**/   
}en_protocol_code_t;

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
}protocol_obj_t;

typedef struct
{
    uint8_t  *p_buf;       /*数据指针*/
    uint32_t  n_mem_len;   /*内存大小*/
    uint32_t  n_w_cnt;     /*数据读计数*/
    uint32_t  n_r_cnt;     /*数据写计数*/
}transmission_obj_t;

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
}comm_port_t;

typedef enum
{
    EN_COMSERVICE_LOG_TYPE_STRING,
    EN_COMSERVICE_LOG_TYPE_DEBUG,
    EN_COMSERVICE_LOG_TYPE_DEBUG1,
    EN_COMSERVICE_LOG_TYPE_INFO,
    EN_COMSERVICE_LOG_TYPE_ERROR,
    EN_COMSERVICE_LOG_TYPE_NUM,
}en_comservice_log_type_t;

#define LOG_STRING(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_STRING,\
                                    tag,format,##args)
/* LOG_DEBUG 和 LOG_DEBUG1均不带\r\n */
#define LOG_DEBUG(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_DEBUG,\
                                    tag,format,##args)
/* LOG_DEBUG1打印不带[DEBUG] 和 LOG_TAG */
#define LOG_DEBUG1(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_DEBUG1,\
                                    tag,format,##args)
#define LOG_INFO(tag,format,args...)    comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_INFO,\
                                    tag,format,##args)
#define LOG_ERROR(tag,format,args...)   comservice_log_info_printf(EN_COMSERVICE_LOG_TYPE_ERROR,\
                                    tag,format,##args)
#define LOG_DEBUG_HEX(tag, data, len)   comservice_log_info_hex_printf(EN_COMSERVICE_LOG_TYPE_DEBUG,\
                                    tag,data, len)
#define LOG_INFO_HEX(tag, data, len)   comservice_log_info_hex_printf(EN_COMSERVICE_LOG_TYPE_INFO,\
                                    tag,data, len)
#define LOG_ERROR_HEX(tag, data, len)   comservice_log_info_hex_printf(EN_COMSERVICE_LOG_TYPE_ERROR,\
                                    tag,data, len)

void DebugInit(void);
void DebugProcess(void);
void comservice_log_info_printf(en_comservice_log_type_t e_type, char *string, char *format,...);
uint16_t float_cov_uint16(float value);
float uint16_cov_float(uint16_t value);
#endif












