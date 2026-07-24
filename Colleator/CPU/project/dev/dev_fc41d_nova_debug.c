#include "dev_fc41d_common.h"
#include "subdev.h"
//#include "aes1.h"
#include "proto_nova_uniform.h"
#include "pcs_op.h"
#include "pcs_op_nova.h"
#include "nova_dev_upgrade.h"

#if 1
// 说明:
// 1. 测试采集器升直接级监控卡 0x 33 4e 4f 56 41 D0 00 00 00 00 00 00
// 2. 测试采集器获取PCS Modbus 0x 33 4e 4f 56 41 D0 00 00 00 00 00 00
// 3. 测试采集器获取DEV MQTT 等信息 0x 33 4e 4f 56 41 EB 00 00 00 00 00 00
// 4. 测试采集器升直接级监控卡 0x 33 4e 4f 56 41 D0 00 00 00 00 00 00
// 5. 测试采集器升直接级监控卡 0x 33 4e 4f 56 41 D0 00 00 00 00 00 00

#endif 

#define SPI_FLASH_LOG_ADDR						0x3E8000+0x2000

#define AT_FC41D_NOVA_URC							"NOVA"

extern DevFc41dTask_t* _DevFc41dTaskGetHeadPtr(DevFc41d_t *fc41d);
extern int32_t _DevFc41dNovaRequest(DevFc41d_t *fc41d, 
						int8_t type,
						int32_t upload_type,
						UniformProtocolHead_t *uniform);

int32_t _DevFc41dNovaRequestReadTestMqttSub(DevFc41d_t *fc41d, 
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
	}


	fc41d->priv_data->mqtt.test_sub_process = 0;
	fc41d->priv_data->mqtt.test_sub = FC41D_MQTT_SUB_TEST_SIZE;
	return 0;
}

int32_t _DevFc41dNovaRequestReadDebugResp(DevFc41d_t *fc41d, 
								int32_t upload_type,
                                UniformProtocolHead_t *uniform,
								UniformProtocolBodyHead_t *body,
								const char *send_data)
{
	const uint8_t *data = NULL;
	uint16_t len = 0;

	if (body)
	{
		data = (const uint8_t *)body->tlv.data;
		len = body->tlv.data_len;
	}

	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.cmd = uniform->body.tlv.cmd;
	write_ctx.trim_send_len = (int32_t)tim_get_systime();
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&uniform, sizeof(UniformProtocolHead_t));
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	phead->dst = (0x1F);
	phead->src = (0x1F);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;
	phead->body.tlv.cmd = (uniform->body.tlv.cmd);
	phead->body.tlv.err = 0;

	char buf[512] = {0};
	snprintf(buf, sizeof(buf), "%s", send_data);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf, strlen(buf));

	write_ctx.finish = 1;
	if (write_ctx.finish)
	{
	if (upload_type == 0)
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

int32_t _DevFc41dNovaRequestReadDebugGetUpgradeFile(DevFc41d_t *fc41d, 
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
	write_ctx->debug = 1;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	uint32_t start_addr = SPI_FLASH_LOG_ADDR;
	const int32_t buf_len = sizeof(fc41d->priv_data->http.custom.ali_oss.tmp_md5_size)
		+sizeof(fc41d->priv_data->http.custom.ali_oss.tmp_md5);
	uint8_t buf[buf_len] = {0};

	SpiFlashReadData(start_addr, (uint8_t *)buf, buf_len);

	body->tlv.err = 0;
	body->tlv.data_len = buf_len;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf,  buf_len);

	write_ctx->finish = 1;
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

int32_t _DevFc41dNovaRequestReadDebugGetUpgradeMd5(DevFc41d_t *fc41d, 
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
	write_ctx->debug = 1;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	uint32_t start_addr = SPI_FLASH_LOG_ADDR;
	const int32_t buf_len = sizeof(fc41d->priv_data->http.custom.ali_oss.tmp_md5_size)
		+sizeof(fc41d->priv_data->http.custom.ali_oss.tmp_md5);
	uint8_t buf[buf_len] = {0};

	SpiFlashReadData(start_addr, (uint8_t *)buf, buf_len);

	body->tlv.err = 0;
	body->tlv.data_len = buf_len;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf,  buf_len);

	write_ctx->finish = 1;
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

int32_t _DevFc41dNovaRequestReadDebug(DevFc41d_t *fc41d, 
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
	write_ctx->debug = 1;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));


	if (write_ctx->cmd == PROTO_UNIFORM_R_DEBUG)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
								FC41D_PROCESS_WIFI_STATION_STAT_UPDATE,
								DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
							FC41D_PROCESS_WIFI_STATION_GET_IP,
							DEV_FC41D_TASK_STAT_WRITE, 0);

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

#if 0
int32_t _DevFc41dNovaRequestReadDebugAes(DevFc41d_t *fc41d, 
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
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	WORD key_schedule[60] = {0};
	BYTE enc_buf[128] = {0};
	BYTE plaintext[128] = {"1234567890111"};
	BYTE ciphertext[128] = {0};
	BYTE iv[16] = {-2, 19, -74, 26, 58, -60, -40, -43, 116, 69, 106, 73, 104, -4, -14, -37};
	BYTE key[32] = {97, 115, 100, 102, 103, 104, 48, 49, 50, 51, 57, 52, 108, 106, 55, 103, 56, 57, 106, 105, 103, 100, 114, 115, 118, 56, 51, 53, 52, 107, 98, 103};

	aes_key_setup(key, key_schedule, 256);
	int32_t enc_buf_len = strlen((char *)plaintext);
	enc_buf_len = ((enc_buf_len%AES_BLOCK_SIZE)>0?1:0)*AES_BLOCK_SIZE
		+ (enc_buf_len/AES_BLOCK_SIZE)*AES_BLOCK_SIZE;
	aes_encrypt_cbc(plaintext, enc_buf_len, enc_buf, key_schedule, 256, iv);
	memcpy(ciphertext, enc_buf, enc_buf_len);
	aes_decrypt_cbc(ciphertext, enc_buf_len, enc_buf, key_schedule, 256, iv);

	DevFc41dTask_t *task = _DevFc41dTaskGetHeadPtr(fc41d);
	char buf[512] = {0xDD, 0xDD, 0xDD, 0xDD};
	memcpy(buf+sizeof(int32_t), ciphertext, enc_buf_len);

	body->tlv.err = 0;
	body->tlv.data_len = enc_buf_len+sizeof(int32_t);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf, 
        enc_buf_len+sizeof(int32_t));


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
#endif 

int32_t _DevFc41dNovaRequestReadDebugGetProto(DevFc41d_t *fc41d, 
								int32_t upload_type,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	write_ctx.uniform_type = 1;	// 第一包TLV用自己的L长度
	//write_ctx.debug = 1;
	write_ctx.cmd = uniform->body.tlv.cmd;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)uniform, sizeof(UniformProtocolHead_t)
						-sizeof(UniformProtocolBodyHead_t));
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)data, len);

	write_ctx.finish = 1;
	if (write_ctx.finish)
	{
	if (upload_type == 0)
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

int32_t _DevFc41dNovaRequestReadDebugGetPcsModbus(DevFc41d_t *fc41d, 
								int32_t upload_type, int8_t req_type,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	write_ctx.uniform_type = 1;	// 第一包TLV用自己的L长度
	write_ctx.debug = 1;
	write_ctx.cmd = uniform->body.tlv.cmd;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)uniform, sizeof(UniformProtocolHead_t));
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	int32_t debug_idx = ((int32_t)data[0]) << 24 |  ((int32_t)data[1]) << 16 |
		((int32_t)data[2]) << 8 | ((int32_t)data[3]);
	uint16_t debug_addr = ((uint16_t)data[4]) << 8 | ((uint16_t)data[5]);
	uint16_t debug_reg_num = ((uint16_t)data[6]) << 8 | ((uint16_t)data[7]);
	if (req_type == 0)
	{
		DevGetDebugReq(debug_idx, debug_addr, debug_reg_num);
	}
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)gPcsDevDebugModebusInfo, 
						sizeof(gPcsDevDebugModebusInfo));

	write_ctx.finish = 1;
	if (write_ctx.finish)
	{
	if (upload_type == 0)
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

int32_t _DevFc41dNovaRequestWriteDebugSetPcsUpgrade(DevFc41d_t *fc41d, 
								int32_t upload_type, int8_t req_type,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	int32_t upgrade_flag = NovaDevUpgradeStatGet();
	if (upgrade_flag != NOVA_DEV_UPGRADE_STAT_PREPARE)
	{
		NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
		return 0;
	}

	return 0;
}

int32_t _DevFc41dNovaRequestWriteDebugModule(DevFc41d_t *fc41d, 
								DevFc41dWriteCtx_t * write_ctx,
								UniformProtocolHead_t *uniform,
								const uint8_t *data, uint16_t len)
{
	write_ctx->debug = 1;
	write_ctx->cmd = uniform->body.tlv.cmd;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	uint32_t start_addr = SPI_FLASH_LOG_ADDR;
	const int32_t buf_len = sizeof(fc41d->priv_data->http.custom.ali_oss.tmp_md5_size)
		+sizeof(fc41d->priv_data->http.custom.ali_oss.tmp_md5);
	uint8_t buf[buf_len] = {0};

	SpiFlashReadData(start_addr, (uint8_t *)buf, buf_len);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf,  buf_len);

	write_ctx->finish = 1;
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

int32_t _DevFc41dNovaRequestWriteDebugWriteMonitorSn(DevFc41d_t *fc41d, 
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
	write_ctx->debug = 1;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	// call pcs
	NovaDevUpdateMonitorSN((const char *)data, len);

	body->tlv.err = 0;
	body->tlv.data_len = len;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));


	int32_t send_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)data, len);
	send_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, send_len-len-len, len);

	write_ctx->finish = 1;
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

int32_t _DevFc41dNovaRequestDebugMonitorAuth(DevFc41d_t *fc41d, 
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
	write_ctx->debug = 1;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	// call pcs
	NovaDbgAuth((const uint8_t  *)data, len);

	body->tlv.err = 0;
	body->tlv.data_len = len;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));


	int32_t send_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)data, len);
	send_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, send_len-len-len, len);

	write_ctx->finish = 1;
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

int32_t _DevFc41dNovaRequestDebugClsMonitorTotalFlash(DevFc41d_t *fc41d, 
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
	}
	int32_t ret = 0;

	DevNovaSet_t *pdata = (DevNovaSet_t *)data;
	int32_t read_len = 0;
	uint16_t reg_addr = 0;

	EN_MODBUS_FUNC_CODE func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
	reg_addr = 0x5064;
	uint8_t reg_data[4] = { 0x01, 0x00, 0x00, 0x00};
	ret = NovaDbgClsMonitorTotalFlash(func_code, reg_addr, 1, (uint8_t *)&reg_data[0]);
	if (ret != 0)
	{
		return ret;
	}

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_NOVA_GET_MONITOR_HARDFAULT, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

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

	return ret;
}

int32_t _DevFc41dNovaRequestDebugOtaFc41d(DevFc41d_t *fc41d, 
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
		write_ctx->cmd = PROTO_UNIFORM_W_DEBUG_OTA_FC41D;
	}
	int32_t cmd_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	int32_t send_len = cmd_len - sizeof(DevFc41dWriteCtx_t);

	write_ctx->debug = 0;
	write_ctx->next_cmd = PROTO_UNIFORM_W_DEBUG_OTA_FC41D;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = uniform->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	int32_t ret = 0;
	
	fc41d->priv_data->http.ota_fc41d_reboot_time = tim_get_systime();
	fc41d->priv_data->http.ota_fc41d = 1;
	fc41d->priv_data->http.url_len 
		= snprintf(fc41d->priv_data->http.url, FC41D_HTTP_URL_LEN, "AT+QWLANOTA=%s\r\n", data);

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	write_ctx->finish = 1;
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
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_OTA_FC41D,
								DEV_FC41D_TASK_STAT_WRITE, 0);
#if 0
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
							FC41D_PROCESS_MQTT_PUB_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
#endif 
	}
	}

	return 0;
}

int32_t _DevFc41dNovaRequestDebugGetMonitorHardfault(DevFc41d_t *fc41d, 
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
	}
	int32_t ret = 0;

	DevNovaSet_t *pdata = (DevNovaSet_t *)data;
	int32_t read_len = 0;
	uint16_t reg_addr = 0x8200;

	EN_MODBUS_FUNC_CODE func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;
	if (data[0] == 0x00)
	{
		reg_addr = 0x8201;
	}
	else if (data[0] == 0x01)
	{
		reg_addr = 0x8200;
	}
	ret = NovaDbgGetMonitorHardfault(func_code, reg_addr, 1, (uint8_t *)&reg_addr);
	if (ret != 0)
	{
		return ret;
	}

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_NOVA_GET_MONITOR_HARDFAULT, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

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

	return ret;
}

int32_t _DevFc41dUrcNovaDebug(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	//return 0;
	char *tmp = (char *)data;

	if (len < 5) 
	{
		// not enough
		return 0;
	}

	if (tmp[0] == 0x33)
	{
        ++tmp;
        if (strncmp(tmp, AT_FC41D_NOVA_URC, strlen(AT_FC41D_NOVA_URC)))
        {
		// not found "NOVA" header [02 4e 4f 56 41 01]
		return 1;
        }
	}
    else
    {
        return 0;
    }

	int32_t upload_type = 0;
	int32_t tmp_len = strlen(AT_FC41D_NOVA_URC);
	uint8_t meta_uniform[1024] = {0};
	UniformProtocolHead_t *puniform = (UniformProtocolHead_t *)meta_uniform;
	puniform->src = (0x40);
	puniform->dst = (0x10);
	if (tmp[tmp_len] == 0x01) 
	{
		if (tmp[tmp_len + 1] == 0x00) // 重启
		{
			_DevFc41dNovaRequestReadDebugResp(fc41d, upload_type, puniform, NULL, "OK");
			delay_1ms(2000);

			// reset
			__set_FAULTMASK(1);
			NVIC_SystemReset();

			return len;
		}
		else if (tmp[tmp_len + 1] == 0x01) // 恢复出厂
		{
			CollectorSpiFlashEraseCfg();
			_DevFc41dNovaRequestReadDebugResp(fc41d, upload_type, puniform, NULL, "OK");
			delay_1ms(2000);

			// reset
			__set_FAULTMASK(1);
			NVIC_SystemReset();

			return len;
		}
		else if (tmp[tmp_len + 1] == 0x02) // 开启调试模式设置采集器类型
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEBUG_MODULE);
		}
		else if (tmp[tmp_len + 1] == 0x03) // 写入监控卡SN
		{
			// sn:AMP249201234567[0x334e4f56410103414D50323439323031323334353637]
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEBUG_WRITE_MONITOR_SN);
			puniform->body.tlv.data_len = 0;
			puniform->body.tlv.data_len = &data[len-1] - &tmp[tmp_len+2];
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+2], puniform->body.tlv.data_len);
			}
			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}
		else if (tmp[tmp_len + 1] == 0x04) // 监控卡认证功能开关
		{
			// 并网功率软限制:[0x334e4f56410104000101]
			// 并网功率硬限制:[0x334e4f56410104000201]
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEBUG_MONITOR_AUTH);
			puniform->body.tlv.data_len = 0;
			puniform->body.tlv.data_len = &data[len] - &tmp[tmp_len+2];
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+2], puniform->body.tlv.data_len);
			}
			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}
		else if (tmp[tmp_len + 1] == 0x05) // 获取监控卡hardfault
		{
			// 获取 [0x334e4f5641010500]
			// 清空 [0x334e4f5641010501]
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEBUG_GET_MONITOR_HARDFAULT);
			puniform->body.tlv.data_len = 0;
			puniform->body.tlv.data_len = &data[len] - &tmp[tmp_len+2];
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+2], puniform->body.tlv.data_len);
			}
			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}
		else if (tmp[tmp_len + 1] == 0x06) // 清理监控卡统计Flash
		{
			// 获取 [0x334e4f5641010600]
			// 清空 [0x334e4f5641010601]
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEBUG_CLS_MONITOR_TOTAL_FLASH);
			puniform->body.tlv.data_len = 0;
			puniform->body.tlv.data_len = &data[len] - &tmp[tmp_len+2];
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+2], puniform->body.tlv.data_len);
			}
			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}
		else 
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_R_WIFI_SCAN); // 搜索WIFI
		}
	} 
	else if (tmp[tmp_len] == 0x02) // 获取WIFI状态
	{
		upload_type = 0;
		if (tmp[tmp_len+1] != 0x00)
		{
			upload_type = 1;
		}

		if (tmp[tmp_len+2] == 0x00) // 设置OTA升级FC41D
		{
			// http://192.168.1.105/http_server_tmp/FC41DAAR03A10beta1213V02_ota.rbl
			// 334e4f5641020000 687474703A2F2F3139322E3136382E312E3130352F687474705F7365727665725F746D702F4643343144414152303341313062657461313231335630325F6F74612E72626C
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEBUG_OTA_FC41D);
			puniform->body.tlv.data_len = 0;
			puniform->body.tlv.data_len = &data[len] - &tmp[tmp_len+3];
			if (upload_type == 1)
			{
				puniform->body.tlv.data_len -= 3;
			}
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+3], puniform->body.tlv.data_len);
				puniform->body.tlv.data[puniform->body.tlv.data_len] = 0;
			}
			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}

		if (tmp[tmp_len+2] == 0x01) // 设置连接指定WIFI(ERR)
		{
			// const char wifi_info[] = {"NovaStar_Energy,12345678"};
			// 334e4f5641020001 4E6F7661537461725F456E657267792C3132333435363738
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_WIFI_INFO);
			puniform->body.tlv.data_len = &data[len] - &tmp[tmp_len+3];
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+3], puniform->body.tlv.data_len);
			}

			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}
		else if (tmp[tmp_len+2] == 0x02) // 设置Property
		{
			const uint8_t property_info[] = {};
			// 334e4f5641020002 01011200
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_PROPERTY);
			puniform->body.tlv.data_len = &data[len] - &tmp[tmp_len+3];
			if (puniform->body.tlv.data_len > 0)
			{
				memcpy(puniform->body.tlv.data, &tmp[tmp_len+3], puniform->body.tlv.data_len);
			}

			_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			return len;
		}
	}
	else if (tmp[tmp_len] == 0x05) // 获取采集器DEV INFO
	{
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEV_INFO);
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}

		if (tmp[tmp_len+2] == 0x01)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEV_NAME);
		}
		else if (tmp[tmp_len+2] == 0x02) // 获取Kstar逆变器ModelID, SN
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEV_PRODUCT);
		}
	}
	else if (tmp[tmp_len] == 0x06) // 设置采集器DEV INFO
	{
		if (tmp[tmp_len+1] == 0x01) // 设置采集器DEV INFO(长度超限)
		{
			char info[68] = {0};
			for (int8_t i = 2; i < sizeof(info); ++i) 
			{
				info[i] = i+0x30;
			}
			info[0] = 0x12;
			info[1] = 0x34;
			memcpy(puniform->body.tlv.data, info, sizeof(info));
			puniform->body.tlv.data_len = (sizeof(info));
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_INFO);
		}
		else
		{
			char info[] = {"..SN:123456"};
			info[0] = 0x12;
			info[1] = 0x34;
			memcpy(puniform->body.tlv.data, info, sizeof(info));
			puniform->body.tlv.data_len = (sizeof(info));
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_INFO);
		}
	}
	else if (tmp[tmp_len] == 0x07) // 获取采集器DEV MQTT INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEV_MQTT_INFO);
	}
	else if (tmp[tmp_len] == 0x08) // 设置采集器DEV MQTT INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			char mqtt_info[] = {".10.41.91.176:1883"};
			mqtt_info[0] = 0x00;
			memcpy(puniform->body.tlv.data, mqtt_info, sizeof(mqtt_info));
			puniform->body.tlv.data_len = (sizeof(mqtt_info));
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_MQTT_INFO);
		}
		else if (tmp[tmp_len+1] == 0x02)
		{
			char mqtt_info[] = {".172.16.81.177:1883"};
			mqtt_info[0] = 0x00;
			memcpy(puniform->body.tlv.data, mqtt_info, sizeof(mqtt_info));
			puniform->body.tlv.data_len = (sizeof(mqtt_info));
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_MQTT_INFO);
		}
		else if (tmp[tmp_len+1] == 0x03)
		{
			char mqtt_info[] = {".172.16.82.118:63940"};
			mqtt_info[0] = 0x00;
			memcpy(puniform->body.tlv.data, mqtt_info, sizeof(mqtt_info));
			puniform->body.tlv.data_len = (sizeof(mqtt_info));
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_MQTT_INFO);
		}
		else if (tmp[tmp_len+1] == 0x04)
		{
			char mqtt_info[] = {".sit-ems-emqx.novaicloud.com:1883"};
			mqtt_info[0] = 0x01;
			memcpy(puniform->body.tlv.data, mqtt_info, sizeof(mqtt_info));
			puniform->body.tlv.data_len = (sizeof(mqtt_info));
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_MQTT_INFO);
		}
		else if (tmp[tmp_len+1] == 0x05)
		{
			uint32_t tmp_data_len = (uint32_t)(&tmp[tmp_len+2]) - (uint32_t)data;
			tmp_data_len = len - tmp_data_len;
			puniform->body.tlv.data[0] = 0x02;
			memcpy(&puniform->body.tlv.data[1], &tmp[tmp_len+2], tmp_data_len);
			puniform->body.tlv.data_len = tmp_data_len+1;
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_MQTT_INFO);
		}
		else if (tmp[tmp_len+1] == 0x06)
		{
			puniform->body.tlv.cmd = 0;
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_CLOSE,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		}
	}
	else if (tmp[tmp_len] == 0x09) // mqtt pub
	{
		int32_t mqtt_len = 0;
		char mqtt_info[] = {"test_pub..."};
		mqtt_len = strlen(mqtt_info);
		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_R_KSTAR_REGISTER, 
								   0, (const uint8_t *)mqtt_info, mqtt_len);
	}
	else if (tmp[tmp_len] == 0x0A) // mqtt pub 10 bytes
	{
		int32_t mqtt_len = 0;
		char mqtt_info[512] = {0};
		DevKstarRegisterInfo_t *kstar = (DevKstarRegisterInfo_t *)mqtt_info;
		kstar->reg_start = 3000; // 3000~4000
		kstar->reg_size = 5;
		for (int32_t i = 0; i < kstar->reg_size; ++i)
		{
			kstar->data[i] = i+0x01;
            ++i;
			kstar->data[i] = i+0x01;
		}
		mqtt_len = 2*kstar->reg_size+sizeof(DevKstarRegisterInfo_t);

		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_R_KSTAR_REGISTER, 
								   0, (const uint8_t *)mqtt_info, mqtt_len);
	}
	else if (tmp[tmp_len] == 0x0B) // mqtt pub 1000 bytes
	{
		int32_t mqtt_len = 0;
		char mqtt_info[2048] = {0};
		DevKstarRegisterInfo_t *kstar = (DevKstarRegisterInfo_t *)mqtt_info;
		kstar->reg_start = 3000; // 3000~4000
		kstar->reg_size = 900;
		for (int32_t i = 0; i < kstar->reg_size*2; ++i)
		{
			kstar->data[i] = i+0x01;
			++i;
			kstar->data[i] = i+0x01;
		}
		mqtt_len = 2*kstar->reg_size+sizeof(DevKstarRegisterInfo_t);

		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_R_KSTAR_REGISTER, 
								   0, (const uint8_t *)mqtt_info, mqtt_len);
	}
	else if (tmp[tmp_len] == 0x0C) // mqtt pub kstar dev info
	{
		int32_t mqtt_len = 0;
		char mqtt_info[512] = {0};
		DevKstarSysInfo_t *kstar = (DevKstarSysInfo_t *)mqtt_info;
		snprintf(kstar->machine_id, sizeof(kstar->machine_id), "%s", "kstar-12344");
		snprintf(kstar->battery_name, sizeof(kstar->battery_name), "%s", "kstar-battery01");
		kstar->arm_ver = 0x01;
		kstar->arm_test_ver = 0x02;
		kstar->dsp_ver = 0x13;
		kstar->dsp_test_ver = 0x14;
		kstar->dsp2_ver = 0x25;
		kstar->dsp2_test_ver = 0x26;
		mqtt_len = sizeof(DevKstarSysInfo_t);

		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_R_KSTAR_SYS_INFO, 
								   0, (const uint8_t *)mqtt_info, mqtt_len);
	}
	else if (tmp[tmp_len] == 0x0D) // mqtt pub kstar dev info
	{
		int32_t mqtt_len = 0;
		char mqtt_info[512] = {0};
		DevKstarTelemetry_t *kstar = (DevKstarTelemetry_t *)mqtt_info;
		mqtt_len = sizeof(DevKstarTelemetry_t);

		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_R_KSTAR_REGISTER, 
								   0, (const uint8_t *)mqtt_info, mqtt_len);
	}
	else if (tmp[tmp_len] == 0x0E) // 修改SN
	{
		int32_t sn_len = *(uint8_t*)(&tmp[tmp_len+1]);
		char sn[32] = {0};

		memcpy(sn, tmp+tmp_len+2, sn_len);
		CollectorUpdateSn((const uint8_t*)sn, sn_len);
	}
	else if (tmp[tmp_len] == 0x0F) // 修改采集器名字
	{
		if (tmp[tmp_len+1] == 0x01) 
		{
			upload_type = 1;
		}

		if (tmp[tmp_len+2] == 0x01) 
		{
			upload_type = 1;
			char name[64] = {"Collector1"};
			CollectorUpdateName((const uint8_t*)name, strlen(name));
			fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_W_DEV_NAME, 0, NULL, 0);
		}
		else if (tmp[tmp_len+2] == 0x02)
		{
			char name[64] = {"Collector2"};
			CollectorUpdateName((const uint8_t*)name, strlen(name));
			memcpy(puniform->body.tlv.data, name, strlen(name));
			puniform->body.tlv.data_len = strlen(name);
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_NAME);
		}
		else if (tmp[tmp_len+2] == 0x03)
		{
			char name[64] = {"Collector3"};
			CollectorUpdateName((const uint8_t*)name, strlen(name));
			memcpy(puniform->body.tlv.data, name, strlen(name));
			puniform->body.tlv.data_len = strlen(name);
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_DEV_NAME);
		}
	}
	else if (tmp[tmp_len] == 0x11) // 采集器升级
	{
		if (tmp[tmp_len+1] == 0x01) 
		{
			upload_type = 1;
		}

		if (tmp[tmp_len+2] == 0x01)
		{
			if (tmp[tmp_len+3] == 0x01)
			{
				puniform->body.tlv.cmd = (PROTO_UNIFORM_R_TEST_UPGRADE_FILE);
				_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			}
			else
			{
				puniform->body.tlv.cmd = (PROTO_UNIFORM_R_TEST_UPGRADE_MD5);
				_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
			}
			return len;
		}
		else if (tmp[tmp_len+2] == 0x02)
		{
			fc41d->http_req(fc41d, NULL, PROTO_UNIFORM_R_HTTP, 0, NULL, 0);
		}
		else if (tmp[tmp_len+2] == 0x03)
		{
			fc41d->http_req(fc41d, NULL, PROTO_UNIFORM_W_DEV_UPGRADE+100, 0, NULL, 0);
		}
		else if (tmp[tmp_len+2] == 0x04)
		{
			fc41d->http_req(fc41d, NULL, PROTO_UNIFORM_W_DEV_UPGRADE+200, 0, NULL, 0);
		}
		return len;
	}
	else if (tmp[tmp_len] == 0x12) // 采集器升级
	{
		fc41d->priv_data->http.custom.ali_oss.size = 1000000;
		fc41d->priv_data->http.custom.ali_oss.pkg_size = 1024*6;
		fc41d->priv_data->http.custom.ali_oss.recv_size = 0;
		fc41d->priv_data->http.custom.ali_oss.pkg_num = 0;

		if (tmp[tmp_len+1] == 0x01) 
		{
			upload_type = 1;
		}

		if (tmp[tmp_len+2] == 0x00) 
		{
			fc41d->priv_data->http.custom.ali_oss.debug_type = 0;
			return len;
		}
		else if (tmp[tmp_len+2] == 0x01) 
		{
			fc41d->priv_data->http.custom.ali_oss.debug_type = 1;
			return len;
		}
		else if (tmp[tmp_len+2] == 0x02) 
		{
			char url[128] = {0};
			snprintf(url, sizeof(url), "%s", (const char *)&tmp[tmp_len+2]);
			snprintf(fc41d->priv_data->http.url, FC41D_HTTP_URL_LEN, "%s", url);
		}
		else if (tmp[tmp_len+2] == 0x03)
		{
			char date[128] = {0};
			snprintf(date, sizeof(date), "%s", (const char *)&tmp[tmp_len+2]);
			snprintf(fc41d->priv_data->http.custom.ali_oss.date, 
					 FC41D_HTTP_ALIYUN_OSS_DATE_LEN, "%s", date);
		}
		else if (tmp[tmp_len+2] == 0x04)
		{
			char auth[128] = {0};
			snprintf(auth, sizeof(auth), "%s", (const char *)&tmp[tmp_len+2]);
			snprintf(fc41d->priv_data->http.custom.ali_oss.auth, 
					 FC41D_HTTP_ALIYUN_OSS_AUTH_LEN, "%s", auth);
		}
		else if (tmp[tmp_len+2] == 0x05)
		{

			char *aliyun_data[5] = {NULL};
			uint16_t modelid[3] = {0};

			modelid[0] = *(uint16_t *)&tmp[tmp_len+3];
			modelid[1] = gCollector->type;
			if (modelid[0] != modelid[1])
			{
				uint8_t mqtt_info[128] = {0};
				int32_t mqtt_len = 0;

				snprintf((char *)mqtt_info, sizeof(mqtt_info), 
						 "dev_mode_id:%02X, upgrade_pkg_mode_id:%02X",
						 modelid[1], modelid[0]);
				fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_W_DEV_UPGRADE, 1, 
								(const uint8_t *)mqtt_info, strlen((const char *)mqtt_info));
				return len;
			}

			aliyun_data[0] = (char *)&tmp[tmp_len+5];

			for (int32_t i = 1; i < 5; ++i)
			{
				aliyun_data[i] = strstr(aliyun_data[i-1], "\n");
				if (aliyun_data[i] == NULL)
				{
					return len;
				}
				*aliyun_data[i] = 0;
				--aliyun_data[i];
				if (*aliyun_data[i] == '\r')
				{
					*aliyun_data[i] = 0;
					++aliyun_data[i];
				}
				++aliyun_data[i];
                ++aliyun_data[i];
			}

			int32_t type = atoi(aliyun_data[0]);
			if (type != fc41d->priv_data->http.custom_type)
			{
				// free old
				switch (fc41d->priv_data->http.custom_type)
				{
				case DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS:
#ifdef FC41D_HTTP_ALIYUN_OSS
					DevFc41dAliyunUninit((void *)fc41d);
#endif 
					break;
				}
			}

			modelid[2] = gCollector->pcs_modelid;
            fc41d->priv_data->http.custom_type = type;
			// malloc new
			switch (fc41d->priv_data->http.custom_type)
			{
			case DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS:
#ifdef FC41D_HTTP_ALIYUN_OSS
				DevFc41dAliyunInit((void *)fc41d, modelid[2], atoi(aliyun_data[1]));
#endif 
				break;
			}
			
			fc41d->priv_data->http.custom.ali_oss.size = atoi(aliyun_data[1]);
			snprintf(fc41d->priv_data->http.url, FC41D_HTTP_URL_LEN, "%s", aliyun_data[2]);
			snprintf(fc41d->priv_data->http.custom.ali_oss.date, 
					 FC41D_HTTP_ALIYUN_OSS_DATE_LEN, "%s", aliyun_data[3]);
			snprintf(fc41d->priv_data->http.custom.ali_oss.auth, 
					 FC41D_HTTP_ALIYUN_OSS_AUTH_LEN, "%s", aliyun_data[4]);
			fc41d->http_req(fc41d, NULL, PROTO_UNIFORM_W_DEV_UPGRADE, 0, (const uint8_t *)"1", 1);
            return len;
		}
		else if (tmp[tmp_len+2] == 0x05)
		{
			// 读取Flash数据
		}
	}
	else if (tmp[tmp_len] == 0x13) // 设置kstar设备
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}

		if (tmp[tmp_len+2] == 0x01)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_KSTAR_REGISTER);
		}
		else if (tmp[tmp_len+2] == 0x02)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_KSTAR_SET_SYS);
		}
		else if (tmp[tmp_len+2] == 0x03)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_KSTAR_OPT_REGISTER);
		}
		else if (tmp[tmp_len+2] == 0x04)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_KSTAR_REGISTER);
			puniform->body.tlv.err = 1;
		}
		else if (tmp[tmp_len+2] == 0x05)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_KSTAR_SET_SYS);
			puniform->body.tlv.err = 1;
		}
		else if (tmp[tmp_len+2] == 0x06)
		{
			puniform->body.tlv.cmd = (PROTO_UNIFORM_W_KSTAR_OPT_REGISTER);
			puniform->body.tlv.err = 1;
		}

	}
	else if (tmp[tmp_len] == 0x14) // AES 256 test
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG_AES);
	}
	else if (tmp[tmp_len] == 0xD0) // 采集器升级监控卡指令
	{
		if (len - tmp_len < 5)
		{
			return len;
		}

		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG_GET_PCS_MODBUS);

		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NOVA;
		fc41d->priv_data->write_stat.timeout = tim_get_systime();
		_DevFc41dNovaRequestWriteDebugSetPcsUpgrade(fc41d, upload_type, 
					tmp[tmp_len+1],
					puniform, (const uint8_t *)&tmp[tmp_len+2], len-7);
		return len;
	}
	else if (tmp[tmp_len] == 0xEA) // 获取采集器获取modbus数据
	{
		if (len - tmp_len < 5)
		{
			return len;
		}

		/*
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		*/

		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG_GET_PCS_MODBUS);

		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NOVA;
		fc41d->priv_data->write_stat.timeout = tim_get_systime();
		_DevFc41dNovaRequestReadDebugGetPcsModbus(fc41d, upload_type, 
					tmp[tmp_len+1],
					puniform, (const uint8_t *)&tmp[tmp_len+2], len-7);
		return len;
	}
	else if (tmp[tmp_len] == 0xEB) // 获取采集器DEV MQTT 等 INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG_GET_PROTO);

		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NOVA;
		fc41d->priv_data->write_stat.timeout = tim_get_systime();
		_DevFc41dNovaRequestReadDebugGetProto(fc41d, upload_type, 
					puniform, (const uint8_t *)&tmp[tmp_len+2], len-7);
		return len;
	}
	else if (tmp[tmp_len] == 0xEC) // 获取采集器DEV MQTT 等 INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG_GET_WIFI_INFO);
	}
	else if (tmp[tmp_len] == 0xED) // 获取采集器DEV MQTT 等 INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG_GET_MQTT_INFO);
	}
	else if (tmp[tmp_len] == 0xEE) // 获取采集器DEV MQTT 等 INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			upload_type = 1;
			puniform->dst = (0x02);
		}
		puniform->body.tlv.cmd = (PROTO_UNIFORM_R_DEBUG);
	}
	else if (tmp[tmp_len] == 0xEF) // 获取采集器DEV MQTT 等 INFO
	{
		if (tmp[tmp_len+1] == 0x01)
		{
			delay_1ms(5*1000);
		}
		else 
		{
			delay_1ms(7*1000);
		}
		return len;
	}

	_DevFc41dNovaRequest(fc41d, -2, upload_type, puniform);
	return len;
}

