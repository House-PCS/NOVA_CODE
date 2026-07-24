#include "nova_dev_upgrade.h"
#include "md5.h"
#include "dev_modbus_h.h"
#include "dev_spiflash.h"
#include "proto_nova_uniform.h"

#define USE_CHECK_SUM							1

#define MAX_SEND_DATA_LEN						512*4

#define NOVA_DEV_UPGRADE_FLASH_FLAG_ADDR		0x040000
#define NOVA_DEV_UPGRADE_FLASH_DATA_ADDR		0x041000

#define NOVA_DEV_UPGRADE_HEADER  0x5A

#define NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_SRC	1
#define NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_DEST	0

static NovaDevUpgradeCtx_t g_upgrade_ctx = {0};

static uint16_t _CheckSum(uint8_t *p, uint16_t size)
{
    uint16_t i;
    uint16_t ret = 0;
    for(i = 0; i < size; i++)
    {
        ret += p[i];
    }
    return ret;
}

void NovaDevUpgradeCtxReset()
{
	memset(&g_upgrade_ctx, 0, sizeof(NovaDevUpgradeCtx_t));
}

int32_t NovaDevUpgradeStatSet(EN_NOVA_DEV_UPGRADE_STAT stat)
{
	g_upgrade_ctx.stat = (int32_t)stat;
	return 0;
}

EN_NOVA_DEV_UPGRADE_STAT NovaDevUpgradeStatGet()
{
	return g_upgrade_ctx.stat;
}

int32_t NovaDevUpgradeTypeSet(EN_NOVA_DEV_UPGRADE_TYPE type)
{
	g_upgrade_ctx.type = (int32_t)type;
	return 0;
}

EN_NOVA_DEV_UPGRADE_TYPE NovaDevUpgradeTypeGet()
{
	return g_upgrade_ctx.type;
}

int32_t NovaDevUpgradeFirmwareSetSize(uint32_t size)
{
	g_upgrade_ctx.total_size = size;
	return 0;
}

uint32_t NovaDevUpgradeFirmwareGetSize()
{
	return g_upgrade_ctx.total_size;
}

int32_t NovaDevUpgradeFirmwareSendData(uint8_t *data, uint16_t len)
{
	uint8_t buf[128] = {0};
	if (len > sizeof(buf)-2)
	{
		return -1;
	}
	memcpy(buf, data, len);
#if USE_CHECK_SUM
	uint16_t check_sum = _CheckSum((uint8_t *)&buf[0],  len);
	memcpy(&buf[len], &check_sum, sizeof(uint16_t));
    ModbusRTUSendWithoutCheckSum((uint8_t *)&buf[0], len+sizeof(uint16_t));
#else 
    ModbusRTUSendWithoutCheckSum((uint8_t *)&buf[0], len);
#endif 
	return 0;
}

int32_t NovaDevUpgradeFirmwareSetBaud(uint32_t idx_baud)
{
	uint8_t buf[64] = {0};
	uint16_t *p = NULL;

	p = (uint16_t*)&buf[0];
	*p = 0x501F;
	++p;
	*p = 0x01;
	++p;
	*p = (idx_baud<<8);
	++p;

#if USE_CHECK_SUM
	uint16_t check_sum = _CheckSum((uint8_t *)&buf[0],  sizeof(uint16_t)*3);
	*p = check_sum;
    ModbusRTUSendWithoutCheckSum((uint8_t *)&buf[0], sizeof(uint16_t)*4);
#else 
    ModbusRTUSendWithoutCheckSum((uint8_t *)&buf[0], sizeof(uint16_t)*3);
#endif 
	return 0;
}


static int32_t _g_test_send_len[2] = {0};
int32_t NovaDevUpgradeFirmwareStart()
{

	memset(_g_test_send_len, 0, sizeof(_g_test_send_len));
	g_upgrade_ctx.send_size = 0;
	g_upgrade_ctx.pre_send_size = 0;

	uint8_t buf[64] = {0};
	NovaDevUpgradePkg_t *pkg = (NovaDevUpgradePkg_t *)buf;
	pkg->head = NOVA_DEV_UPGRADE_HEADER;
	pkg->addr_src = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_SRC;
	pkg->addr_dest = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_DEST;
	pkg->cmd = ntohs(NOVA_DEV_UPGRADE_CMD_FIRMWARE_START);
	pkg->data_len = 0;

	g_upgrade_ctx.flag = 0;
	uint32_t read_len = NovaDevUpgradeSpiFlashRead(pkg->data, pkg->data_len, 
												  g_upgrade_ctx.send_size);
	// 开始升级需要发送升级包大小+真实数据偏移
	pkg->data_len = sizeof(uint32_t)+sizeof(uint8_t)+sizeof(uint8_t)*16;
	memcpy(&pkg->data[0], &g_upgrade_ctx.total_size, sizeof(uint32_t)); // 包大小
	pkg->data[4] = 0x02; // 真实数据偏移2byte

	// read md5
	NovaDevUpgradeGetSpiFlashMd5(&pkg->data[5], g_upgrade_ctx.total_size);

#if USE_CHECK_SUM
	uint16_t check_sum = _CheckSum((uint8_t *)pkg, sizeof(NovaDevUpgradePkg_t)
								   +pkg->data_len);
	*(uint16_t *)&pkg->data[pkg->data_len] = check_sum;
    ModbusRTUSendWithoutCheckSum((uint8_t *)pkg, (uint16_t)sizeof(NovaDevUpgradePkg_t)
								 +sizeof(uint16_t)+pkg->data_len);
#else 
    ModbusRTUSendWithoutCheckSum((uint8_t *)pkg, (uint16_t)sizeof(NovaDevUpgradePkg_t)
								 +pkg->data_len);
#endif 
	return 0;
}

int32_t NovaDevUpgradeFirmwareSend(int32_t resend)
{
	static uint32_t pkg_size = 0;
	if (resend)
	{
		if (g_upgrade_ctx.send_size >= g_upgrade_ctx.total_size)
		{
			return 0;
		}
		g_upgrade_ctx.send_size -= g_upgrade_ctx.pre_send_size;
		--pkg_size;
	}
	uint8_t buf[MAX_SEND_DATA_LEN+sizeof(NovaDevUpgradePkg_t)
		+sizeof(NovaDevUpgradeDataAttr_t)+sizeof(uint16_t)] = {0};
	NovaDevUpgradePkg_t *pkg = (NovaDevUpgradePkg_t *)buf;
	NovaDevUpgradeDataAttr_t *attr = (NovaDevUpgradeDataAttr_t *)&pkg->data[0];

	if (g_upgrade_ctx.send_size == 0)
	{
		pkg_size = 0;
	}
	++pkg_size;

	pkg->head = NOVA_DEV_UPGRADE_HEADER;
	pkg->addr_src = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_SRC;
	pkg->addr_dest = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_DEST;
	pkg->cmd = ntohs(NOVA_DEV_UPGRADE_CMD_FIRMWARE_SEND);
	pkg->data_len = sizeof(buf)-sizeof(NovaDevUpgradePkg_t)
		-sizeof(NovaDevUpgradeDataAttr_t)-sizeof(uint16_t);
	attr->seq = pkg_size;

	uint32_t read_len = NovaDevUpgradeSpiFlashRead(attr->data, pkg->data_len, 
												  g_upgrade_ctx.send_size);
	if (read_len == 0)
	{
		return 0;
	}

	_g_test_send_len[0] += read_len;
	_g_test_send_len[1] = read_len;
	pkg->data_len = read_len+sizeof(NovaDevUpgradeDataAttr_t);

	read_len = sizeof(NovaDevUpgradePkg_t);
	uint16_t buf_len = (uint16_t)(sizeof(NovaDevUpgradePkg_t)+pkg->data_len);
	if (buf_len > sizeof(buf))
	{
		return 0;
	}

#if USE_CHECK_SUM
	uint16_t check_sum = _CheckSum((uint8_t *)pkg, sizeof(NovaDevUpgradePkg_t)+pkg->data_len);
	*(uint16_t *)&pkg->data[pkg->data_len] = check_sum;

	read_len = pkg->data_len;
	//pkg->data_len = ntohs(sizeof(buf)-sizeof(NovaDevUpgradePkg_t)-sizeof(uint16_t));
    ModbusRTUSendWithoutCheckSum((uint8_t *)pkg, (uint16_t)sizeof(NovaDevUpgradePkg_t)
								 + read_len
								 +sizeof(uint16_t));
#else
	ModbusRTUSendWithoutCheckSum(pkg->data, pkg->data_len);
#endif 

	read_len -= sizeof(NovaDevUpgradeDataAttr_t);
	g_upgrade_ctx.pre_send_size = read_len;
	g_upgrade_ctx.send_size += read_len;
	return read_len;
}

int32_t NovaDevUpgradeFirmwareSendDebug()
{
	uint8_t buf[512] = {0};
	NovaDevUpgradePkg_t *pkg = (NovaDevUpgradePkg_t *)buf;
	pkg->head = NOVA_DEV_UPGRADE_HEADER;
	pkg->addr_src = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_SRC;
	pkg->addr_dest = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_DEST;
	pkg->cmd = NOVA_DEV_UPGRADE_CMD_FIRMWARE_SEND;
	pkg->data_len = sizeof(buf)-sizeof(NovaDevUpgradePkg_t)-sizeof(uint16_t);

	while (1) {
	uint32_t read_len = NovaDevUpgradeSpiFlashRead(pkg->data, pkg->data_len, 
												  g_upgrade_ctx.send_size);
	if (read_len == 0)
	{
		return 0;
	}

	pkg->data_len = read_len;

	read_len = sizeof(NovaDevUpgradePkg_t);
	uint16_t buf_len = (uint16_t)(sizeof(NovaDevUpgradePkg_t)+pkg->data_len);
	if (buf_len > sizeof(buf))
	{
		return 0;
	}

#if USE_CHECK_SUM
	uint16_t check_sum = _CheckSum((uint8_t *)pkg, sizeof(NovaDevUpgradePkg_t)+pkg->data_len);
	*(uint16_t *)&pkg->data[pkg->data_len] = check_sum;

#else
	ModbusRTUSendData(pkg->data, pkg->data_len);
#endif 

	g_upgrade_ctx.send_size += pkg->data_len;
	}
	return 0;
}

int32_t NovaDevUpgradeFirmwareEnd()
{
	uint8_t buf[16] = {0};
	NovaDevUpgradePkg_t *pkg = (NovaDevUpgradePkg_t *)buf;
	pkg->head = NOVA_DEV_UPGRADE_HEADER;
	pkg->addr_src = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_SRC;
	pkg->addr_dest = NOVA_DEV_UPGRADE_CMD_FIRMWARE_ADDR_DEST;
	pkg->cmd = ntohs(NOVA_DEV_UPGRADE_CMD_FIRMWARE_END);
	pkg->data_len = 0;

	g_upgrade_ctx.send_size = g_upgrade_ctx.total_size;

#if USE_CHECK_SUM
	uint16_t check_sum = _CheckSum((uint8_t *)pkg, sizeof(NovaDevUpgradePkg_t));
	*(uint16_t *)&pkg->data[0] = check_sum;
    ModbusRTUSendWithoutCheckSum((uint8_t *)pkg, (uint16_t)sizeof(NovaDevUpgradePkg_t)
								 +sizeof(uint16_t));
#else 
    ModbusRTUSendWithoutCheckSum((uint8_t *)pkg, (uint16_t)sizeof(NovaDevUpgradePkg_t));
#endif 
	return 0;
}

int32_t NovaDevUpgradeIsComplete()
{

	return 0;
}

uint32_t NovaDevUpgradeSpiFlashAddr()
{
	return NOVA_DEV_UPGRADE_FLASH_DATA_ADDR + sizeof(NovaDevUpgradeCtx_t);
}

int32_t NovaDevCleanUpgradeFlashSector()
{
	int32_t count = 0;
    uint32_t addr = NOVA_DEV_UPGRADE_FLASH_DATA_ADDR; 
	while (count < 28)
	{
		SpiFlashEraseBlock64K(addr+count*16*0x1000);
		++count;
	}
	return 0;
}

int32_t NovaDevUpgradeSpiFlashSaveCtxDebug(uint32_t len)
{
	uint32_t start_addr = NOVA_DEV_UPGRADE_FLASH_DATA_ADDR;

	g_upgrade_ctx.total_size = len;

    SpiFlashWriteBytesWithErase(start_addr, (uint8_t *)&g_upgrade_ctx, 
											sizeof(NovaDevUpgradeCtx_t));
	return sizeof(NovaDevUpgradeCtx_t);
}

int32_t NovaDevUpgradeGetSpiFlashMd5(uint8_t *md5, uint32_t total_size)
{
	uint32_t start_addr = 0, read_len = 0, read_tmp = 0;
	NovaDevUpgradeCtx_t upgrade_ctx = {0};
	uint8_t buf[512] = {0};

	upgrade_ctx.total_size = total_size;
	if (upgrade_ctx.total_size > 1024*1024*4) // > 4MB
	{
		upgrade_ctx.total_size = 0;
	}
	if (upgrade_ctx.total_size == 0)
	{
		start_addr = NOVA_DEV_UPGRADE_FLASH_FLAG_ADDR;
		SpiFlashReadData(start_addr, (uint8_t *)&upgrade_ctx, sizeof(NovaDevUpgradeCtx_t));

		if (upgrade_ctx.total_size == 0)
		{
			return -1;
		}

		if (upgrade_ctx.total_size > 1024*1024*4) // > 4MB
		{
			return -2;
		}
	}

	MD5_CTX md5ctx;

	MD5Init(&md5ctx);
	start_addr = NOVA_DEV_UPGRADE_FLASH_DATA_ADDR;
	read_tmp = sizeof(buf);
	while(read_len < upgrade_ctx.total_size)
	{
		if (upgrade_ctx.total_size - read_len < read_tmp)
		{
			read_tmp = upgrade_ctx.total_size - read_len;
		}
		SpiFlashReadData(start_addr+sizeof(NovaDevUpgradeCtx_t)+read_len, 
						 buf, read_tmp);
		MD5Update(&md5ctx, buf, read_tmp);
		read_len += read_tmp;
	}
	MD5Final(md5, &md5ctx);

	return 0;
}

int32_t NovaDevUpgradeSpiFlashSaveCtx(uint32_t len, uint8_t md5[16])
{
	uint32_t start_addr = NOVA_DEV_UPGRADE_FLASH_FLAG_ADDR;

	if (g_upgrade_ctx.total_size != len)
	{
		g_upgrade_ctx.total_size = len;
	}
	memcpy(g_upgrade_ctx.md5, md5, sizeof(g_upgrade_ctx.md5));
    SpiFlashWriteBytesWithErase(start_addr, (uint8_t *)&g_upgrade_ctx, 
											sizeof(NovaDevUpgradeCtx_t));
	return sizeof(NovaDevUpgradeCtx_t);
}

int32_t NovaDevUpgradeSpiFlashWrite(uint8_t *data, uint32_t len, int32_t back_size)
{
	uint32_t spi_flash_addr = NOVA_DEV_UPGRADE_FLASH_DATA_ADDR + sizeof(NovaDevUpgradeCtx_t)
											+ g_upgrade_ctx.total_size + back_size;

    SpiFlashWriteBytesWithErase(spi_flash_addr, data, len);
	g_upgrade_ctx.total_size += len;
	g_upgrade_ctx.total_size += back_size;
	return len;
}

uint32_t NovaDevUpgradeSpiFlashRead(uint8_t *data, uint32_t len, uint32_t idx)
{
	int32_t ret = len, tmp = 0;
	uint32_t start_addr = 0;

	if (!g_upgrade_ctx.flag)
	{
		start_addr = NOVA_DEV_UPGRADE_FLASH_FLAG_ADDR;
		memset(&g_upgrade_ctx, 0, sizeof(sizeof(NovaDevUpgradeCtx_t)));
		SpiFlashReadData(start_addr, (uint8_t *)&g_upgrade_ctx, sizeof(NovaDevUpgradeCtx_t));
		g_upgrade_ctx.flag = 1;
		g_upgrade_ctx.send_size = 0;
		g_upgrade_ctx.pre_send_size = 0;
		g_upgrade_ctx.download_size = 0;
	}

	if (len == 0 || data == NULL)
	{
		return 0;
	}
	start_addr = NOVA_DEV_UPGRADE_FLASH_DATA_ADDR;

	tmp = g_upgrade_ctx.total_size - g_upgrade_ctx.send_size;
	if (tmp <= 0)
	{
		return 0;
	}

	if (tmp < len)
	{
		ret = tmp;
	}
	SpiFlashReadData(start_addr+sizeof(NovaDevUpgradeCtx_t)+idx, (uint8_t *)data, len);
	return ret;
}

int32_t NovaDevUpgradeGetErrorCode(uint8_t *data, int32_t len)
{
	if (len < sizeof(NovaDevUpgradeAck_t))
	{
		return -1;
	}

	NovaDevUpgradeAck_t *p_ack = (NovaDevUpgradeAck_t *)data;

	return p_ack->errcode;
}

int32_t NovaDevUpgradeGetCmd(uint8_t *data, int32_t len)
{
	if (len < sizeof(NovaDevUpgradeAck_t))
	{
		return -1;
	}

	NovaDevUpgradeAck_t *p_ack = (NovaDevUpgradeAck_t *)data;

	return htons(p_ack->cmd);
}


