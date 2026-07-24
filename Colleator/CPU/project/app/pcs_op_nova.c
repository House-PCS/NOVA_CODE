#include "pcs_op_nova.h"
#include "nova_dev_upgrade.h"
#include "sys_info.h"
#include "collector.h"
#include "proto_nova_uniform.h"
#include "fc41d.h"
#include "dev_fc41d.h"

#define NOVA_DEV_DEBUG_SN "FFE"
#define NOVA_DEV_DEBUG_MONITOR_SN ("AMP01MONITOR"NOVA_DEV_DEBUG_SN)
#define NOVA_DEV_DEBUG_PCS_SN ("AMP01PCSFFFF"NOVA_DEV_DEBUG_SN)

#define NOVA_DEV_UPGRADE_USE_APP_BAUD		0 // 是否使用APP层设置升级波特率(使用后APP层会保存当前设置的波特率)

#define NOVA_DEV_ACK_TIME_OUT   2000
#define NOVA_DEV_POLLING_GAP    200

static int8_t gTTT = 0;

typedef enum
{
    EN_NOVA_OP_TYPE_READ_TYPE_INFO,
    EN_NOVA_OP_TYPE_READ_BASE_INFO,
    EN_NOVA_OP_TYPE_READ_SYS_INFO,
    EN_NOVA_OP_TYPE_WRITE_BASE_INFO,
    EN_NOVA_OP_TYPE_WRITE_SET,

	// 升级相关
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_END_SET_BAUD,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_START,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_SEND,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_END,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_RE_SEND,
	EN_NOVA_OP_TYPE_WRITE_UPGRADE_ALREADY_EXIST,

	// 状态信息收集
    EN_NOVA_OP_TYPE_READ_MONITOR_HARDFAULT,	// 读取监控卡hardfault
    EN_NOVA_OP_TYPE_CLS_MONITOR_TOTALFLASH,	// 清空监控卡统计Flash

    EN_NOVA_OP_TYPE_MAX,	

//    EN_NOVA_OP_TYPE_WRITE_REMOTE,
}EN_NOVA_OP_TYPE;

typedef enum
{
    EN_NOVA_OP_STATUS_IDLE,
    EN_NOVA_OP_STATUS_SEND,
    EN_NOVA_OP_STATUS_WAIT_ACK,
    EN_NOVA_OP_STATUS_ACK_ANALYSIS,
    EN_NOVA_OP_STATUS_WAIT_GAP,

    EN_NOVA_OP_STATUS_WAIT_UPGRADE_START_SET_BAUD,
    EN_NOVA_OP_STATUS_WAIT_UPGRADE_END_SET_BAUD,
    EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE,
    EN_NOVA_OP_STATUS_WAIT_UPGRADE_DEBUG,
    EN_NOVA_OP_STATUS_WAIT_UPGRADE_DOWNLOAD_TO_FLASH,
    
}EN_NOVA_OP_STATUS;

/*第一个字段是寄存器起始地址，
 * 第二个字段是寄存器数量
 * 第三个字段是是否已读
  */
#if 0
static uint16_t _gNovaBaseInfoRegDiv[][5] = {
	{0x5100,	0x30, 0,	0x5100,	0x30}, // [0x5100,0x512F]负载、电网、电池用电量统计
};

#else 
static uint16_t _gNovaBaseInfoRegDiv[][6] = {
	{0x5000,	0x29, 0,	0x5000,	0x29, 0}, // [0x5000,0x5028]产品基本信息(ModeID, SN, 版本,AI模式等信息)
	{0x503A,	0x29, 0,	0x503A,	0x29, 0}, // [0x503A,0x5062]削峰填谷时间及联网状态,电池充放电信息
	{0x5066,	0x2A, 0,	0x5066,	0x2A, 0}, // [0x5066,0x508F]电池基本信息及设备运行状态
	{0x5100,	0x37, 0,	0x5100,	0x37, 0}, // [0x5100,0x5136]负载、电网、电池、远端负载用电量统计
    
    {0x5F80,    0x12, 0,    0x5F80, 0x12, 0}, // [0x5F80,0x5F91]CSIP功率平均值信息

	{0x6000,	0x0F, 0,	0x6000,	0x0F, 0}, // [0x6000,0x600E]逆变器基本信息(ModeID, SN)
	{0x600F,	0x0C, 0,	0x600F,	0x0C, 0}, // [0x600F,0x601A]逆变器运行状态信息
	{0x6020,	0x05, 0,	0x6020,	0x05, 0}, // [0x6020,0x6024]电池侧电压、电流、功率
	{0x6026,	0x13, 0,	0x6026,	0x13, 0}, // [0x6026,0x6038]电表电压、电流、功率(单相只获取R相)
	{0x6039,	0x04, 0,	0x6039,	0x04, 0}, // [0x6039,0x603C]油机信息
	{0x6044,	0x12, 0,	0x6044,	0x12, 0}, // [0x6044,0x6055]逆变电压、电流、功率(单相只获取R相)
	{0x6059,	0x1B, 0,	0x6059,	0x1B, 0}, // [0x6059,0x6073]负载电压、电流、功率
	{0x607C,	0x02, 0,	0x607C,	0x02, 0}, // [0x607C,0x607D]负载总用电量
	{0x6085,	0x02, 0,	0x6085,	0x02, 0}, // [0x6085~0x6086]买电总电量
	//{0x608C,	0x02, 0,	0x608C,	0x02, 0}, // [0x608C~0x608D]卖电总电量
	{0x608C,	0x11, 0,	0x608C,	0x11, 0}, // [0x608C~0x609C]卖电总电量、电池充放电用电量、远端负载用电量
	{0x609F,	0x34, 0,	0x609F,	0x34, 0}, // [0x609F,0x60D2]PV电压、电流、功率、发电量、并网时间
    {0x61E2,    0x10, 0,    0x61E2, 0x10, 0}, // [0x61E2,0x61F1]电池电压及网侧功率相关
	{0x61F6,	0x39, 0,	0x61F6,	0x39, 0}, // [0x61F6,0x622E]逆变器高级信息设置/获取
    
    {0x6A00,    0x20, 0,    0x6A00, 0x20, 0}, // [0x6A00,0x6A1F]CSIP认证上传的pcs功率信息
    
    //bms1 重要信息
	{0x7000,	0x0D, 0,	0x7000,	0x0D, 0}, // [0x7000,0x700C]BMS基本信息获取
	{0x701E,	0x1C, 0,	0x701E,	0x1C, 0}, // [0x701E~0x7039]单节电池详细电压、电流等信息获取
	{0x704B,	0x20, 0,	0x704B,	0x20, 0}, // [0x704B~0x706A]电池容量、充放电次数、状态等信息
	{0x707C,	0x1B, 0,	0x707C,	0x1B, 0}, // [0x707C,0x7096]电芯温度详细信息
	{0x70A8,	0x07, 0,	0x70A8,	0x07, 0}, // [0x70A8~0x70AE]电池错误码
	{0x70C0,	0x08, 0,	0x70C0,	0x08, 0}, // [0x70C0,0x70C7]电池充放电MOS管控制及电池系统时间信息
    
    //bms2 重要信息
	{0x7200,	0x0D, 0,	0x7200,	0x0D, 0}, // [0x7200,0x720C]BMS基本信息获取
	{0x721E,	0x1C, 0,	0x721E,	0x1C, 0}, // [0x721E~0x7239]单节电池详细电压、电流等信息获取
	{0x724B,	0x20, 0,	0x724B,	0x20, 0}, // [0x724B~0x726A]电池容量、充放电次数、状态等信息
	{0x727C,	0x1B, 0,	0x727C,	0x1B, 0}, // [0x727C,0x7296]电芯温度详细信息
	{0x72A8,	0x07, 0,	0x72A8,	0x07, 0}, // [0x72A8~0x72AE]电池错误码
	{0x72C0,	0x08, 0,	0x72C0,	0x08, 0}, // [0x72C0,0x72C7]电池充放电MOS管控制及电池系统时间信息
    
    //bms3 重要信息
	{0x7400,	0x0D, 0,	0x7400,	0x0D, 0}, // [0x7400,0x740C]BMS基本信息获取
	{0x741E,	0x1C, 0,	0x741E,	0x1C, 0}, // [0x741E~0x7439]单节电池详细电压、电流等信息获取
	{0x744B,	0x20, 0,	0x744B,	0x20, 0}, // [0x744B~0x746A]电池容量、充放电次数、状态等信息
	{0x747C,	0x1B, 0,	0x747C,	0x1B, 0}, // [0x747C,0x7496]电芯温度详细信息
	{0x74A8,	0x07, 0,	0x74A8,	0x07, 0}, // [0x74A8~0x74AE]电池错误码
	{0x74C0,	0x08, 0,	0x74C0,	0x08, 0}, // [0x74C0,0x74C7]电池充放电MOS管控制及电池系统时间信息
    
    //bms4 重要信息
	{0x7600,	0x0D, 0,	0x7600,	0x0D, 0}, // [0x7600,0x760C]BMS基本信息获取
	{0x761E,	0x1C, 0,	0x761E,	0x1C, 0}, // [0x761E~0x7639]单节电池详细电压、电流等信息获取
	{0x764B,	0x20, 0,	0x764B,	0x20, 0}, // [0x764B~0x766A]电池容量、充放电次数、状态等信息
	{0x767C,	0x1B, 0,	0x767C,	0x1B, 0}, // [0x767C,0x7696]电芯温度详细信息
	{0x76A8,	0x07, 0,	0x76A8,	0x07, 0}, // [0x76A8~0x76AE]电池错误码
	{0x76C0,	0x08, 0,	0x76C0,	0x08, 0}, // [0x76C0,0x76C7]电池充放电MOS管控制及电池系统时间信息


	{0x9000,	0x0D, 0,	0x9000,	0x0D, 0}, // [0x9000,0x900C]电弧检测寄存器信息
	{0x910B,	0x03, 0,	0x910B,	0x03, 0}, // [0x910B,0x910D]电弧检测灵敏度、绝缘检测使能标记、升级状态
	// {0x9018,	0x01, 0,	0x9018,	0x01, 0}, // [0x9018,0x9018]电弧检测各通道电弧发生状态
	// {0x9049,	0x01, 0,	0x9049,	0x01, 0}, // [0x9049,0x9049]电弧检测绝缘检测阻抗状态信息

};

#endif 


#pragma pack(2)
typedef struct _nova_base_info_inner_stru_t
{
	uint16_t p00[0x29];
	uint16_t p01[0x29];
	uint16_t p02[0x2A];
	uint16_t p05[0x37];
    uint16_t p06[0x12];

	uint16_t p07[0x0F];
	uint16_t p08[0x0C];
	uint16_t p09[0x05];
	uint16_t p10[0x13];
	uint16_t p11[0x04];
	uint16_t p12[0x12];
	uint16_t p13[0x1B];
	uint16_t p14[0x02];
	uint16_t p15[0x02];
	uint16_t p16[0x11];
	uint16_t p17[0x34];
    uint16_t p18[0x10];
	uint16_t p19[0x39];
    uint16_t p20[0x20];

	uint16_t p21[0x0D];
	uint16_t p22[0x1C];
	uint16_t p23[0x20];
	uint16_t p24[0x1B];
	uint16_t p25[0x07];
	uint16_t p26[0x08];
    
    uint16_t p27[0x0D];
	uint16_t p28[0x1C];
	uint16_t p29[0x20];
	uint16_t p30[0x1B];
	uint16_t p31[0x07];
	uint16_t p32[0x08];
    
    uint16_t p33[0x0D];
	uint16_t p34[0x1C];
	uint16_t p35[0x20];
	uint16_t p36[0x1B];
	uint16_t p37[0x07];
	uint16_t p38[0x08];
    
    uint16_t p39[0x0D];
	uint16_t p40[0x1C];
	uint16_t p41[0x20];
	uint16_t p42[0x1B];
	uint16_t p43[0x07];
	uint16_t p44[0x08];

	uint16_t p45[0x0D];
	uint16_t p46[0x03];

	//uint16_t p27[0x0A];
}NovaInvBaseInfoInner_t;

typedef struct _nova_monitor_hardfault_inner_stru_t
{
	HardFaultInfo_t fault;
} NovaMonitorHardfualtInner_t;

#pragma pack()

static NovaInvBaseInfoInner_t _gNovaInvBaseInfoInner = {0};
static NovaMonitorHardfualtInner_t _gNovaMonitorHardfaultInner = {0};

/**
 * @brief 
 *
 * @param idx
 * @param modbus_rev
 *
 * @return 
 */
static int32_t _NovaInvBaseInfoInnerUpdate(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	int32_t i = 0, j = sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]),
			pos = 0;
	uint16_t byte_num = modbus_rev->p_data[2], byte_num2 = 0;
	uint16_t *paddr = (uint16_t *)&_gNovaInvBaseInfoInner;
	for (i = 0; i < j; ++i)
	{
		paddr = (uint16_t *)&_gNovaInvBaseInfoInner;
		paddr += pos;
		if (_gNovaBaseInfoRegDiv[i][2] == 2)
		{
			memset(paddr, 0, _gNovaBaseInfoRegDiv[i][1]);
		}
		else if (_gNovaBaseInfoRegDiv[i][2] == 1)
		{
			if (i == idx)
			{
				if (i == 0)
				{
					int32_t xx = 0;
					xx = 0;
				}

				if (_gNovaBaseInfoRegDiv[idx][0] < _gNovaBaseInfoRegDiv[idx][3])
				{
					paddr += (_gNovaBaseInfoRegDiv[idx][3]
							-_gNovaBaseInfoRegDiv[idx][0]);
				}

				byte_num2 = _gNovaBaseInfoRegDiv[idx][1] 
								- (_gNovaBaseInfoRegDiv[idx][3] 
								- _gNovaBaseInfoRegDiv[idx][0]);
				for(uint16_t i = 0; i < byte_num/2 && i < byte_num2; ++i)
				{
					paddr[i] = (modbus_rev->p_data[3+i*2] << 8) 
						| modbus_rev->p_data[3+i*2+1];
				}
			}
		}

		pos += _gNovaBaseInfoRegDiv[i][1];
	}

	return 0;
}

static int32_t _NovaMonitorHardfaultInnerUpdate(ModbusRTURevData_t *modbus_rev)
{
	uint16_t byte_num = modbus_rev->p_data[2], byte_num2 = sizeof(_gNovaMonitorHardfaultInner);
	uint16_t *paddr = (uint16_t *)&_gNovaMonitorHardfaultInner;

	memset(paddr, 0, byte_num2);
	if (byte_num2 >= byte_num*2)
	{
		byte_num2 = byte_num*2;
	}

	memcpy(paddr, &modbus_rev->p_data[3], byte_num2/2);

	return 0;
}

static uint32_t _gNovaUpdateData = 0;
uint8_t _gNovaWriteCmdNum;//当前待写操作设备的命令数量

static uint16_t  _gNovaBaseInfoCache[800];
static uint16_t _gNovaInvSet[5][88];
static uint8_t  _gNovaInvSetCount;
static uint16_t _gNovaInvSet2[16];
static uint32_t _gNovaDataLen = 0;


// 2代机设备类型检测需要的寄存器
uint16_t gNovaDevCheckRegSize = 0;
uint16_t gNovaDevCheckReg[][3] = 
{
	{0x5000,	0x01, 0},	// Monitor Modelid
	{0x5001, 	0x0A, 0},	// Monitor SN
	{0x506B, 	0x0A, 0},	// Monitor AMP设备标识
	{0x6000, 	0x01, 0},	// pcs modelid
};
static uint8_t _gNovaDevCheckRet[sizeof(uint16_t) * 21];

static int32_t _TestAddr(Queue_t *queue)
{
	DevOp_t dev_op;

	dev_op.slave_addr = NOVA_DEV_ADDR;
	dev_op.op_type = EN_NOVA_OP_TYPE_READ_TYPE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	QueueClear(queue);

	uint16_t reg[][3] = {
		{0X500B,	0x01, 0},	// debug
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


int32_t NovaDevCheckReq(Queue_t *queue)
{
	gNovaInvBaseInfo = (NovaInvBaseInfo_t *)gCommonBaseInfo;
#ifdef COLLECTOR_PCS_TYPE_DEBUG
	return _TestAddr(queue);
#endif 

	DevOp_t dev_op;

	dev_op.slave_addr = NOVA_DEV_ADDR;
	dev_op.op_type = EN_NOVA_OP_TYPE_READ_TYPE_INFO;
	dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;

	gNovaDevCheckRegSize = 0;
	QueueClear(queue);
	for (int32_t i = 0; i < sizeof(gNovaDevCheckReg)/sizeof(gNovaDevCheckReg[0]); ++i)
	{
		dev_op.idx = i;
		dev_op.reg_addr = gNovaDevCheckReg[i][0]; // 逆变器型号
		dev_op.reg_num = gNovaDevCheckReg[i][1];
		QueuePut(queue, &dev_op);
		++gNovaDevCheckRegSize;
	}

	return 0;
}

static int32_t _NovaBaseInfoInit()
{
	// update gCommonBaseInfo
	gNovaInvBaseInfo = (NovaInvBaseInfo_t *)gCommonBaseInfo;

	int32_t i = 0, j = sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]), 
			pos = 0; 
	uint16_t *paddr = (uint16_t *)gNovaInvBaseInfo;
	uint16_t *paddr1 = NULL;

	for (i = 0; i < j; ++i)
	{
		paddr1 = paddr + pos;

		paddr1[0] = _gNovaBaseInfoRegDiv[i][0];
		paddr1[1] = _gNovaBaseInfoRegDiv[i][1];
		paddr1[1] += _gNovaBaseInfoRegDiv[i+1][1];
		paddr1[1] += _gNovaBaseInfoRegDiv[i+2][1];
		pos += 0x02;

		pos += _gNovaBaseInfoRegDiv[i][1];
	}

	gCollectorDevInfo.pcs_info_updated = 0;

	return 0;
}

int32_t NovaDevCheck(uint16_t reg_addr, ModbusRTURevData_t *modbus_rev)
{
	int32_t addr = 0;
	uint8_t index = 0;
	uint8_t *pdata = _gNovaDevCheckRet;
	uint16_t byte_num = modbus_rev->p_data[2];

	for(index = 0; index < sizeof(gNovaDevCheckReg)/sizeof(gNovaDevCheckReg[0]); 
		index++)
	{
		if (index > 0)
		{
			addr += gNovaDevCheckReg[index-1][1] * sizeof(uint16_t);
		}
		if(gNovaDevCheckReg[index][0] == reg_addr)
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

	if (gNovaDevCheckReg[0][2] == 0 || gNovaDevCheckReg[1][2] == 0 
		|| gNovaDevCheckReg[2][2] == 0 || gNovaDevCheckReg[3][2] == 0)
	{
		// 数据不全
		return -2;
	}

	// 数据预处理
	pdata = _gNovaDevCheckRet;
	uint16_t product_type = (uint16_t)pdata[0];
	uint16_t dev_type = (uint16_t)pdata[1];

	// check 合法性
	if (pdata[0] == 0xC2  // modelid判断
		&& !memcmp((uint8_t *)&pdata[22], "AMP-01", sizeof("AMP-01")) // 2代机判断
		)
	{
		gCollectorDevInfo.modelid[0] = _gNovaDevCheckRet[1];
		gCollectorDevInfo.modelid[1] = _gNovaDevCheckRet[0];
		uint16_t modelid = *(uint16_t*)gCollectorDevInfo.modelid;
		uint8_t *p_sn = (uint8_t *)&_gNovaDevCheckRet[2];
		// modbus 大端需要反转
		for (int32_t i = 0; 
			 i < gNovaDevCheckReg[2][1] * sizeof(uint16_t); i+=2)
		{
			gCollectorDevInfo.sn[i] = p_sn[i+1];
			gCollectorDevInfo.sn[i+1] = p_sn[i];
		}
		/*
		memcpy(gCollectorDevInfo.sn, (uint8_t *)&_gNovaDevCheckRet[2], 
			   gNovaDevCheckReg[2][1] * sizeof(uint8_t));
			   */
		gCollector->pcs_modelid = modelid;
		memset(gCollector->pcs_sn, 0, sizeof(gCollector->pcs_sn));
		memcpy(gCollector->pcs_sn, gCollectorDevInfo.sn, 
			   gNovaDevCheckReg[1][1] * sizeof(uint16_t));

		//gCollector->pcs_modelid = *(uint16_t *)(&_gNovaDevCheckRet[42]);

#if NOVA_PCS_SN_DEBUG
		// debug
		/*
		memcpy(gCollectorDevInfo.modelid, (uint8_t *)&gCollectorSubDevMap[5].modelid, 
			   sizeof(uint16_t));*/

		memset(gCollectorDevInfo.sn, 0, sizeof(gCollectorDevInfo.sn));
		memcpy(gCollectorDevInfo.sn, NOVA_DEV_DEBUG_MONITOR_SN, 
			   sizeof(NOVA_DEV_DEBUG_MONITOR_SN));

		/*
		gCollector->pcs_modelid = gCollectorSubDevMap[5].modelid;*/
		memset(gCollector->pcs_sn, 0, sizeof(gCollector->pcs_sn));
		memcpy(gCollector->pcs_sn, NOVA_DEV_DEBUG_PCS_SN, sizeof(NOVA_DEV_DEBUG_PCS_SN));

		DevChangeDebugSn();
		// debug end 
#endif 

		_NovaBaseInfoInit();

		gCollectorDevInfo.pcs_info_updated = 1;
		return 0;
	}

	return -4;
}

int32_t NovaDevReadBaseInfoReqPush(Queue_t *queue, DevOpInfo_t *p_dev_op_info)
{
	int32_t upgrade_flag = NovaDevUpgradeStatGet();
	if (upgrade_flag > NOVA_DEV_UPGRADE_STAT_NONE && upgrade_flag < NOVA_DEV_UPGRADE_STAT_COMPLETE)
	{
		return 0;
	}


	DevOp_t dev_op;

	dev_op.slave_addr = NOVA_DEV_ADDR;

	/*base info*/
	dev_op.op_type = EN_NOVA_OP_TYPE_READ_BASE_INFO;
	static uint32_t i = 0;
	uint32_t j = sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]);

	if (i > j)
	{
		// 第二次轮询，可以开启上报
		++gCollectorDevInfo.pcs_info_updated;
	}

	p_dev_op_info->loop_flag = 0;
	for(;; )
	{
		dev_op.idx = i%j;
		dev_op.func_code = EN_MODBUS_FUNC_CODE_READ_REMAIN;
		dev_op.reg_addr = _gNovaBaseInfoRegDiv[i%j][3];
		dev_op.reg_num = _gNovaBaseInfoRegDiv[i%j][1] - (_gNovaBaseInfoRegDiv[i%j][3] - _gNovaBaseInfoRegDiv[i%j][0]);

		if (!QueuePut(queue, &dev_op))
		{
			break;
		}
        i++;
		if (i > 0 && (i % j == 0))
		{
			p_dev_op_info->loop_flag = 1;
			//++i;
			break;
		}
	}

	return 0;
}


/**
  * @brief  Nova base info read.
  * @param  modbus format
  * @return NULL
  * @note   寄存器较多
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
static void _NovaWriteSetSingle(uint16_t reg_addr, uint16_t reg_data)
{
    ModbusWriteSingleRemain(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_data);
}

/**
  * @brief  Nova Write Set.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static void _NovaWriteSetMulti(uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
    ModbusWriteMultiRemain(gDevReqPacket, NOVA_DEV_ADDR, reg_addr, reg_num, p_reg_data);
}

/**
  * @brief  Nova base info copy to gNovaInvBaseInfo.
  * @param  modbus format
  * @return NULL
  * @note NULL
  */
static int32_t _NovaBaseInfoUpdate(int32_t idx, ModbusRTURevData_t *modbus_rev)
{
	return _NovaInvBaseInfoInnerUpdate(idx, modbus_rev);
}

int32_t NovaDevReadBaseInfoReqPop(DevOpCtx_t *p_dev_op_ctx, DevOpInfo_t *p_op_info, 
								   Queue_t *op_queue, Queue_t *modbus_queue, 
								   DevOp_t *p_dev_op_get1, 
								   ModbusRTURevData_t *p_modbus_dev_rev)
{
    static DevOp_t *p_dev_op_get;
    static uint8_t  func_code_get;
    static uint16_t reg_addr_get;
    static uint16_t reg_num_get;
    static int32_t  op_type_get;	
    static uint32_t upgrade_resend_count = 0;
    static uint32_t timing_upgrade_count = 0;
	static int32_t upgrade_send_count = 0;

	int32_t upgrade_flag = NovaDevUpgradeStatGet();
	if (p_op_info->status == EN_NOVA_OP_STATUS_IDLE)
	{
		DevOp_t dev_op;
		dev_op.slave_addr = NOVA_DEV_ADDR;
		dev_op.func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;

#if NOVA_DEV_UPGRADE_USE_APP_BAUD
		if (upgrade_flag == NOVA_DEV_UPGRADE_STAT_PREPARE)
		{
			upgrade_send_count = 0;
			dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD;
			QueueClear(op_queue);
			QueuePut(op_queue, &dev_op);
		}
		else if (upgrade_flag == NOVA_DEV_UPGRADE_STAT_START)
		{
			dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START;
			QueueClear(op_queue);
			QueuePut(op_queue, &dev_op);
		}
#else
		if (upgrade_flag == NOVA_DEV_UPGRADE_STAT_PREPARE)
		{
			upgrade_send_count = 0;
			DevModbusHostRTUReInit(9600);
			dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START;
			QueueClear(op_queue);
			QueuePut(op_queue, &dev_op);
		}
#endif /* NOVA_DEV_UPGRADE_USE_APP_BAUD */
	}

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
				case EN_NOVA_OP_TYPE_READ_TYPE_INFO:
				case EN_NOVA_OP_TYPE_READ_MONITOR_HARDFAULT:
				{
					_NovaReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
                    func_code_get = p_dev_op_get->func_code;
                    reg_addr_get = p_dev_op_get->reg_addr;
                    reg_num_get = p_dev_op_get->reg_num;
                    op_type_get = p_dev_op_get->op_type;
					p_op_info->status = EN_DEV_OP_STATUS_WAIT_ACK;
					p_op_info->op_timing_count = tim_get_systime();
					break;
				}
				case EN_NOVA_OP_TYPE_READ_BASE_INFO:
				{
					if (p_dev_op_get->idx >= 0 && (p_dev_op_get->idx 
						< sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0])))
					{
						if (p_dev_op_get->idx == 2)
						{
							int32_t yy = 0;
							yy = 1;
						}
						_gNovaBaseInfoRegDiv[p_dev_op_get->idx][2] = 0;
					}
					_NovaReadSysInfo(p_dev_op_get->reg_addr, p_dev_op_get->reg_num);
                    func_code_get = p_dev_op_get->func_code;
                    reg_addr_get = p_dev_op_get->reg_addr;
                    reg_num_get = p_dev_op_get->reg_num;
                    op_type_get = p_dev_op_get->op_type;
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_ACK;
					p_op_info->op_timing_count = tim_get_systime();

					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_SET:
				case EN_NOVA_OP_TYPE_CLS_MONITOR_TOTALFLASH:
				{
					if (p_dev_op_get->reg_addr != 0x5020 && 
						p_dev_op_get->reg_addr != 0x505A && 
						p_dev_op_get->reg_addr != 0x500D)
					{
						gTTT = 1;
						int32_t xx = 0;
						xx = 1;
					}
					if (p_dev_op_get->func_code == EN_MODBUS_FUNC_CODE_WRITE_MULTI)
					{
						_NovaWriteSetMulti(p_dev_op_get->reg_addr, p_dev_op_get->reg_num, p_dev_op_get->p_data);
					}
					else 
					{
						// 为了兼容函数，需要将参数转换为小端序
						uint16_t d = (uint16_t)(*(uint16_t*)(p_dev_op_get->p_data));
                        uint8_t *pd = (uint8_t *)&d;
                        pd[0] = p_dev_op_get->p_data[1];
                        pd[1] = p_dev_op_get->p_data[0];
                        d = (uint16_t)p_dev_op_get->p_data[0] << 8 | (uint16_t)p_dev_op_get->p_data[1];
                        //d = (uint16_t)p_dev_op_get->p_data[0] >> 8 | (uint16_t)p_dev_op_get->p_data[1];
						_NovaWriteSetSingle(p_dev_op_get->reg_addr, d);
					}
                    func_code_get = p_dev_op_get->func_code;
                    reg_addr_get = p_dev_op_get->reg_addr;
                    reg_num_get = p_dev_op_get->reg_num;
                    op_type_get = p_dev_op_get->op_type;
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_ACK;
					p_op_info->op_timing_count = tim_get_systime();
					if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_WAITING_SEND)
					{
						p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_SENDING;
						_gNovaWriteCmdNum--;
					}
					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD:
				{
					_NovaWriteSetSingle(0x501F, 0x04);
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_START_SET_BAUD;
					p_op_info->op_timing_count = tim_get_systime();
					timing_upgrade_count = (uint32_t)(tim_get_systime());
					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_UPGRADE_START:
				{
					upgrade_resend_count = 0;
					NovaDevUpgradeFirmwareStart();
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE;
					p_op_info->op_timing_count = tim_get_systime();
					timing_upgrade_count = (uint32_t)(tim_get_systime());
					NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_START);
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
					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_UPGRADE_SEND:
				{
					if (upgrade_resend_count)
					{
						int32_t xx = 0;
						xx = 1;
					}
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
					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_UPGRADE_ALREADY_EXIST:
				{
					delay_1ms(1000);
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE;
					NovaDevUpgradeFirmwareEnd();
					p_op_info->op_timing_count = tim_get_systime();
					timing_upgrade_count = (uint32_t)(tim_get_systime());
					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_UPGRADE_END:
				{
#if NOVA_DEV_UPGRADE_USE_APP_BAUD
					_NovaWriteSetSingle(0x501F, 0x00);
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_UPGRADE_END_SET_BAUD;
#else
					// 发送升级完成标记后，监控卡自动改回波特率
					DevModbusHostRTUReInit(9600);
#endif /* NOVA_DEV_UPGRADE_USE_APP_BAUD */
					upgrade_resend_count = 0;
					NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_COMPLETE);
					p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
					p_op_info->op_timing_count = tim_get_systime();
					timing_upgrade_count = (uint32_t)(tim_get_systime());
					break;
				}
				default:break;
				}
			}
			else 
			{
//				if (p_dev_op_ctx->init_flag == 1)
//				{
//					p_dev_op_ctx->init_flag = 0;
//				}
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
					NovaDevUpgradeSpiFlashWrite(pdata, plen, 0);
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
		case EN_NOVA_OP_STATUS_WAIT_UPGRADE_START_SET_BAUD:
		case EN_NOVA_OP_STATUS_WAIT_UPGRADE_END_SET_BAUD:
		{
			DevOp_t dev_op;
			if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
			{
				delay_1ms(100);
				uint8_t rev_slave_addr = p_modbus_dev_rev->p_data[0];
				uint8_t rev_func_code = p_modbus_dev_rev->p_data[1];

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

				uint16_t baud = (uint16_t)p_modbus_dev_rev->p_data[4] >> 8 
					| (uint16_t)p_modbus_dev_rev->p_data[5];
				switch (baud)
				{
				case 0:
					DevModbusHostRTUReInit(9600);
					break;
				case 4:
					DevModbusHostRTUReInit(115200);
					break;
				}

				if (p_op_info->status == EN_NOVA_OP_STATUS_WAIT_UPGRADE_START_SET_BAUD)
				{
					NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_START);
				}
				else if (p_op_info->status == EN_NOVA_OP_STATUS_WAIT_UPGRADE_END_SET_BAUD)
				{
					NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_COMPLETE);
				}
				p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
				break;
			}
			else 
			{
				uint32_t cur_time = (uint32_t)(tim_get_systime());
				if (cur_time-timing_upgrade_count > 10*1000)
				{
					uint32_t cur_upgrade_state = NovaDevUpgradeStatGet();
					if (cur_upgrade_state != NOVA_DEV_UPGRADE_STAT_COMPLETE)
					{
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD;
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
		case EN_NOVA_OP_STATUS_WAIT_UPGRADE_WRITE:
		{
			DevOp_t dev_op;
			int32_t errcode = 0, cmd = 0;
			if(QueueGet(modbus_queue, (void *)&p_modbus_dev_rev) == true)
			{
				delay_1ms(200);
				cmd = NovaDevUpgradeGetCmd((uint8_t *)&p_modbus_dev_rev->p_data[0],
											 p_modbus_dev_rev->data_size);
				errcode = NovaDevUpgradeGetErrorCode((uint8_t *)&p_modbus_dev_rev->p_data[0],
											 p_modbus_dev_rev->data_size);
				if (errcode && cmd != NOVA_DEV_UPGRADE_CMD_FIRMWARE_START)
				{
					if (upgrade_resend_count > 10)
					{
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD;
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

				switch (cmd)
				{
				case NOVA_DEV_UPGRADE_CMD_FIRMWARE_START:

#if !NOVA_DEV_UPGRADE_USE_APP_BAUD
					delay_1ms(2000);
					DevModbusHostRTUReInit(115200); // 开始升级并设置波特率
#endif /* NOVA_DEV_UPGRADE_USE_APP_BAUD */
					uint8_t errcode = NovaDevUpgradeGetErrorCode(
										 (uint8_t *)&p_modbus_dev_rev->p_data[0],
										 p_modbus_dev_rev->data_size);
					if (errcode == 1) 
					{
						// 当前状态不能升级
						break;
					}
					else if (errcode == 2)
					{
						++upgrade_send_count;
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_SEND);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_ALREADY_EXIST;
						p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
						p_op_info->op_timing_count = tim_get_systime();
						QueuePut(op_queue, &dev_op);
						break;
					}

				case NOVA_DEV_UPGRADE_CMD_FIRMWARE_SEND:
					++upgrade_send_count;
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
				if (cur_time-timing_upgrade_count > 3*1000)
				{
					uint32_t cur_upgrade_state = NovaDevUpgradeStatGet();
					if (upgrade_resend_count > 10)
					{
						if (cur_upgrade_state != NOVA_DEV_UPGRADE_STAT_COMPLETE)
						{
							NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
							dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD;
						}
						else
						{
							NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_NONE);
							dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_END;
						}
						p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
						NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
						dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_UPGRADE_START_SET_BAUD;
					}
					else
					{
						if (cur_upgrade_state == NOVA_DEV_UPGRADE_STAT_START)
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
					}

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

				if (reg_addr_get != 0x5020 && 
					reg_addr_get != 0x505A && 
					reg_addr_get != 0x500D && gTTT == 1)
				{
					gTTT = 0;
					int32_t xx = 0;
					xx = 1;
				}

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
				if(rev_func_code != func_code_get)
				{
                    gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_ERROR_CODE;
					break;//功能码不匹配
				}
				gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_OK;
				p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_NULL;
				uint16_t byte_num = 0;
				switch(op_type_get)
				{
				case EN_NOVA_OP_TYPE_READ_TYPE_INFO:
				{
					byte_num = p_modbus_dev_rev->p_data[2];
					
					if(reg_num_get * 2 != byte_num)//数据请求和回读的长度不匹配
					{
						p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
						p_op_info->status = EN_DEV_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
					}
					else//数据OK
					{
						if (!NovaDevCheck(reg_addr_get, p_modbus_dev_rev))
						{
							p_dev_op_ctx->init_flag = 2;
						}
                        gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_OK;
						p_op_info->status = EN_DEV_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
					}
					break;
				}
				case EN_NOVA_OP_TYPE_READ_BASE_INFO:
				{
					uint16_t byte_num = p_modbus_dev_rev->p_data[2];

					if(reg_num_get * 2 != byte_num)//数据请求和回读的长度不匹配
					{
						p_op_info->rev_error_code = EN_MODBUS_ERROR_CODE_SLAVE_DATA_WRONG;
						p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();
					}
					else//数据OK
					{
						uint8_t index = 0;
                        gCollectorDevInfo.inv_comm_flag = EN_INV_COMM_OK;
						for(index = 0; index < sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]); index++)
						{
							if(_gNovaBaseInfoRegDiv[index][0] == reg_addr_get)
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
                        
						_gNovaBaseInfoRegDiv[index][2] = 1;

						_NovaBaseInfoUpdate(index, p_modbus_dev_rev);

						p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
						p_op_info->op_timing_count = tim_get_systime();

					}
					break;
				}
				case EN_NOVA_OP_TYPE_READ_MONITOR_HARDFAULT:
				{
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
					p_op_info->op_timing_count = tim_get_systime();
					if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
					{
						_NovaMonitorHardfaultInnerUpdate(p_modbus_dev_rev);
						p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_OK;
						if(_gNovaWriteCmdNum == 0)
						{
							gDevSetting = 0;
						}
						else
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
						}
					}
					break;
				}
				case EN_NOVA_OP_TYPE_WRITE_SET:
				{
					if (rev_func_code == 0x06 || rev_func_code == 0x10)
					{
						int32_t xx = 0;
						xx = 1;
						if (reg_addr_get != 0x505A)
						{
							xx = 2;
						}
					}
					p_op_info->status = EN_NOVA_OP_STATUS_WAIT_GAP;
					p_op_info->op_timing_count = tim_get_systime();
					if(p_op_info->write_comm_status == EN_DEV_WRITE_COMM_STATUS_SENDING)
					{
						p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_ACK_OK;
						if(_gNovaWriteCmdNum == 0)
						{
							gDevSetting = 0;
						}
						else
						{
							p_op_info->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
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
			p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
			break;
		}
        case EN_NOVA_OP_STATUS_WAIT_GAP:
        {
            if((uint32_t)(tim_get_systime() - p_op_info->op_timing_count) > NOVA_DEV_POLLING_GAP)
            {
//				if (p_dev_op_ctx->init_flag == 1 && gNovaDevCheckRegSize == 0)
//				{
//					p_dev_op_ctx->init_flag = 0;
//				}
                p_op_info->status = EN_NOVA_OP_STATUS_IDLE;
            }
            break;
        }
	}

	return 0;
}

uint16_t _NovaUserTypeGet()
{
	uint8_t *paddr1 = (uint8_t *)&_gNovaInvBaseInfoInner;
	uint16_t *paddr2 = (uint16_t *)&paddr1[26];
	return *paddr2;
}

int32_t _NovaInvSetUserType(int32_t idx, EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, 
						 uint16_t reg_num, uint8_t *p_reg_data)
{
    DevOp_t dev_op;
    uint16_t index;
    uint8_t *p_origin = p_reg_data;
    
    /*大小端转换*/
    for(uint16_t i = idx, j = 0; j < reg_num && i < sizeof(_gNovaInvSet2); j++, i++)
    {
        _gNovaInvSet2[i] = (p_origin[2 * j] << 8) | p_origin[2 * j + 1];
        //_gNovaInvSet2[i] = (p_origin[2 * j]) | p_origin[2 * j + 1];
    }
    
    
    dev_op.op_type = EN_NOVA_OP_TYPE_WRITE_SET;
    dev_op.slave_addr = NOVA_DEV_ADDR;
    dev_op.func_code = func_code;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = reg_num;
    dev_op.p_data = (uint8_t *)&_gNovaInvSet2[idx];
    if(QueueJumpPut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
		++_gNovaWriteCmdNum;
		gDevSetting = 1;
        gDevSettingTimecount = tim_get_systime();
    }
	return idx+reg_num;
}

int32_t NovaInvSetAppend(int32_t idx, int32_t op_type, EN_MODBUS_FUNC_CODE func_code, 
						 uint16_t reg_addr, uint16_t reg_num, uint8_t *p_reg_data)
{
	if (op_type < EN_NOVA_OP_TYPE_READ_TYPE_INFO || op_type >= EN_NOVA_OP_TYPE_MAX)
	{
		return -1;
	}

    DevOp_t dev_op;
    uint16_t index;
    uint8_t *p_origin = p_reg_data;
    
    /*大小端转换*/
    for(uint16_t i = idx, j = 0; j < reg_num && i < sizeof(_gNovaInvSet[0]); j++, i++)
    {
        _gNovaInvSet[_gNovaInvSetCount][i] = (p_origin[2 * j] << 8) | p_origin[2 * j + 1];
    }
    
    
    dev_op.op_type = (EN_NOVA_OP_TYPE)op_type;
    dev_op.slave_addr = NOVA_DEV_ADDR;
    dev_op.func_code = func_code;
    dev_op.reg_addr = reg_addr;
    dev_op.reg_num = reg_num;
    dev_op.p_data = (uint8_t *)&_gNovaInvSet[_gNovaInvSetCount][idx];
    _gNovaInvSetCount++;
    if(_gNovaInvSetCount >= sizeof(_gNovaInvSet)/sizeof(_gNovaInvSet[0]))
    {
        _gNovaInvSetCount = 0;
    }
    if(QueueJumpPut(&gDevOpQueue, &dev_op) == true)
    {
        gDevOpInfo->write_comm_status = EN_DEV_WRITE_COMM_STATUS_WAITING_SEND;
		++_gNovaWriteCmdNum;
		gDevSetting = 1;
        gDevSettingTimecount = tim_get_systime();
    }
	return idx+reg_num;
}

static int32_t _NovaUpdateTargetRegsterValue(int32_t idx, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data)
{
	int32_t i = 0, j = sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]), k = -1,
			m = 0, n = 0, l = 0, p = 0, pos = 0;
	uint16_t *paddr = (uint16_t *)&_gNovaInvBaseInfoInner;
	uint16_t *paddr1 = NULL;
	uint16_t *paddr2 = NULL;

	// 1. find reg scope in _gNovaBaseInfoRegDiv
	for (i = 0; i < j; ++i)
	{
		if (_gNovaBaseInfoRegDiv[i][2] == 2)
		{
			continue;
		}
		paddr1 = paddr + pos;
		// 统计上报寄存器段所有寄存器地址数量
		n = _gNovaBaseInfoRegDiv[i][1];
		for (m = i+1; m < j; ++m)
		{
			if (_gNovaBaseInfoRegDiv[m][5] != 1)
			{
				break;
			}
			n += _gNovaBaseInfoRegDiv[m][1];
		}
		// paddr1[0];  // reg start
		// paddr1[1];  // reg num
		if (reg_addr >= _gNovaBaseInfoRegDiv[i][0])
		{
			p = m-1;

			// 判断寄存器数量是否超过当前scope范围
			l = reg_addr-_gNovaBaseInfoRegDiv[i][0];
			if (l + reg_num < n)
			{
				k = i;
				paddr2 = &paddr1[l];
				// 2. update target reg
				// memcpy((uint8_t *)paddr2, (uint8_t *)&_gNovaInvSet[idx], reg_num*2);
				memcpy((uint8_t *)paddr2, (uint8_t *)p_reg_data, reg_num*2);
				return 0;
			}
			i = p;
		}
		pos += n;
	}

	return -1;
}

int32_t NovaInvSet(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data)
{
	if (Fc41dCheckUpgrade())
	{
		return -1;
	}
	uint16_t user_type = 0;
	uint16_t send_len = 0;
    uint8_t *p_origin = p_reg_data;

	int32_t upgrade_flag = NovaDevUpgradeStatGet();
	if (upgrade_flag > NOVA_DEV_UPGRADE_STAT_NONE && upgrade_flag < NOVA_DEV_UPGRADE_STAT_COMPLETE)
	{
		return 0;
	}

    if (reg_addr >= 0x5A30 && reg_addr <= 0x5ff0)
	{
		int32_t xx = 0;
		xx = 1;
	}    
    //?????
	NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_WRITE_SET, func_code, reg_addr, reg_num, 
					 (uint8_t *)p_reg_data);                                                                                      //?????
	_NovaUpdateTargetRegsterValue(send_len, reg_addr, reg_num, (uint8_t *)p_reg_data);

#if 1
	user_type = _NovaUserTypeGet();
	if (user_type != 1)
	{
		user_type = 1;
		// 设置用户权限
		send_len = _NovaInvSetUserType(send_len, EN_MODBUS_FUNC_CODE_WRITE_SINGLE, 0x500D, 1, 
									(uint8_t *)&user_type);
	}
#endif 

	return 0;
}

int32_t NovaDataGetByAddr(uint8_t *data, uint16_t data_len, uint16_t reg_addr, uint16_t reg_len)
{
	if (data == NULL || data_len == 0)
	{
		return -1;
	}

	int32_t i = 0, 
			j = sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]), 
			k = 0, 
			pos = 0, pos_start = 0,
			total_len = 0;
	uint16_t cp_len = 0;
	uint16_t *paddr = (uint16_t *)data;
	uint16_t *paddr1 = NULL;
	uint16_t *paddr2 = (uint16_t *)&_gNovaInvBaseInfoInner;

	memset(data, 0, data_len);

	for (i = 0; i < j; ++i)
	{
		if (_gNovaBaseInfoRegDiv[i][0] != reg_addr)
		{
			// 找到需要获取的寄存器参数
			paddr2 += _gNovaBaseInfoRegDiv[i][1]; // 找到下一个寄存器参数
			continue;
		}

		if (data_len - pos < _gNovaBaseInfoRegDiv[i][1])
		{
			// 数据溢出
			return -1;
		}

		cp_len = 0;
		if (reg_len == 0)
		{
			cp_len = _gNovaBaseInfoRegDiv[i][1];
		}
		else if (reg_len <= _gNovaBaseInfoRegDiv[i][1])
		{
			cp_len = reg_len;
		}
		else 
		{
			cp_len = _gNovaBaseInfoRegDiv[i][1];
		}
		paddr1 = paddr + pos;

		pos_start = pos;
		paddr1[0] = _gNovaBaseInfoRegDiv[i][0];
		paddr1[1] = reg_len; // 设置待返回的实际寄存器长度，可能会多退少补
		total_len += 0x04+paddr1[1]*2;
		memcpy(&paddr1[2], paddr2, 2* /* paddr1[1] */ cp_len); // 拷贝实际“多退少补”的寄存器长度
		paddr2 += _gNovaBaseInfoRegDiv[i][1]; // 找到下一个寄存器参数
		pos += 0x02 + paddr1[1];
		paddr1 += 2;
		k = 1;
		if (k)
		{
			pos_start = pos;
		}
		break;
	}
	_gNovaDataLen = pos;
	_gNovaDataLen = total_len;
	return _gNovaDataLen;
}

int32_t NovaDataGet(void **data)
{
	gNovaInvBaseInfo = (NovaInvBaseInfo_t *)gCommonBaseInfo;
	int32_t i = 0, 
			j = sizeof(_gNovaBaseInfoRegDiv)/sizeof(_gNovaBaseInfoRegDiv[0]), 
			k = 0, 
			pos = 0, pos_start = 0,
			total_len = 0;
	uint16_t *paddr = (uint16_t *)gNovaInvBaseInfo;
	uint16_t *paddr1 = NULL;
	uint16_t *paddr2 = (uint16_t *)&_gNovaInvBaseInfoInner;

	memset(paddr, 0, sizeof(gCommonBaseInfo));

	for (i = 0; i < j; ++i)
	{
		if (_gNovaBaseInfoRegDiv[i][2] == 2)
		{
			//paddr2 += _gNovaBaseInfoRegDiv[i][1];
			continue;
		}

		paddr1 = paddr + pos;

		pos_start = pos;
		paddr1[0] = _gNovaBaseInfoRegDiv[i][0];
		paddr1[1] = _gNovaBaseInfoRegDiv[i][1];
		total_len += 0x04+paddr1[1]*2;
		memcpy(&paddr1[2], paddr2, 2*paddr1[1]);
		paddr2 += paddr1[1];
		pos += 0x02 + paddr1[1];
		paddr1 += 2;
		k = 1;
		if (k)
		{
			pos_start = pos;
		}
	}
	_gNovaDataLen = pos;
	_gNovaDataLen = total_len;
	*data = (void *)gNovaInvBaseInfo;
	return _gNovaDataLen;
}

int32_t NovaDevUpdateWifiStat()
{
	uint16_t stat = 1;
	if (Fc41dWifiStatGet() > 0)
	{
		++stat;
	}
	if (Fc41dMqttStatGet() > 0)
	{
		++stat;
	}

	return NovaInvSet(EN_MODBUS_FUNC_CODE_WRITE_SINGLE, 0x505A, 1, (uint8_t *)&stat);
}

int32_t NovaDevUpdateMonitorSN(const char *sn, int32_t sn_len)
{
	if (sn_len <= 0 || sn_len > 19)
	{
		return -1;
	}
	uint8_t _sn[20] = {0};
	uint16_t user_type = 0;
	uint16_t send_len = 0;
	memcpy(_sn, (uint8_t *)sn, sn_len);
	user_type = _NovaUserTypeGet();
	// 设置SN
	send_len = NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_WRITE_SET, 
								EN_MODBUS_FUNC_CODE_WRITE_MULTI, 0x5001, sizeof(_sn)/2, _sn);

	if (user_type != 2)
	{
		user_type = 2;
		// 设置用户权限
		send_len = NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_WRITE_SET, 
									EN_MODBUS_FUNC_CODE_WRITE_SINGLE, 0x500D, 1, 
									(uint8_t *)&user_type);
	}
	return 0;
}

int32_t NovaDbgAuth(const uint8_t *data, int32_t len)
{
	if (data == NULL || len <= 0)
	{
		return -1;
	}
	uint16_t user_type = 0;
	uint16_t send_len = 0;

	char idx[8] = {0};
	memcpy(idx, data, 6);

	if (idx[0] == 0x00 && idx[1] == 0x01)
	{
		// 设置并网功率软限制
		send_len = NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_WRITE_SET, 
									EN_MODBUS_FUNC_CODE_WRITE_MULTI, 0x6252, 1, (uint8_t *)&idx[2]);
	}
	else if (idx[0] == 0x00 && idx[1] == 0x02)
	{
		// 设置并网功率硬限制
		send_len = NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_WRITE_SET, 
									EN_MODBUS_FUNC_CODE_WRITE_MULTI, 0x6253, 1, (uint8_t *)&idx[2]);
	}

	user_type = _NovaUserTypeGet();
	if (user_type != 1)
	{
		user_type = 1;
		// 设置用户权限
		send_len = NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_WRITE_SET, 
									EN_MODBUS_FUNC_CODE_WRITE_SINGLE, 0x500D, 1, 
									(uint8_t *)&user_type);
	}

	return 0;
}

int32_t NovaDbgClsMonitorTotalFlash(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data)
{
	uint16_t user_type = 0;
	uint16_t send_len = 0;
    uint8_t *p_origin = p_reg_data;

	if (reg_addr != 0x505A)
	{
		int32_t xx = 0;
		xx = 1;
	}

	NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_CLS_MONITOR_TOTALFLASH, func_code, reg_addr, 
					 reg_num, (uint8_t *)p_reg_data);
	return 0;
}

int32_t NovaDbgGetMonitorHardfault(EN_MODBUS_FUNC_CODE func_code, uint16_t reg_addr, uint16_t reg_num, 
					   uint8_t *p_reg_data)
{
	uint16_t user_type = 0;
	uint16_t send_len = 0;
    uint8_t *p_origin = p_reg_data;

	if (reg_addr != 0x505A)
	{
		int32_t xx = 0;
		xx = 1;
	}

	NovaInvSetAppend(send_len, EN_NOVA_OP_TYPE_READ_MONITOR_HARDFAULT, func_code, reg_addr, 
					 reg_num, (uint8_t *)p_reg_data);
	return 0;
}

uint8_t* NovaDbgGetMonitorHardfualtInfo(void)
{
	return (uint8_t *)&_gNovaMonitorHardfaultInner;
}


