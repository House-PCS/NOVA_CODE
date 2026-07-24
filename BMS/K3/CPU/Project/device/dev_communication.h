
#ifndef __DEV_COMMUNICATION_H_
#define __DEV_COMMUNICATION_H_

#include "uart.h"
#include "queue.h"
#include "bsp.h"
#include "dev_spiflash.h"
#include "sys_info.h"
#pragma pack(1)
typedef struct _protocal_request_stru_t   /*包头7字节*/
{
    uint8_t  packet_head ;      /* 固定为0x5A */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint16_t data_field_len;    /* 读写传输的数据长度，字节为单位 */
    uint8_t  *data_field;       /* data段 */
    uint16_t check_sum;         /* 校验和() */
} ProtocalRequestPacket_t;
#pragma pack()

#pragma pack(1)
typedef struct  _protocal_read_response_stru_t/*包头 字节*/
{
    uint8_t  packet_head ;      /* 固定为0xA5 */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint8_t  error_code;        /* error  */
    uint16_t data_field_len;    /* 读写传输的数据长度，字节为单位 */   
}ProtocalReadResponsePacket_t;
#pragma pack()

#pragma pack(1)
typedef struct  _protocal_response_stru_t/*包头 字节*/
{
    uint8_t  packet_head ;      /* 固定为0xA5 */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint8_t  error_code;        /* error  */   
    uint16_t check_sum;         /* 校验和() */    
}ProtocalResponsePacket_t;
#pragma pack()

/**************************************** enum cfg **********************************************/
typedef enum
{
    EN_DATA_INVALID = 0,
    EN_DATA_U8,
    EN_DATA_S8,
    EN_DATA_U16,
    EN_DATA_S16,
    EN_DATA_U32,
    EN_DATA_S32,
    EN_DATA_STRING,
} EN_DATA_TYPE;

typedef enum
{
    EN_ATTR_INVALID = 0,
    EN_ATTR_RO,
    EN_ATTR_WO,
    EN_ATTR_RW,
} EN_DATA_ATTR_RW;

typedef enum
{
    COM_TYPE_UART_DATA,
    
}EN_COM_REV_DATA_TYPE;

#define REQUST_PACKET_HEAD      0x5A
#define RESPONSE_PACKET_HEAD    0xA5
#define SEND_ADDR               0x00 /* 主控 */
#define RECV_ADDR               0x03 /* BMS */

#define MON_ADDR	            0x00
#define PCS_ADDR				0x01
#define ARC_ADDR				0x02
#define BMS_ADDR 			    0x03
#define UI_ADDR				    0x04

#define  ACK_OK                 0x00             /* ack success */
#define  ACK_ERROR              0x01             /* ack error */
#define  ACK_WAIT               0x02 
#define  ACK_INVALID            0xFF             /* ack invalid */


#define COM_RX_SIZE		(1024*3)
#define COM_TX_SIZE		(2048)
#define COM_REV_QUEUE_DEPTH	    5
#define BLOCK_SIZE              4096

#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF
#define READ_ALLBMS_DATA        0x0001
#define WRITE_ALLBMS_DATA       0x8001
#define WRITE_PARTBMS_DATA      0x8002

#define BMS_FIRMWARE_ADDR       0x00001000
#define UPGRADE_FLAG_ADDR       0x00000000


typedef struct _com_rev_data_stru_t
{
	EN_COM_REV_DATA_TYPE        data_type;
	uint16_t			        data_size;
	uint8_t				        *pData;
}ComRevData_t; 

typedef struct _com_send_data_stru_t
{
	EN_COM_REV_DATA_TYPE        data_type;
	uint16_t			        data_size;
	uint8_t				        *pData;
}ComSendData_t;

extern Queue_t	gComRevQueue;
extern uint8_t gBMSFirmwareType[4];
extern ProtocalRequestPacket_t     gstDebReadPacketData;



extern uint8_t garrComTxBuf[COM_TX_SIZE];
extern uint8_t garrComRxBuf[COM_RX_SIZE]; 

bool CommunicationInit(void);
void ComDataProcess(void);
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode);
void UpgradeDataSendPolling(void);
void AllBMSDataSendPolling(uint8_t* p_data_buff );
void WriteBMSDataSendPolling(void);
bool ProtoUnpackJudge(ComRevData_t *p);
uint16_t UartCtrlMSB2LSBU16(uint16_t* input);
void Uart0DebugSendPolling(uint8_t* p_data_buff);
#endif

















