#ifndef __PCS_OP_H_
#define __PCS_OP_H_


#include "dev_modbus_h.h"

typedef enum
{
    EN_DEV_OP_TYPE_READ_TYPE_INFO,
    EN_DEV_OP_TYPE_READ_BASE_INFO,
    EN_DEV_OP_TYPE_READ_SYS_INFO,
    EN_DEV_OP_TYPE_WRITE_BASE_INFO,
    EN_DEV_OP_TYPE_WRITE_SET,
//    EN_DEV_OP_TYPE_WRITE_REMOTE,
}EN_DEV_OP_TYPE;

typedef enum
{
    EN_DEV_OP_STATUS_IDLE,
    EN_DEV_OP_STATUS_SEND,
    EN_DEV_OP_STATUS_WAIT_ACK,
    EN_DEV_OP_STATUS_ACK_ANALYSIS,
    EN_DEV_OP_STATUS_WAIT_GAP,
    
}EN_DEV_OP_STATUS;


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
    EN_DEV_WRITE_COMM_STATUS_IDLE,			// 0
    EN_DEV_WRITE_COMM_STATUS_WAITING_SEND,	// 1
    EN_DEV_WRITE_COMM_STATUS_SENDING,			// 2
    EN_DEV_WRITE_COMM_STATUS_NO_ACK,			// 3
    EN_DEV_WRITE_COMM_STATUS_ACK_ERROR,		// 4
    EN_DEV_WRITE_COMM_STATUS_ACK_OK			// 5
}EN_DEV_COMM_STATUS;

typedef enum
{
	EN_DEV_OP_DEVICE_TYPE_NONE = 0,
	EN_DEV_OP_DEVICE_TYPE_NOVA,			// 自研
	EN_DEV_OP_DEVICE_TYPE_KSTAR,		// 科士达
	EN_DEV_OP_DEVICE_TYPE_MEGAREVO,		// 迈格瑞能
	EN_DEV_OP_DEVICE_TYPE_SURO,			// 硕日

	EN_DEV_OP_DEVICE_TYPE_MAX
} EN_DEV_OP_DEVICE_TYPE;

typedef struct _DevOpCtx_t
{
	int8_t init_flag;
	EN_DEV_OP_DEVICE_TYPE type;
	uint8_t addr;

	int32_t base_info_reg_div_size;
	uint16_t *p_base_info_reg_div[2];
	bool *p_base_info_get_flag;


	int32_t dev_valid_base_reg_addr_size;
	uint16_t *p_dev_valid_base_reg_addr;

	int32_t dev_valid_inv_reg_addr_size;
	uint16_t *p_dev_valid_inv_reg_addr;

	int32_t dev_valid_remote_reg_addr_size;
	uint16_t *p_dev_valid_remote_reg_addr;

} DevOpCtx_t;


#define DEV_OP_QUEUE_DEPTH        50

typedef struct _dev_operation_stru_t
{
    int32_t				debug;
	int32_t				idx;
    int32_t				op_type;
    uint8_t             slave_addr;
    uint8_t             func_code;
    uint16_t            reg_addr;
    uint16_t            reg_num;
    uint8_t             *p_data;
}DevOp_t;


#define DEV_ACK_TIME_OUT   2000
#define DEV_POLLING_GAP    500

typedef struct _op_info_stru_t
{
	int8_t					loop_flag;
    int32_t					status;
    EN_DEV_COMM_STATUS		write_comm_status;
    uint32_t                op_timing_count;
    bool                    slave_ack_timeout;
    EN_MODBUS_ERROR_CODE    rev_error_code;
    uint8_t                 pack_num;
}DevOpInfo_t;

#define DEV_ADDR  0x01

extern Queue_t    gDevOpQueue;
extern DevOpInfo_t *gDevOpInfo;
extern uint8_t  gDevReqPacket[320];
extern int8_t gDevSetting;
extern uint32_t gDevSettingTimecount;

EN_DEV_COMM_STATUS DevOpWriteStatusGet();
void DevOpProcess();
void DevOpInit();

void ModbusReadInput(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);

void ModbusReadRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num);

void ModbusWriteMultiRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_value);

void ModbusWriteSingleRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t value);


int32_t DevBaseSet(uint16_t reg_addr, uint16_t reg_data);

int32_t DevInvSet(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data);

int32_t DevRemoteSet(uint16_t reg_addr, uint16_t reg_data);

EN_DEV_COMM_STATUS DevWriteStatusGet();

/**
 * @brief 获取PCS上报的数据
 *
 * @param info 指向内部待上报内存
 *
 * @return 待上报数据长度
 */
const int32_t DevGetInvSysInfo(void **info, uint32_t *cmd_code);


/**
 * @brief 修改采集器名字为蓝牙MAC
 *
 * @return 
 */
int32_t DevChangeDebugSn();

/**
 * @brief 获取PCS的SN
 *
 * @param sn 出参,待获取的SN地址
 *
 * @return sn总长度
 */
int32_t DevGetPcsSn(uint8_t **sn);

uint16_t DevGetPcsType();

int32_t DevOpProcessStatus();

int32_t DevGetDebugReq(int32_t debug_idx, uint16_t addr, uint16_t reg_num);

#endif
