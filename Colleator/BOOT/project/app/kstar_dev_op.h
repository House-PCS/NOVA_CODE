
#ifndef __KSTAR_DEV_OP_H_
#define __KSTAR_DEV_OP_H_


#include "dev_modbus_h.h"




typedef enum
{
    EN_KSTAR_OP_TYPE_READ_BASE_INFO,
    EN_KSTAR_OP_TYPE_READ_SYS_INFO,
    
}EN_KSTAR_OP_TYPE;

typedef enum
{
    EN_KSTAR_OP_STATUS_IDLE,
    EN_KSTAR_OP_STATUS_SEND,
    EN_KSTAR_OP_STATUS_WAIT_ACK,
    EN_KSTAR_OP_STATUS_ACK_ANALYSIS,
    EN_KSTAR_OP_STATUS_WAIT_GAP,
    
}EN_KSTAR_OP_STATUS;


typedef enum
{
    EN_MODBUS_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_FUNC_CODE_WRITE_MULTI = 0x10
}EN_MODBUS_FUNC_CODE;


typedef enum
{
    EN_MODBUS_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_ERROR_CODE_INVALID_DATA = 0x03,
    EN_MODBUS_ERROR_CODE_SLAVE_BUSY = 0x06,
    EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG = 0xFF
}EN_MODBUS_ERROR_CODE;

#define KSTAR_DEV_OP_QUEUE_DEPTH        20

typedef struct _kstar_dev_operation_stru_t
{
    EN_KSTAR_OP_TYPE    op_type;
    uint8_t             slave_addr;
    uint8_t             func_code;
    uint16_t            reg_addr;
    uint16_t            reg_num;
    uint8_t             *p_data;
}KstarDevOp_t;


#define KSTAR_DEV_ACK_TIME_OUT   2000
#define KSTAR_DEV_POLLING_GAP    500
typedef struct _kstar_op_info_stru_t
{
    EN_KSTAR_OP_STATUS      status;
    uint32_t                op_timing_count;
    bool                    slave_ack_timeout;
    EN_MODBUS_ERROR_CODE    rev_error_code;
    uint8_t                 pack_num;
}KstarOpInfo_t;

#define KSTAR_DEV_ADDR  0x01


void KstarDevOpProcess();
void KstarDevOpInit();







#endif


