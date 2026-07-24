
#ifndef __PROTO_UNIFORM_H_
#define __PROTO_UNIFORM_H_

#include "dev_rs485.h"
#include "dev_modbus_s.h"

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
} protocal_request_packet_t;
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
} protocal_response_packet_t;

typedef struct  _protocal_response_stru_t/*包头 字节*/
{
    uint8_t  packet_head ;      /* 固定为0xA5 */
    uint8_t  send_addr;         /* 发送方的地址 */
    uint8_t  recv_addr;         /* 接收方的地址 */
    uint16_t cmd;               /* 不同cmd对应的数据格式不同 */
    uint8_t  error_code;        /* error  */
    uint16_t data_field_len;    /* 读写传输的数据长度，字节为单位 */
}ProtocalReadResponsePacket_t;
#pragma pack()

typedef enum
{
	COM_TYPE_UART_DATA,
	COM_TYPE_UPGRADE_DATA
    
}EN_COM_REV_DATA_TYPE;

#define REQUST_PACKET_HEAD      0x5A
#define RESPONSE_PACKET_HEAD    0xA5

#define SEND_ADDR               0x00 /* BMS */
#define RECV_ADDR               0x02 /* AC */




#define  ACK_OK                 0x00             /* ack success */
#define  ACK_ERROR              0x01             /* ack error */
#define  ACK_INVALID            0xFF             /* ack invalid */


#define COM_RX_SIZE		        2100
#define COM_TX_SIZE		        1024
#define COM_REV_QUEUE_DEPTH	    5
#define BLOCK_SIZE              4096

#define UPGRADE_START           0x80F0
#define UPGRADE_BUSY            0x80F1
#define UPGRADE_END             0x80FF
#define HISTORY_ENERGY          0x000C


#define ARC_FIRMWARE_ADDR       0x00041000     //暂定
#define ARC_UPGRADE_FLAG_ADDR   0x00040000     //暂定
#define UPGRADE_FLAG            0xA0
#define MODULE_CFG_QUEUE_DEPTH  8

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

#pragma pack(1)
typedef struct
{
	bool  							is_update_data;
	uint32_t						overtime;
	ComRevData_t					com_revdata;
}ComRevUpdateData_t; 

#pragma pack()

extern Queue_t	gComRevQueue;
extern uint8_t gBMSFirmwareType[4];
extern ProtocalReadResponsePacket_t  gstReadResponsePacketData;
extern uint8_t garrComTxBuf[COM_TX_SIZE];
extern uint8_t garrComRxBuf[COM_RX_SIZE]; 
extern uint8_t gBMSFirmwareType[4];
extern uint8_t gMPPTFirmwareType[4];
extern uint8_t gARCFirmwareType[4];
extern ComRevUpdateData_t com_revupgradedata;


#define UPPER_CMD_EVENT_READ_SN                    0x00FF0002
#define UPPER_CMD_EVENT_WRITE_SN                   0x80FF0002

#pragma pack(1)
typedef struct _uniform_protocol_head_stru_t
{
	uint32_t				delimiter;
	uint8_t					verion;
	uint8_t					frame_type;
	uint32_t				dst;
	uint32_t				src;
	uint16_t				tunnel_id;
	uint16_t				frame_Length;
	uint16_t				body_checksum;
	uint16_t				head_checksum;
	
	uint32_t				seq_id;
	uint16_t				packet_num;
	uint16_t				packet_index;
	uint8_t					request_body_type;
	uint8_t					reserved;
}UniformProtocolHead_t;
#pragma pack()


void ProtoUniformRevProcess();
bool CommunicationInit(void);
void ComDataProcess(void);
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode);
void DataSendPolling(uint8_t read_responsed);
bool ProtoUnpackJudge(ComRevData_t *p);

void SysRestart(void);
void UpgradeCompletehandle(void);

#endif






