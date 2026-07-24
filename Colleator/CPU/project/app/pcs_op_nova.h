#ifndef __PCS_OP_NOVA_H_
#define __PCS_OP_NOVA_H_

#include "dev_modbus_h.h"
#include "pcs_op.h"
#include "queue.h"

#define NOVA_DEV_ADDR  0x01

// NOVA 逆变器系统信息
#define PROTO_UNIFORM_R_NOVA_SYS_INFO		0x00FFA015
#define PROTO_UNIFORM_W_NOVA_SYS_INFO		0x80FFA016

#pragma pack(2)
typedef struct _nova_base_info_stru_t
{
	uint16_t p00[0x02+0x25];
	uint16_t p01[0x02+0x29];
	uint16_t p02[0x02+0x0F];
	uint16_t p05[0x02+0x28];
	uint16_t p07[0x02+0x0E];
	uint16_t p08[0x02+0x0C];
	uint16_t p09[0x02+0x05];
	uint16_t p10[0x02+0x13];
	uint16_t p11[0x02+0x04];
	uint16_t p12[0x02+0x12];
	uint16_t p13[0x02+0x1B];
	uint16_t p14[0x02+0x02];
	uint16_t p15[0x02+0x02];
	uint16_t p16[0x02+0x34];
	uint16_t p17[0x02+0x39];
	uint16_t p18[0x02+0x0D];
	uint16_t p19[0x02+0x1C];
	uint16_t p20[0x02+0x20];
	uint16_t p21[0x02+0x1B];
	uint16_t p22[0x02+0x07];
	uint16_t p23[0x02+0x08];
}NovaInvBaseInfo_t;

#pragma pack()


#pragma pack(1)
typedef struct _dev_nova_set_t
{
	uint16_t reg_addr;		// 寄存器地址(逆变器设置类型)
	uint16_t reg_size;		// 寄存器数量
	uint8_t data[0];		// 寄存器数据
} DevNovaSet_t;
#pragma pack()

extern NovaInvBaseInfo_t *gNovaInvBaseInfo;

int32_t NovaDevCheckReq(Queue_t *queue);

int32_t NovaDevCheck(uint16_t reg_addr, ModbusRTURevData_t *modbus_rev);

int32_t NovaDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info);

int32_t NovaDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get1, 
								   ModbusRTURevData_t *p_modbus_dev_rev);

int32_t NovaInvSet(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data);

int32_t NovaInvSetAppend(int32_t idx, int32_t op_type, EN_MODBUS_FUNC_CODE func_code, 
						 uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data);

int32_t NovaDataGetByAddr(uint8_t *data, uint16_t data_len, uint16_t reg_addr, uint16_t reg_len);

int32_t NovaDataGet(void **data);

int32_t NovaDevUpdateWifiStat();

int32_t NovaDevUpdateMonitorSN(const char *sn, int32_t sn_len);

int32_t NovaDbgAuth(const uint8_t *data, int32_t len);

int32_t NovaDbgGetMonitorHardfault(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data);

int32_t NovaDbgClsMonitorTotalFlash(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data);

uint8_t* NovaDbgGetMonitorHardfualtInfo(void);

#endif /* __PCS_OP_NOVA_H_ */
