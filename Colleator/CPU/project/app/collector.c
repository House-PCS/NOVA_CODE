#include "collector.h"
#include "fc41d.h"
#include "systick.h"
#include "kstar.h"
#include "sys_info.h"
#include "proto_nova_uniform.h"
#include "base64.h"
#include "miniz.h"
#include "md5.h"
#include "aes2.h"
//#include "rsa2048.h"
//#include "rsa_key.h"
#include "dev_board_id.h"
#include "pcs_op.h"
#include "pcs_op_kstar.h"
#include "peri_process.h"
#include "nova_dev_upgrade.h"

#define COLLECTOR_FIRMWARE_ADDR			((uint32_t)0x08007f00+4)
#define COLLECTOR_SN_ADDR				((uint32_t)0x200200)

#define COLLECTOR_CFG_ADDR_FLAG			((uint32_t)0x202000)
#define COLLECTOR_CFG_ADDR				((uint32_t)0x202100)

#define COLLECTOR_CFG_ADDR_FLAG_BAK		((uint32_t)0x203000)
#define COLLECTOR_CFG_ADDR_BAK			((uint32_t)0x203100)

#define COLLECTOR_CFG_INFO_FLAG			0x0A0A0A0A

#define SN_HEAD_ADDR					0x200000
#define SN_HEAD_BACKUP_ADDR      		0x202000
#define SN_ADDR							(SN_HEAD_ADDR + 0x200)
#define SN_BACKUP_ADDR           		(SN_HEAD_BACKUP_ADDR + 0x200)

#define COLLECTOR_TYPE_WIFI				0xC100						// 默认类型
#define COLLECTOR_TYPE_4G				0xC101						// 默认类型

#if COLLECTOR_DEBUG
#define COLLECTOR_SN_DEFAULT			"COLLECTOR1FFFFFFF"		// 默认SN
#else
#define COLLECTOR_SN_DEFAULT			"COLLECTOR0FFFFFFF"		// 默认SN
#endif 
#define COLLECTOR_FIRMWARE_DEFAULT		"0.0.1"						// 默认固件版本


#define COLLECTOR_DC_AC_TYPE			0x0000						// 默认逆变器类型
#if COLLECTOR_DEBUG
#define COLLECTOR_DC_AC					"BluE-S 5000D M1"			// 默认逆变器SN
#else
#define COLLECTOR_DC_AC					"-ERROR"					// 默认逆变器SN
#endif 

#define COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD_MAX			300		// 秒
#if COLLECTOR_DEBUG
#define COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD_DEFAULT		10		// 秒
#else
#define COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD_DEFAULT		60		// 秒
#endif 

#define COLLECTOR_PROPERTY_DATA_GET_PERIOD_MAX				300		// 秒
#define COLLECTOR_PROPERTY_DATA_GET_PERIOD_DEFAULT			60		// 秒

static int32_t gCollectInit = 0;
#if COLLECTOR_DEBUG
const uint8_t gCollectorName[] = {"NovaCollector"};
#else
const uint8_t gCollectorName[] = {
	0x41, 0x4D, 0x50,
	0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46, 0x46,
};				// 默认名称
/*
const uint8_t gCollectorName[] = {
	0xE6, 0x88, 0x91, 
	0xE7, 0x9A, 0x84, 
	0xE8, 0xAE, 0xBE, 
	0xE5, 0xA4, 0x87,
};				// 默认名称
*/
#endif 

typedef struct _collector_spi_flash_sector_512_t
{
	uint8_t flag[4];			// 0x200000~0x200003
	uint8_t reserved[150];		// 0x200004~0x200099
	uint8_t modelid[2];			// 0x200100~0x200199
	uint8_t sn[0];				// 0x200200~0x200299
} CollectorSpiFlashSector_512_t;

const CollectorMqttTopic_t gCollectorMqttTopicSub[] = 
{
	{COLLECTOR_MQTT_TOPIC_TYPE_SUB, 2, "EMS-PUB/%d/"}
};

const CollectorMqttTopic_t gCollectorMqttTopicPub[] = 
{
	{COLLECTOR_MQTT_TOPIC_TYPE_PUB, 1, "EMS/%d/"},
};

typedef struct _collector_upload_func_t 
{
	int32_t (*fn)(void *userdata);
	void *userdata;
} CollectorUploadFunc_t;

static uint8_t _gCollectorData[1024+512] = {0};
Collector_t *gCollector = (Collector_t *)_gCollectorData;
char gFirmwareVer[64] = {0};

static int32_t _CollectorUpdateWifiInfo(void *userdata);
static int32_t _CollectorUploadDevInfo(void *userdata);
static int32_t _CollectorUploadInvSysInfo(void *userdata);
static int32_t _CollectorCheckMqttSubTopicStatus(void *userdata);
static CollectorUploadFunc_t gCollectorUploadFunc[] = 
{
	{ _CollectorCheckMqttSubTopicStatus, NULL},
	{ _CollectorUpdateWifiInfo, NULL},
	{ _CollectorUploadDevInfo, NULL},
	{ _CollectorUploadInvSysInfo, NULL},
#if KSTAR_PCS_BAUD_RATE
	{ _CollectorUploadKstarInvSysInfo, NULL},
	{ _CollectorUploadKstarInvBaseInfo, NULL},
#endif 
};

int32_t CollectorSetProperty(uint8_t idx, const uint8_t *data, uint8_t len)
{
	CollectorProperty_t *property = gCollector->property;

	for (int32_t i = 0; 
		 (len+sizeof(uint16_t)) < (((uint8_t*)property)-((uint8_t*)&_gCollectorData)); 
		 ++i,
		 property = (CollectorProperty_t *)((&property->data)+sizeof(uint8_t)*property->len))
	{
		if (property->idx == idx)
		{
			// update
			property->len = sizeof(uint8_t)*len;
			uint8_t *tmp = &property->data;
			memcpy(tmp, data, len);
            break;
		}
		else if (property->idx == 0)
		{
			// new
			++gCollector->property_size;
			property->idx = idx;
			property->len = sizeof(uint8_t)*len;
			uint8_t *tmp = &property->data;
			memcpy(tmp, data, len);
			return 0;
		}
	}


	return -1;
}

uint8_t* CollectorGetProperty(uint8_t idx, uint8_t *len)
{
	CollectorProperty_t *property = gCollector->property;

	for (int32_t i = 0; i < gCollector->property_size; ++i,
		 property = (CollectorProperty_t *)((&property->data)+sizeof(uint8_t)*property->len))
	{
		if (property->idx == idx)
		{
			*len = property->len;
			return &property->data;
		}
	}


	return NULL;
}

void CollectorSetDefault()
{
	CollectorProperty_t *property = NULL;
	uint16_t tmp_value = 0;

	memset(_gCollectorData, 0, sizeof(_gCollectorData));
	memcpy(gCollector->sn, COLLECTOR_SN_DEFAULT, strlen(COLLECTOR_SN_DEFAULT));
	memcpy(gCollector->name, gCollectorName, sizeof(gCollectorName));
	memcpy(gCollector->firmware, COLLECTOR_FIRMWARE_DEFAULT, strlen(COLLECTOR_FIRMWARE_DEFAULT));
	gCollector->pcs_modelid = COLLECTOR_DC_AC_TYPE;

	tmp_value = COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD_DEFAULT;
	CollectorSetProperty(COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD, 
						 (const uint8_t *)&tmp_value, sizeof(uint16_t));

	tmp_value = COLLECTOR_PROPERTY_DATA_GET_PERIOD_DEFAULT;
	CollectorSetProperty(COLLECTOR_PROPERTY_DATA_GET_PERIOD, 
						 (const uint8_t *)&tmp_value, sizeof(uint16_t));

    gCollector->property_size = 2;

	gCollector->head.s_url_len = 0;

	// cal crc
	uint16_t crc = calculateCRC16((const uint8_t *)&gCollector->type, 
								  sizeof(_gCollectorData)-sizeof(CollectorHead_t));
	gCollector->head.crc = crc;
}

void SNRead(char *p)
{
    uint8_t buf[256];
    SpiFlashReadData(SN_HEAD_ADDR, buf, 4);
    if(buf[0] != 0x0A || buf[1] != 0x0A || buf[2] != 0x0A || buf[3] != 0x0A)
    {
        SpiFlashReadData(SN_HEAD_BACKUP_ADDR, buf, 4);
        if(buf[0] != 0x0A || buf[1] != 0x0A || buf[2] != 0x0A || buf[3] != 0x0A)
        {
            uint8_t i;
            /*Ä¬ÈÏSN*/
            for(i = 0; i < 20; i++)
            {
                p[i] = 'A';
            }
            p[i] = 0x00;
        }
        else
        {
            SpiFlashFastReadData(SN_BACKUP_ADDR, buf, 256);
            /*ÔÝ¶¨²»³¬¹ý20¸ö×Ö·û£¬²»°üº¬\0*/
            if(strlen((char *)buf) > 20)
            {
                memcpy(p, buf, 20);
                p[20] = 0x00;
            }
            else
            {
                strcpy((char *)p, (char *)buf);
            }
        }
    }
    else
    {
        SpiFlashReadData(SN_ADDR, buf, 256);
        /*ÔÝ¶¨²»³¬¹ý20¸ö×Ö·û£¬²»°üº¬\0*/
        if(strlen((char *)buf) > 20)
        {
            memcpy(p, buf, 20);
            p[20] = 0x00;
        }
        else
        {
            strcpy((char *)p, (char *)buf);
        }
    }
}

int32_t CollectorWriteDefaultInfo()
{

	CollectorSetDefault();
	memcpy(gCollector->firmware, gFirmwareVer, sizeof(gFirmwareVer));
	CollectorSpiFlashWriteCfg();
	return 0;
}

int32_t CollectorSpiFlashEraseCfg()
{
	uint32_t start_addr = COLLECTOR_CFG_ADDR;

	uint16_t crc = calculateCRC16((const uint8_t *)&gCollector->type, 
						 sizeof(_gCollectorData)-sizeof(CollectorHead_t));
	gCollector->head.crc = 0;

    SpiFlashWriteBytesWithErase(start_addr, (uint8_t *)_gCollectorData, sizeof(_gCollectorData));
	return 0;
}

int32_t CollectorSpiFlashWriteCfg()
{
	// update slave
	uint32_t start_addr = 0;
	uint32_t flag = 0x00;

	uint16_t crc = calculateCRC16((const uint8_t *)&gCollector->type, 
						 sizeof(_gCollectorData)-sizeof(CollectorHead_t));
	gCollector->head.crc = crc;

	start_addr = COLLECTOR_CFG_ADDR_FLAG_BAK;
	SpiFlashEraseSector(start_addr);

	start_addr = COLLECTOR_CFG_ADDR_BAK;
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr, (uint8_t *)_gCollectorData, 
											sizeof(_gCollectorData));

    SpiFlashWriteBytesWithoutErasenoAligned(start_addr+sizeof(_gCollectorData), 
								(uint8_t *)&gCollector->head.s_url_len, 
								sizeof(gCollector->head.s_url_len));
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr+sizeof(_gCollectorData)
								+sizeof(gCollector->head.s_url_len), 
								(uint8_t *)gCollector->head.s_url, 
								gCollector->head.s_url_len);

	start_addr = COLLECTOR_CFG_ADDR_FLAG_BAK;
	flag = COLLECTOR_CFG_INFO_FLAG;
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr, (uint8_t *)&flag, sizeof(flag));


	// update master
	start_addr = COLLECTOR_CFG_ADDR_FLAG;
	SpiFlashEraseSector(start_addr);

	start_addr = COLLECTOR_CFG_ADDR;
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr, (uint8_t *)_gCollectorData, 
											sizeof(_gCollectorData));

    SpiFlashWriteBytesWithoutErasenoAligned(start_addr+sizeof(_gCollectorData), 
								(uint8_t *)&gCollector->head.s_url_len, 
								sizeof(gCollector->head.s_url_len));
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr+sizeof(_gCollectorData)
								+sizeof(gCollector->head.s_url_len), 
								(uint8_t *)gCollector->head.s_url, 
								gCollector->head.s_url_len);

	start_addr = COLLECTOR_CFG_ADDR_FLAG;
	flag = COLLECTOR_CFG_INFO_FLAG;
    SpiFlashWriteBytesWithoutErasenoAligned(start_addr, (uint8_t *)&flag, sizeof(flag));

	return 0;
}

static int32_t _CollectorSpiFlashReadCfg()
{
	uint32_t *p = (uint32_t *)(COLLECTOR_FIRMWARE_ADDR);
	uint8_t *p_ver = (uint8_t *)p;
	snprintf(gFirmwareVer, sizeof(gFirmwareVer), "%d.%d.%d", p_ver[0], p_ver[1], p_ver[2]);

	char sn[24] = {0};
	SNRead((char *)sn);

	uint16_t crc = 0;
    uint32_t start_addr = 0;
	uint32_t flag = 0;

	start_addr = COLLECTOR_CFG_ADDR_FLAG;
	flag = 0;

	SpiFlashReadData(start_addr, (uint8_t *)&flag, sizeof(flag));

	if (flag != COLLECTOR_CFG_INFO_FLAG)
	{
		// master sector err
		start_addr = COLLECTOR_CFG_ADDR_FLAG_BAK;
		flag = 0;
		SpiFlashReadData(start_addr, (uint8_t *)&flag, sizeof(flag));

		if (flag != COLLECTOR_CFG_INFO_FLAG)
		{
			// slave sector err
			CollectorWriteDefaultInfo();
			return -1;
		}
		flag = 2;
		start_addr = COLLECTOR_CFG_ADDR_BAK;
	}
	else
	{
		start_addr = COLLECTOR_CFG_ADDR;
	}

	SpiFlashReadData(start_addr, (uint8_t *)_gCollectorData, sizeof(_gCollectorData));
	crc = calculateCRC16((const uint8_t *)&gCollector->type, 
						 sizeof(_gCollectorData)-sizeof(CollectorHead_t));
	if (crc != gCollector->head.crc 
		|| gCollector->property_size < COLLECTOR_PROPERTY_DATA_GET_PERIOD)
		//|| memcmp(gCollector->firmware, gFirmwareVer, sizeof(gFirmwareVer)))
	{
		CollectorWriteDefaultInfo();
	}

#if COLLECTOR_DEBUG
	//snprintf(gFirmwareVer, sizeof(gFirmwareVer), "%d.%d.%d", 0,1,0);
	//memcpy(gCollector->firmware, gFirmwareVer, sizeof(gFirmwareVer));

	uint16_t tmp_value = COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD_DEFAULT;
	CollectorSetProperty(COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD, 
						 (const uint8_t *)&tmp_value, sizeof(uint16_t));
#endif 

	memcpy(gCollector->sn, sn, sizeof(sn));
	int32_t sn_empty_size = 0;
	for (int32_t i = 0; i < sizeof(sn) && i < 16; ++i)
	{
		if (sn[i] == 0)
		{
			++sn_empty_size;
		}
	}
	if (sn_empty_size > 6)
	{
		memcpy(gCollector->sn, COLLECTOR_SN_DEFAULT, strlen(COLLECTOR_SN_DEFAULT));
	}
	// update name
	if (!gCollector->name_flag)
	{
		memset(gCollector->name, 0, sizeof(gCollector->name));
		if (gCollector->sn[0] != 'A' && gCollector->sn[1] != 'M' && gCollector->sn[2] != 'P') 
		{
			gCollector->name[0] = 'A';
			gCollector->name[1] = 'M';
			gCollector->name[2] = 'P';
			memcpy(&gCollector->name[3], &gCollector->sn[0], 5);
			memcpy(&gCollector->name[8], &gCollector->sn[9], 6);
		} 
		else 
		{
			memcpy(&gCollector->name[0], &gCollector->sn[0], 15);
		}
	}

	memset(gCollector->pcs_sn, 0, sizeof(gCollector->pcs_sn));
	gCollector->type = 0;
	gCollector->mqtt.type = 0;

	SpiFlashReadData(start_addr+sizeof(_gCollectorData), 
					 (uint8_t *)&gCollector->head.s_url_len,
					 sizeof(gCollector->head.s_url_len));

	if (gCollector->head.s_url_len > 0 && gCollector->head.s_url_len < 1024)
	{
		gCollector->head.s_url = (char *)malloc(gCollector->head.s_url_len+1);
		if (gCollector->head.s_url)
		{
			memset(gCollector->head.s_url, 0, gCollector->head.s_url_len+1);
			SpiFlashReadData(start_addr+sizeof(_gCollectorData)
							 +sizeof(gCollector->head.s_url_len),
							 (uint8_t *)gCollector->head.s_url, 
							 gCollector->head.s_url_len);
			gCollector->mqtt.type = gCollector->head.s_url[0];
		}
	}

	memcpy(gCollector->firmware, gFirmwareVer, sizeof(gFirmwareVer));
	Fc41dWifiSet((const char *)gCollector->wifi.ssid, (const char *)gCollector->wifi.pwd);

	return 0;
}

int32_t CollectorUpdateSn(const uint8_t *sn, int32_t len)
{
	if (len >= sizeof(gCollector->sn)) 
	{
		return -1;
	}

	memcpy(gCollector->sn, sn, len);
	return 0;
}

int32_t CollectorUpdateName(const uint8_t *name, int32_t len)
{
	if (len >= sizeof(gCollector->name)) 
	{
		return -1;
	}

	gCollector->name_flag = 1;
	memset(gCollector->name, 0, sizeof(gCollector->name));
	memcpy(gCollector->name, name, len);
	return 0;
}

static int32_t _CollectorUpdateWifiInfo(void *userdata)
{
	return Fc41dUpdateWifiInfo();
}

static int32_t _CollectorUploadDevInfo(void *userdata)
{
	uint32_t mqtt_len = 0;
	char mqtt_info[512] = {0};
	char tmp[2][128] = {0};

	UniformProtocolTlvBodyHead_t *body = (UniformProtocolTlvBodyHead_t *)mqtt_info;
	// wifi info
	Fc41dGetWifiSsid((uint8_t *)tmp[1], sizeof(tmp[1]));
	snprintf(tmp[0], sizeof(tmp[0]), "%s,%d", tmp[1], Fc41dGetWifiRssi());
	body->cmd = PROTO_UNIFORM_R_WIFI_INFO;
	body->err = 0;
	body->data_len = strlen(tmp[0])+1;
	memcpy(body->data, tmp[0], strlen(tmp[0]));
	mqtt_len += sizeof(UniformProtocolTlvBodyHead_t) + body->data_len;

	// dev info
	body = (UniformProtocolTlvBodyHead_t *)(body->data+body->data_len);
	// snprintf(tmp[0], sizeof(tmp[0]), "%s\r\n%s\r\n", (const char *)gCollector->sn, 
	// 		 (const char *)gCollector->firmware);
	snprintf(tmp[0], sizeof(tmp[0]), "%s\r\n%s\r\n%s\r\n", (const char *)gCollector->sn, 
			 (const char *)gCollector->firmware, Fc41dGetFc41dVer());
	body->cmd = PROTO_UNIFORM_R_DEV_INFO;
	body->err = 0;
	body->data_len = sizeof(gCollector->type) + strlen(tmp[0]) + 1;

#if 1
	{
		// C100 传大端序
		uint8_t *type = (uint8_t *)&gCollector->type;
		body->data[0] = type[1];
		body->data[1] = type[0];
	}
#else
	{
		uint8_t *type = (uint8_t *)&gCollector->type;
		body->data[0] = type[0];
		body->data[1] = type[1];
	}
	memcpy(body->data, &gCollector->type, sizeof(gCollector->type));
#endif 
	memcpy(body->data+sizeof(gCollector->type), tmp[0], strlen(tmp[0]));
	mqtt_len += sizeof(UniformProtocolTlvBodyHead_t) + body->data_len;

	// mqtt info
	body = (UniformProtocolTlvBodyHead_t *)(body->data+body->data_len);
	memset(tmp[0], 0, sizeof(tmp[0]));
	Fc41dGetMqttServerIp((uint8_t *)tmp[1], sizeof(tmp[1]));
	snprintf(tmp[0], sizeof(tmp[0]), "%s:", tmp[1]);
	Fc41dGetMqttServerPort((uint8_t *)tmp[1], sizeof(tmp[1]));
	snprintf(tmp[0]+strlen(tmp[0]), sizeof(tmp[0])-strlen(tmp[0]), "%s", tmp[1]);
	body->cmd = PROTO_UNIFORM_R_DEV_MQTT_INFO;
	body->err = 0;
	body->data_len = sizeof(uint8_t) + strlen(tmp[0]) + 1;
	int8_t type = Fc41dGetMqttServerType();
	memcpy(body->data, &type, sizeof(type));
	memcpy(body->data, tmp[0], strlen(tmp[0]));
	mqtt_len += sizeof(UniformProtocolTlvBodyHead_t) + body->data_len;

	// dev name
	body = (UniformProtocolTlvBodyHead_t *)(body->data+body->data_len);
	snprintf(tmp[0], sizeof(tmp[0]), "%s", (const char *)gCollector->name);
	body->cmd = PROTO_UNIFORM_R_DEV_NAME;
	body->err = 0;
	body->data_len = strlen(tmp[0]) + 1;
	memcpy(body->data, tmp[0], strlen(tmp[0]));
	mqtt_len += sizeof(UniformProtocolTlvBodyHead_t) + body->data_len;

	// collector property
	body = (UniformProtocolTlvBodyHead_t *)(body->data+body->data_len);
	uint16_t property[2] = {0};
	uint8_t property_len = 0;
	uint8_t *tmp_property = CollectorGetProperty(COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD, &property_len);
	if (tmp_property != NULL)
	{
		memcpy(&property[0], tmp_property, property_len<sizeof(uint16_t)?property_len:sizeof(uint16_t));
	}
	tmp_property = CollectorGetProperty(COLLECTOR_PROPERTY_DATA_GET_PERIOD, &property_len);
	if (tmp_property != NULL)
	{
		memcpy(&property[1], tmp_property, property_len<sizeof(uint16_t)?property_len:sizeof(uint16_t));
	}

	body->cmd = PROTO_UNIFORM_R_DEV_PROPERTY;
	body->err = 0;
	body->data_len = sizeof(property);
	memcpy(body->data, property, sizeof(property));
	mqtt_len += sizeof(UniformProtocolTlvBodyHead_t) + body->data_len;

	uint8_t *l = body->data + body->data_len;
	mqtt_len = (uint32_t)&(*l) - (uint32_t)&(*mqtt_info);

	return Fc41dMqttPubMeta(0, (const uint8_t *)mqtt_info, mqtt_len);
}

static int32_t _CollectorUploadInvSysInfo(void *userdata)
{
	uint32_t cmd_code = 0;
	uint8_t *mqtt_info = NULL;
	int32_t mqtt_len = DevGetInvSysInfo((void *)&mqtt_info, &cmd_code);

	return Fc41dMqttPub(0, cmd_code, 0, (const uint8_t *)mqtt_info, mqtt_len);
}

#if KSTAR_PCS_BAUD_RATE
static int32_t _CollectorUploadKstarInvSysInfo(void *userdata)
{
	int32_t mqtt_len = 0;
	char mqtt_info[sizeof(KstarInvSysInfo_t)] = {0};
	memcpy(mqtt_info, (char*)&gKstarInvSysInfo, 
		   sizeof(KstarInvSysInfo_t));
	mqtt_len = sizeof(KstarInvSysInfo_t);

	return Fc41dMqttPub(0, PROTO_UNIFORM_R_KSTAR_SYS_INFO, 0, (const uint8_t *)mqtt_info, mqtt_len);
}

static int32_t _CollectorUploadKstarInvBaseInfo(void *userdata)
{
	// sizeof(gKstarInvBaseInfo); //1586
	// sizeof(gKstarInvSysInfo); //38
	// kstar 3000~3792 register

	int32_t mqtt_len = 0;
	char mqtt_info[sizeof(DevKstarRegisterInfo_t) + sizeof(gKstarInvBaseInfo)] = {0};
	DevKstarRegisterInfo_t *kstar = (DevKstarRegisterInfo_t *)mqtt_info;
	kstar->reg_start = 3000; // 3000~4000
	kstar->reg_size = sizeof(gKstarInvBaseInfo)/2;
	memcpy(mqtt_info+sizeof(DevKstarRegisterInfo_t), (char*)&gKstarInvBaseInfo, 
		   sizeof(gKstarInvBaseInfo));
	mqtt_len = 2*kstar->reg_size+sizeof(DevKstarRegisterInfo_t);

	return Fc41dMqttPub(0, PROTO_UNIFORM_R_KSTAR_REGISTER, 0, (const uint8_t *)mqtt_info, mqtt_len);
}
#endif 


static int32_t _CollectorCheckMqttSubTopicStatus(void *userdata)
{
	return Fc41dCheckMqttSubTopicStatus();
}

int32_t CollectorUpdateCfgCheck()
{
	int32_t update_spiflash = 0;
	char wifi_ssid[32] = {0};
	char wifi_pwd[64] = {0};
	char mqtt_server_ip[128] = {0};
	char mqtt_server_port[6] = {0};
	int8_t mqtt_server_type = Fc41dGetMqttServerType();

	Fc41dGetWifiSsid((uint8_t *)wifi_ssid, sizeof(wifi_ssid));
	Fc41dGetWifiPwd((uint8_t *)wifi_pwd, sizeof(wifi_pwd));
    
	if (gCollector->mqtt.type != mqtt_server_type)
	{
		gCollector->mqtt.type = mqtt_server_type;
		update_spiflash += 1;
	}

	if (memcmp(gCollector->wifi.ssid, wifi_ssid, sizeof(wifi_ssid)) != 0
		|| memcmp(gCollector->wifi.pwd, wifi_pwd, sizeof(wifi_pwd)) != 0)
	{
		memcpy(gCollector->wifi.ssid, wifi_ssid, sizeof(wifi_ssid));
		memcpy(gCollector->wifi.pwd, wifi_pwd, sizeof(wifi_pwd));
		update_spiflash += 2;
	}

	return update_spiflash;
}


static int32_t CollectorProcessGetDcAcSn()
{
	if (!gCollectorDevInfo.pcs_info_updated)
	{
		return -1;
	}
	uint8_t *p_dev =  NULL;
    int32_t k = DevGetPcsSn(&p_dev);
	//gCollector->pcs_modelid = DevGetPcsType();
#ifdef COLLECTOR_PCS_TYPE_DEBUG
	for (int32_t i = 0, j = 0; i < k; i+=2, j+=2)
	{
		//gCollector->pcs_sn[j] = p_dev[i+1];
		//gCollector->pcs_sn[j+1] = p_dev[i];
		gCollector->pcs_sn[j] = p_dev[i];
		gCollector->pcs_sn[j+1] = p_dev[i+1];
	}
	//gCollector->pcs_sn[1] = COLLECTOR_DBG_SN;
#endif 

	Fc41dMqttSet(gCollector->mqtt.type, NULL, NULL);
	if (gCollector->mqtt.type < 2)
	{
		char mqtt_info[2][256] = { {0} }; // 0-> ssid, 1->passwd
		char *p = (char *)gCollector->head.s_url;

		for (uint16_t i = 1, j = 0, k = 0; i < gCollector->head.s_url_len; ++i) 
		{
			if (p[i] == ':' && p[i+1] >= '0' && p[i+1] <= '9') 
			{
				j = 0;
				++k;
				continue;
			}
			if (k == 1 && p[i] < '0' || p[i] > '9')
			{
				break;
			}
			mqtt_info[k][j++] = p[i];
		}

		Fc41dMqttSet(gCollector->mqtt.type, mqtt_info[0], mqtt_info[1]);
	}
	return 0;
}

static int32_t GetCollectorType()
{
	if (gCollector->type == 0 && Fc41dGetStat() > 0) 
	{
		EN_COLLECTOR_SUB_BOARD_TYPE type = SubBoardTypeGet();
		switch (type)
		{
		case EN_COLLECTOR_SUB_BOARD_TYPE_WIFI:
			gCollector->type = COLLECTOR_TYPE_WIFI;
			break;
		case EN_COLLECTOR_SUB_BOARD_TYPE_4G:
			gCollector->type = COLLECTOR_TYPE_4G;
			break;
		}
	}
	return 0;
}

int32_t CollectorProcess()
{
	Fc41dProcess();
	GetCollectorType();

	if (CollectorProcessGetDcAcSn() != 0)
	{
		memset(gCollector->pcs_sn, 0, sizeof(gCollector->pcs_sn));
		return -1;
	}

	LedSetInitStatus(1);
	if (DevOpProcessStatus() == 2)
	{
		LedSetInitStatus(0);
	}

	uint32_t upload_idx = 0;
	static uint32_t mqtt_stat[2] = {0};

	uint16_t property[1] = {0};
	uint8_t property_len = 0;
	uint8_t *tmp = CollectorGetProperty(COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD, &property_len);
	if (tmp == NULL)
	{
		return 0;
	}
	memcpy(&property[0], tmp, property_len<sizeof(uint16_t)?property_len:sizeof(uint16_t));
	if (property[0] < 20)
	{
		property[0] = 20;
	}

	static int32_t ble_conn[2] = {0};
	ble_conn[0] = GetBleConnStat();
	if (ble_conn[0] != ble_conn[1])
	{
		gCollector->head.timeout_upload = tim_get_systime();
		gCollector->head.force_upload = 0;
		if (!ble_conn[0])
		{
			gCollector->head.force_upload = 1;
		}
		ble_conn[1] = ble_conn[0];
	}
	if (ble_conn[0])
	{
		property[0] = 60;
	}

	do {
		uint32_t cur_time = 0;
		int32_t upgrade_flag = NovaDevUpgradeStatGet();

		if (Fc41dCheckUpgrade() || Fc41dCheckNetSetting()
			|| (upgrade_flag > NOVA_DEV_UPGRADE_STAT_NONE 
				&& upgrade_flag < NOVA_DEV_UPGRADE_STAT_COMPLETE)) 
		{
			gCollector->head.timeout_upload = tim_get_systime();
			gCollector->head.force_upload = 0;
			break;
		}

		mqtt_stat[0] = Fc41dMqttStatGet();

		if (mqtt_stat[0] <= 0)
		{
			// mqtt 掉线(需定时重连)
			mqtt_stat[1] = 0;
			gCollector->head.timeout_upload = tim_get_systime();
			gCollector->head.force_upload = 0;
			break;
		}

		if (Fc41dIsBusy())
		{
			//gCollector->head.timeout_upload = tim_get_systime();
			gCollector->head.force_upload = 0;
			break;
		}

		if (!gCollectorDevInfo.pcs_info_updated)
		{
			break;
		}

		if (mqtt_stat[1] == 0)
		{
			if (!Fc41dGetTaskSize())
			{
				mqtt_stat[1] = 1;
			}
		}


		cur_time = tim_get_systime();
		cur_time -= gCollector->head.timeout_upload;
		if((mqtt_stat[0] == 1 && mqtt_stat[1] == 1) 
		   ||  gCollector->head.force_upload 
		   || cur_time > (property[0]*1000))
		{
#if COLLECTOR_DEBUG
			//break;
#endif 
			upload_idx = gCollector->head.upload_seq%
				(sizeof(gCollectorUploadFunc)/sizeof(gCollectorUploadFunc[0]));

			if (Fc41dProcessing() == 0) 
			{
#if COLLECTOR_UPLOAD_DATA
				if (gCollectorUploadFunc[upload_idx].fn(NULL) == 0)
				{
					++gCollector->head.upload_seq;
				}
#endif 
			}

			if (gCollector->head.upload_seq
				== sizeof(gCollectorUploadFunc)/sizeof(gCollectorUploadFunc[0]))
			{
				mqtt_stat[1] = 2;
                gCollector->head.upload_seq = 0;
				gCollector->head.timeout_upload = tim_get_systime();
				gCollector->head.force_upload = 0;
			}
		}

	} while (0);

	return 0;
}

#if 1

#define my_max(a,b) (((a) > (b)) ? (a) : (b))
#define my_min(a,b) (((a) < (b)) ? (a) : (b))
#define BUF_SIZE (1024)

int32_t test_zlib()
{
	return 0;
}


#endif 

#if 1
int32_t test_aes_cbc()
{
	uint8_t ciphertext[2][128] = {
		//{89, 111, 94, -12, 116, 6, -8, -37, -67, 107, -60, 1, -76, -103, -97, -50}
		//{15, 94, 86, 89, -8, -89, 16, -128, 18, 124, 60, 52, -59, 64, 104, 48}
		{113, -89, 65, 89, -23, 31, 28, 19, -68, -105, -6, 126, -28, 101, -103, 1, -37, -38, -30, 86, -51, -9, 91, 72, -51, 61, -28, -3, -49, -1, 66, 0}
	};
	uint8_t iv[1][16] = {
		//{-2, 19, -74, 26, 58, -60, -40, -43, 116, 69, 106, 73, 104, -4, -14, -37}
		{-109, 123, -112, 113, -100, -80, -93, 13, -100, -58, -39, 112, -109, 125, -76, -72}
	};
	uint8_t key[1][32] = {
		 //{97, 115, 100, 102, 103, 104, 48, 49, 50, 51, 57, 52, 108, 106, 55, 103, 56, 57, 106, 105, 103, 100, 114, 115, 118, 56, 51, 53, 52, 107, 98, 103}
		{-109, 97, 24, -19, 28, 49, -4, 58, -52, 106, -52, 98, -19, 84, 39, 0, 97, -40, 102, 50, -28, 32, 31, -102, -47, 126, -108, 57, -13, -66, -59, -120}
	};

	uint8_t enc_buf[2][128] = {0};
	uint8_t plaintext[2][128] = {
		{"abcdefghij00klmnopqretuvwxyz123456789"},
		{0}
	};
	int32_t enc_buf_len = strlen((char *)plaintext[0]);
	enc_buf_len = ((enc_buf_len%AES_BLOCKLEN)>0?1:0)*AES_BLOCKLEN
		+ (enc_buf_len/AES_BLOCKLEN)*AES_BLOCKLEN;

	AES256_CBC_ctx ctx;
	AES256_CBC_init(&ctx, key[0], iv[0]);
	AES256_CBC_decrypt(&ctx, 1, enc_buf[0], ciphertext[0]);
	AES256_CBC_init(&ctx, key[0], iv[0]);
	int32_t i = 0, j = enc_buf_len/AES_BLOCKLEN;
//	for (i = 0; i < enc_buf_len; i+=16)
//	{
//
//	}
	AES256_CBC_encrypt(&ctx, j, enc_buf[1], plaintext[0]);
    AES256_CBC_init(&ctx, key[0], iv[0]);
	AES256_CBC_decrypt(&ctx, j, plaintext[1], enc_buf[1]);

	return 0;
}

#else 
int32_t test_aes_cbc()
{
	WORD key_schedule[60] = {0};
	BYTE enc_buf[2][128] = {0};
	BYTE ciphertext[2][128] = {
		{89, 111, 94, -12, 116, 6, -8, -37, -67, 107, -60, 1, -76, -103, -97, -50}
	};
	BYTE iv[1][16] = {
		{-2, 19, -74, 26, 58, -60, -40, -43, 116, 69, 106, 73, 104, -4, -14, -37}
	};
	BYTE key[1][32] = {
		{97, 115, 100, 102, 103, 104, 48, 49, 50, 51, 57, 52, 108, 106, 55, 103, 56, 57, 106, 105, 103, 100, 114, 115, 118, 56, 51, 53, 52, 107, 98, 103}
	};

	BYTE plaintext[2][32] = {
		{"1234567890111"},
		{0}
	};
    
	aes_key_setup(key[0], key_schedule, 256);
	int32_t enc_buf_len = strlen((char *)plaintext[0]);
	enc_buf_len = ((enc_buf_len%AES_BLOCK_SIZE)>0?1:0)*AES_BLOCK_SIZE
		+ (enc_buf_len/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	aes_encrypt_cbc(plaintext[0], enc_buf_len, enc_buf[0], key_schedule, 256, iv[0]);
    memcpy(ciphertext[1], enc_buf[0], enc_buf_len);

    aes_decrypt_cbc(ciphertext[0], enc_buf_len, enc_buf[0], key_schedule, 256, iv[0]);
	aes_decrypt_cbc(ciphertext[1], enc_buf_len, enc_buf[1], key_schedule, 256, iv[0]);

	return 0;
}

int32_t test_rsa()
{

	int ret;
    rsa_pk_t pk = {0};
    rsa_sk_t sk = {0};
    uint8_t output[256];

    // message to encrypt
    //uint8_t input [256] = { 0x21,0x55,0x53,0x53,0x53,0x53};
	uint8_t input[256] = {"1234567890"};

    unsigned char msg [256];
    uint32_t outputLen, msg_len;
    uint8_t  inputLen;

    // copy keys.h message about public key and private key to the flash RAM
    pk.bits = KEY_M_BITS;
    memcpy(&pk.modulus         [RSA_MAX_MODULUS_LEN-sizeof(key_m) ],  key_m,  sizeof(key_m ));
    memcpy(&pk.exponent        [RSA_MAX_MODULUS_LEN-sizeof(key_e) ],  key_e,  sizeof(key_e ));
    sk.bits = KEY_M_BITS;
    memcpy(&sk.modulus         [RSA_MAX_MODULUS_LEN-sizeof(key_m) ],  key_m,  sizeof(key_m ));
    memcpy(&sk.public_exponet  [RSA_MAX_MODULUS_LEN-sizeof(key_e) ],  key_e,  sizeof(key_e ));
    memcpy(&sk.exponent        [RSA_MAX_MODULUS_LEN-sizeof(key_pe)],  key_pe, sizeof(key_pe));
    memcpy(&sk.prime1          [RSA_MAX_PRIME_LEN - sizeof(key_p1)],  key_p1, sizeof(key_p1));
    memcpy(&sk.prime2          [RSA_MAX_PRIME_LEN - sizeof(key_p2)],  key_p2, sizeof(key_p2));
    memcpy(&sk.prime_exponent1 [RSA_MAX_PRIME_LEN - sizeof(key_e1)],  key_e1, sizeof(key_e1));
    memcpy(&sk.prime_exponent2 [RSA_MAX_PRIME_LEN - sizeof(key_e2)],  key_e2, sizeof(key_e2));
    memcpy(&sk.coefficient     [RSA_MAX_PRIME_LEN - sizeof(key_c) ],  key_c,  sizeof(key_c ));

    inputLen = strlen((const char*)input);

    // public key encrypt
    rsa_public_encrypt(output, &outputLen, input, inputLen, &pk);

    // private key decrypt
    rsa_private_decrypt(msg, &msg_len, output, outputLen, &sk);

    // private key encrypt
    rsa_private_encrypt(output, &outputLen, input, inputLen, &sk);

    // public key decrypted
    rsa_public_decrypt(msg, &msg_len, output, outputLen, &pk);

    return 0;
}

int32_t test_md5()
{
	unsigned char md5[16];
	char md5_current[33] = {0};
	MD5_CTX mdContext;
    //FILE *pf = fopen("test_128kb.txt", "rb");
    int bytes = 0, i = 0;
    char data[1024] = {"0x12345678901234"};
	char buf_temp[2] = {0};

	//snprintf(data, sizeof(data), "%s", COLLECTOR_SN_DEFAULT);

	MD5Init(&mdContext);
	/*
	while ((bytes = fread (data, 1, 1024, pf)) != 0){
		MD5Update(&mdContext, data, bytes);
	}
    fclose(pf);
	*/
	bytes = strlen(data);
	MD5Update(&mdContext, (unsigned char *)data, bytes);
	MD5Final(md5,&mdContext);
	memset(md5_current, 0, sizeof(md5_current));


	for(i = 0; i < 16; i++){
		sprintf(buf_temp,"%02x", md5[i]);
		strncat(md5_current,buf_temp,2);
	}

	return 0;
}

int32_t test_base64()
{
	char input[] = {"MTIzNDU2Nzg5MEFCQ1xyXG4="}; // 1234567890ABC\r\n
	char output[512] = {0};
	base64_decode((unsigned char *)input, strlen(input), (unsigned char *)output);
	return 0;
}
#endif 

int32_t CollectorUploadRightNow()
{
	gCollector->head.force_upload = 1;
	return 0;
}

int32_t CollectorInit()
{
	//test_base64();
    //test_md5();
	//test_rsa();
	//test_aes_cbc();
	LedSetInitStatus(1);
	Fc41dInit();
	_CollectorSpiFlashReadCfg();
	gCollector->head.timeout_upload = tim_get_systime();
	return 0;
}

