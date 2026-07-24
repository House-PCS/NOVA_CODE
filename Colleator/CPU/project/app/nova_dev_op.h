#ifndef __NOVA_DEV_OP_H_
#define __NOVA_DEV_OP_H_


#include "dev_modbus_h.h"




typedef enum
{
    EN_NOVA_OP_TYPE_READ_BASE_INFO,
    EN_NOVA_OP_TYPE_READ_SYS_INFO,
    EN_NOVA_OP_TYPE_WRITE_BASE_INFO,
    EN_NOVA_OP_TYPE_WRITE_SET,

	// 升级相关
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_START,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_SEND,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_END,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_RE_SEND,

//    EN_NOVA_OP_TYPE_WRITE_REMOTE,
}EN_NOVA_OP_TYPE;

typedef enum
{
    EN_NOVA_OP_STATUS_IDLE,
    EN_NOVA_OP_STATUS_SEND,
    EN_NOVA_OP_STATUS_WAIT_ACK,
    EN_NOVA_OP_STATUS_ACK_ANALYSIS,
    EN_NOVA_OP_STATUS_WAIT_GAP,

    EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE,
    EN_NOVA_OP_STATUS_WAIT_UPGRADE_DEBUG,
    EN_NOVA_OP_STATUS_WAIT_UPGRADE_DOWNLOAD_TO_FLASH,
    
}EN_NOVA_OP_STATUS;


typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
}EN_MODBUS_FUNC_CODE;


typedef enum
{
    EN_MODBUS_ERROR_CODE_NULL = 0x00,
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
    EN_MODBUS_ERROR_CODE_SLAVE_BUSY = 0x06,
    EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG = 0xFF
}EN_MODBUS_ERROR_CODE;

typedef enum
{
    EN_NOVA_WRITE_COMM_STATUS_IDLE,			// 0
    EN_NOVA_WRITE_COMM_STATUS_WAITING_SEND,	// 1
    EN_NOVA_WRITE_COMM_STATUS_SENDING,			// 2
    EN_NOVA_WRITE_COMM_STATUS_NO_ACK,			// 3
    EN_NOVA_WRITE_COMM_STATUS_ACK_ERROR,		// 4
    EN_NOVA_WRITE_COMM_STATUS_ACK_OK			// 5
}EN_NOVA_COMM_STATUS;


#define NOVA_DEV_OP_QUEUE_DEPTH        20

typedef struct _nova_dev_operation_stru_t
{
    EN_NOVA_OP_TYPE    op_type;
    uint8_t             slave_addr;
    uint8_t             func_code;
    uint16_t            reg_addr;
    uint16_t            reg_num;
    uint8_t             *p_data;
}NovaDevOp_t;


#define NOVA_DEV_ACK_TIME_OUT   2000
#define NOVA_DEV_POLLING_GAP    500
typedef struct _nova_op_info_stru_t
{
    EN_NOVA_OP_STATUS      status;
    EN_NOVA_COMM_STATUS    write_comm_status;
    uint32_t                op_timing_count;
    bool                    slave_ack_timeout;
    EN_MODBUS_ERROR_CODE    rev_error_code;
    uint8_t                 pack_num;
}NovaOpInfo_t;

#define NOVA_DEV_ADDR  0x01


void NovaDevOpProcess();
void NovaDevOpInit();


int32_t NovaBaseSet(uint16_t reg_addr, uint16_t reg_data);
int32_t NovaInvSet(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data);
int32_t NovaRemoteSet(uint16_t reg_addr, uint16_t reg_data);
EN_NOVA_COMM_STATUS NovaWriteStatusGet();



#endif

