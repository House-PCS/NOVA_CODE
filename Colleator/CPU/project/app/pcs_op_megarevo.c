#include "pcs_op_megarevo.h"
#include "sys_info.h"
#include "collector.h"
#include "proto_nova_uniform.h"
#include "dev_fc41d.h"

#define MEGAREVO_DEV_ACK_TIME_OUT   2000
#define MEGAREVO_DEV_POLLING_GAP    500

typedef enum
{
    EN_MEGAREVO_OP_TYPE_READ_TYPE_INFO,
    EN_MEGAREVO_OP_TYPE_READ_BASE_INFO,
    EN_MEGAREVO_OP_TYPE_READ_SYS_INFO,
    EN_MEGAREVO_OP_TYPE_WRITE_SET,
//    EN_MEGAREVO_OP_TYPE_WRITE_REMOTE,
}EN_MEGAREVO_OP_TYPE;

typedef enum
{
    EN_MEGAREVO_OP_STATUS_IDLE,
    EN_MEGAREVO_OP_STATUS_SEND,
    EN_MEGAREVO_OP_STATUS_WAIT_ACK,
    EN_MEGAREVO_OP_STATUS_ACK_ANALYSIS,
    EN_MEGAREVO_OP_STATUS_WAIT_GAP,
    
}EN_MEGAREVO_OP_STATUS;


/*第一个字段是寄存器起始地址，
 * 第二个字段是寄存器数量
 * 第三个字段是是否已读
 * 第四个字段是实际开始寄存器地址
 * 第五个字段是实际能读寄存器数量
  */
static uint16_t _gMegarevoBaseInfoRegDiv[][5] = {
	{0x3000, 0x09, 0, 0x3000, 0x09}, // X 厂商信息信息
	{0x1213, 0x34, 0, 0x1214, 0x09}, // SN, 各个硬件软件版本信息
	{0x1404, 0x02, 0, 0x1404, 0x09}, // X 开关按钮状态
	{0x3100, 0x0E, 0, 0x3100, 0x09}, // 告警字、状态字
	{0x3110, 0x10, 0, 0x3110, 0x09}, // 电网信息
	{0x3120, 0x10, 0, 0x3120, 0x09}, // 负载侧信息
	{0x3130, 0x0D, 0, 0x3130, 0x09}, // PV侧信息
	{0x313D, 0x12, 0, 0x313D, 0x09}, // 电池侧信息
	{0x3150, 0x3B, 0, 0x3150, 0x09}, // 整机模拟量
	{0x3190, 0x1A, 0, 0x3190, 0x09}, // 逆变侧信息
	{0x31B0, 0x0E, 0, 0x31B0, 0x09}, // DSP调试信息
	{0x31C0, 0x06, 0, 0x31C0, 0x09}, // X 并机模拟数据
	{0x31D0, 0x10, 0, 0x31D0, 0x09}, // 发电机模拟数据
	{0x31E0, 0x10, 0, 0x31E0, 0x09}, // X 组三相电网模拟数据（单相机和美标机）
	{0x31F0, 0x10, 0, 0x31F0, 0x09}, // X 组三相负载模拟数据（单相机和美标机）
	{0x3210, 0x80, 0, 0x3210, 0x09}, // X PACK 1-16数据
	{0x3290, 0x08, 0, 0x3290, 0x09}, // X PACK 总信息数据
	{0x3299, 0x05, 0, 0x3299, 0x09}, // X PACK 使用信息数据
	{0x32A0, 0x0A, 0, 0x32A0, 0x09}, // X DSP调试信息2
	{0x3300, 0x3B, 0, 0x3300, 0x09}, // X 设备内部使用
	{0x3400, 0x70, 0, 0x3400, 0x09}, // 1设置的参数信息
	{0x3470, 0x27, 0, 0x3470, 0x09}, // 2设置的参数信息
	{0x3500, 0x10, 0, 0x3500, 0x09}, // 时间设置信息
	{0x3602, 0x2B, 0, 0x3602, 0x09}, // 内部设置信息
	{0x3800, 0x0F, 0, 0x3800, 0x09}, // X 拉祜检测信息
	{0x2200, 0x4F, 0, 0x2200, 0x09}, // 电网安规参数
	{0x2300, 0x24, 0, 0x2300, 0x09}, // X 铭牌数据
	{0x2282, 0x0A, 0, 0x2282, 0x09}, // 其他模拟数据
};

#pragma pack(2)
typedef struct _megarevo_base_info_inner_stru_t
{
	uint16_t manufacturer[0x09];
	uint16_t version[0x34];
	uint16_t power[0x02];
	uint16_t warning[0x0E];
	uint16_t grid[0x10];
	uint16_t load[0x10];
	uint16_t pv[0x0D];
	uint16_t battery[0x12];
	uint16_t unit_sim[0x3B];
	uint16_t pcs[0x1A];
	uint16_t dsp_debug[0x0E];
	uint16_t combine_sim[0x06];
	uint16_t generator_sim[0x10];
	uint16_t phase3_grid[0x10];
	uint16_t phase3_load[0x10];
	uint16_t pack[0x80];
	uint16_t pack_sum[0x08];
	uint16_t pack_using[0x05];
	uint16_t dsp_debug2[0x0A];
	uint16_t inner_use[0x3B];
	uint16_t setting[0x70];
	uint16_t setting2[0x27];
	uint16_t date[0x10];
	uint16_t inner_setting[0x2B];
	uint16_t discharge_check[0x0F];
	uint16_t grid_safe[0x4F];
	uint16_t nameplate[0x24];
	uint16_t others_sim[0x0A];
} MegarevoInvBaseInfoInner_t;
#pragma pack()

static MegarevoInvBaseInfoInner_t _gMegarevoInvBaseInfoInner = {0};

static int32_t _MegarevoInvBaseInfoInnerUpdate(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	int32_t i = 0, j = sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]),
			pos = 0;
	uint16_t byte_num = modbus_rev->p_data[2], byte_num2 = 0;
	uint16_t *paddr = (uint16_t *)&_gMegarevoInvBaseInfoInner;
	for (i = 0; i < j; ++i)
	{
		paddr = (uint16_t *)&_gMegarevoInvBaseInfoInner;
		paddr += pos;
		if (_gMegarevoBaseInfoRegDiv[i][2] == 2)
		{
			memset(paddr, 0, _gMegarevoBaseInfoRegDiv[i][1]);
		}
		else if (_gMegarevoBaseInfoRegDiv[i][2] == 1)
		{
			if (i == idx)
			{
				if (_gMegarevoBaseInfoRegDiv[idx][0] < _gMegarevoBaseInfoRegDiv[idx][3])
				{
					paddr += (_gMegarevoBaseInfoRegDiv[idx][3]
							-_gMegarevoBaseInfoRegDiv[idx][0]);
				}

				byte_num2 = _gMegarevoBaseInfoRegDiv[idx][1] 
								- (_gMegarevoBaseInfoRegDiv[idx][3] 
								- _gMegarevoBaseInfoRegDiv[idx][0]);
				for(uint16_t i = 0; i < byte_num/2 && i < byte_num2; ++i)
				{
					paddr[i] = (modbus_rev->p_data[3+i*2] << 8) 
						| modbus_rev->p_data[3+i*2+1];
				}
			}
		}

		pos += _gMegarevoBaseInfoRegDiv[i][1];
	}

	return 0;
}


static uint32_t _gMegarevoUpdateData = 0;
static uint8_t _gMegarevoWriteCmdNum = 0;//当前待写操作设备的命令数量
static uint16_t  _gMegarevoBaseInfoCache[800];
static uint16_t _gMegarevoInvSet[88];

static uint32_t _gMegarevoDataLen = 0;


// Megarevo设备类型检测需要的寄存器
uint16_t gMegarevoDevCheckRegSize = 0;
uint16_t gMegarevoDevCheckReg[][3] = {
	{0x121C, 0x0A, 0},	// 型号
	{0x1234, 0x06, 0},	// SN
};
static uint8_t _gMegarevoDevCheckRet[sizeof(uint16_t) * 16];

int32_t _MegarevoSetTest()
{
	extern DevFc41d_t *gFc41d;
	extern int32_t _DevFc41dNovaCmd(DevFc41d_t *fc41d, 
										   int32_t upload_type,
										   UniformProtocolHead_t *uniform,
										   UniformProtocolBodyHead_t *body);
	uint8_t data[sizeof(UniformProtocolHead_t)+100] = { 
	};
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)data;
	uniform->body.tlv.data[0] = 0x10;
	uniform->body.tlv.data[1] = 0x31;
	uniform->body.tlv.data[2] = 0x01;
	uniform->body.tlv.data[3] = 0x00;
	uniform->body.tlv.data[4] = 0x8F;
	uniform->body.tlv.data[5] = 0x08;
	uniform->body.tlv.cmd = PROTO_UNIFORM_W_MEGAREVO_SYS_INFO;
	uniform->body.tlv.data_len = sizeof(data);
	_DevFc41dNovaCmd(gFc41d, 0, uniform, &uniform->body);

	return -1;
}

static int32_t _MegarevoSetTest2()
{
	const char sn[] = "1234567890";
	memcpy(gCollectorDevInfo.sn, (uint8_t *)sn, sizeof(sn));
	gCollector->pcs_modelid = gCollectorSubDevMap[3].modelid;

	uint32_t cmd_code = 0;
	uint8_t *mqtt_info = NULL;
	int32_t mqtt_len = DevGetInvSysInfo((void *)&mqtt_info, &cmd_code);
	return 0;
}

static int32_t _TestAddr(Queue_t *queue)
{
	DevOp_t dev_op;

	dev_op.slave_addr = MEGAREVO_DEV_ADDR;
	dev_op.op_type = EN_MEGAREVO_OP_TYPE_READ_TYPE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	QueueClear(queue);

	uint16_t reg[][3] = {
		{0x3100, 0x05, 0}, // X PACK 1-16数据
	};
	static int32_t j = 0;

	if (j == 0)
	{
		j = reg[0][1];
	}
	dev_op.reg_addr = reg[0][0]; // 逆变器型号
	dev_op.reg_num = j++;
	QueuePut(queue, &dev_op);

	return -100;
}

int32_t MegarevoDevDebugReq(Queue_t *queue, int32_t debug_idx, 
							uint16_t addr, uint16_t reg_num)
{

	memset(&gPcsDevDebugModebusInfo[0], 0, sizeof(gPcsDevDebugModebusInfo));
	if (debug_idx == 0)
	{
		return -1;
	}

	DevOp_t dev_op = {0};
	dev_op.debug = debug_idx;
	dev_op.slave_addr = MEGAREVO_DEV_ADDR;
	dev_op.op_type = EN_MEGAREVO_OP_TYPE_READ_BASE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	QueueClear(queue);

	dev_op.reg_addr = addr;
	dev_op.reg_num =reg_num;
	QueuePut(queue, &dev_op);

	return 0;
}



int32_t MegarevoDevCheckReq(Queue_t *queue)
{
	gMegarevoInvBaseInfo = (MegarevoInvBaseInfo_t *)gCommonBaseInfo;
#ifdef COLLECTOR_PCS_TYPE_DEBUG
	// return _MegarevoSetTest();
	// return _MegarevoSetTest2();
	return _TestAddr(queue);
#endif 

	DevOp_t dev_op = {0};

	dev_op.debug = 0;
	dev_op.slave_addr = MEGAREVO_DEV_ADDR;
	dev_op.op_type = EN_MEGAREVO_OP_TYPE_READ_TYPE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	gMegarevoDevCheckRegSize = 0;
	QueueClear(queue);
	for (int32_t i = 0; 
		 i < sizeof(gMegarevoDevCheckReg)/sizeof(gMegarevoDevCheckReg[0]); ++i)
	{
		dev_op.idx = i;
		dev_op.reg_addr = gMegarevoDevCheckReg[i][0]; // 逆变器型号
		dev_op.reg_num = gMegarevoDevCheckReg[i][1]; // 适配迈格瑞能读取bug，多读一个reg
		QueuePut(queue, &dev_op);
		++gMegarevoDevCheckRegSize;
	}
	return 0;
}

static int32_t _MegarevoBaseInfoInit()
{
	// update gCommonBaseInfo
	gMegarevoInvBaseInfo = (MegarevoInvBaseInfo_t *)gCommonBaseInfo;

	int32_t i = 0, j = sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]), 
			pos = 0; 
	uint16_t *paddr = (uint16_t *)gMegarevoInvBaseInfo;
	uint16_t *paddr1 = NULL;

	for (i = 0; i < j; ++i)
	{
		paddr1 = paddr + pos;
		if (i != 21)
		{
			paddr1[0] = _gMegarevoBaseInfoRegDiv[i][0];
			paddr1[1] = _gMegarevoBaseInfoRegDiv[i][1];
			if (i == 20)
			{
				paddr1[1] += _gMegarevoBaseInfoRegDiv[i+1][1];
			}
			pos += 0x02;
		}

		pos += _gMegarevoBaseInfoRegDiv[i][1];
	}

	gCollectorDevInfo.pcs_info_updated = 0;

	return 0;
}

int32_t MegarevoDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev)
{
	int32_t addr = 0;
	uint8_t index = 0;
	uint8_t *pdata = _gMegarevoDevCheckRet;
	uint16_t byte_num = modbus_rev->p_data[2];

	for(index = 0; index < 2 
		&& index < sizeof(gMegarevoDevCheckReg)/sizeof(gMegarevoDevCheckReg[0]); index++)
	{
		if (index > 0)
		{
			addr += gMegarevoDevCheckReg[index-1][1] * sizeof(uint16_t);
		}
		if(gMegarevoDevCheckReg[index][0] == dev_op->reg_addr)
		{
			pdata += addr;
			memcpy(pdata, &modbus_rev->p_data[3], byte_num);
			gMegarevoDevCheckReg[index][2] = 1;
			break;
		}
	}

	if(index == sizeof(gMegarevoDevCheckReg)/sizeof(gMegarevoDevCheckReg[0]))
	{
		return -1;
	}

	if (gMegarevoDevCheckReg[0][2] == 0 || gMegarevoDevCheckReg[1][2] == 0)
	{
		// 数据不全
		return -2;
	}

	// check 合法性
	if ((_gMegarevoDevCheckRet[0] == 'K' || _gMegarevoDevCheckRet[0] == 'k')
		&& (_gMegarevoDevCheckRet[1] == '5')
		&& (_gMegarevoDevCheckRet[20] == 'F' || _gMegarevoDevCheckRet[20] == 'f')
		)
	{

		_MegarevoBaseInfoInit();

		memcpy(gCollectorDevInfo.modelid, (uint8_t *)&gCollectorSubDevMap[3].modelid, 
			   sizeof(uint16_t));
		memcpy(gCollectorDevInfo.sn, (uint8_t *)&_gMegarevoDevCheckRet[20], 
			   gMegarevoDevCheckReg[1][1] * sizeof(uint16_t));
		gCollector->pcs_modelid = gCollectorSubDevMap[3].modelid;

		return 0;
	}
	else if ((_gMegarevoDevCheckRet[0] == '0' && _gMegarevoDevCheckRet[1] == '1'
			  && _gMegarevoDevCheckRet[2] == 0 )
			 && (_gMegarevoDevCheckRet[3] == 'K' || _gMegarevoDevCheckRet[3] == 'k')
			 && (_gMegarevoDevCheckRet[20] == 'F' || _gMegarevoDevCheckRet[20] == 'f')
			)
	{

		memcpy(gCollectorDevInfo.modelid, (uint8_t *)&gCollectorSubDevMap[4].modelid, 
			   sizeof(uint16_t));
		memcpy(gCollectorDevInfo.sn, (uint8_t *)&_gMegarevoDevCheckRet[20], 
			   gMegarevoDevCheckReg[1][1] * sizeof(uint16_t));
		gCollector->pcs_modelid = gCollectorSubDevMap[4].modelid;

		_MegarevoBaseInfoInit();
		return 0;
	}

	return -3;
}

int32_t MegarevoDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info)
{

	DevOp_t dev_op = {0};

	dev_op.debug = 0;
	dev_op.slave_addr = MEGAREVO_DEV_ADDR;

	/*base info*/
	dev_op.op_type = EN_MEGAREVO_OP_TYPE_READ_BASE_INFO;
	static uint32_t i = 0;
	uint32_t j = sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]);

	if (i > j)
	{
		// 第二次轮询，可以开启上报
		++gCollectorDevInfo.pcs_info_updated;
	}

	p_dev_op_info->loop_flag = 0;
	for(;; ++i)
	{
		//_gMegarevoBaseInfoRegDiv[i%j][2] = 0;
		dev_op.idx = i%j;
		dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;
		dev_op.reg_addr = _gMegarevoBaseInfoRegDiv[i%j][3];
		dev_op.reg_num = _gMegarevoBaseInfoRegDiv[i%j][1] - (_gMegarevoBaseInfoRegDiv[i%j][3] - _gMegarevoBaseInfoRegDiv[i%j][0]);

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
  * @brief  Megarevo base info read.
  * @param  modbus format
  * @return NULL
  * @note   寄存器较多
  */
static void _MegarevoReadBaseInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadInput(gDevReqPacket, MEGAREVO_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Megarevo sys info read.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _MegarevoReadSysInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadRemain(gDevReqPacket, MEGAREVO_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Megarevo Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _MegarevoWriteSetSingle(uint16_t reg_addr, uint16_t reg_data)
{
    ModbusWriteSingleRemain(gDevReqPacket, MEGAREVO_DEV_ADDR, reg_addr, reg_data);
}

/**
  * @brief  Megarevo Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _MegarevoWriteSetMulti(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    ModbusWriteMultiRemain(gDevReqPacket, MEGAREVO_DEV_ADDR, reg_addr, reg_num, p_reg_data);
}

/**
  * @brief  Megarevo base info copy to gMegarevoInvBaseInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static int32_t _MegarevoBaseInfoUpdate(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	return _MegarevoInvBaseInfoInnerUpdate(idx, modbus_rev);
}

static int32_t _MegarevoBaseInfoUpdate_old(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	int32_t i = 0, j = sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]), 
			pos = 0; 
	uint16_t *paddr = (uint16_t *)gMegarevoInvBaseInfo;
	uint16_t *paddr1 = NULL;
	uint16_t byte_num = modbus_rev->p_data[2], byte_num2 = 0;
	if (idx < 0 || idx >= j)
	{
		return -1;
	}

	for (i = 0; i < j; ++i)
	{
		paddr1 = paddr + pos;
		if (i != 21)
		{
			paddr1[0] = _gMegarevoBaseInfoRegDiv[i][0];
			paddr1[1] = _gMegarevoBaseInfoRegDiv[i][1];
			if (i == 20)
			{
				paddr1[1] += _gMegarevoBaseInfoRegDiv[i+1][1];
			}
			pos += 0x02;
		}
		pos += _gMegarevoBaseInfoRegDiv[i][1];

		if (i == idx)
		{
			_gMegarevoBaseInfoRegDiv[i][2] = 1;
			// cpy data 
			if (i != 21)
			{
				paddr1 += 2;
			}
			if (_gMegarevoBaseInfoRegDiv[i][0] < _gMegarevoBaseInfoRegDiv[i][3])
			{
				paddr1 += (_gMegarevoBaseInfoRegDiv[i][3]-_gMegarevoBaseInfoRegDiv[i][0]);
			}

			byte_num2 = _gMegarevoBaseInfoRegDiv[i][1] - (_gMegarevoBaseInfoRegDiv[i][3] - _gMegarevoBaseInfoRegDiv[i][0]);;
			for(uint16_t i = 0; i < byte_num/2 && i < byte_num2; ++i)
			{
				paddr1[i] = (modbus_rev->p_data[3+i*2] << 8) | modbus_rev->p_data[3+i*2+1];
			}

#if 1
			// 构造告警数据
			if (i == 3)
			{
				static uint16_t j = 1;
				for(uint16_t i = 0; i < 14; ++i)
				{
					paddr1[i] = j++;
				}
			}
#endif 

			break;
		}
	}

	return 0;
}

int32_t MegarevoDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get1, 
								   ModbusRTURevData_t *p_modbus_dev_rev)
{
    static DevOp_t *p_dev_op_get;
	switch(p_op_info->status)
	{
		case EN_MEGAREVO_OP_STATUS_IDLE:
			{
				/*IDLE模式下不接受从机发的数据*/
				QueueClear(modbus_queue);
				/*检测gMegarevoOpQueue队列，判断是否有需要操作从机的命令*/
				if(QueueGet(op_queue, (void *)&p_dev_op_get) == true)
				{
					switch(p_dev_op_get->op_type)
					{
					case EN_MEGAREVO_OP_TYPE_READ_TYPE_INFO:
					{
						_MegarevoReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_DEV_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						break;
					}
					case EN_MEGAREVO_OP_TYPE_READ_BASE_INFO:
					{
						if (p_dev_op_get->idx >= 0 && (p_dev_op_get->idx 
							< sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0])))
						{
							_gMegarevoBaseInfoRegDiv[p_dev_op_get->idx][2] = 0;
						}
						_MegarevoReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();

						break;
					}
					case EN_MEGAREVO_OP_TYPE_WRITE_SET:
					{
						if (p_dev_op_get->func_code == EN_MODBUS_FUNC_CODE_WRITE_MULTI)
						{
							_MegarevoWriteSetMulti(p_dev_op_get->reg_addr, 
												   p_dev_op_get->reg_num, 
												   p_dev_op_get->p_data);
						}
						else 
						{
							// 为了兼容函数，需要将参数转换为小端序
							uint16_t d = (uint16_t)(*(uint16_t*)(p_dev_op_get->p_data));
							d = p_dev_op_get->p_data[0] >> 8 | p_dev_op_get->p_data[1];
							_MegarevoWriteSetSingle(p_dev_op_get->reg_addr, d);
						}
						p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
							_gMegarevoWriteCmdNum--;
						}
						break;
					}
					default:break;
					}
				}
				break;
			}
		case EN_MEGAREVO_OP_STATUS_WAIT_ACK:
			{
				if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
				{
					uint8_t rev_slave_addr = p_modbus_dev_rev->p_data[0];
					uint8_t rev_func_code = p_modbus_dev_rev->p_data[1];

					if (p_dev_op_get->reg_addr == 0x3100)
					{
						int32_t xx = 0;
					}

					if (gMegarevoDevCheckRegSize > 0)
					{
						--gMegarevoDevCheckRegSize;
					}

					if(rev_slave_addr != p_dev_op_get->slave_addr)
					{
						break;//不是给本设备的包
					}
					p_op_info->slave_ack_timeout = false;
					if((rev_func_code & 0x80) == 0x80)//逆变器设备通信异常
					{
						p_op_info->rev_error_code = p_modbus_dev_rev->p_data[2];//rev_func_code & 0x7F;
						p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
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
					case EN_MEGAREVO_OP_TYPE_READ_TYPE_INFO:
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
							if (!MegarevoDevCheck(p_dev_op_get, p_modbus_dev_rev))
							{
								p_dev_op_ctx->init_flag = 2;
							}
							p_op_info->status = EN_MEGAREVO_OP_STATUS_IDLE;
							//p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();
						}
						break;
					}
					case EN_MEGAREVO_OP_TYPE_READ_BASE_INFO:
					{
						uint16_t byte_num = p_modbus_dev_rev->p_data[2];

						if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
						{
							p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
							p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();
						}
						else//数据OK
						{
							if (p_dev_op_get->debug != 0)
							{
								int32_t debug_info_len = 0;
								if (sizeof(p_dev_op_get->debug) + byte_num + 2 >= sizeof(gPcsDevDebugModebusInfo))
								{
									p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
									p_op_info->op_timing_count = tim_get_systime();
									break;
								}

								debug_info_len += sizeof(p_dev_op_get->debug);
								memcpy(&gPcsDevDebugModebusInfo[0], &p_dev_op_get->debug, sizeof(p_dev_op_get->debug));

								memcpy(&gPcsDevDebugModebusInfo[0]+debug_info_len, p_modbus_dev_rev->p_data, byte_num*2+2);
							}
							uint8_t index = 0;
							for(index = 0; index < sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]); index++)
							{
								if(_gMegarevoBaseInfoRegDiv[index][3] == p_dev_op_get->reg_addr)
								{
									break;
								}
							}
							if(index == sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]))
							{
								p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
								p_op_info->op_timing_count = tim_get_systime();
								break;
							}

							p_dev_op_get->idx = index;
							_gMegarevoBaseInfoRegDiv[index][2] = 1;

							_MegarevoBaseInfoUpdate(p_dev_op_get->idx, p_modbus_dev_rev);

							p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();

						}
						break;
					}
					case EN_MEGAREVO_OP_TYPE_WRITE_SET:
					{
						p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
						{
							if(_gMegarevoWriteCmdNum == 0)
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
					if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > MEGAREVO_DEV_ACK_TIME_OUT)
					{
						if (gMegarevoDevCheckRegSize > 0)
						{
							--gMegarevoDevCheckRegSize;
						}
						p_op_info->slave_ack_timeout = true;
						p_op_info->status = EN_MEGAREVO_OP_STATUS_WAIT_GAP;
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
		case EN_MEGAREVO_OP_STATUS_ACK_ANALYSIS:
		{
			//_MegarevoBaseInfoUpdate(p_dev_op_get->idx, p_modbus_dev_rev);
			p_op_info->status = EN_MEGAREVO_OP_STATUS_IDLE;
			break;
		}
        case EN_MEGAREVO_OP_STATUS_WAIT_GAP:
        {
            if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > MEGAREVO_DEV_POLLING_GAP)
            {
//				if (p_dev_op_ctx->init_flag == 1 && gMegarevoDevCheckRegSize == 0)
//				{
//					p_dev_op_ctx->init_flag = 0;
//				}
                p_op_info->status = EN_MEGAREVO_OP_STATUS_IDLE;
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
int32_t MegarevoInvSet(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data)
{
    DevOp_t dev_op;
    uint16_t index;
    uint8_t *p_origin = p_reg_data;
    
    /*大小端转换*/
    for(uint16_t i = 0; i < reg_num; i++)
    {
        _gMegarevoInvSet[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    
    
    dev_op.op_type = EN_MEGAREVO_OP_TYPE_WRITE_SET;
    dev_op.slave_addr = MEGAREVO_DEV_ADDR;
    dev_op.func_code = func_code;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = reg_num;
    dev_op.p_data = (uint8_t *)&_gMegarevoInvSet[0];
    if(QueuePut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
		++_gMegarevoWriteCmdNum;
		gDevSetting = 1;
    }
	return 0;
}

int32_t MegarevoDataGet(void **data)
{
	gMegarevoInvBaseInfo = (MegarevoInvBaseInfo_t *)gCommonBaseInfo;
	int32_t i = 0, 
			j = sizeof(_gMegarevoBaseInfoRegDiv)/sizeof(_gMegarevoBaseInfoRegDiv[0]), 
			k = 0, 
			pos = 0, pos_start = 0,
			total_len = 0;
	uint16_t *paddr = (uint16_t *)gMegarevoInvBaseInfo;
	uint16_t *paddr1 = NULL;
	uint16_t *paddr2 = (uint16_t *)&_gMegarevoInvBaseInfoInner;

	memset(paddr, 0, sizeof(gCommonBaseInfo));

	for (i = 0; i < j; ++i)
	{
		if (_gMegarevoBaseInfoRegDiv[i][2] == 2)
		{
			//paddr2 += _gMegarevoBaseInfoRegDiv[i][1];
			continue;
		}

		paddr1 = paddr + pos;

		k = 1;
		pos_start = pos;
		if (i != 21)
		{
			paddr1[0] = _gMegarevoBaseInfoRegDiv[i][0];
			paddr1[1] = _gMegarevoBaseInfoRegDiv[i][1];
			if (i == 20)
			{
				k = 0;
				paddr1[1] += _gMegarevoBaseInfoRegDiv[i+1][1];
			}
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

		//pos += _gMegarevoBaseInfoRegDiv[i][1];
		if (k)
		{
			pos_start = pos;
		}
	}
	_gMegarevoDataLen = pos;
	_gMegarevoDataLen = total_len;
	*data = (void *)gMegarevoInvBaseInfo;
	return _gMegarevoDataLen;
}


