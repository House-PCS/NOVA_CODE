#ifndef __PCS_OP_SURO_H_
#define __PCS_OP_SURO_H_

#include "dev_modbus_h.h"
#include "pcs_op.h"
#include "queue.h"

#define SURO_DEV_ADDR  0x01

// SURO 逆变器系统信息
#define PROTO_UNIFORM_R_SURO_SYS_INFO		0x00FFA013
#define PROTO_UNIFORM_W_SURO_SYS_INFO		0x80FFA014

#pragma pack(2)
typedef struct _suro_base_info_stru_t
{
	uint16_t p00[0x02+0x3A];
	uint16_t p01[0x02+0x12];
	uint16_t p02[0x02+0x40];
	uint16_t p05[0x02+0x3A];
	uint16_t p07[0x02+0x20];
	uint16_t p08[0x02+0x28];
	uint16_t p09[0x02+0x4E];
	uint16_t p10[0x02+0x212];
}SuroInvBaseInfo_t;

#pragma pack()


#pragma pack(1)
typedef struct _dev_suro_set_t
{
	uint16_t reg_addr;		// 寄存器地址(逆变器设置类型)
	uint16_t reg_size;		// 寄存器数量
	uint8_t data[0];		// 寄存器数据
} DevSuroSet_t;
#pragma pack()

extern SuroInvBaseInfo_t *gSuroInvBaseInfo;

int32_t SuroDevCheckReq(Queue_t *queue);

int32_t SuroDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev);

int32_t SuroDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info);

int32_t SuroDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get1, 
								   ModbusRTURevData_t *p_modbus_dev_rev);

int32_t SuroInvSet(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data);

int32_t SuroDataGet(void **data);


#endif
