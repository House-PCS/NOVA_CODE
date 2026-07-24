#include "pcs_op_kstar.h"
#include "sys_info.h"
#include "kstar_reg_judge.h"

#define KSTAR_DEV_ACK_TIME_OUT   2000
#define KSTAR_DEV_POLLING_GAP    500

typedef enum
{
    EN_KSTAR_OP_TYPE_READ_TYPE_INFO,
    EN_KSTAR_OP_TYPE_READ_BASE_INFO,
    EN_KSTAR_OP_TYPE_READ_SYS_INFO,
    EN_KSTAR_OP_TYPE_WRITE_BASE_INFO,
    EN_KSTAR_OP_TYPE_WRITE_SET,
//    EN_KSTAR_OP_TYPE_WRITE_REMOTE,
}EN_KSTAR_OP_TYPE;

typedef enum
{
    EN_KSTAR_OP_STATUS_IDLE,
    EN_KSTAR_OP_STATUS_SEND,
    EN_KSTAR_OP_STATUS_WAIT_ACK,
    EN_KSTAR_OP_STATUS_ACK_ANALYSIS,
    EN_KSTAR_OP_STATUS_WAIT_GAP,
    
}EN_KSTAR_OP_STATUS;


/*第一个字段是寄存器起始地址，第二个字段是寄存器数量*/    
static uint16_t _gKstarBaseInfoRegDiv[][3] = {
	{3000, 87, 0}, 
	{3097, 91, 0}, 
	{3193, 53, 0}, 
	{3251, 68, 0}, 
    {3441, 34, 0}, 
	{3485, 34, 0}, 
	{3529, 34, 0}, 
	{3573, 34, 0}, 
	{3617, 34, 0}, 
	{3661, 34, 0}, 
	{3705, 34, 0}, 
	{3749, 34, 0}
};

/*Base Inv Remote下发合法地址判断索引*/
static const uint16_t _gKstarValidBaseRegAddr[] = {3044, 3059, 3060, 3061, 3062, 3068, 3072, 3073, 3074, 3078, 3079, 3080, 3081, 3082, 3083, 3084, 3086, 3179, 
                                                   3181, 3182, 3183, 3184, 3185, 3186, 3193, 3195, 3197, 3199, 3200, 3201, 3202, 3203, 3204, 3205, 3206, 3207,
                                                   3208, 3209, 3210, 3211, 3212, 3213, 3214, 3215, 3216, 3217, 3218, 3219, 3220, 3221, 3222, 3223, 3251, 3252,
                                                   3253, 3254, 3255, 3256, 3257, 3258, 3259, 3260, 3261, 3262, 3263, 3264, 3265, 3314, 3317, 3318};
//static const uint16_t _gKstarValidInvRegAddr[] = {3160, 3168, 3200, 3208, 3228, 3239};
static const uint16_t _gKstarValidInvRegAddr[] = {3160, 3168, 3200, 3208, 3239};//逆变器SN号不动
static const uint16_t _gKstarValidRemoteRegAddr[] = {3240, 3241, 3242};


static uint8_t _gkstarWriteCmdNum = 0;//当前待写操作设备的命令数量

static uint16_t _gKstarBaseSet[800];
static uint16_t _gKstarInvSet[88];
static uint16_t _gKstarRemoteSet[6];

static uint16_t  _gKstarBaseInfoCache[800];
static uint16_t  _gKstarSysInfoCache[40];

// 科士达设备类型检测需要的寄存器
uint16_t gKstarDevCheckRegSize = 0;
static uint16_t gKstarDevCheckReg[][3] = {{3045, 1, 0}, {3228, 11, 0}};
static uint8_t _gKstarDevCheckRet[sizeof(uint16_t) * 12];


int32_t KstarDevCheckReq(Queue_t *queue)
{
	DevOp_t dev_op;

	dev_op.slave_addr = KSTAR_DEV_ADDR;
	dev_op.op_type = EN_KSTAR_OP_TYPE_READ_TYPE_INFO;

	gKstarDevCheckRegSize = 0;
	QueueClear(queue);
	for (int32_t i = 0; i < sizeof(gKstarDevCheckReg)/sizeof(gKstarDevCheckReg[0]); ++i)
	{
		dev_op.reg_addr = gKstarDevCheckReg[i][0]; // 逆变器型号
		dev_op.reg_num = gKstarDevCheckReg[i][1];
		QueuePut(queue, &dev_op);
		++gKstarDevCheckRegSize;
	}

	return 0;
}

int32_t KstarDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev)
{
	int32_t addr = 0;
	uint8_t index = 0;
	uint8_t *pdata = _gKstarDevCheckRet;
	uint16_t byte_num = modbus_rev->p_data[2];

	for(index = 0; index < sizeof(gKstarDevCheckReg)/sizeof(gKstarDevCheckReg[0]); index++)
	{
		addr += gKstarDevCheckReg[index][1] * sizeof(uint16_t);
		if(gKstarDevCheckReg[index][0] == dev_op->reg_addr)
		{
			pdata += addr;
			memcpy(pdata, &modbus_rev->p_data[3], byte_num);
			gKstarDevCheckReg[index][2] = 1;
			break;
		}
	}

	if(index == sizeof(gKstarDevCheckReg)/sizeof(gKstarDevCheckReg[0]))
	{
		return -1;
	}

	if (gKstarDevCheckReg[0][2] == 0 || gKstarDevCheckReg[1][2] == 0)
	{
		// 数据不全
		return -2;
	}

	// check 合法性

	return 0;
}

int32_t KstarDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info)
{
	DevOp_t dev_op;

	dev_op.slave_addr = KSTAR_DEV_ADDR;

#if 0
	/*sys info*/ //先读sys info(非关键信息，避免系统一上电，串口第一个字节多带一个0x00导致读取失败的情况)
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;
	dev_op.op_type = EN_DEV_OP_TYPE_READ_SYS_INFO;
	dev_op.reg_addr = 3200;
	dev_op.reg_num = 19;
	QueuePut(queue, &dev_op);
#endif 

	/*base info*/
	dev_op.op_type = EN_KSTAR_OP_TYPE_READ_BASE_INFO;
	for(uint8_t i = 0; i < sizeof(_gKstarBaseInfoRegDiv)/sizeof(_gKstarBaseInfoRegDiv[0]); i++)
	{
		dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_INPUT;
		dev_op.reg_addr = _gKstarBaseInfoRegDiv[i][0];
		dev_op.reg_num = _gKstarBaseInfoRegDiv[i][1];
		QueuePut(queue, &dev_op);
	}

	return 0;
}

/**
  * @brief  Kstar base info read.
  * @param  modbus format
  * @return NULL
  * @note   寄存器较多，分多次回读，3000~3086, 3097~3187, 3193~3245, 3251~3318, 
                               PACK:3441~3474, 3485~3518, 3529~3562, 3573~3606, 3617~3650, 3661~3694, 3705~3738, 3749~3782
  */
static void _KstarReadBaseInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadInput(gDevReqPacket, KSTAR_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Kstar sys info read.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarReadSysInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadRemain(gDevReqPacket, KSTAR_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Kstar Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarWriteBaseInfo(uint16_t reg_addr, uint16_t reg_data)
{
    ModbusWriteSingleRemain(gDevReqPacket, KSTAR_DEV_ADDR, reg_addr, reg_data);
}

/**
  * @brief  Kstar Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarWriteSet(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    ModbusWriteMultiRemain(gDevReqPacket, KSTAR_DEV_ADDR, reg_addr, reg_num, p_reg_data);
}

/**
  * @brief  Kstar base info copy to gKstarInvBaseInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarBaseInfoUpdate()
{
    uint16_t *p_data = (uint16_t *)&gKstarInvBaseInfo;
    uint8_t *p_origin = (uint8_t *)_gKstarBaseInfoCache;

    for(uint16_t i = 0; i < sizeof(KstarInvBaseInfo_t) / 2; i++)
    {
        p_data[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    gCollectorDevInfo.pcs_info_updated = true;
}

/**
  * @brief  Kstar sys info copy to gKstarInvSysInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _KstarSysInfoUpdate()
{
    uint16_t *p_data = (uint16_t *)&gKstarInvSysInfo;
    uint8_t *p_origin = (uint8_t *)_gKstarSysInfoCache;

    for(uint16_t i = 0; i < sizeof(KstarInvSysInfo_t) / 2; i++)
    {
        p_data[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    gCollectorDevInfo.kstar_sys_info_updated = true;
}

int32_t KstarDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get, 
								   ModbusRTURevData_t *p_modbus_dev_rev)
{
	switch(p_op_info->status)
	{
		case EN_KSTAR_OP_STATUS_IDLE:
			{
				/*IDLE模式下不接受从机发的数据*/
				QueueClear(modbus_queue);
				/*检测gKstarOpQueue队列，判断是否有需要操作从机的命令*/
				if(QueueGet(op_queue, (void *)&p_dev_op_get) == true)
				{
					switch(p_dev_op_get->op_type)
					{
					case EN_KSTAR_OP_TYPE_READ_TYPE_INFO:
					{
						_KstarReadBaseInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_DEV_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						break;
					}
					case EN_KSTAR_OP_TYPE_READ_BASE_INFO:
					{
						if(p_dev_op_get->reg_addr >= _gKstarBaseInfoRegDiv[4 + p_op_info->pack_num][0])/*自动跳过不存在的电池包数据获取*/
						{
							break;
						}
						_KstarReadBaseInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();

						break;
					}
					case EN_KSTAR_OP_TYPE_READ_SYS_INFO:
					{
						_KstarReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						break;
					}
					case EN_KSTAR_OP_TYPE_WRITE_BASE_INFO:
					{
						_KstarWriteBaseInfo(p_dev_op_get->reg_addr, (p_dev_op_get->p_data[1] << 8) | p_dev_op_get->p_data[0]);
						p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
							_gkstarWriteCmdNum--;
						}
						break;
					}
					case EN_KSTAR_OP_TYPE_WRITE_SET:
					{
						_KstarWriteSet(p_dev_op_get->reg_addr, p_dev_op_get->reg_num, p_dev_op_get->p_data);
						p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
							_gkstarWriteCmdNum--;
						}
						break;
					}
					default:break;
					}
				}
				break;
			}
		case EN_KSTAR_OP_STATUS_WAIT_ACK:
			{
				if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
				{
					uint8_t rev_slave_addr = p_modbus_dev_rev->p_data[0];
					uint8_t rev_func_code = p_modbus_dev_rev->p_data[1];

					if (gKstarDevCheckRegSize > 0)
					{
						--gKstarDevCheckRegSize;
					}

					if(rev_slave_addr != p_dev_op_get->slave_addr)
					{
						break;//不是给本设备的包
					}
					p_op_info->slave_ack_timeout = false;
					if((rev_func_code & 0x80) == 0x80)//逆变器设备通信异常
					{
						p_op_info->rev_error_code = p_modbus_dev_rev->p_data[2];//rev_func_code & 0x7F;
						p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
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
					case EN_KSTAR_OP_TYPE_READ_TYPE_INFO:
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
							if (!KstarDevCheck(p_dev_op_get, p_modbus_dev_rev))
							{
								p_dev_op_ctx->init_flag = 2;
							}
                            p_op_info->op_timing_count = tim_get_systime();
                        }
						break;
					}
					case EN_KSTAR_OP_TYPE_READ_BASE_INFO:
						{
							uint16_t byte_num = p_modbus_dev_rev->p_data[2];

							if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
							{
								p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
								p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
								p_op_info->op_timing_count = tim_get_systime();
							}
							else//数据OK
							{
								uint8_t index = 0;
								for(index = 0; index < sizeof(_gKstarBaseInfoRegDiv)/sizeof(_gKstarBaseInfoRegDiv[0]); index++)
								{
									if(_gKstarBaseInfoRegDiv[index][0] == p_dev_op_get->reg_addr)
									{
										break;
									}
								}
								if(index == sizeof(_gKstarBaseInfoRegDiv)/sizeof(_gKstarBaseInfoRegDiv[0]))
								{
									p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
									p_op_info->op_timing_count = tim_get_systime();
									break;
								}

								memcpy((uint8_t *)&_gKstarBaseInfoCache[p_dev_op_get->reg_addr - 3000], &p_modbus_dev_rev->p_data[3], byte_num);
								_gKstarBaseInfoRegDiv[index][2] = 1;
								p_op_info->status = EN_KSTAR_OP_STATUS_ACK_ANALYSIS;
								p_op_info->op_timing_count = tim_get_systime();

								if(index == 0)//第一包中包含了电池包数，3077
								{
									p_op_info->pack_num = (p_modbus_dev_rev->p_data[154 + 3] << 8) | p_modbus_dev_rev->p_data[155 + 3];
									if(p_op_info->pack_num > 8)
									{
										p_op_info->pack_num = 8;
									}
								}
							}
							break;
						}
					case EN_KSTAR_OP_TYPE_READ_SYS_INFO:
						{
							uint16_t byte_num = p_modbus_dev_rev->p_data[2];

							if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
							{
								p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
								p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
								p_op_info->op_timing_count = tim_get_systime();
							}
							else//数据OK
							{
								memcpy((uint8_t *)&_gKstarSysInfoCache[p_dev_op_get->reg_addr - 3200], &p_modbus_dev_rev->p_data[3], byte_num);
								p_op_info->status = EN_KSTAR_OP_STATUS_ACK_ANALYSIS;
								p_op_info->op_timing_count = tim_get_systime();
							}
							break;
						}
					case EN_KSTAR_OP_TYPE_WRITE_BASE_INFO:
						{
							p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();
							if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
							{
								if(_gkstarWriteCmdNum == 0)
								{
									p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_OK;
								}
							}
							break;
						}
					case EN_KSTAR_OP_TYPE_WRITE_SET:
						{
							p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
							p_op_info->op_timing_count = tim_get_systime();
							if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
							{
								if(_gkstarWriteCmdNum == 0)
								{
									p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_OK;
								}
							}
							break;
						}
					default:break;
					}
				}
				else
				{
					if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > KSTAR_DEV_ACK_TIME_OUT)
					{
						if (gKstarDevCheckRegSize > 0)
						{
							--gKstarDevCheckRegSize;
						}
						p_op_info->slave_ack_timeout = true;
						p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
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
		case EN_KSTAR_OP_STATUS_ACK_ANALYSIS:
			{
				p_op_info->status = EN_KSTAR_OP_STATUS_WAIT_GAP;
				uint8_t base_info_div_num = 4 + p_op_info->pack_num;
				if(p_dev_op_get->op_type == EN_KSTAR_OP_TYPE_READ_BASE_INFO)
				{
					/*base info copy*/
					uint8_t index = 0;
					for(index = 0; index < base_info_div_num; index++)
					{
						if(_gKstarBaseInfoRegDiv[index][2] == 0)
						{
							break;
						}
					}
					if(index == base_info_div_num)
					{
						_KstarBaseInfoUpdate();
					}
				}
				else if(p_dev_op_get->op_type == EN_KSTAR_OP_TYPE_READ_SYS_INFO)
				{
					/*sys info copy*/
					_KstarSysInfoUpdate();
				}
			}
        case EN_KSTAR_OP_STATUS_WAIT_GAP:
        {
            if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > KSTAR_DEV_POLLING_GAP)
            {
//				if (gKstarDevCheckRegSize == 0)
//				{
//					p_dev_op_ctx->init_flag = 0;
//				}
                p_op_info->status = EN_KSTAR_OP_STATUS_IDLE;
            }
            break;
        }
	}

	return 0;
}


/**
  * @brief  操作逆变器Base寄存器接口
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
int32_t KstarBaseSet(uint16_t reg_addr, uint16_t reg_data)
{
    DevOp_t dev_op;
    uint16_t index;
    /*地址合法性判断*/
    for(index = 0; index < sizeof(_gKstarValidBaseRegAddr)/sizeof(uint16_t); index++)
    {
        if(reg_addr == _gKstarValidBaseRegAddr[index])
        {
            break;
        }
    }
    if(index >= sizeof(_gKstarValidBaseRegAddr)/sizeof(uint16_t))
    {
        return -1;
    }
    
    
    /*对数据进行过滤，防止上位机误操作导致危险*/
    if(KstarRegBaseJudge(reg_addr, reg_data) == false)
    {
        return -2;
    }
    
    _gKstarBaseSet[reg_addr - 3000] = reg_data;
    
    dev_op.op_type = EN_KSTAR_OP_TYPE_WRITE_BASE_INFO;
    dev_op.slave_addr = KSTAR_DEV_ADDR;
    dev_op.func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = 1;
    dev_op.p_data = (uint8_t *)&_gKstarBaseSet[reg_addr - 3000];
    
    if(QueuePut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
        ++_gkstarWriteCmdNum;
		gDevSetting = 1;
    }
	return 0;
}


/**
  * @brief  操作逆变器设置寄存器接口
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
int32_t KstarInvSet(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    DevOp_t dev_op;
    uint16_t index;
    uint8_t *p_origin = p_reg_data;
    /*地址合法性判断*/
    for(index = 0; index < sizeof(_gKstarValidInvRegAddr)/sizeof(uint16_t); index++)
    {
        if(reg_addr == _gKstarValidInvRegAddr[index])
        {
            break;
        }
    }
    if(index >= sizeof(_gKstarValidInvRegAddr)/sizeof(uint16_t))
    {
        return -1;
    }
    
    /*对数据进行过滤，防止上位机误操作导致危险*/
    if(KstarRegInvJudge(reg_addr, reg_num, p_reg_data) == false)
    {
        return -2;
    }
    
    /*大小端转换*/
    for(uint16_t i = 0; i < reg_num; i++)
    {
        _gKstarInvSet[reg_addr - 3160 + i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    
    
    dev_op.op_type = EN_KSTAR_OP_TYPE_WRITE_SET;
    dev_op.slave_addr = KSTAR_DEV_ADDR;
    dev_op.func_code = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = reg_num;
    dev_op.p_data = (uint8_t *)&_gKstarInvSet[reg_addr - 3160];
    if(QueuePut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
        ++_gkstarWriteCmdNum;
		gDevSetting = 1;
    }
	return 0;
}


/**
  * @brief  操作逆变器Base寄存器接口
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
int32_t KstarRemoteSet(uint16_t reg_addr, uint16_t reg_data)
{
    DevOp_t dev_op;
    uint16_t index;
    /*地址合法性判断*/
    for(index = 0; index < sizeof(_gKstarValidRemoteRegAddr)/sizeof(uint16_t); index++)
    {
        if(reg_addr == _gKstarValidRemoteRegAddr[index])
        {
            break;
        }
    }
    if(index >= sizeof(_gKstarValidRemoteRegAddr)/sizeof(uint16_t))
    {
        return -1;
    }
    
    /*对数据进行过滤，防止上位机误操作导致危险*/
    if(KstarRegRemoteJudge(reg_addr, reg_data) == false)
    {
        return -1;
    }
    _gKstarRemoteSet[reg_addr - 3240] = reg_data;
    
    dev_op.op_type = EN_KSTAR_OP_TYPE_WRITE_BASE_INFO;
    dev_op.slave_addr = KSTAR_DEV_ADDR;
    dev_op.func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = 1;
    dev_op.p_data = (uint8_t *)&_gKstarRemoteSet[reg_addr - 3240];
    if(QueuePut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
        ++_gkstarWriteCmdNum;
		gDevSetting = 1;
    }
    return 0;
}


