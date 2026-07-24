#include "config.h"

#include "fc41d.h"
#include "pcs_op.h"
#include "sys_info.h"
#include "collector.h"
#include "nova_dev_upgrade.h"

#include "pcs_op_nova.h"
#include "pcs_op_kstar.h"
#include "pcs_op_megarevo.h"
#include "pcs_op_suro.h"

Queue_t    gDevOpQueue;

static DevOpCtx_t _gDevOpCtx = {0};
DevOpCtx_t *gDevOpCtx = &_gDevOpCtx;

static DevOpInfo_t _gDevOpInfo;
DevOpInfo_t *gDevOpInfo = &_gDevOpInfo;

uint8_t  gDevReqPacket[320];

int8_t gDevSetting = 0;
uint32_t gDevSettingTimecount = 0;

void DevOpInit()
{
    QueueInit(&gDevOpQueue, DEV_OP_QUEUE_DEPTH, sizeof(DevOp_t));
}


/**
  * @brief  Send Modbus func04 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
void ModbusReadInput(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_READ_INPUT;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    
    ModbusRTUSend(p_packet, 6);
}


/**
  * @brief  Send Modbus func03 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
void ModbusReadRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_READ_REMAIN;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    
    ModbusRTUSend(p_packet, 6);
}


/**
  * @brief  Send Modbus func10 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
void ModbusWriteMultiRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_value)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (reg_num >> 8) & 0xFF;
    p_packet[5] = reg_num & 0xFF;
    p_packet[6] = reg_num * 2;
    memcpy(&p_packet[7], p_reg_value, p_packet[6]);
    
    ModbusRTUSend(p_packet, p_packet[6] + 7);
}


/**
  * @brief  Send Modbus func06 Data.
  * @param  modbus format
  * @return NULL
  * @note   none
  */
void ModbusWriteSingleRemain(uint8_t *p_packet, uint8_t slave_addr, uint16_t reg_addr, uint16_t value)
{
    p_packet[0] = slave_addr;
    p_packet[1] = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
    p_packet[2] = (reg_addr >> 8) & 0xFF;
    p_packet[3] = reg_addr & 0xFF;
    p_packet[4] = (value >> 8) & 0xFF;
    p_packet[5] = value & 0xFF;
    
    ModbusRTUSend(p_packet, 6);
}

EN_DEV_COMM_STATUS DevOpWriteStatusGet()
{
    return _gDevOpInfo.write_comm_status;
}

int32_t DevOpProcessStatus()
{
	return _gDevOpCtx.init_flag;
}

/*大轮询调用*/
void DevOpProcess()
{

#if ONLY_MQTT_TEST
	_gDevOpCtx.init_flag = 2;
	_gDevOpCtx.type = EN_DEV_OP_DEVICE_TYPE_NOVA;
	gCollectorDevInfo.pcs_info_updated = 1;
	memcpy(gCollectorDevInfo.modelid, (uint8_t *)&gCollectorSubDevMap[1].modelid, 
		   sizeof(uint16_t));
	gCollector->pcs_modelid = gCollectorSubDevMap[1].modelid;
	memcpy(gCollectorDevInfo.sn, (uint8_t *)gCollector->sn, 
		   sizeof(gCollectorDevInfo.sn)-1);
#endif 

    static uint32_t timing_read_count = 0xFFFFFFFF - 5 * 1000;
    static DevOp_t *p_dev_op_get;
    static ModbusRTURevData_t *p_modbus_dev_rev;
	static int32_t try_read_addr = (int32_t)EN_DEV_OP_DEVICE_TYPE_NONE;
	int32_t check_dev_type = 0;
    static uint32_t init_flag_timeout = 0;

    /*每隔一定时间，发送读设备寄存器队列*/
	DevOp_t dev_op;

	if (_gDevOpCtx.init_flag == 0)
	{
		init_flag_timeout = tim_get_systime();
		if (!QueueValidSize(&gDevOpQueue))
		{
			_gDevOpCtx.init_flag = 1;
			_gDevOpCtx.type = ((++_gDevOpCtx.type)>=5?1:_gDevOpCtx.type)%5;

			dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;
			dev_op.reg_addr = 0;
			dev_op.reg_num = 0;

#ifdef COLLECTOR_PCS_TYPE_DEBUG
			_gDevOpCtx.init_flag = 0;
			_gDevOpCtx.type = EN_DEV_OP_DEVICE_TYPE_MEGAREVO; // debug
			_gDevOpCtx.type = EN_DEV_OP_DEVICE_TYPE_SURO; // debug
			_gDevOpCtx.type = EN_DEV_OP_DEVICE_TYPE_NOVA; // debug
#endif /* COLLECTOR_PCS_TYPE_DEBUG */
			switch (_gDevOpCtx.type)
			{
			case EN_DEV_OP_DEVICE_TYPE_NOVA:
				check_dev_type = NovaDevCheckReq(&gDevOpQueue);
				break;
			case EN_DEV_OP_DEVICE_TYPE_KSTAR:
				check_dev_type = KstarDevCheckReq(&gDevOpQueue);
				break;
			case EN_DEV_OP_DEVICE_TYPE_MEGAREVO:
				//dev_op.op_type = EN_NOVA_OP_TYPE_READ_TYPE_INFO;
				check_dev_type = MegarevoDevCheckReq(&gDevOpQueue);
				break;
			case EN_DEV_OP_DEVICE_TYPE_SURO:
				//dev_op.op_type = EN_NOVA_OP_TYPE_READ_TYPE_INFO;
				check_dev_type = SuroDevCheckReq(&gDevOpQueue);
				break;
			default:
				break;
			}

			if (check_dev_type != -100)
			{
				timing_read_count = tim_get_systime();
			}
		}
	}
	else if (_gDevOpCtx.init_flag == 1)
	{
		if (tim_get_systime() - init_flag_timeout > 1000*10)
		{
			_gDevOpCtx.init_flag = 0;
			QueueClear(&gDevOpQueue);
			return;
		}
	}
	else if (_gDevOpCtx.init_flag == 2)
	{
		uint32_t t = (uint32_t)(tim_get_systime() - timing_read_count);
		int32_t upgrade_flag = NovaDevUpgradeStatGet();
		uint16_t queue_num = QueueValidSize(&gDevOpQueue);

		if((t > gCollectorDevInfo.read_pcs_period * 1000 
		   || upgrade_flag > NOVA_DEV_UPGRADE_STAT_NONE) && _gDevOpInfo.status == 0)
		{
            
			extern uint8_t _gNovaWriteCmdNum;
			if (!queue_num 
				&& !gDevSetting)
			{
				if (Fc41dCheckUpgrade())
				{
					return;
				}
				dev_op.slave_addr = _gDevOpCtx.addr;

				/*base info*/
				switch (_gDevOpCtx.type)
				{
				case EN_DEV_OP_DEVICE_TYPE_NOVA:
                    NovaDevUpdateWifiStat();
					NovaDevReadBaseInfoReqPush(&gDevOpQueue, &_gDevOpInfo);
					break;
				case EN_DEV_OP_DEVICE_TYPE_KSTAR:
					KstarDevReadBaseInfoReqPush(&gDevOpQueue, &_gDevOpInfo);
					break;
				case EN_DEV_OP_DEVICE_TYPE_MEGAREVO:
					MegarevoDevReadBaseInfoReqPush(&gDevOpQueue, &_gDevOpInfo);
					break;
				case EN_DEV_OP_DEVICE_TYPE_SURO:
					SuroDevReadBaseInfoReqPush(&gDevOpQueue, &_gDevOpInfo);
					break;
				default:
					break;
				}
				if (_gDevOpInfo.loop_flag)
				{
					timing_read_count = tim_get_systime();
				}
			}
		}
        if((uint32_t)(tim_get_systime() - gDevSettingTimecount) > 10000)
        {
            gDevSetting = 0;
        }
	}


	switch (_gDevOpCtx.type)
	{
	case EN_DEV_OP_DEVICE_TYPE_NOVA:
		NovaDevReadBaseInfoReqPop(&_gDevOpCtx, &_gDevOpInfo, &gDevOpQueue, 
								  &gModbusMTURevQueue, p_dev_op_get, 
								  p_modbus_dev_rev);
		break;
	case EN_DEV_OP_DEVICE_TYPE_KSTAR:
		KstarDevReadBaseInfoReqPop(&_gDevOpCtx, &_gDevOpInfo, &gDevOpQueue, 
								   &gModbusMTURevQueue, p_dev_op_get, 
								   p_modbus_dev_rev);
		break;
	case EN_DEV_OP_DEVICE_TYPE_MEGAREVO:
		MegarevoDevReadBaseInfoReqPop(&_gDevOpCtx, &_gDevOpInfo, &gDevOpQueue, 
								   &gModbusMTURevQueue, p_dev_op_get, 
								   p_modbus_dev_rev);
		break;
	case EN_DEV_OP_DEVICE_TYPE_SURO:
		SuroDevReadBaseInfoReqPop(&_gDevOpCtx, &_gDevOpInfo, &gDevOpQueue, 
								   &gModbusMTURevQueue, p_dev_op_get, 
								   p_modbus_dev_rev);
		break;
	default:
		break;
	}

}

const int32_t DevGetInvSysInfo(void **info, uint32_t *cmd_code)
{
	*info = NULL;
	switch (_gDevOpCtx.type)
	{
	case EN_DEV_OP_DEVICE_TYPE_NOVA:
		*cmd_code = PROTO_UNIFORM_R_NOVA_SYS_INFO;
		return NovaDataGet(info);
	case EN_DEV_OP_DEVICE_TYPE_KSTAR:
		*info = (void *)&gKstarInvSysInfo;
		*cmd_code = PROTO_UNIFORM_R_KSTAR_SYS_INFO;
		return sizeof(KstarInvSysInfo_t);
	case EN_DEV_OP_DEVICE_TYPE_MEGAREVO:
		*cmd_code = PROTO_UNIFORM_R_MEGAREVO_SYS_INFO;
		return MegarevoDataGet(info);
	case EN_DEV_OP_DEVICE_TYPE_SURO:
		*cmd_code = PROTO_UNIFORM_R_SURO_SYS_INFO;
		return SuroDataGet(info);
	default:
		return 0;
	}
	return 0;
}

int32_t DevChangeDebugSn()
{
#if COLLECTOR_DEBUG
#include "fc41d.h"
	uint8_t mac[16] = {0};
	if (Fc41dGetBleMac(mac, sizeof(mac)))
	{
		for(int i = 0, j = 0; j < sizeof(mac); ++j) 
		{
			snprintf((char *)&gCollectorDevInfo.sn[i+5], 
					 sizeof(gCollectorDevInfo.sn)-5, 
					 "%02x", mac[j]);
			i += 2;
		}
	}
#endif 

	return 0;
}

int32_t DevGetPcsSn(uint8_t **sn)
{
	*sn = gCollectorDevInfo.sn;
	return sizeof(gCollectorDevInfo.sn);
}

uint16_t DevGetPcsType()
{
	return *(uint16_t*)gCollectorDevInfo.modelid;
}


int32_t DevGetDebugReq(int32_t debug_idx, uint16_t addr, uint16_t reg_num)
{
	switch (_gDevOpCtx.type)
	{
	case EN_DEV_OP_DEVICE_TYPE_NOVA:
		NovaDevCheckReq(&gDevOpQueue);
		break;
	case EN_DEV_OP_DEVICE_TYPE_KSTAR:
		KstarDevCheckReq(&gDevOpQueue);
		break;
	case EN_DEV_OP_DEVICE_TYPE_MEGAREVO:
		MegarevoDevDebugReq(&gDevOpQueue, debug_idx, addr, reg_num);
		break;
	case EN_DEV_OP_DEVICE_TYPE_SURO:
		SuroDevCheckReq(&gDevOpQueue);
		break;
	default:
		break;
	}
	return 0;
}


