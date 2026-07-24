
#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#include "queue.h"
#include "bsp.h"
#include "dev_communication.h"


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
} ProtocalReadResponsePacket_t;
#pragma pack()


#define READ_REQUEST_FIXED_HEAD_SIZE        (7)
#define READ_RESPONSE_FIXED_HEAD_SIZE       (6)
#define WRITE_REQUEST_FIXED_HEAD_SIZE       (7)
#define WRITE_RESPONSE_FIXED_HEAD_SIZE      (8)


#define  ACK_OK                             (0x00)             /* ack success */
#define  ACK_ERROR                          (0x01)             /* ack error */
#define  ACK_UP_ING                         (0x02)             /* ack up ing */


#define COM_RX_SIZE             (2100)
#define COM_TX_SIZE             (2100)
#define COM_REV_QUEUE_DEPTH     (5)
#define BLOCK_SIZE              (4096)

#define UPGRADE_DATA_PACK_SIZE  (2048)

#define UPGRADE_START               (0x80F0)
#define UPGRADE_BUSY                (0x80F1)
#define UPGRADE_END                 (0x80FF)
                                    
#define MODBUS_ADDR_SET             (0x000D)
#define MODBUS_PACK_GET             (0x000E)
#define PACK_INFO_SEND              (0x000A)
#define CC_ON_CMD_SEND              (0x000B)        /*限流命令下发*/
#define ENDURANCE_TEST_DATA_GET     (0x0010)        /*耐久度测试数据获取*/
#define ENDURANCE_TEST_DATA_CLEAR   (0x0011)        /*耐久度测试数据清除*/
#define UART_DIR_CALI_CMD           (0x0012)        /*串口方向校准命令下发*/
#define SHUTDOWN_CMD                (0x0013)        /*关机命令下发*/

/***** 升级地址及标志 *****/
#define BMS_FIRMWARE_ADDR       (0x00001000)
#define UPGRADE_FLAG_ADDR       (0x00000000)
#define UPGRADE_FLAG            (0xA0)

typedef struct _com_rev_data_stru_t
{
    uint32_t                    uart_peri;
    uint16_t                    data_size;
    uint8_t                     *pData;
} ComRevData_t;


extern Queue_t gComRevQueue;
extern ProtocalReadResponsePacket_t  gstReadResponsePacketData;

void CommunicationInit(void);
uint32_t CheckSumFun(uint8_t *pdata, uint16_t len, uint8_t mode);
bool UpCommonPackProcess(void);
void DownCommonPackProcess(void);
void ShutDownCmdSend(void);
#endif

















