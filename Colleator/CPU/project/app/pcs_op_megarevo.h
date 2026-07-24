#ifndef __PCS_P_MEGAREVO_H_
#define __PCS_P_MEGAREVO_H_

#include "dev_modbus_h.h"
#include "pcs_op.h"
#include "queue.h"

#define MEGAREVO_DEV_ADDR  0x01

// megarevo 逆变器系统信息
#define PROTO_UNIFORM_R_MEGAREVO_SYS_INFO		0x00FFA011
#define PROTO_UNIFORM_W_MEGAREVO_SYS_INFO		0x80FFA012

#pragma pack(2)

typedef struct _megarevo_base_info_stru_t
{
	uint16_t manufacturer[0x02+0x09];
	uint16_t version[0x02+0x34];
	uint16_t power[0x02+0x02];
	uint16_t warning[0x02+0x0E];
	uint16_t grid[0x02+0x10];
	uint16_t load[0x02+0x10];
	uint16_t pv[0x02+0x0D];
	uint16_t battery[0x02+0x12];
	uint16_t unit_sim[0x02+0x3B];
	uint16_t pcs[0x02+0x1A];
	uint16_t dsp_debug[0x02+0x0E];
	uint16_t combine_sim[0x02+0x06];
	uint16_t generator_sim[0x02+0x10];
	uint16_t phase3_grid[0x02+0x10];
	uint16_t phase3_load[0x02+0x10];
	uint16_t pack[0x02+0x80];
	uint16_t pack_sum[0x02+0x08];
	uint16_t pack_using[0x02+0x05];
	uint16_t dsp_debug2[0x02+0x0A];
	uint16_t inner_use[0x02+0x3B];
	uint16_t setting[0x02+0x70];
	uint16_t setting2[0x27];
	uint16_t date[0x02+0x10];
	uint16_t inner_setting[0x02+0x2B];
	uint16_t discharge_check[0x02+0x0F];
	uint16_t grid_safe[0x02+0x4F];
	uint16_t nameplate[0x02+0x24];
	uint16_t others_sim[0x02+0x0A];
} MegarevoInvBaseInfo_t;

#pragma pack()


#pragma pack(1)
typedef struct _dev_megarevo_set_t
{
	uint16_t reg_addr;		// 寄存器地址(逆变器设置类型)
	uint16_t reg_size;		// 寄存器数量
	uint8_t data[0];		// 寄存器数据
} DevMegarevoSet_t;
#pragma pack()

extern MegarevoInvBaseInfo_t *gMegarevoInvBaseInfo;

int32_t MegarevoDevCheckReq(Queue_t *queue);

int32_t MegarevoDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev);

int32_t MegarevoDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info);


int32_t MegarevoDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get1, 
								   ModbusRTURevData_t *p_modbus_dev_rev);

int32_t MegarevoInvSet(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data);

int32_t MegarevoDataGet(void **data);

int32_t MegarevoDevDebugReq(Queue_t *queue, int32_t debug_idx, 
							uint16_t addr, uint16_t reg_num);

#endif
