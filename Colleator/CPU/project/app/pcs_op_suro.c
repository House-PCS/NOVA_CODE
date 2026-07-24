#include "pcs_op_suro.h"
#include "sys_info.h"
#include "collector.h"
#include "proto_nova_uniform.h"
#include "dev_fc41d.h"

#define SURO_DEV_ACK_TIME_OUT   2000
#define SURO_DEV_POLLING_GAP    500

typedef enum
{
    EN_SURO_OP_TYPE_READ_TYPE_INFO,
    EN_SURO_OP_TYPE_READ_BASE_INFO,
    EN_SURO_OP_TYPE_READ_SYS_INFO,
    EN_SURO_OP_TYPE_WRITE_BASE_INFO,
    EN_SURO_OP_TYPE_WRITE_SET,
//    EN_SURO_OP_TYPE_WRITE_REMOTE,
}EN_SURO_OP_TYPE;

typedef enum
{
    EN_SURO_OP_STATUS_IDLE,
    EN_SURO_OP_STATUS_SEND,
    EN_SURO_OP_STATUS_WAIT_ACK,
    EN_SURO_OP_STATUS_ACK_ANALYSIS,
    EN_SURO_OP_STATUS_WAIT_GAP,
    
}EN_SURO_OP_STATUS;

/*第一个字段是寄存器起始地址，
 * 第二个字段是寄存器数量
 * 第三个字段是是否已读
  */
static uint16_t _gSuroBaseInfoRegDiv[][5] = {
	{0x10,		0x11, 0, 0x10,	 0x01}, // p00-0基本信息1
	{0x21,		0x14, 0, 0x21,	 0x01}, // p00-1基本信息2
	{0x35,		0x15, 0, 0x35,	 0x01}, // p00-1基本信息3
	{0x100, 	0x12, 0, 0x100,  0x12}, // P01直流数据区
	{0x200,		0x20, 0, 0x200,	 0x04}, // P02逆变器数据区1
	{0x220,		0x20, 0, 0x220,	 0x04}, // P02逆变器数据区2
	{0xE000,	0x20, 0, 0xE000, 0x3A}, // P05电池相关参数设置区1
	{0xE020,	0x1A, 0, 0xE020, 0x3A}, // P05电池相关参数设置区2
	{0xE200, 	0x20, 0, 0xE200, 0x20}, // p07逆变器参数设置信息
	{0xE400, 	0x20, 0, 0xE400, 0x28}, // p08逆变器并网参数设置信息1
	{0xE420, 	0x08, 0, 0xE420, 0x28}, // p08逆变器并网参数设置信息2
	{0xF000, 	0x20, 0, 0xF000, 0x4E}, // p09电量统计历史数据1
	{0xF020, 	0x20, 0, 0xF020, 0x4E}, // p09电量统计历史数据2
	{0xF040, 	0x0E, 0, 0xF040, 0x4E}, // p09电量统计历史数据3
	{0xF800, 	0x20, 0, 0xF800, 0xF0}, // p10故障历史记录1
	{0xF820, 	0x20, 0, 0xF820, 0xF0}, // p10故障历史记录2
	{0xF840, 	0x20, 0, 0xF840, 0xF0}, // p10故障历史记录3
	{0xF860, 	0x20, 0, 0xF860, 0xF0}, // p10故障历史记录4
	{0xF880, 	0x20, 0, 0xF880, 0xF0}, // p10故障历史记录5
	{0xF8A0, 	0x20, 0, 0xF8A0, 0xF0}, // p10故障历史记录6
	{0xF8C0, 	0x20, 0, 0xF8C0, 0xF0}, // p10故障历史记录7
	{0xF8E0, 	0x20, 0, 0xF8E0, 0xF0}, // p10故障历史记录8
	{0xF900, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录9
	{0xF920, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录10
	{0xF940, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录11
	{0xF960, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录12
	{0xF980, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录13
	{0xF9A0, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录14
	{0xF9C0, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录15
	{0xF9E0, 	0x20, 0, 0xF8F0, 0xF0}, // p10故障历史记录16
	{0xFA00, 	0x12, 0, 0xF8F0, 0xF0}, // p10故障历史记录17
};


#pragma pack(2)
typedef struct _suro_base_info_inner_stru_t
{
	uint16_t p00[0x3A];
	uint16_t p01[0x12];
	uint16_t p02[0x40];
	uint16_t p05[0x3A];
	uint16_t p07[0x20];
	uint16_t p08[0x28];
	uint16_t p09[0x4E];
	uint16_t p10[0x212];
}SuroInvBaseInfoInner_t;
#pragma pack()

static SuroInvBaseInfoInner_t _gSuroInvBaseInfoInner = {0};

static int32_t _SuroInvBaseInfoInnerUpdate(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	int32_t i = 0, j = sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0]),
			pos = 0;
	uint16_t byte_num = modbus_rev->p_data[2], byte_num2 = 0;
	uint16_t *paddr = (uint16_t *)&_gSuroInvBaseInfoInner;
	for (i = 0; i < j; ++i)
	{
		paddr = (uint16_t *)&_gSuroInvBaseInfoInner;
		paddr += pos;
		if (_gSuroBaseInfoRegDiv[i][2] == 2)
		{
			memset(paddr, 0, _gSuroBaseInfoRegDiv[i][1]);
		}
		else if (_gSuroBaseInfoRegDiv[i][2] == 1)
		{
			if (i == idx)
			{
				if (_gSuroBaseInfoRegDiv[idx][0] < _gSuroBaseInfoRegDiv[idx][3])
				{
					paddr += (_gSuroBaseInfoRegDiv[idx][3]
							-_gSuroBaseInfoRegDiv[idx][0]);
				}

				byte_num2 = _gSuroBaseInfoRegDiv[idx][1] 
								- (_gSuroBaseInfoRegDiv[idx][3] 
								- _gSuroBaseInfoRegDiv[idx][0]);
				for(uint16_t i = 0; i < byte_num/2 && i < byte_num2; ++i)
				{
					paddr[i] = (modbus_rev->p_data[3+i*2] << 8) 
						| modbus_rev->p_data[3+i*2+1];
				}
			}
		}

		pos += _gSuroBaseInfoRegDiv[i][1];
	}

	return 0;
}


static uint32_t _gSuroUpdateData = 0;
static uint8_t _gSuroWriteCmdNum;//当前待写操作设备的命令数量

static uint16_t  _gSuroBaseInfoCache[800];
static uint16_t _gSuroInvSet[88];
static uint32_t _gSuroDataLen = 0;


// 硕日设备类型检测需要的寄存器
uint16_t gSuroDevCheckRegSize = 0;
uint16_t gSuroDevCheckReg[][3] = 
{
	{0x0B,	0x01, 0},	// 型号
	{0x1B, 	0x01, 0},	// 机型
	{0x35, 	0x14, 0},	// SN
};
static uint8_t _gSuroDevCheckRet[sizeof(uint16_t) * 12];

static int32_t _TestAddr(Queue_t *queue)
{
	DevOp_t dev_op;

	dev_op.slave_addr = SURO_DEV_ADDR;
	dev_op.op_type = EN_SURO_OP_TYPE_READ_TYPE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	QueueClear(queue);

	uint16_t reg[][3] = {
		{0X20C,	0x03, 0},	// debug
	};
	static int32_t j = 0;

	if (j == 0)
	{
		j = reg[0][1];
	}

	dev_op.reg_addr = reg[0][0]; // 逆变器型号
	dev_op.reg_num = j;
	QueuePut(queue, &dev_op);
	++j;

	return -100;
}


int32_t SuroDevCheckReq(Queue_t *queue)
{
	gSuroInvBaseInfo = (SuroInvBaseInfo_t *)gCommonBaseInfo;
#ifdef COLLECTOR_PCS_TYPE_DEBUG
	return _TestAddr(queue);
#endif 

	DevOp_t dev_op;

	dev_op.slave_addr = SURO_DEV_ADDR;
	dev_op.op_type = EN_SURO_OP_TYPE_READ_TYPE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	gSuroDevCheckRegSize = 0;
	QueueClear(queue);
	for (int32_t i = 0; i < sizeof(gSuroDevCheckReg)/sizeof(gSuroDevCheckReg[0]); ++i)
	{
		dev_op.idx = i;
		dev_op.reg_addr = gSuroDevCheckReg[i][0]; // 逆变器型号
		dev_op.reg_num = gSuroDevCheckReg[i][1];
		QueuePut(queue, &dev_op);
		++gSuroDevCheckRegSize;
	}

	return 0;
}

static int32_t _SuroBaseInfoInit()
{
	// update gCommonBaseInfo
	gSuroInvBaseInfo = (SuroInvBaseInfo_t *)gCommonBaseInfo;

	int32_t i = 0, j = sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0]), 
			pos = 0; 
	uint16_t *paddr = (uint16_t *)gSuroInvBaseInfo;
	uint16_t *paddr1 = NULL;

	for (i = 0; i < j; ++i)
	{
		paddr1 = paddr + pos;
		if (i == 0)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+2][1];
			pos += 0x02;
		}
		else if (i == 3)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			pos += 0x02;
		}
		else if (i == 4)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			pos += 0x02;
		}
		else if (i == 6)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			pos += 0x02;
		}
		else if (i == 8)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			pos += 0x02;
		}
		else if (i == 9)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			pos += 0x02;
		}
		else if (i == 11)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+2][1];
			pos += 0x02;
		}
		else if (i == 14)
		{
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+2][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+3][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+4][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+5][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+6][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+7][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+8][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+9][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+10][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+11][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+12][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+13][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+14][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+15][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+16][1];
			pos += 0x02;
		}

		pos += _gSuroBaseInfoRegDiv[i][1];
	}

	gCollectorDevInfo.pcs_info_updated = 0;

	return 0;
}

int32_t SuroDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev)
{
	int32_t addr = 0;
	uint8_t index = 0;
	uint8_t *pdata = _gSuroDevCheckRet;
	uint16_t byte_num = modbus_rev->p_data[2];

	for(index = 0; index < 3 && index < sizeof(gSuroDevCheckReg)/sizeof(gSuroDevCheckReg[0]); 
		index++)
	{
		if (index > 0)
		{
			addr += gSuroDevCheckReg[index-1][1] * sizeof(uint16_t);
		}
		if(gSuroDevCheckReg[index][0] == dev_op->reg_addr)
		{
			pdata += addr;
			memcpy(pdata, &modbus_rev->p_data[3], byte_num);
			gSuroDevCheckReg[index][2] = 1;
			break;
		}
	}

	if(index == sizeof(gSuroDevCheckReg)/sizeof(gSuroDevCheckReg[0]))
	{
		return -1;
	}

	if (gSuroDevCheckReg[0][2] == 0 || gSuroDevCheckReg[1][2] == 0 
		|| gSuroDevCheckReg[2][2] == 0)
	{
		// 数据不全
		return -2;
	}

	// 数据预处理
	pdata = _gSuroDevCheckRet;
	uint16_t product_type = (uint16_t)pdata[1];
	pdata += 2;
	uint16_t dev_type = (uint16_t)pdata[1];
	pdata += 2;
	addr = gSuroDevCheckReg[2][1] * sizeof(uint16_t);
	int32_t j = 0;
	for (index = 0; index < addr; index+=2)
	{
		pdata[j++] = pdata[index+1];
	}
	pdata[j] = 0;

	// check 合法性
	if (product_type == 0x04
		&& (pdata[0] == 'S' || pdata[0] == 's')
		&& (pdata[1] == 'R' || pdata[1] == 'r')
		&& pdata[2] == '-')
	{
		memcpy(gCollectorDevInfo.modelid, (uint8_t *)&gCollectorSubDevMap[5].modelid, 
			   sizeof(uint16_t));
		memcpy(gCollectorDevInfo.sn, (uint8_t *)&_gSuroDevCheckRet[4], 
			   gSuroDevCheckReg[2][1] * sizeof(uint8_t));
		gCollector->pcs_modelid = gCollectorSubDevMap[5].modelid;

		_SuroBaseInfoInit();
		return 0;
	}

	return -3;
}

int32_t SuroDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info)
{
	DevOp_t dev_op;

	dev_op.slave_addr = SURO_DEV_ADDR;

	/*base info*/
	dev_op.op_type = EN_SURO_OP_TYPE_READ_BASE_INFO;
	static uint32_t i = 0;
	uint32_t j = sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0]);

	if (i > j)
	{
		// 第二次轮询，可以开启上报
		++gCollectorDevInfo.pcs_info_updated;
	}

	p_dev_op_info->loop_flag = 0;
	for(;; ++i)
	{
		dev_op.idx = i%j;
		dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;
		dev_op.reg_addr = _gSuroBaseInfoRegDiv[i%j][3];
		dev_op.reg_num = _gSuroBaseInfoRegDiv[i%j][1] - (_gSuroBaseInfoRegDiv[i%j][3] - _gSuroBaseInfoRegDiv[i%j][0]);

		if (!QueuePut(queue, &dev_op))
		{
			break;
		}
		if (i > 0 && (i % j == 0))
		{
			p_dev_op_info->loop_flag = 1;
			++i;
			break;
		}
	}

	return 0;
}


/**
  * @brief  Suro base info read.
  * @param  modbus format
  * @return NULL
  * @note   寄存器较多
  */
static void _SuroReadBaseInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadInput(gDevReqPacket, SURO_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Suro sys info read.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _SuroReadSysInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadRemain(gDevReqPacket, SURO_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Suro Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _SuroWriteSetSingle(uint16_t reg_addr, uint16_t reg_data)
{
    ModbusWriteSingleRemain(gDevReqPacket, SURO_DEV_ADDR, reg_addr, reg_data);
}

/**
  * @brief  Suro Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _SuroWriteSetMulti(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    ModbusWriteMultiRemain(gDevReqPacket, SURO_DEV_ADDR, reg_addr, reg_num, p_reg_data);
}

/**
  * @brief  Suro base info copy to gSuroInvBaseInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static int32_t _SuroBaseInfoUpdate(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	return _SuroInvBaseInfoInnerUpdate(idx, modbus_rev);
}

int32_t SuroDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get1, 
								   ModbusRTURevData_t *p_modbus_dev_rev)
{
    static DevOp_t *p_dev_op_get;
	switch(p_op_info->status)
	{
		case EN_SURO_OP_STATUS_IDLE:
			{
				/*IDLE模式下不接受从机发的数据*/
				QueueClear(modbus_queue);
				/*检测gSuroOpQueue队列，判断是否有需要操作从机的命令*/
				if(QueueGet(op_queue, (void *)&p_dev_op_get) == true)
				{
					switch(p_dev_op_get->op_type)
					{
					case EN_SURO_OP_TYPE_READ_TYPE_INFO:
					{
						_SuroReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_DEV_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						break;
					}
					case EN_SURO_OP_TYPE_READ_BASE_INFO:
					{
						if (p_dev_op_get->idx >= 0 && (p_dev_op_get->idx 
							< sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0])))
						{
							if (p_dev_op_get->idx == 2)
							{
								int32_t yy = 0;
								yy = 1;
							}
							_gSuroBaseInfoRegDiv[p_dev_op_get->idx][2] = 0;
						}
						_SuroReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_SURO_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();

						break;
					}
					case EN_SURO_OP_TYPE_WRITE_SET:
					{
						if (p_dev_op_get->func_code == EN_MODBUS_FUNC_CODE_WRITE_MULTI)
						{
							_SuroWriteSetMulti(p_dev_op_get->reg_addr, 
												   p_dev_op_get->reg_num, 
												   p_dev_op_get->p_data);
						}
						else 
						{
							// 为了兼容函数，需要将参数转换为小端序
							uint16_t d = (uint16_t)(*(uint16_t*)(p_dev_op_get->p_data));
							d = p_dev_op_get->p_data[0] >> 8 | p_dev_op_get->p_data[1];
							_SuroWriteSetSingle(p_dev_op_get->reg_addr, d);
						}
						p_op_info->status = EN_SURO_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
							_gSuroWriteCmdNum--;
						}
						break;
					}
					default:break;
					}
				}
				else 
				{
//					if (p_dev_op_ctx->init_flag == 1)
//					{
//						p_dev_op_ctx->init_flag = 0;
//					}
				}
				break;
			}
		case EN_SURO_OP_STATUS_WAIT_ACK:
			{
				if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
				{
					uint8_t rev_slave_addr = p_modbus_dev_rev->p_data[0];
					uint8_t rev_func_code = p_modbus_dev_rev->p_data[1];

					if (gSuroDevCheckRegSize > 0)
					{
						--gSuroDevCheckRegSize;
					}

					if(rev_slave_addr != p_dev_op_get->slave_addr)
					{
						break;//不是给本设备的包
					}
					p_op_info->slave_ack_timeout = false;
					if((rev_func_code & 0x80) == 0x80)//逆变器设备通信异常
					{
						p_op_info->rev_error_code = p_modbus_dev_rev->p_data[2];//rev_func_code & 0x7F;
						p_op_info->status = EN_SURO_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
						gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_ERROR_CODE;
						p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_ERROR;
						break;
					}
					if(rev_func_code != p_dev_op_get->func_code)
					{
						break;//功能码不匹配
					}
					gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_OK;
					p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_NULL;
					uint16_t byte_num = 0;
					switch(p_dev_op_get->op_type)
					{
					case EN_SURO_OP_TYPE_READ_TYPE_INFO:
					{
                        byte_num = p_modbus_dev_rev->p_data[2];
                        
                        if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
                        {
                            p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
                            p_op_info->status = EN_DEV_OP_STATUS_WAIT_GAP;
                            p_op_info->op_timing_count = tim_get_systime();
                        }
                        else//数据OK
                        {
							if (!SuroDevCheck(p_dev_op_get, p_modbus_dev_rev))
							{
								p_dev_op_ctx->init_flag = 2;
							}
                            p_op_info->status = EN_DEV_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();
						}
						break;
					}
					case EN_SURO_OP_TYPE_READ_BASE_INFO:
					{
						uint16_t byte_num = p_modbus_dev_rev->p_data[2];

						if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
						{
							p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
							p_op_info->status = EN_SURO_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();
						}
						else//数据OK
						{
							uint8_t index = 0;
							for(index = 0; index < sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0]); index++)
							{
								if(_gSuroBaseInfoRegDiv[index][0] == p_dev_op_get->reg_addr)
								{
									break;
								}
							}
							if(index == sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0]))
							{
								p_op_info->status = EN_SURO_OP_STATUS_WAIT_GAP;
								p_op_info->op_timing_count = tim_get_systime();
								break;
							}

							p_dev_op_get->idx = index;
							_gSuroBaseInfoRegDiv[index][2] = 1;

							_SuroBaseInfoUpdate(p_dev_op_get->idx, p_modbus_dev_rev);

							p_op_info->status = EN_SURO_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();

						}
						break;
					}
					case EN_SURO_OP_TYPE_WRITE_SET:
					{
						p_op_info->status = EN_SURO_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
						{
							if(_gSuroWriteCmdNum == 0)
							{
								p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_OK;
								gDevSetting = 0;
							}
						}
						break;
					}
					default:break;
					}
				}
				else
				{
					if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > SURO_DEV_ACK_TIME_OUT)
					{
						if (gSuroDevCheckRegSize > 0)
						{
							--gSuroDevCheckRegSize;
						}

						p_op_info->slave_ack_timeout = true;
						p_op_info->status = EN_SURO_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
						gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_FAIL;
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_NO_ACK;
						}
					}
				}
				break;
			}
		case EN_SURO_OP_STATUS_ACK_ANALYSIS:
		{
			p_op_info->status = EN_SURO_OP_STATUS_IDLE;
			break;
		}
        case EN_SURO_OP_STATUS_WAIT_GAP:
        {
            if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > SURO_DEV_POLLING_GAP)
            {
//				if (p_dev_op_ctx->init_flag == 1 && gSuroDevCheckRegSize == 0)
//				{
//					p_dev_op_ctx->init_flag = 0;
//				}
                p_op_info->status = EN_SURO_OP_STATUS_IDLE;
            }
            break;
        }
	}

	return 0;
}

/**
  * @brief  操作逆变器设置寄存器接口
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
int32_t SuroInvSet(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data)
{
    DevOp_t dev_op;
    uint16_t index;
    uint8_t *p_origin = p_reg_data;
    
    /*大小端转换*/
    for(uint16_t i = 0; i < reg_num; i++)
    {
        _gSuroInvSet[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    
    
    dev_op.op_type = EN_SURO_OP_TYPE_WRITE_SET;
    dev_op.slave_addr = SURO_DEV_ADDR;
    dev_op.func_code = func_code;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = reg_num;
    dev_op.p_data = (uint8_t *)&_gSuroInvSet[0];
    if(QueuePut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
		++_gSuroWriteCmdNum;
		gDevSetting = 1;
    }
	return 0;
}

int32_t SuroDataGet(void **data)
{
	gSuroInvBaseInfo = (SuroInvBaseInfo_t *)gCommonBaseInfo;
	int32_t i = 0, 
			j = sizeof(_gSuroBaseInfoRegDiv)/sizeof(_gSuroBaseInfoRegDiv[0]), 
			k = 0, 
			pos = 0, pos_start = 0,
			total_len = 0;
	uint16_t *paddr = (uint16_t *)gSuroInvBaseInfo;
	uint16_t *paddr1 = NULL;
	uint16_t *paddr2 = (uint16_t *)&_gSuroInvBaseInfoInner;

	memset(paddr, 0, sizeof(gCommonBaseInfo));

	for (i = 0; i < j; ++i)
	{
		if (_gSuroBaseInfoRegDiv[i][2] == 2)
		{
			//paddr2 += _gSuroBaseInfoRegDiv[i][1];
			continue;
		}

		paddr1 = paddr + pos;

		if (i == 0)
		{
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+2][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 3)
		{
			k = 1;
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 4)
		{
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 6)
		{
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 8)
		{
			k = 1;
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 9)
		{
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 11)
		{
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+2][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else if (i == 14)
		{
			pos_start = pos;
			paddr1[0] = _gSuroBaseInfoRegDiv[i][0];
			paddr1[1] = _gSuroBaseInfoRegDiv[i][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+1][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+2][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+3][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+4][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+5][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+6][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+7][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+8][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+9][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+10][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+11][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+12][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+13][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+14][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+15][1];
			paddr1[1] += _gSuroBaseInfoRegDiv[i+16][1];
			total_len += 0x04+paddr1[1]*2;
			memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
			paddr2 += paddr1[1];
			pos += 0x02 + paddr1[1];
			paddr1 += 2;
		}
		else 
		{
			continue;
		}
		//pos += _gSuroBaseInfoRegDiv[i][1];
		if (k)
		{
			pos_start = pos;
		}
	}
	_gSuroDataLen = pos;
	_gSuroDataLen = total_len;
	*data = (void *)gSuroInvBaseInfo;
	return _gSuroDataLen;
}

