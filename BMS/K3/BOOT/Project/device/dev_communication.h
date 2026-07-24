
#ifndef __DEV_COMMUNICATION_H_
#define __DEV_COMMUNICATION_H_

#include "uart.h"
#include "queue.h"
#include "bsp.h"
#include "dev_spiflash.h"
#include "dev_system_power.h"

/* protocal request packet */
#pragma pack(1)
typedef struct  /*包头7字节*/
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

/* protocal response packet */
#pragma pack(1)
typedef struct  /*包头 字节*/
{
    uint8_t  packet_head ;      /* 固定为0xA5 */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint8_t  error_code;        /* error  */
    uint16_t check_sum;         /* 校验和() */
} ProtocalResponsePacket_t;
#pragma pack()

typedef enum
{
    COM_TYPE_UART_DATA,
    
}EN_COM_REV_DATA_TYPE;

#define _COM_RX_SIZE			2100
#define COM_REV_QUEUE_DEPTH		5
#define BLOCK_SIZE          	4096

#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF
#define READ_ALLBMS_DATA        0x0001
#define WRITE_ALLBMS_DATA       0x8001
#define WRITE_PARTBMS_DATA      0x8002

#define BMS_FIRMWARE_ADDR   0x00001000
#define UPGRADE_FLAG_ADDR   0x00000000
#define UPGRADE_FLAG   0xA0

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

#define MODULE_CFG_QUEUE_DEPTH			8
extern Queue_t	gComRevQueue;
extern uint8_t gBMSFirmwareType[4];
extern uint16_t gBMSVerExt[6];
extern uint8_t gBMSFirmwareUpgradeEnd;
bool CommunicationInit(void);
void ComDataProcess(void);
uint32_t CheckSum(uint8_t *pdata, uint16_t len, uint8_t mode);

#endif

















