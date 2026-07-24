/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        mid_comservice.h
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


#ifndef __MID_COMSERVICE_H__
#define __MID_COMSERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "F28x_Project.h"
#include "cpu_timers.h"
#include <string.h>
#include "queue.h"
#include "crc.h"
#include "sci.h"

typedef enum
{
    EN_COMSERVICE_DEBUG_INDEX,
    EN_COMSERVICE_COM_INDEX,
	EN_COMSERVICE_INDEX_NUM,
    EN_COMSERVICE_INDEX_INVALID = 0xff,
}en_comservice_index_t;

typedef enum
{
    EN_PROTOCOL_STATE_IDLE = 0,         /**/
    EN_PROTOCOL_STATE_FRAME_COLLECT_COM,    /**/
	EN_PROTOCOL_STATE_FRAME_COLLECT_MOD,    /**/
    EN_PROTOCOL_STATE_DATA_COLLECT_COM,     /**/
	EN_PROTOCOL_STATE_DATA_COLLECT_MOD,     /**/
    EN_PROTOCOL_STATE_RESPONSE,         /**/
	EN_PROTOCOL_TIMEOUT,
}en_protocol_state_t;

typedef enum
{
    EN_PROTOCOL_CODE_READ = 0,        /**/
    EN_PROTOCOL_CODE_WRITE,           /**/
}en_protocol_code_t;

typedef struct
{
    uint8_t  *p_buf;       /*数据指针*/
    uint32_t  n_mem_len;   /*内存大小*/
    uint32_t  n_w_cnt;     /*数据读计数*/
    uint32_t  n_r_cnt;     /*数据写计数*/
}transmission_obj_t;

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
	uint8_t  rx_complete_flag;

    protocol_obj_t     protocol_obj;                                          /*协议处理数据对象*/
//    int32_t (*pfun_tx[COMSERVICE_DOWN_NUM])(uint32_t n_index, uint8_t *p_data, uint32_t n_len);    /*发送回调*/
//    uint32_t n_txfun_index[COMSERVICE_DOWN_NUM];                              /*对应发送接口的索引*/
    uint32_t           n_rx_index;                                            /*对应接收的接口索引，DMA需要*/
    uint32_t           n_dev_index;                                           /*本机通信的索引码*/
    uint8_t            c_downup_flag;                                         /*转发通道的标志*/
    transmission_obj_t rx_obj;                                                /*接收数据对象*/
    transmission_obj_t tx_obj;                                                /*发送数据对象*/
    uint32_t           n_timeout;
}CommPort_t;
//
typedef struct
{
    uint8_t    n_frame_delimiter;   /*frame const string*/
    uint8_t    n_frame_delimiter_ack;   /*frame const string*/
    uint8_t     c_frame_len;        /*frame head len*/
    uint8_t     c_frame_len_rep;        /*frame rep head len*/
    uint8_t     c_check_start;      /*校验起始位置*/
    uint8_t     c_other_len;        /*不计入协议中，额外的字段长度*/
}cmoservice_cfg_t;


typedef struct  /*包头7字节*/
{
    uint8_t  packet_head ;      /* 固定为0x5A */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd_low;               /* 不同cmd对应的数据格式不同 */
	uint16_t cmd_high;               /* 不同cmd对应的数据格式不同 */
    uint16_t data_field_len_low;    /* 读写传输的数据长度，字节为单位 */
    uint16_t data_field_len_high;    /* 读写传输的数据长度，字节为单位 */
    uint16_t  *data_field;       /* data段 */
    uint16_t check_sum_low;         /* 校验和() */
    uint16_t check_sum_high;         /* 校验和() */
} protocal_request_packet_t;


/* protocal response packet */
typedef struct  /*包头 字节*/
{
	uint16_t  packet_head ;      /* 固定为0xA5 */
	uint16_t  send_addr;         /* 发送方的地址 */
	uint16_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd_low;               /* 不同cmd对应的数据格式不同 */
	uint16_t cmd_high;               /* 不同cmd对应的数据格式不同 */
	uint16_t  error_code;        /* error  */
    uint16_t check_sum_low;         /* 校验和() */
    uint16_t check_sum_high;         /* 校验和() */
} protocal_response_packet_t;

typedef struct  _protocal_response_stru_t/*包头 字节*/
{
	uint16_t  packet_head ;      /* 固定为0xA5 */
	uint16_t  send_addr;         /* 发送方的地址 */
	uint16_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd_low;               /* 不同cmd对应的数据格式不同 */
    uint16_t cmd_high;               /* 不同cmd对应的数据格式不同 */
    uint16_t  error_code;        /* error  */
    uint16_t data_field_len_low;    /* 读写传输的数据长度，字节为单位 */
    uint16_t data_field_len_high;    /* 读写传输的数据长度，字节为单位 */
}ProtocalReadResponsePacket_t;

typedef enum
{
	TYPE_MODBUS_DATA,
	TYPE_COM_DATA,
}EN_UART_DATA_TYPE;

typedef struct _comm_host_rev_data_stru_t
{
    uint8_t                     slave_addr;
    EN_UART_DATA_TYPE           data_type;
	uint16_t			        data_size;
	uint8_t				        *pData;
}SciRevData_t;




extern Queue_t	gstSciRevQueue;


void MidComserviceInit(void);
int32_t mid_comservice_data_in(en_comservice_index_t e_index, uint8_t *p_data, uint32_t n_len);
void MidComserviceHandle(void);
void ScidRxHandler(uint8_t data);
void ScidSendMsgToBuff(const char* format, ...);
void ScidSendMsg();

#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
