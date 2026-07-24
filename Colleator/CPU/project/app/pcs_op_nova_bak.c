#include "pcs_op_nova.h"
#include "sys_info.h"
#include "nova_reg_judge.h"
#include "nova_dev_upgrade.h"
#include "fc41d.h"

#define NOVA_DEV_ACK_TIME_OUT   2000
#define NOVA_DEV_POLLING_GAP    500

typedef enum
{
	EN_NOVA_OP_TYPE_READ_TYPE_INFO,
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
    EN_NOVA_WRITE_COMM_STATUS_IDLE,			// 0
    EN_NOVA_WRITE_COMM_STATUS_WAITING_SEND,	// 1
    EN_NOVA_WRITE_COMM_STATUS_SENDING,			// 2
    EN_NOVA_WRITE_COMM_STATUS_NO_ACK,			// 3
    EN_NOVA_WRITE_COMM_STATUS_ACK_ERROR,		// 4
    EN_NOVA_WRITE_COMM_STATUS_ACK_OK			// 5
}EN_NOVA_COMM_STATUS;


/* 第1个字段表示操作码
 * 第2个字段是寄存器起始地址，第3个字段是寄存器数量,
 *第4个字段是寄存器是否已读，第5个字段是否只读一次*/
static uint16_t _gNovaBaseInfoRegDiv[][5] = {
	{0x03, 0x5000, 0x18, 0x00, 0x01},	// 设备SN，版本等基本信息
	{0x03, 0x5100, 0x16, 0x00, 0x00},	// 用电量统计汇总数据
	{0x03, 0x6000, 0x10, 0x00, 0x01},	// PCS基本信息
	{0x03, 0x6010, 0x0B, 0x00, 0x00},	// PCS基本信息
	{0x03, 0x6020, 0x06, 0x00, 0x00},	// PCS 电池侧实时参数
	{0x03, 0x6026, 0x50, 0x00, 0x00},	// PCS RST实时参数
};

/*Base Inv Remote下发合法地址判断索引*/
static const uint16_t _gNovaValidBaseRegAddr[] = {0x5012};
static const uint16_t _gNovaValidInvRegAddr[] = {3160, 3168, 3200, 3208, 3239};//逆变器SN号不动
static const uint16_t _gNovaValidRemoteRegAddr[] = {3240, 3241, 3242};


// Nova设备类型检测需要的寄存器
uint16_t gNovaDevCheckRegSize = 0;
uint16_t gNovaDevCheckReg[][3] = 
{
	{3045, 1, 0}, 
	{3228, 11, 0}
};
static uint8_t _gNovaDevCheckRet[sizeof(uint16_t) * 12];


static uint8_t _gNovaWriteCmdNum;//当前待写操作设备的命令数量

static uint16_t  _gNovaBaseInfoCache[800];
static uint16_t  _gNovaSysInfoCache[40];

int32_t NovaDevCheckReq(Queue_t *queue)
{
	DevOp_t dev_op;

	dev_op.slave_addr = NOVA_DEV_ADDR;
	dev_op.op_type = EN_NOVA_OP_TYPE_READ_TYPE_INFO;

	gNovaDevCheckRegSize = 0;
	QueueClear(queue);
	for (int32_t i = 0; i < sizeof(gNovaDevCheckReg)/sizeof(gNovaDevCheckReg[0]); ++i)
	{
		dev_op.reg_addr = gNovaDevCheckReg[i][0]; // 逆变器型号
		dev_op.reg_num = gNovaDevCheckReg[i][1];
		QueuePut(queue, &dev_op);
		++gNovaDevCheckRegSize;
	}

	return 0;
}

int32_t NovaDevCheck(DevOp_t *dev_op, ModbusRTURevData_t *modbus_rev)
{
	int32_t addr = 0;
	uint8_t index = 0;
	uint8_t *pdata = _gNovaDevCheckRet;
	uint16_t byte_num = modbus_rev->p_data[2];

	for(index = 0; index < sizeof(gNovaDevCheckReg)/sizeof(gNovaDevCheckReg[0]); index++)
	{
		addr += gNovaDevCheckReg[index][1] * sizeof(uint16_t);
		if(gNovaDevCheckReg[index][0] == dev_op->reg_addr)
		{
			pdata += addr;
			memcpy(pdata, &modbus_rev->p_data[3], byte_num);
			gNovaDevCheckReg[index][2] = 1;
			break;
		}
	}

	if(index == sizeof(gNovaDevCheckReg)/sizeof(gNovaDevCheckReg[0]))
	{
		return -1;
	}

	if (gNovaDevCheckReg[0][2] == 0 || gNovaDevCheckReg[1][2] == 0)
	{
		// 数据不全
		return -2;
	}

	// check 合法性

	return 0;
}

int32_t NovaDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info)
{
	DevOp_t dev_op;

	dev_op.slave_addr = NOVA_DEV_ADDR;

    static uint32_t timing_upgrade_count = 0;
    static uint32_t upgrade_resend_count = 0;
	int32_t upgrade_flag = NovaDevUpgradeStatGet();

    /*每隔一定时间，发送读设备寄存器队列*/
	if (upgrade_flag == NOVA_DEV_UPGRADE_STAT_START)
	{
		DevOp_t dev_op;
		uint32_t cur_time = (uint32_t)(tim_get_systime());
		if (cur_time-p_dev_op_info->op_timing_count > 5*1000)
		{
			timing_upgrade_count = (uint32_t)(tim_get_systime());
			dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START;
			p_dev_op_info->status = EN_NOVA_OP_STATUS_IDLE;
			QueuePut(queue, &dev_op);
		}
	}
	else 
	{
		if (p_dev_op_info->status != EN_NOVA_OP_STATUS_WAIT_UPGRADE_DOWNLOAD_TO_FLASH)
		{
			p_dev_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_DEBUG;
			p_dev_op_info->op_timing_count = tim_get_systime();
			timing_upgrade_count = (uint32_t)(tim_get_systime());
		}
	}

	return 0;
}

static void _NovaReadMonitor(uint8_t func_code, uint16_t reg_addr, uint16_t reg_num)
{
	if (func_code == EN_MODBUS_FUNC_CODE_READ_REMAIN)
	{
		ModbusReadRemain(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_num);
	}
	else if (func_code == EN_MODBUS_FUNC_CODE_READ_INPUT)
	{
		ModbusReadInput(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_num);
	}

}



/**
  * @brief  nova base info read.
  * @param  modbus format
  * @return NULL
  * @note   寄存器较多，分多次回读，3000~3086, 3097~3187, 3193~3245, 3251~3318, 
                               PACK:3441~3474, 3485~3518, 3529~3562, 3573~3606, 3617~3650, 3661~3694, 3705~3738, 3749~3782
  */
static void _NovaReadBaseInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadInput(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Nova sys info read.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _NovaReadSysInfo(uint16_t reg_addr, uint16_t reg_num)
{
    ModbusReadRemain(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_num);
}

/**
  * @brief  Nova Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _NovaWriteBaseInfo(uint16_t reg_addr, uint16_t reg_data)
{
    ModbusWriteSingleRemain(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_data);
}

/**
  * @brief  Nova Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _NovaWriteSet(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    ModbusWriteMultiRemain(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_num, p_reg_data);
}

/**
  * @brief  Nova base info copy to gNovaInvBaseInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _NovaBaseInfoUpdate()
{
    uint16_t *p_data = (uint16_t *)&gNovaInvBaseInfo;
    uint8_t *p_origin = (uint8_t *)_gNovaBaseInfoCache;

    for(uint16_t i = 0; i < sizeof(NovaInvBaseInfo_t) / 2; i++)
    {
        p_data[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    gCollectorDevInfo.pcs_info_updated = true;
}

/**
  * @brief  Nova sys info copy to gNovaInvSysInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _NovaSysInfoUpdate()
{
    uint16_t *p_data = (uint16_t *)&gNovaInvSysInfo;
    uint8_t *p_origin = (uint8_t *)_gNovaSysInfoCache;

    for(uint16_t i = 0; i < sizeof(NovaInvSysInfo_t) / 2; i++)
    {
        p_data[i] = (p_origin[2 * i] << 8) | p_origin[2 * i + 1];
    }
    gCollectorDevInfo.kstar_sys_info_updated = true;
}

int32_t NovaDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								  Queue_t *op_queue, Queue_t *modbus_queue, 
								  DevOp_t *p_dev_op_get, ModbusRTURevData_t *p_modbus_dev_rev)
{
    static uint32_t upgrade_resend_count = 0;
    static uint32_t timing_upgrade_count = 0;

	switch(p_op_info->status)
	{
		case EN_NOVA_OP_STATUS_IDLE:
		{
			/*IDLE模式下不接受从机发的数据*/
			QueueClear(modbus_queue);
			/*检测gNovaOpQueue队列，判断是否有需要操作从机的命令*/
			if(QueueGet(op_queue, (void *)&p_dev_op_get) == true)
			{
				switch(p_dev_op_get->op_type)
				{
					case EN_NOVA_OP_TYPE_WRITE_UPGRADE_START:
					{
						upgrade_resend_count = 0;
						NovaDevUpgradeFirmwareStart();
						p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE;
						p_op_info->op_timing_count = tim_get_systime();
						timing_upgrade_count = (uint32_t)(tim_get_systime());
						delay_1ms(100);
						break;
					}
					case EN_NOVA_OP_TYPE_WRITE_UPGRADE_RE_SEND:
					{
						++upgrade_resend_count;
						//int32_t send_len = NovaDevUpgradeFirmwareSendDebug();
						int32_t send_len = NovaDevUpgradeFirmwareSend(1);
						p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE;
						if (send_len <= 0)
						{
							NovaDevUpgradeFirmwareEnd();
						}
						p_op_info->op_timing_count = tim_get_systime();
						timing_upgrade_count = (uint32_t)(tim_get_systime());
						delay_1ms(100);
						break;
					}
					case EN_NOVA_OP_TYPE_WRITE_UPGRADE_SEND:
					{
						upgrade_resend_count = 0;
						//int32_t send_len = NovaDevUpgradeFirmwareSendDebug();
						int32_t send_len = NovaDevUpgradeFirmwareSend(0);
						p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE;
						if (send_len <= 0)
						{
							NovaDevUpgradeFirmwareEnd();
						}
						p_op_info->op_timing_count = tim_get_systime();
						timing_upgrade_count = (uint32_t)(tim_get_systime());
						delay_1ms(100);
						break;
					}
					case EN_NOVA_OP_TYPE_WRITE_UPGRADE_END:
					{
						upgrade_resend_count = 0;
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_COMPLETE);
						p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
						p_op_info->op_timing_count = tim_get_systime();
						timing_upgrade_count = (uint32_t)(tim_get_systime());
						//delay_1ms(100);
						break;
					}
					case EN_NOVA_OP_TYPE_READ_TYPE_INFO:
                        _NovaReadBaseInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
                        p_op_info->status = EN_DEV_OP_STATUS_WAIT_ACK;
                        p_op_info->op_timing_count = tim_get_systime();
						break;
					case EN_NOVA_OP_TYPE_READ_BASE_INFO:
					{
                        _NovaReadMonitor(p_dev_op_get->func_code, p_dev_op_get->reg_addr, 
										 p_dev_op_get->reg_num);
                        p_op_info->status = EN_NOVA_OP_STATUS_WAIT_ACK;
                        p_op_info->op_timing_count = tim_get_systime();
						break;
					}
					case EN_NOVA_OP_TYPE_READ_SYS_INFO:
					{
						_NovaReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
						p_op_info->status = EN_NOVA_OP_STATUS_WAIT_ACK;
						p_op_info->op_timing_count = tim_get_systime();
						break;
					}
					case EN_NOVA_OP_TYPE_WRITE_BASE_INFO:
					{
                        _NovaWriteBaseInfo(p_dev_op_get->reg_addr, (p_dev_op_get->p_data[1] << 8) | p_dev_op_get->p_data[0]);
                        p_op_info->status = EN_NOVA_OP_STATUS_WAIT_ACK;
                        p_op_info->op_timing_count = tim_get_systime();
                        if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
                        {
                            p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
                            _gNovaWriteCmdNum--;
                        }
                        
						break;
					}
					case EN_NOVA_OP_TYPE_WRITE_SET:
					{
                        _NovaWriteSet(p_dev_op_get->reg_addr, p_dev_op_get->reg_num, p_dev_op_get->p_data);
                        p_op_info->status = EN_NOVA_OP_STATUS_WAIT_ACK;
                        p_op_info->op_timing_count = tim_get_systime();
                        if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
                        {
                            p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
                            _gNovaWriteCmdNum--;
                        }
						break;
					}
					default:
						break;
					}
				}
			break;
		}
		case EN_NOVA_OP_STATUS_WAIT_UPGRADE_DOWNLOAD_TO_FLASH:
		{
			if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
			{
				uint8_t *pdata = (uint8_t *)&p_modbus_dev_rev->p_data[0];
				uint16_t plen = p_modbus_dev_rev->data_size;
				uint8_t md5[16] = {0};

				if (plen == 2 && pdata[0] == 0xDE && pdata[1] == 0xDF)
				{
					NovaDevUpgradeSpiFlashSaveCtx(NovaDevUpgradeFirmwareGetSize(), md5);
				}
				else if (plen == 2 && pdata[0] == 0xDE && pdata[1] == 0xED)
				{
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_DEBUG;
				}
				else
				{
					NovaDevUpgradeSpiFlashWrite(pdata, plen);
				}

				p_op_info->op_timing_count = tim_get_systime();
				timing_upgrade_count = (uint32_t)(tim_get_systime());
			}
			break;
		}
		case EN_NOVA_OP_STATUS_WAIT_UPGRADE_DEBUG:
		{
			if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
			{
				uint8_t *pdata = (uint8_t *)&p_modbus_dev_rev->p_data[0];
				uint16_t plen = p_modbus_dev_rev->data_size;
				NovaDevUpgradeCtxReset();
				p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_DOWNLOAD_TO_FLASH;
				p_op_info->op_timing_count = tim_get_systime();
				timing_upgrade_count = (uint32_t)(tim_get_systime());
			}

			break;
		}
		case EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE:
		{
			DevOp_t dev_op;
			int32_t errcode = 0;
			if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
			{
				errcode = NovaDevUpgradeGetErrorCode((uint8_t *)&p_modbus_dev_rev->p_data[0],
											 p_modbus_dev_rev->data_size);
				if (errcode)
				{
					if (upgrade_resend_count > 10)
					{
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_START);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START;
					}
					else
					{
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_SEND);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_RE_SEND;
					}
					p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
					p_op_info->op_timing_count = tim_get_systime();
					QueuePut(op_queue, &dev_op);
					break;
				}
				switch (NovaDevUpgradeGetCmd((uint8_t *)&p_modbus_dev_rev->p_data[0],
											 p_modbus_dev_rev->data_size))
				{
				case NOVA_DEV_UPGRADE_CMD_FIRMWARE_START:
				case NOVA_DEV_UPGRADE_CMD_FIRMWARE_SEND:
					NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_SEND);
					dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_SEND;
					p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
					p_op_info->op_timing_count = tim_get_systime();
					QueuePut(op_queue, &dev_op);
					break;
				case NOVA_DEV_UPGRADE_CMD_FIRMWARE_END:
					dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_END;
					p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
					p_op_info->op_timing_count = tim_get_systime();

					QueuePut(op_queue, &dev_op);
					break;
				default:
					break;
				}
			}
			else 
			{
				uint32_t cur_time = (uint32_t)(tim_get_systime());
				if (cur_time-timing_upgrade_count > 10*1000)
				{
					if (NovaDevUpgradeStatGet() != NOVA_DEV_UPGRADE_STAT_COMPLETE)
					{
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_START);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START;
					}
					else
					{
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_NONE);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_END;
					}
					p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
					p_op_info->op_timing_count = tim_get_systime();
					QueuePut(op_queue, &dev_op);
				}
			}
			break;
		}
		case EN_NOVA_OP_STATUS_WAIT_ACK:
		{
			if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
			{
				uint8_t rev_slave_addr = p_modbus_dev_rev->p_data[0];
				uint8_t rev_func_code = p_modbus_dev_rev->p_data[1];

				if (gNovaDevCheckRegSize > 0)
				{
					--gNovaDevCheckRegSize;
				}
				if(rev_slave_addr != p_dev_op_get->slave_addr)
				{
					break;//不是给本设备的包
				}
				p_op_info->slave_ack_timeout = false;
				if((rev_func_code & 0x80) == 0x80)//逆变器设备通信异常
				{
					p_op_info->rev_error_code = p_modbus_dev_rev->p_data[2];//rev_func_code & 0x7F;
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
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
					case EN_NOVA_OP_TYPE_READ_TYPE_INFO:
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
							if (!NovaDevCheck(p_dev_op_get, p_modbus_dev_rev))
							{
								p_dev_op_ctx->init_flag = 2;
							}
                            p_op_info->op_timing_count = tim_get_systime();
                        }
						break;
					}
                    case EN_NOVA_OP_TYPE_READ_BASE_INFO:
                    {
                        uint16_t byte_num = p_modbus_dev_rev->p_data[2];
                        
                        if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
                        {
                            p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
                            p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
                            p_op_info->op_timing_count = tim_get_systime();
                        }
                        else//数据OK
                        {
                            uint8_t index = 0;
                            for(index = 0; index < sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]); index++)
                            {
                                if(_gNovaBaseInfoRegDiv[index][0] == p_dev_op_get->reg_addr)
                                {
                                    break;
                                }
                            }
                            if(index == sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]))
                            {
                                p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
                                p_op_info->op_timing_count = tim_get_systime();
                                break;
                            }
                            
                            memcpy((uint8_t *)&_gNovaBaseInfoCache[p_dev_op_get->reg_addr 
								   - 0x5000], &p_modbus_dev_rev->p_data[3], byte_num);
                            _gNovaBaseInfoRegDiv[index][2] = true;
                            p_op_info->status = EN_NOVA_OP_STATUS_ACK_ANALYSIS;
                            p_op_info->op_timing_count = tim_get_systime();
                            
                            if(index == 0)//第一包中包含了电池包数，0x5017
                            {
                                p_op_info->pack_num = (
									 _gNovaBaseInfoCache[0x5017-0x5000-1] & 0x00FF << 8) ;

                                p_op_info->pack_num |= (
									  _gNovaBaseInfoCache[0x5017-0x5000-1] & 0xFF00 >> 8);
                                if(p_op_info->pack_num > 8)
                                {
                                    p_op_info->pack_num = 8;
                                }
                            }
                        }
                        break;
                    }
                    case EN_NOVA_OP_TYPE_READ_SYS_INFO:
                    {
                        uint16_t byte_num = p_modbus_dev_rev->p_data[2];
                        
                        if(p_dev_op_get->reg_num * 2 != byte_num)//数据请求和回读的长度不匹配
                        {
                            p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
                            p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
                            p_op_info->op_timing_count = tim_get_systime();
                        }
                        else//数据OK
                        {
                            memcpy((uint8_t *)&_gNovaSysInfoCache[p_dev_op_get->reg_addr - 3200], &p_modbus_dev_rev->p_data[3], byte_num);
                            p_op_info->status = EN_NOVA_OP_STATUS_ACK_ANALYSIS;
                            p_op_info->op_timing_count = tim_get_systime();
                        }
                        break;
                    }
                    case EN_NOVA_OP_TYPE_WRITE_BASE_INFO:
                    {
                        p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
                        p_op_info->op_timing_count = tim_get_systime();
                        if(p_op_info->write_comm_status == EN_NOVA_WRITE_COMM_STATUS_SENDING)
                        {
                            if(_gNovaWriteCmdNum == 0)
                            {
                                p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_OK;
                            }
                        }
                        break;
                    }
                    case EN_NOVA_OP_TYPE_WRITE_SET:
                    {
                        p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
                        p_op_info->op_timing_count = tim_get_systime();
                        if(p_op_info->write_comm_status == EN_NOVA_WRITE_COMM_STATUS_SENDING)
                        {
                            if(_gNovaWriteCmdNum == 0)
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
				if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > NOVA_DEV_ACK_TIME_OUT)
				{
					if (gNovaDevCheckRegSize > 0)
					{
						--gNovaDevCheckRegSize;
					}
					p_op_info->slave_ack_timeout = true;
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
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
        case EN_NOVA_OP_STATUS_ACK_ANALYSIS:
        {
			p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
            uint8_t base_info_div_num = p_op_info->pack_num;
            if(p_dev_op_get->op_type == EN_NOVA_OP_TYPE_READ_BASE_INFO)
            {
                /*base info copy*/
                uint8_t index = 0;
                for(index = 0; index < base_info_div_num; index++)
                {
                    if(!_gNovaBaseInfoRegDiv[index][2])
                    {
                        break;
                    }
                }
                if(index == base_info_div_num)
                {
                    _NovaBaseInfoUpdate();
                }
            }
            else if(p_dev_op_get->op_type == EN_NOVA_OP_TYPE_READ_SYS_INFO)
            {
                /*sys info copy*/
                _NovaSysInfoUpdate();
            }
//            else if(p_dev_op_get->op_type == EN_NOVA_OP_TYPE_WRITE_BASE_INFO || p_dev_op_get->op_type == EN_NOVA_OP_TYPE_WRITE_SET)
//            {
//                
//            }
            p_op_info->op_timing_count = tim_get_systime();
            break;
        }
        case EN_NOVA_OP_STATUS_WAIT_GAP:
        {
            if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > NOVA_DEV_POLLING_GAP)
            {

				if (gNovaDevCheckRegSize == 0)
				{
					p_dev_op_ctx->init_flag = 0;
				}
                p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
            }
            break;
        }
        
        default:break;
    }
    //NovaBaseSet(0x5012, 0x1234);
    
    if(p_op_info->status != EN_NOVA_OP_STATUS_IDLE)
    {
        if(gCollectorDevInfo.inv_comm_flag != EN_INV_COMM_OK)
        {
            if(p_dev_op_get->op_type == EN_NOVA_OP_TYPE_READ_BASE_INFO)
            {
                gCollectorDevInfo.pcs_info_updated = false;
            }
            else if(p_dev_op_get->op_type == EN_NOVA_OP_TYPE_READ_SYS_INFO)
            {
                gCollectorDevInfo.kstar_sys_info_updated = false;
            }
            else if(p_dev_op_get->op_type == EN_NOVA_OP_TYPE_WRITE_BASE_INFO || p_dev_op_get->op_type == EN_NOVA_OP_TYPE_WRITE_SET)
            {
                if(p_op_info->write_comm_status != EN_DEV_WRITE_COMM_STATUS_NO_ACK)
                {
                    p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_ERROR;
                }
            }
        }
    }
    
    if(gModbusMTURevQueue.valid_num == 0)/*如果队列为空，则写数量也应该清零*/
    {
        _gNovaWriteCmdNum = 0;
    }

	return 0;
}

