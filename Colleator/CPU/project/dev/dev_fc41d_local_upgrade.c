#include "dev_fc41d_common.h"
#include "subdev.h"
#include "dev_spiflash.h"
#include "nova_dev_upgrade.h"


#define DEBUG_SKIP_MD5_CHECK 1

typedef struct _LocalUpgrade_t
{
	uint32_t recv_size;
	uint32_t pkg_num;
	uint32_t last_pkg_size;		// 最后一次写数据大小
	uint32_t last_flash_addr;	// 最后一次写flash地址
	uint8_t firmware[64];
	uint8_t recv_md5[16];		// 接收到下发的md5
} LocalUpgrade_t;

LocalUpgrade_t gLocalUpgrade = {0};

extern int32_t string_to_bytes(char *s, int l, char *d);
extern int32_t _DevFc41dCleanUpgradeFlashSector();
extern int32_t _DevFc41dSetUpgradeFlashSectorFlag(void *fc41d);
extern int32_t CollectorSpiFlashWriteCfg();

static uint8_t _gUpgradeDataMd5MiddleBuf[2048] = {0};

int32_t _LocalUpgradeStructCheck(DevFc41d_t *fc41d)
{
	uint8_t *buf = _gUpgradeDataMd5MiddleBuf;
	int32_t buf_len = 0, total_len = 0, i = 0, j = 0;
	int32_t spi_flash_addr = NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR;
	uint32_t check_sum = 0;
	char firmware[64] = {0};

	memset(_gUpgradeDataMd5MiddleBuf, 0, sizeof(_gUpgradeDataMd5MiddleBuf));
	total_len = gLocalUpgrade.recv_size - sizeof(check_sum);
	buf_len = sizeof(_gUpgradeDataMd5MiddleBuf);

	while (total_len > 0)
	{
		if (buf_len > total_len) 
		{
			buf_len = total_len;
		}
		SpiFlashReadData(spi_flash_addr + i, buf, buf_len);

		for (j = 0; j < buf_len; ++j)
		{
			check_sum += buf[j];
		}

		if (i == 0)
		{
			// check header 
			if (!UPGRADE_DATA_STRUCT_CHECK(buf, gLocalUpgrade.recv_size))
			{
				return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE;
			}
			memcpy(gLocalUpgrade.firmware, buf+sizeof(int32_t)*2+sizeof(uint16_t), 
				   sizeof(uint8_t)*3);
			snprintf(firmware, sizeof(firmware), "%d.%d.%d", gLocalUpgrade.firmware[0], 
					 gLocalUpgrade.firmware[1], gLocalUpgrade.firmware[2]);
		}

		total_len -= buf_len;
		i += buf_len;
	}

	// check len
	if (total_len != 0)
	{
		return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE;
	}
	if (i+sizeof(check_sum) != gLocalUpgrade.recv_size)
	{
		return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE2;
	}
	// read last check_sum
	uint32_t check_sum2 = 0;
	SpiFlashReadData(spi_flash_addr + i, (uint8_t *)&check_sum2, sizeof(check_sum2));
	if (check_sum != check_sum2)
	{
		return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_CHECK_SUM;
	}
	memcpy(gCollector->firmware, firmware, sizeof(firmware));

	return 0;
}

int32_t _LocalUpgradePcsStructCheck(DevFc41d_t *fc41d, uint8_t *md5)
{
	if (memcmp(md5, gLocalUpgrade.recv_md5, sizeof(gLocalUpgrade.recv_md5)) != 0)
	{
		return -1;
	}

	return 0;
}

static int32_t _LocalUpgradePcsStart(DevFc41d_t *fc41d, uint8_t *md5, uint8_t *out_md5)
{
	int32_t ret = 0;
	uint8_t __md5[16] = {0};
	uint8_t *_md5 = __md5;

	if (out_md5 != NULL)
	{
		_md5 = out_md5;
	}

	memset((uint8_t *)&gLocalUpgrade, 0, sizeof(LocalUpgrade_t));
	// get etag
	ret = string_to_bytes((char *)md5, 32, (char *)gLocalUpgrade.recv_md5);

	// get flash md5
	ret = NovaDevUpgradeGetSpiFlashMd5(_md5, 0);
	if (ret == 0)
	{
		// cmp md5 with spi flash
		if (memcmp(gLocalUpgrade.recv_md5, _md5, sizeof(__md5)) == 0)
		{
			if (memcmp(_md5, gLocalUpgrade.recv_md5, 
					   sizeof(gLocalUpgrade.recv_md5)) == 0)
			{
				// 已经下载过文件
				return 1;
			}
		}
	}

	// clean flash 64 ~ 5111 sector
	NovaDevCleanUpgradeFlashSector();
	NovaDevUpgradeCtxReset();

	return 0;
}

static int32_t _LocalUpgradePcsDownFile(DevFc41d_t *fc41d,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	int32_t back_size = 0;
	// check nova.seq
	if (gLocalUpgrade.pkg_num == uniform->frame.seq_id)
	{
		// 需要回退到上一包
		gLocalUpgrade.recv_size -= gLocalUpgrade.last_pkg_size;
		gLocalUpgrade.recv_size = gLocalUpgrade.recv_size < 0? 0:gLocalUpgrade.recv_size;
		back_size = -gLocalUpgrade.last_pkg_size;
	}

	gLocalUpgrade.pkg_num = uniform->frame.seq_id;
	gLocalUpgrade.last_pkg_size = len;
	gLocalUpgrade.recv_size += len;
	gLocalUpgrade.last_flash_addr = gLocalUpgrade.recv_size;

	return NovaDevUpgradeSpiFlashWrite((uint8_t *)data, len, back_size);
}

static int32_t _LocalUpgradePcsEnd( DevFc41d_t *fc41d,
							    UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	// 校验Flash中的升级包
	int32_t ret = 0;
	uint8_t md5[16] = {0};

	ret = NovaDevUpgradeGetSpiFlashMd5(md5, gLocalUpgrade.recv_size);
	if (ret != 0)
	{
		return -1;
	}

#if DEBUG_SKIP_MD5_CHECK

#else
	ret = _LocalUpgradePcsStructCheck(fc41d, md5);
	if (ret != 0)
	{
		return -2;
	}
#endif 

	// set upgrade flash ctx
	NovaDevUpgradeSpiFlashSaveCtx(gLocalUpgrade.recv_size, md5);
	NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);

	return ret;
}

int32_t _LocalUpgradeCollectorGetSpiFlashMd5(uint8_t *md5, uint32_t total_size)
{
	uint32_t start_addr = 0, read_len = 0, read_tmp = 0;
	LocalUpgrade_t upgrade_ctx = {0};
	uint8_t buf[512] = {0};

	upgrade_ctx.recv_size = total_size;
	if (upgrade_ctx.recv_size > 1024*1024*1) // > 1MB
	{
		upgrade_ctx.recv_size = 0;
	}
	if (upgrade_ctx.recv_size == 0)
	{
		start_addr = NOVA_UPGRADE_COLLECTOR_FLASH_CTX_ADDR;
		SpiFlashReadData(start_addr, (uint8_t *)&upgrade_ctx, sizeof(LocalUpgrade_t));

		if (upgrade_ctx.recv_size == 0)
		{
			return -1;
		}

		if (upgrade_ctx.recv_size > 1024*1024*1) // > 1MB
		{
			return -2;
		}

		gLocalUpgrade.recv_size = upgrade_ctx.recv_size;
		gLocalUpgrade.pkg_num = upgrade_ctx.pkg_num;
		gLocalUpgrade.last_pkg_size = upgrade_ctx.last_pkg_size;
		gLocalUpgrade.last_flash_addr = upgrade_ctx.last_flash_addr;
	}

	MD5_CTX md5ctx;

	MD5Init(&md5ctx);
	start_addr = NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR;
	read_tmp = sizeof(buf);
	while(read_len < upgrade_ctx.recv_size)
	{
		if (upgrade_ctx.recv_size - read_len < read_tmp)
		{
			read_tmp = upgrade_ctx.recv_size - read_len;
		}
		SpiFlashReadData(start_addr+read_len, buf, read_tmp);
		MD5Update(&md5ctx, buf, read_tmp);
		read_len += read_tmp;
	}
	MD5Final(md5, &md5ctx);

	return 0;
}

static int32_t _LocalUpgradeCollectorStart(DevFc41d_t *fc41d, uint8_t *md5, uint8_t *out_md5)
{
	int32_t ret = 0;
	uint8_t __md5[16] = {0};
	uint8_t *_md5 = __md5;

	if (out_md5 != NULL)
	{
		_md5 = out_md5;
	}

	memset((uint8_t *)&gLocalUpgrade, 0, sizeof(LocalUpgrade_t));
	// get etag
	ret = string_to_bytes((char *)md5, 32, (char *)gLocalUpgrade.recv_md5);

	// get flash md5
	ret = _LocalUpgradeCollectorGetSpiFlashMd5(_md5, 0);
	if (ret == 0)
	{
		// cmp md5 with spi flash
		if (memcmp(gLocalUpgrade.recv_md5, _md5, sizeof(__md5)) == 0)
		{
			if (memcmp(_md5, gLocalUpgrade.recv_md5, 
					   sizeof(gLocalUpgrade.recv_md5)) == 0)
			{
				// 已经下载过文件
				if (_LocalUpgradeStructCheck(fc41d) != 0)
				{
					return ret;
				}
				// set upgrade flash flag
				_DevFc41dSetUpgradeFlashSectorFlag(fc41d);
				// update firmware ver
				CollectorSpiFlashWriteCfg();
				return 1;
			}
		}
	}

	memset((uint8_t *)&gLocalUpgrade, 0, sizeof(LocalUpgrade_t));
	// get etag
	ret = string_to_bytes((char *)md5, 32, (char *)gLocalUpgrade.recv_md5);
	// clean flsh 0 ~ 63 sector
	_DevFc41dCleanUpgradeFlashSector();
	return 0;
}

static int32_t _LocalUpgradeCollectorDownFile(DevFc41d_t *fc41d,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	//uint16_t file_len = len - sizeof(UniformProtocolHead_t);
	uint16_t file_len = len;

	// check nova.seq
	if (gLocalUpgrade.pkg_num == uniform->frame.seq_id)
	{
		// 需要回退到上一包
		gLocalUpgrade.recv_size -= gLocalUpgrade.last_pkg_size;
		gLocalUpgrade.recv_size = gLocalUpgrade.recv_size < 0? 0:gLocalUpgrade.recv_size;
	}


	// write to flash
	uint32_t spi_flash_addr = NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR + gLocalUpgrade.recv_size;
	SpiFlashWriteBytesWithErase(spi_flash_addr, (uint8_t *)data, file_len);

	gLocalUpgrade.pkg_num = uniform->frame.seq_id;
	gLocalUpgrade.last_pkg_size = file_len;
	gLocalUpgrade.last_flash_addr = spi_flash_addr;
	gLocalUpgrade.recv_size += file_len;
	return 0;
}

int32_t _LocalUpgradeCollectorSpiFlashSaveCtx(uint32_t len, uint8_t md5[16])
{
	uint32_t start_addr = NOVA_UPGRADE_COLLECTOR_FLASH_CTX_ADDR;

    SpiFlashWriteBytesWithErase(start_addr, (uint8_t *)&gLocalUpgrade, sizeof(LocalUpgrade_t));
	return 0;
}

static int32_t _LocalUpgradeCollectorEnd( DevFc41d_t *fc41d,
							    UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	// 校验Flash中的升级包
	int32_t ret = 0;
	uint8_t md5[16] = {0};

	ret = _LocalUpgradeCollectorGetSpiFlashMd5(md5, gLocalUpgrade.recv_size);
	if (ret != 0)
	{
		return -1;
	}

	ret = _LocalUpgradeStructCheck(fc41d);
	if (ret != 0)
	{
		return ret;
	}

#if COLLECTOR_UPGRADE_TEST
	return 0;
#endif 

	_LocalUpgradeCollectorSpiFlashSaveCtx(gLocalUpgrade.recv_size, md5);
	// set upgrade flash flag
	_DevFc41dSetUpgradeFlashSectorFlag(fc41d);

	// update firmware ver
	CollectorSpiFlashWriteCfg();

	return ret;
}

static int32_t _LocalUpgradeStart(DevFc41d_t *fc41d, 
								int32_t status,
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body)
{
	const uint8_t *data = NULL;
	uint16_t len = 0;

	if (body)
	{
		data = (const uint8_t *)body->tlv.data;
		len = body->tlv.data_len;
		write_ctx->cmd = body->tlv.cmd;
	}

	write_ctx->trim_send_len = (int32_t)tim_get_systime();
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.data_len = 0;
	body->tlv.err = 0;

	// 检测是否正在升级
	int32_t dev_upgrade_stat = NovaDevUpgradeStatGet();
	if (dev_upgrade_stat != NOVA_DEV_UPGRADE_STAT_NONE 
		&& dev_upgrade_stat != NOVA_DEV_UPGRADE_STAT_COMPLETE)
	{
		body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_UPGRADING;
	}

	/*
	if (len > 1)
	{
		// 协议长度超出
		phead->body.tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_BAD_PROTO_LEN1;
	} else 
	*/
	if (len < 1)
	{
		// 协议长度不足
		phead->body.tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_BAD_PROTO_LEN2;
	}

	if (phead->body.tlv.err != 0)
	{
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
							(const uint8_t *)&body->tlv, 
							sizeof(UniformProtocolBodyHead_t));
		fc41d->priv_data->upgrade.pkg_type = COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NONE;
		fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
		fc41d->priv_data->upgrade.time = 0;

		if (write_ctx->finish)
		{
			if (write_ctx->upload_type == 0)
			{
				fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
										FC41D_PROCESS_BLE_STATUS_SEND_LEN,
										DEV_FC41D_TASK_STAT_WRITE, 0);
			}
			else
			{
				fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
										FC41D_PROCESS_MQTT_PUB_SEND_LEN,
										DEV_FC41D_TASK_STAT_WRITE, 0);
			}
		}
		return 0;
	}

	uint8_t type = data[0];

#if LOCAL_PCS_UPGRADE
	type = COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NOVA_5K;
#endif 

	fc41d->priv_data->upgrade.type = EN_UPGRADE_TYPE_LOCAL;
	fc41d->priv_data->upgrade.pkg_type = type;
	fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_INIT;
	fc41d->priv_data->upgrade.time = tim_get_systime();

	int32_t ret = 0;
	uint8_t tmp_md5[16] = {0};

	switch (type)
	{
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_COLLECTOR:
		ret = _LocalUpgradeCollectorStart(fc41d, (uint8_t *)&data[5], tmp_md5);
		if (ret == 1) // 已经下载过文件
		{
			body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_ALREADY_DOWNLOAD;
			write_ctx->cmd = PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START;
			write_ctx->next_cmd = PROTO_UNIFORM_W_RESET_NOW;
			write_ctx->upgrade_reset = 1;

			// 直接reset
		}
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NOVA_5K:
		ret = _LocalUpgradePcsStart(fc41d, (uint8_t *)&data[5], tmp_md5);
		if (ret == 1) // 已经下载过文件
		{
			body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_ALREADY_DOWNLOAD;

			// 直接向下位机发送升级包
			NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
		}
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_5K:
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_10K:
		break;
	default:
		body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_BAD_FILE_TYPE;
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
							(const uint8_t *)&body->tlv, 
							sizeof(UniformProtocolBodyHead_t));
		fc41d->priv_data->upgrade.pkg_type = COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NONE;
		fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
		fc41d->priv_data->upgrade.time = 0;
		break;
	}

	string_to_bytes((char *)&data[5], 32, (char *)gLocalUpgrade.recv_md5);
	if (ret < 0)
	{
		body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_DEBUG;
		if (ret == -2)
		{
			body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_BAD_PROTO_LEN1;
		}
		fc41d->priv_data->upgrade.pkg_type = COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NONE;
		fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
		fc41d->priv_data->upgrade.time = 0;
	}

#if DEBUG_SKIP_MD5_CHECK

#else 
		if (len - 5 < 32)
		{
			body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_FILE_LOCAL_MD5;
			fc41d->priv_data->upgrade.pkg_type = COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NONE;
			fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
			fc41d->priv_data->upgrade.time = 0;
			break;
		}
#endif 
		body->tlv.data_len = sizeof(ret) + 32 + sizeof(gLocalUpgrade.recv_md5) + sizeof(tmp_md5);
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
							(const uint8_t *)&body->tlv, 
							sizeof(UniformProtocolBodyHead_t));

		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)&ret, sizeof(ret));
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)&data[5], 32);
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)gLocalUpgrade.recv_md5, sizeof(gLocalUpgrade.recv_md5));
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)tmp_md5, sizeof(tmp_md5));


	if (write_ctx->finish)
	{
	if (write_ctx->upload_type == 0)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
							FC41D_PROCESS_BLE_STATUS_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	else
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
							FC41D_PROCESS_MQTT_PUB_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	}

	return 0;

}

static int32_t _LocalUpgradeDownFile(DevFc41d_t *fc41d, 
								int32_t status,
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body)
{
	const uint8_t *data = NULL;
	uint16_t len = 0;

	if (body)
	{
		data = (const uint8_t *)body->tlv.data;
		len = body->tlv.data_len;
		write_ctx->cmd = body->tlv.cmd;
	}

	write_ctx->trim_send_len = (int32_t)tim_get_systime();
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;

	fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_PARSE;

	switch (fc41d->priv_data->upgrade.pkg_type)
	{
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_COLLECTOR:
		_LocalUpgradeCollectorDownFile(fc41d, uniform, data, len);
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NOVA_5K:
		_LocalUpgradePcsDownFile(fc41d, uniform, data, len);
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_5K:
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_10K:
		break;
	default:
		body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_BAD_FILE_TYPE;
		break;
	}

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	if (write_ctx->finish)
	{
	if (write_ctx->upload_type == 0)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
							FC41D_PROCESS_BLE_STATUS_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	else
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
							FC41D_PROCESS_MQTT_PUB_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	}

	return 0;
}

static int32_t _LocalUpgradeEnd(DevFc41d_t *fc41d, 
								int32_t status,
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body)
{
	int32_t ret = -1;
	const uint8_t *data = NULL;
	uint16_t len = 0;

	if (body)
	{
		data = (const uint8_t *)body->tlv.data;
		len = body->tlv.data_len;
		write_ctx->cmd = body->tlv.cmd;
	}

	write_ctx->next_cmd = PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END;
	write_ctx->trim_send_len = (int32_t)tim_get_systime();
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;

	switch (fc41d->priv_data->upgrade.pkg_type)
	{
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_COLLECTOR:
		ret = _LocalUpgradeCollectorEnd(fc41d, uniform, data, len);
		body->tlv.err = -ret;
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NOVA_5K:
		ret = _LocalUpgradePcsEnd(fc41d, uniform, data, len);
		body->tlv.err = -ret;
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_5K:
		body->tlv.err = -ret;
		break;
	case COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_KSTAR_10K:
		body->tlv.err = -ret;
		break;
	default:
		body->tlv.err = COLLECTOR_LOCAL_UPGRADE_ERR_BAD_FILE_TYPE;
		break;
	}

	if (ret != 0)
	{
		fc41d->priv_data->upgrade.pkg_type = COLLECTOR_LOCAL_UPGRADE_FILE_TYPE_NONE;
		fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
		fc41d->priv_data->upgrade.time = 0;
	}

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	if (write_ctx->finish)
	{
	if (write_ctx->upload_type == 0)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
							FC41D_PROCESS_BLE_STATUS_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	else
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
							FC41D_PROCESS_MQTT_PUB_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	}

	return 0;
}

int32_t _DevFc41dProcessNovaRequest_w_dev_local_upgrade(DevFc41d_t *fc41d, 
								int32_t status,
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body)
{
	// 更新timeout时间
	fc41d->priv_data->upgrade.time = tim_get_systime();
	switch (status)
	{
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START:
		return _LocalUpgradeStart(fc41d, status, write_ctx, uniform, body);
		break;
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE:
		return _LocalUpgradeDownFile(fc41d, status, write_ctx, uniform, body);
		break;
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END:
		return _LocalUpgradeEnd(fc41d, status, write_ctx, uniform, body);
		break;
	}

	return -1;
}

