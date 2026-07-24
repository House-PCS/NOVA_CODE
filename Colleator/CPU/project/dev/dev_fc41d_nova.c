#include "dev_fc41d_common.h"
#include "subdev.h"
#include "dev_fc41d_local_upgrade.h"
#include "dev_fc41d_nova_debug.h"
#include "pcs_op_megarevo.h"
#include "pcs_op_suro.h"
#include "pcs_op_nova.h"

extern int32_t _DevFc41dBleSendingStatus(DevFc41d_t *fc41d);
extern int32_t _DevFc41dMqttSendingStatus(DevFc41d_t *fc41d);
extern void _DevFc41dTaskClean(DevFc41d_t *fc41d);
extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);
extern int32_t _DevFc41dReadRelayMultiCmdKstar(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len);

static int32_t _DevFc41dNovaCmdStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_NOVA_CMD_NONE
			:fc41d->priv_data->nova_cmd.status);
}

int32_t _DevFc41dNovaCmdStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->nova_cmd.status = status;
	return 0;
}

static int32_t _DevFc41dNovaReadWifiScan(DevFc41d_t *fc41d, 
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

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	// add scan wifi, ble send task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						  FC41D_PROCESS_WIFI_STATION_WSCAN,
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

static int32_t _DevFc41dNovaReadWifiInfo(DevFc41d_t *fc41d, 
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
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

#if 1
	char tmp[128] = {0};
	// 直接从内存中获取
	snprintf(tmp, sizeof(tmp), "%s,%d", 
			 (const char *)fc41d->priv_data->wifi.info.station.ssid,
			 fc41d->priv_data->wifi.info.station.rssi);
	int32_t tmp_len = strlen(tmp);
	// write result to txbuffer
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)tmp, 
						strlen(tmp));

#else 
	// add scan wifi, ble send task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						  FC41D_PROCESS_WIFI_STATION_STAT,
						  DEV_FC41D_TASK_STAT_WRITE, 0);
#endif 
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

static int32_t _DevFc41dNovaWriteWifiInfo(DevFc41d_t *fc41d, 
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

	// stop mqtt upload
	fc41d->priv_data->wifi.info.station.net_setting_flag = 1;

	char wifi_info[2][64] = { {0} }; // 0-> ssid, 1->passwd
	char *p = (char *)data;

	for (uint16_t i = 0, j = 0, k = 0; i < len; ++i) 
	{
		if (data[i] == ',') 
		{
			j = 0;
			++k;
			continue;
		}
		wifi_info[k][j++] = data[i];
	}
	memset(fc41d->priv_data->wifi.info.station.ssid, 0, 
		   sizeof(fc41d->priv_data->wifi.info.station.ssid));
	memset(fc41d->priv_data->wifi.info.station.pwd, 0, 
		   sizeof(fc41d->priv_data->wifi.info.station.pwd));
	memcpy(fc41d->priv_data->wifi.info.station.ssid, wifi_info[0], 
		   sizeof(fc41d->priv_data->wifi.info.station.ssid)-1);
	memcpy(fc41d->priv_data->wifi.info.station.pwd, wifi_info[1], 
		   sizeof(fc41d->priv_data->wifi.info.station.pwd)-1);

	write_ctx->trim_send_len = (int32_t)tim_get_systime();
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	rbuffer_clear(&fc41d->priv_data->rb_busy.ctx);
	// reset wifi station status
	fc41d->priv_data->wifi.info.station.got_ip_stat = 0;
	fc41d->priv_data->wifi.info.station.conn_stat = 0;

	gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_GRID;
	fc41d->priv_data->mqtt.conn_stat = 0;
	// add connect wifi, ble send task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						FC41D_PROCESS_WIFI_STATION_STOP, 
						DEV_FC41D_TASK_STAT_WRITE, 0);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						  FC41D_PROCESS_WIFI_STATION_CONN,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						  FC41D_PROCESS_WIFI_STATION_CONN_STAT,
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

static int32_t _DevFc41dNovaReadDevInfo(DevFc41d_t *fc41d, 
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

	// cp dev info into tx buffer
	char body_data[512] = {0};

	snprintf(body_data, sizeof(body_data), "%s%s\r\n%s\r\n%s\r\n%s\r\n", 
			"..", gCollector->sn, gCollector->firmware, gCollector->pcs_sn,
			fc41d->priv_data->com_info.ver);
	uint16_t collector_type = (gCollector->type);
	uint8_t *pcollector_type = (uint8_t *)&collector_type;
	body->tlv.err = 0;
	body->tlv.data_len = strlen(body_data);
	body_data[0] = pcollector_type[1];
	body_data[1] = pcollector_type[0];
	//memcpy(body_data, (char *)&collector_type, sizeof(uint16_t));

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)body_data, body->tlv.data_len);

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

static int32_t _DevFc41dNovaWriteDevInfo(DevFc41d_t *fc41d, 
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
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;

	// save collector type|sn
	if (len < sizeof(gCollector->sn))
	{
		memset(gCollector->sn, 0, sizeof(gCollector->sn));
		memcpy((uint8_t *)&gCollector->sn, &data[2], len-2);
		gCollector->sn[sizeof(gCollector->sn)-1] = 0;

		uint8_t buf[] = { 0x0A, 0x0A, 0x0A, 0x0A };
		SpiFlashWriteBytesWithErase(0x200000, buf, sizeof(buf));
		SpiFlashWriteBytesWithErase(0x200000+0x200, gCollector->sn, sizeof(gCollector->sn));
		SpiFlashWriteBytesWithErase(0x202000, buf, sizeof(buf));
		SpiFlashWriteBytesWithErase(0x202000+0x200, gCollector->sn, sizeof(gCollector->sn));
	}
	else
	{
		body->tlv.err = 5; // 长度超限
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

static int32_t _DevFc41dNovaReadMqttInfo(DevFc41d_t *fc41d, 
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
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	// cp mqtt info 
	char mqtt_info[512] = {0};
	int32_t mqtt_info_len = 0;
	snprintf(mqtt_info, sizeof(mqtt_info), ".%s:%s", fc41d->priv_data->mqtt.server_ip, 
			 fc41d->priv_data->mqtt.server_port);
	mqtt_info_len = strlen(mqtt_info);
	mqtt_info[0] = fc41d->priv_data->mqtt.server_type;

	body->tlv.err = 0;
	body->tlv.data_len = mqtt_info_len;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)mqtt_info, mqtt_info_len);

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

static int32_t _DevFc41dNovaWriteMqttInfo(DevFc41d_t *fc41d, 
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

	int32_t ret = 0;

	gCollector->head.change = 1;
	fc41d->priv_data->mqtt.s_url = NULL;
	fc41d->priv_data->mqtt.conn_stat = 0;

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;

	fc41d->priv_data->mqtt.server_type = data[0];
	gCollector->mqtt.type = data[0];
	if (*data < 2)
	{
		char mqtt_info[2][256] = { {0} }; // 0-> ssid, 1->passwd
		char *p = (char *)data;

		for (uint16_t i = 1, j = 0, k = 0; i < len; ++i) 
		{
			if (data[i] == ':') 
			{
				j = 0;
				++k;
				continue;
			}
			mqtt_info[k][j++] = data[i];
		}

		memcpy(fc41d->priv_data->mqtt.server_ip, mqtt_info[0], 
			   sizeof(fc41d->priv_data->mqtt.server_ip));
		memcpy(fc41d->priv_data->mqtt.server_port, mqtt_info[1], 
			   sizeof(fc41d->priv_data->mqtt.server_port));
	}

	gCollector->head.s_url_len = len;
	if (gCollector->head.s_url)
	{
		free(gCollector->head.s_url);
	}
	gCollector->head.s_url = (char *)malloc(gCollector->head.s_url_len+1);
	if (!gCollector->head.s_url)
	{
		body->tlv.err = 1;
	}
	memset(gCollector->head.s_url, 0, gCollector->head.s_url_len+1);
	memcpy(gCollector->head.s_url, (char *)&data[0], gCollector->head.s_url_len);
	fc41d->priv_data->mqtt.s_url = &gCollector->head.s_url[1];
	fc41d->priv_data->mqtt.server_ip[0] = 0;
	fc41d->priv_data->mqtt.server_port[0] = 0;
	fc41d->priv_data->mqtt.username[0] = 0;
	fc41d->priv_data->mqtt.passwd[0] = 0;

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	if (write_ctx->finish)
	{
	if (write_ctx->upload_type == 0)
	{
		write_ctx->cmd_proity = 1;
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

static int32_t _DevFc41dNovaReadDevName(DevFc41d_t *fc41d, 
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
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = strlen((const char *)gCollector->name) + 1;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	// get collector's name 
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&gCollector->name, 
						strlen((const char *)gCollector->name));
	rbuffer_write_byte(&fc41d->priv_data->rb_write.ctx, 0x00);

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

static int32_t _DevFc41dNovaWriteDevName(DevFc41d_t *fc41d, 
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
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	// update name
	if (CollectorUpdateName(data, len) != 0)
	{
		body->tlv.err = 1;
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

static int32_t _DevFc41dNovaReadDevProperty(DevFc41d_t *fc41d, 
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
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	// get collector's property
	uint16_t property[2] = {0};
	uint8_t property_len = 0;
	uint8_t *tmp = CollectorGetProperty(COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD, &property_len);
	if (tmp)
	{
		memcpy(&property[0], tmp, property_len<sizeof(uint16_t)?property_len:sizeof(uint16_t));
	}
	tmp = CollectorGetProperty(COLLECTOR_PROPERTY_DATA_GET_PERIOD, &property_len);
	if (tmp)
	{
		memcpy(&property[1], tmp, property_len<sizeof(uint16_t)?property_len:sizeof(uint16_t));
	}

	body->tlv.err = 0;
	body->tlv.data_len = sizeof(property);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&property, sizeof(property));

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

static int32_t _DevFc41dNovaWriteDevProperty(DevFc41d_t *fc41d, 
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
	gCollector->head.change = 1;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	phead->dst = phead->src;
	phead->src = (0x10);
	phead->frame.seq_id = uniform->frame.seq_id;
	phead->frame.packet_num = 0;
	phead->frame.packet_index = 0;

	CollectorProperty_t *property = (CollectorProperty_t *)data;
	for (int32_t i = 0; i < len;)
	{
		if (property->idx != 0)
		{
			CollectorSetProperty(property->idx, (const uint8_t *)&property->data, 
								   property->len);
		}

		i += property->len + sizeof(uint8_t)*2;
		property = (CollectorProperty_t *)((&property->data)+sizeof(uint8_t)*property->len);
	}

	body->tlv.err = 0;
	body->tlv.data_len = 0;
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

static int32_t _DevFc41dNovaRequestWriteKstarRegister(DevFc41d_t *fc41d, 
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

	DevKstarTelemetry_t *kstar = (DevKstarTelemetry_t *)data;

	// add opt kstar task(危险参数，需要谨慎设置)
	ret = KstarBaseSet(kstar->reg_addr, kstar->reg_val);
	if (ret != 0)
	{
		return ret;
	}

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

	body->tlv.err = 0;
	body->tlv.data_len = 0;
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

	return ret;
}

static int32_t _DevFc41dNovaRequestWriteKstarSetSys(DevFc41d_t *fc41d, 
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

	DevKstarSetSys_t *kstar = (DevKstarSetSys_t *)data;
	int32_t read_len = 0;

	// add opt kstar task
	ret = KstarInvSet(kstar->reg_addr, kstar->reg_size, kstar->data);
	if (ret != 0)
	{
		return ret;
	}

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

	body->tlv.err = 0;
	body->tlv.data_len = 0;
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

	return ret;
}

static int32_t _DevFc41dNovaRequestWriteKstarOptRegister(DevFc41d_t *fc41d, 
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

	// opt kstar register (注意转网络序)
	DevKstarOptReg_t *kstar = (DevKstarOptReg_t *)data;

	// add opt kstar task(危险参数，需要谨慎设置)
	KstarRemoteSet(kstar->reg_addr, kstar->reg_val);

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

	body->tlv.err = 0;
	body->tlv.data_len = 0;
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

	return ret;
}

static int32_t _DevFc41dNovaRequestWriteMegarevoOptRegister(DevFc41d_t *fc41d, 
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

	DevMegarevoSet_t *pdata = (DevMegarevoSet_t *)data;
	int32_t read_len = 0;
	EN_MODBUS_FUNC_CODE func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;

	if (pdata->reg_size > 1)
	{
		func_code = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
	}

	ret = MegarevoInvSet(func_code, pdata->reg_addr, pdata->reg_size, pdata->data);
	if (ret != 0)
	{
		return ret;
	}

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

	body->tlv.err = 0;
	body->tlv.data_len = 0;
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

	return ret;
}

static int32_t _DevFc41dNovaRequestWriteSuroOptRegister(DevFc41d_t *fc41d, 
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

	DevSuroSet_t *pdata = (DevSuroSet_t *)data;
	int32_t read_len = 0;
	EN_MODBUS_FUNC_CODE func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;

	if (pdata->reg_size > 1)
	{
		func_code = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
	}

	ret = SuroInvSet(func_code, pdata->reg_addr, pdata->reg_size, pdata->data);
	if (ret != 0)
	{
		return ret;
	}

	// add wait task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR, 
							DEV_FC41D_TASK_STAT_READ, 0);
	fc41d->priv_data->write_stat.relay_low_machine = tim_get_systime();

	body->tlv.err = 0;
	body->tlv.data_len = 0;
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

	return ret;
}

static int32_t _DevFc41dNovaRequestReadNovaOptRegister(DevFc41d_t *fc41d, 
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
	int32_t ret = 0;
	uint16_t data_src[2] = {0}; // 寄存器起始地址 + 寄存器数量
	uint8_t data_dest[128] = {0};

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	if (len < sizeof(uint16_t))
	{
		// 数据长度不对，进行错误返回，不能直接return
		body->tlv.err = 1; // 数据长度不对，无法查找响应寄存器值
	}
	else 
	{
		memcpy(data_src, data, (len < sizeof(data_src)?len:sizeof(data_src)) );
	}
	
	int32_t read_len = NovaDataGetByAddr(data_dest, sizeof(data_dest), data_src[0], data_src[1]);
	if (read_len < 0)
	{
		body->tlv.err = 2; // 未获取到该寄存器数据
	}
	else 
	{
		body->tlv.data_len = read_len;
	}

	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)data_dest, 
						body->tlv.data_len);

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

static int32_t _DevFc41dNovaRequestWriteNovaOptRegister(DevFc41d_t *fc41d, 
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
	int32_t ret = 0;

	DevNovaSet_t *pdata = (DevNovaSet_t *)data;
	int32_t read_len = 0;
	EN_MODBUS_FUNC_CODE func_code = EN_MODBUS_FUNC_CODE_WRITE_SINGLE;

	if (pdata->reg_size > 1)
	{
		func_code = EN_MODBUS_FUNC_CODE_WRITE_MULTI;
	}

	ret = NovaInvSet(func_code, pdata->reg_addr, pdata->reg_size, pdata->data);
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
							FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR, 
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

static int32_t _DevFc41dNovaRequestReadDevProduct(DevFc41d_t *fc41d, 
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


	char buf[128] = {0};
	uint16_t modelid = gCollector->pcs_modelid;
	snprintf(buf, sizeof(buf), "%s", (const char *)gCollector->pcs_sn);

	body->tlv.err = 0;
	body->tlv.data_len = sizeof(modelid) + strlen(buf) + 1;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));
	
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)&modelid, sizeof(modelid));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf, strlen(buf));
	rbuffer_write_byte(&fc41d->priv_data->rb_write.ctx, 0x00);

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

static int32_t _DevFc41dNovaRequestWriteDevUpgrade(DevFc41d_t *fc41d, 
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

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	uint8_t mqtt_info[128] = {0};
	int32_t mqtt_len = 0;
	char *aliyun_data[16] = {NULL};
	uint16_t modelid[3] = {0};

	modelid[0] = *(uint16_t *)data;
	modelid[1] = gCollector->type;
	modelid[2] = EN_FILE_PKG_TYPE_NOVA_COLLECTOR;

	if (modelid[0] != modelid[1])
	{
		snprintf((char *)mqtt_info, sizeof(mqtt_info), 
				 "dev_mode_id:%02X, upgrade_pkg_mode_id:%02X",
				 modelid[1], modelid[0]);
		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_W_DEV_UPGRADE, 1, 
						(const uint8_t *)mqtt_info, strlen((const char *)mqtt_info));
        return 0;
	}

	aliyun_data[0] = (char *)&data[2];

	for (int32_t i = 1; i < 16; ++i)
	{
		aliyun_data[i] = strstr(aliyun_data[i-1], "\n");
		if (aliyun_data[i] == NULL)
		{
			break;
		}
		*aliyun_data[i] = 0;
		--aliyun_data[i];
		if (*aliyun_data[i] == '\r')
		{
			*aliyun_data[i] = 0;
		}
		++aliyun_data[i];
		++aliyun_data[i];
	}
	char *p = (char *)&data[len];
	*p = 0x00;

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
		case DEV_FC41D_HTTP_CUSTOM_MQTT_INFO:
			DevFc41dHttpMqttInfoUninit((void *)fc41d);
			break;
		}
	}

	fc41d->priv_data->http.custom_type = type;
	// malloc new
	switch (fc41d->priv_data->http.custom_type)
	{
	case DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS:
#ifdef FC41D_HTTP_ALIYUN_OSS
		DevFc41dAliyunInit((void *)fc41d, modelid[2], atoi(aliyun_data[1]));
#endif 
		break;
	case DEV_FC41D_HTTP_CUSTOM_MQTT_INFO:
		DevFc41dHttpMqttInfoInit((void *)fc41d);
		break;
	}

	fc41d->priv_data->http.custom.ali_oss.pkg_size = NOVA_UPGRADE_COLLECTOR_PKG_MAX_SIZE;
	fc41d->priv_data->http.custom.ali_oss.size = atoi(aliyun_data[1]);
	snprintf(fc41d->priv_data->http.url, FC41D_HTTP_URL_LEN, "%s", aliyun_data[2]);
	snprintf(fc41d->priv_data->http.custom.ali_oss.date, 
			 FC41D_HTTP_ALIYUN_OSS_DATE_LEN, "%s", aliyun_data[3]);
	snprintf(fc41d->priv_data->http.custom.ali_oss.auth, 
			 FC41D_HTTP_ALIYUN_OSS_AUTH_LEN, "%s", aliyun_data[4]);
	fc41d->http_req(fc41d, uniform, PROTO_UNIFORM_W_DEV_UPGRADE, 0, (const uint8_t *)"1", 1);

	return 0;
}

static int32_t _DevFc41dNovaRequestWriteDevUpgradePcs(DevFc41d_t *fc41d, 
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

	body->tlv.err = 0;
	body->tlv.data_len = 0;
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
						(const uint8_t *)&body->tlv, 
						sizeof(UniformProtocolBodyHead_t));

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	uint8_t mqtt_info[128] = {0};
	int32_t mqtt_len = 0;
	char *aliyun_data[16] = {NULL};
	uint16_t modelid[3] = {0};

	modelid[0] = *(uint16_t *)data;
	modelid[1] = gCollector->pcs_modelid;
	modelid[2] = gCollector->pcs_modelid;

	if (modelid[0] != modelid[1])
	{
		snprintf((char *)mqtt_info, sizeof(mqtt_info), 
				 "dev_mode_id:%02X, upgrade_pkg_mode_id:%02X",
				 modelid[1], modelid[0]);
		fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_W_DEV_UPGRADE, 1, 
						(const uint8_t *)mqtt_info, strlen((const char *)mqtt_info));
        return 0;
	}

	aliyun_data[0] = (char *)&data[2];

	for (int32_t i = 1; i < 16; ++i)
	{
		aliyun_data[i] = strstr(aliyun_data[i-1], "\n");
		if (aliyun_data[i] == NULL)
		{
			break;
		}
		*aliyun_data[i] = 0;
		--aliyun_data[i];
		if (*aliyun_data[i] == '\r')
		{
			*aliyun_data[i] = 0;
		}
		++aliyun_data[i];
		++aliyun_data[i];
	}
	char *p = (char *)&data[len];
	*p = 0x00;

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
		case DEV_FC41D_HTTP_CUSTOM_MQTT_INFO:
			DevFc41dHttpMqttInfoUninit((void *)fc41d);
			break;
		}
	}

	fc41d->priv_data->http.custom_type = type;
	// malloc new
	switch (fc41d->priv_data->http.custom_type)
	{
	case DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS:
#ifdef FC41D_HTTP_ALIYUN_OSS
		DevFc41dAliyunInit((void *)fc41d, modelid[2], atoi(aliyun_data[1]));
#endif 
		break;
	case DEV_FC41D_HTTP_CUSTOM_MQTT_INFO:
		DevFc41dHttpMqttInfoInit((void *)fc41d);
		break;
	}

	fc41d->priv_data->http.custom.ali_oss.pkg_size = NOVA_UPGRADE_COLLECTOR_PKG_MAX_SIZE;
	fc41d->priv_data->http.custom.ali_oss.size = atoi(aliyun_data[1]);
	snprintf(fc41d->priv_data->http.url, FC41D_HTTP_URL_LEN, "%s", aliyun_data[2]);
	snprintf(fc41d->priv_data->http.custom.ali_oss.date, 
			 FC41D_HTTP_ALIYUN_OSS_DATE_LEN, "%s", aliyun_data[3]);
	snprintf(fc41d->priv_data->http.custom.ali_oss.auth, 
			 FC41D_HTTP_ALIYUN_OSS_AUTH_LEN, "%s", aliyun_data[4]);
	fc41d->http_req(fc41d, uniform, PROTO_UNIFORM_W_DEV_UPGRADE_PCS, 0, (const uint8_t *)"1", 1);

	return 0;
}

static int32_t _DevFc41dUpdateCfgCheck(DevFc41d_t *fc41d)
{
	int32_t update_spiflash = 0;
	char ble_name[21] = {0};
	char wifi_ssid[32] = {0};
	char wifi_pwd[64] = {0};
	char mqtt_server_ip[128] = {0};
	char mqtt_server_port[6] = {0};

	int8_t mqtt_server_type = DevFc41dGetMqttServerType(fc41d);

	DevFc41dGetBleName(fc41d, (uint8_t *)ble_name, sizeof(ble_name));
	DevFc41dGetWifiSsid(fc41d, (uint8_t *)wifi_ssid, sizeof(wifi_ssid));
	DevFc41dGetWifiPwd(fc41d, (uint8_t *)wifi_pwd, sizeof(wifi_pwd));

	if (memcmp(gCollector->name, ble_name, sizeof(ble_name)))
	{
		update_spiflash += 1;
	}
    
	if (gCollector->mqtt.type != mqtt_server_type)
	{
		gCollector->mqtt.type = mqtt_server_type;
		update_spiflash += 2;
	}

	if (memcmp(gCollector->wifi.ssid, wifi_ssid, sizeof(wifi_ssid)) != 0
		|| memcmp(gCollector->wifi.pwd, wifi_pwd, sizeof(wifi_pwd)) != 0)
	{
		memcpy(gCollector->wifi.ssid, wifi_ssid, sizeof(wifi_ssid));
		memcpy(gCollector->wifi.pwd, wifi_pwd, sizeof(wifi_pwd));
		update_spiflash += 4;
	}

	if (gCollector->head.change)
	{
		update_spiflash += 8;
		gCollector->head.change = 0;
	}

	return update_spiflash;
}

static int32_t _DevFc41dNovaUpgrade(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->upgrade.flag;
}

int32_t _DevFc41dNovaRequest(DevFc41d_t *fc41d, 
						int8_t type,
						int32_t upload_type,
						UniformProtocolHead_t *uniform,
						const uint8_t *data, int32_t len)
{
	if (uniform != NULL)
	{
		if (_DevFc41dNovaUpgrade(fc41d) )
		{
			if (uniform->body.tlv.cmd != PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START
			&& uniform->body.tlv.cmd != PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE
			&& uniform->body.tlv.cmd != PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END)
			{
				return -2;
			}
		}
		uniform->frame.packet_num = 0;
		if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_TEST_MQTT_SUB)
		{
			return _DevFc41dNovaRequestReadTestMqttSub(fc41d, NULL, NULL, NULL);
		}

		if (uniform->body.tlv.cmd != PROTO_UNIFORM_W_KSTAR_SET_SYS)
		{
			int32_t xx = 0;
			if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEV_PRODUCT)
			{
				xx = 0;
			}
		}
	}

	// check tx using 
	if (type != -1 && fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		if (fc41d->priv_data->upgrade.flag > COLLECTOR_UPGRADE_NONE)
		{
			return -1;
		}

		if (uniform != NULL)
		{
			rbuffer_write_bytes(&fc41d->priv_data->rb_busy.ctx, (const uint8_t *)uniform, 
								22+uniform->frame_Length);
		}
		return -1;
	}

	//_DevFc41dTaskClean(fc41d);
	// 检测当前是否有在执行的任务，若有任务，则追加到任务队列中进行等待
	int32_t rb_data_len = rbuffer_length(&fc41d->priv_data->nova_cmd.rb_data.ctx);
	int32_t task_size = fc41d->priv_data->task_size;
	if (task_size > 0)
	{
		rbuffer_write_bytes(&fc41d->priv_data->rb_busy.ctx, data, len);
		return -1;
	}

	rbuffer_clear(&fc41d->priv_data->nova_cmd.rb_data.ctx);
	_DevFc41dTaskClean(fc41d);

	DevFc41dWriteCtx_t write_ctx = {0};
	DevFc41dWriteCtx_t *write_ctx_ptr = NULL;

	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)uniform, sizeof(UniformProtocolHead_t)
						- sizeof(UniformProtocolBodyHead_t));
    uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	write_ctx_ptr = (DevFc41dWriteCtx_t *)ptr;

	write_ctx_ptr->upload_type = upload_type;

	// parse TLV
	int32_t total_body_len = 0;
	if (type == -2)
	{
		// debug
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)&uniform->body.tlv, 
							sizeof(UniformProtocolBodyHead_t)+uniform->body.tlv.data_len);
	}
	else if (type == -3)
	{
		// debug
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)&uniform->body.tlv, 
							sizeof(UniformProtocolBodyHead_t)+uniform->body.tlv.data_len);
	}
	else
	{
		int32_t parse_len = uniform->frame_Length - sizeof(UniformProtocolFrame_t);
		int32_t body_len = 0, body_count = 0, body_data_len = 0;
		UniformProtocolBodyHead_t *body = &uniform->body;
		uint8_t *parse_body_ptr = (uint8_t *)body;
		while (parse_len > 0) 
		{
			body = (UniformProtocolBodyHead_t *)parse_body_ptr;
			body_len = sizeof(UniformProtocolBodyHead_t) + body->tlv.data_len;
			total_body_len += body_len;
			parse_body_ptr += body_len;
			parse_len -= body_len;
			if (parse_len < 0)
			{
				break;
			}

			++body_count;
			body_data_len = body->tlv.data_len;

#if 0
			body->tlv.data_len = 0;

			// 在发送之前拼TL要不然无法和V进行连续拼接
			rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
								(const uint8_t *)&body->tlv, sizeof(UniformProtocolBodyHead_t));
			body->tlv.data_len = body_data_len;
#endif 

			// 添加NovaCmd任务
			rbuffer_write_bytes(&fc41d->priv_data->nova_cmd.rb_data.ctx, 
								(const uint8_t *)body, body_len);
		}

		if (body_count > 1)
		{
			write_ctx_ptr->uniform_type = 1;	// 不做第一包TLV 补全总长度
		}

		UniformProtocolHead_t *p_uniform = (UniformProtocolHead_t *)&(write_ctx_ptr->data);
		write_ctx_ptr->uniform_body = (uint8_t *)&p_uniform->body;
	}

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
	//fc41d->task_push_(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
							FC41D_PROCESS_NOVA_CMD_INIT, 
							DEV_FC41D_TASK_STAT_WRITE, 0);

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NOVA;
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	//fc41d->priv_data->write_stat.employ_time = fc41d->priv_data->write_stat.timeout + 10000;

	return total_body_len;
}

int32_t _DevFc41dNovaCmd(DevFc41d_t *fc41d, 
						int32_t upload_type,
						UniformProtocolHead_t *uniform,
						UniformProtocolBodyHead_t *body)
{
	int32_t ret = -1;

	if (body == NULL)
	{
		return ret;
	}

	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->priv_data->write_stat.employ_time = fc41d->priv_data->write_stat.timeout + 10000;
	int32_t cmd = (body->tlv.cmd);
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform1 = (UniformProtocolHead_t *)(write_ctx->data);

	switch (cmd)
	{
	case PROTO_UNIFORM_R_TEST_MQTT_SUB:
		{
			ret = _DevFc41dNovaRequestReadTestMqttSub(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEBUG_WRITE_MONITOR_SN:
		{
			ret = _DevFc41dNovaRequestWriteDebugWriteMonitorSn(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEBUG_MONITOR_AUTH:
		{
			ret = _DevFc41dNovaRequestDebugMonitorAuth(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEBUG_CLS_MONITOR_TOTAL_FLASH:
		{
			ret = _DevFc41dNovaRequestDebugClsMonitorTotalFlash(fc41d, write_ctx, uniform, body);
		}
		break;
	case PROTO_UNIFORM_W_DEBUG_OTA_FC41D:
		{
			ret = _DevFc41dNovaRequestDebugOtaFc41d(fc41d, write_ctx, uniform, body);
		}
		break;
	case PROTO_UNIFORM_W_DEBUG_GET_MONITOR_HARDFAULT:
		{
			ret = _DevFc41dNovaRequestDebugGetMonitorHardfault(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_DEBUG:
	case PROTO_UNIFORM_R_DEBUG_GET_WIFI_INFO:
	case PROTO_UNIFORM_R_DEBUG_GET_MQTT_INFO:
		{
			ret = _DevFc41dNovaRequestReadDebug(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEBUG_MODULE:
		{
			ret = _DevFc41dNovaRequestWriteDebugSetPcsUpgrade(fc41d, write_ctx, uniform,
							(const uint8_t *)body->tlv.data, (body->tlv.data_len));
			break;
		}
	case PROTO_UNIFORM_R_DEBUG_AES:
		{
//			ret = _DevFc41dNovaRequestReadDebugAes(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_TEST_UPGRADE_FILE:
		{
			ret = _DevFc41dNovaRequestReadDebugGetUpgradeFile(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_TEST_UPGRADE_MD5:
		{
			ret = _DevFc41dNovaRequestReadDebugGetUpgradeMd5(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_WIFI_SCAN:
		{
			ret = _DevFc41dNovaReadWifiScan(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_WIFI_INFO:
		{
			ret = _DevFc41dNovaReadWifiInfo(fc41d, write_ctx, uniform, body);

			break;
		}
	case PROTO_UNIFORM_W_WIFI_INFO:
		{
			ret = _DevFc41dNovaWriteWifiInfo(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_DEV_INFO:
		{
			ret = _DevFc41dNovaReadDevInfo(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEV_INFO:
		{
			ret = _DevFc41dNovaWriteDevInfo(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_DEV_MQTT_INFO:
		{
			ret = _DevFc41dNovaReadMqttInfo(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEV_MQTT_INFO:
		{
			ret = _DevFc41dNovaWriteMqttInfo(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_DEV_PRODUCT:
		{
			ret = _DevFc41dNovaRequestReadDevProduct(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_DEV_NAME:
		{
			ret = _DevFc41dNovaReadDevName(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEV_NAME:
		{
			ret = _DevFc41dNovaWriteDevName(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_R_DEV_PROPERTY:
		{
			ret = _DevFc41dNovaReadDevProperty(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEV_PROPERTY:
		{
			ret = _DevFc41dNovaWriteDevProperty(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_KSTAR_REGISTER:
		{
			ret = _DevFc41dNovaRequestWriteKstarRegister(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_KSTAR_SET_SYS:
		{
			ret = _DevFc41dNovaRequestWriteKstarSetSys(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_KSTAR_OPT_REGISTER:
		{
			ret = _DevFc41dNovaRequestWriteKstarOptRegister(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_MEGAREVO_SYS_INFO:
		{
			ret = _DevFc41dNovaRequestWriteMegarevoOptRegister(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_SURO_SYS_INFO:
		{
			ret = _DevFc41dNovaRequestWriteSuroOptRegister(fc41d, write_ctx, uniform, body);
			break;
		}
#if 1
	case PROTO_UNIFORM_R_NOVA_SYS_INFO:
		{
			ret = _DevFc41dNovaRequestReadNovaOptRegister(fc41d, write_ctx,  uniform, body);
			break;
		}
#endif 
	case PROTO_UNIFORM_W_NOVA_SYS_INFO:
		{
			ret = _DevFc41dNovaRequestWriteNovaOptRegister(fc41d, write_ctx,  uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEV_UPGRADE:
		{
#if 0
			ret = _DevFc41dNovaRequestWriteDevUpgradePcs(fc41d, write_ctx, uniform, body);

#else 
			ret = _DevFc41dNovaRequestWriteDevUpgrade(fc41d, write_ctx, uniform, body);
#endif 
			break;
		}
	case PROTO_UNIFORM_W_DEV_UPGRADE_PCS:
		{
			ret = _DevFc41dNovaRequestWriteDevUpgradePcs(fc41d, write_ctx, uniform, body);
			break;
		}
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START:
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE:
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END:
		{
			ret = _DevFc41dProcessNovaRequest_w_dev_local_upgrade(fc41d, cmd, write_ctx, uniform, body);
			break;
		}
	default:
		ret = -1;
		break;
	}

	int32_t update_flag = _DevFc41dUpdateCfgCheck(fc41d);
	if (update_flag)
	{
		CollectorSpiFlashWriteCfg();

		if (update_flag & 0x02)
		{
			fc41d->priv_data->mqtt.conn_stat = 0;
		}
	}

	return ret;
}

int32_t _DevFc41dUrcNova(DevFc41d_t *fc41d, int8_t type, const char *data, int32_t len)
{
	char *tmp = (char *)data;

	if (len < 4) 
	{
		// not enough
		return 0;
	}

	// check mqtt test pub
	if (_DevFc41dMqttSendingStatus(fc41d) || _DevFc41dBleSendingStatus(fc41d))
	{
		if (strncmp(data, "NOVA", strlen("NOVA")) != 0)
		{
			return 0;
		}
	}

	UniformProtocolHead_t *p_uniform = NULL;
	int32_t read_len = 0;
	int32_t read_pkg_len = 0;
	int32_t read_ret = 0;
	char *tmp_next = tmp;
	char *tmp_ptr = NULL;
	char complete[1024] = {0};
	int32_t complete_len = 0;
	int32_t cp_data_len = 0;
	int32_t tmp_frame_len = 0;


	// 过滤掉开头的 \r\n 字符串
	while (*tmp_next == 0x0D || *tmp_next == 0x0A)
	{
		tmp_next++;
		read_len++;
	}
	tmp_ptr = NULL;

	do
	{
		read_ret = proto_decode_uniform((const uint8_t *)tmp_next, len-read_len, &p_uniform, 
										&read_pkg_len);
		if (read_ret) 
		{
			// data parse errr
			return read_len;
		}

		if (p_uniform->body.tlv.cmd != PROTO_UNIFORM_R_TEST_MQTT_SUB)
		{
			int32_t xx = 0;
			xx = 1;
		}

		if (read_pkg_len > len-read_len)
		{
			/*not enough*/
			return 0;
		}
        read_len += read_pkg_len;
		if (p_uniform->frame.packet_index == 0)
		{
			tmp_ptr = (char *)p_uniform;
			cp_data_len = sizeof(UniformProtocolHead_t) - sizeof(UniformProtocolFrame_t)
				- sizeof(UniformProtocolBodyHead_t) + p_uniform->frame_Length;
			tmp_frame_len += p_uniform->frame_Length;
		}
		else 
		{
			tmp_ptr = (char *)(&p_uniform->body);
			cp_data_len = p_uniform->frame_Length - sizeof(UniformProtocolFrame_t);
			tmp_frame_len += cp_data_len;
		}

		memcpy(complete+complete_len, (char *)tmp_ptr, cp_data_len);
		complete_len += cp_data_len;
        
        if (p_uniform->frame.packet_index + 1 >= p_uniform->frame.packet_num)
		{
			p_uniform = (UniformProtocolHead_t *)complete;
			p_uniform->frame_Length = tmp_frame_len;
			break;
		}

		tmp_next += read_pkg_len;
	} while(len-read_len > 0 && complete_len < sizeof(complete));

	if (complete_len > sizeof(complete))
	{
		// out of buffer
		return read_len;
	}

	int32_t upload_type = 0;
	if (p_uniform->src == 0x02)
	{
		upload_type = 1;
	}
	if (p_uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_UPGRADE)
	{
		int32_t xx = 0;
	}
	int32_t body_len = _DevFc41dNovaRequest(fc41d, type, upload_type, p_uniform, 
											(const uint8_t *)data, len);

	return read_len;
}

int32_t _DevFc41dReadNovaCmd(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dNovaCmdStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_NOVA_CMD_NONE:
		break;
	case FC41D_PROCESS_NOVA_CMD_INIT:
		fc41d->task_head_del(fc41d);
		if (fc41d->priv_data->task_size <= 0)
		{
			fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		}
		break;
	case FC41D_PROCESS_RELAY_MULTI_CMD_KSTAR:
		return _DevFc41dReadRelayMultiCmdKstar(fc41d, data, len);
		break;
	}

	return len;
}

int32_t _DevFc41dWriteNovaCmd(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t buf[512] = {0};
	int32_t len[2] = {0};
	uint8_t *ptr = NULL;
	DevFc41dWriteCtx_t *write_ctx = NULL;
	UniformProtocolBodyHead_t *body = NULL;
	UniformProtocolBodyHead_t *write_body = NULL;
	UniformProtocolHead_t *uniform = NULL;
	int32_t upload_type = -1;
	int32_t body_len = 0;
	int8_t flag = 0, fin = 0;
    
    ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	write_ctx = (DevFc41dWriteCtx_t *)ptr;
	upload_type = write_ctx->upload_type;
	uniform = (UniformProtocolHead_t *)(write_ctx->data);

	do 
	{
        write_body = (UniformProtocolBodyHead_t *)write_ctx->uniform_body;
		// pop one cmd
		body = NULL;
		len[0] = rbuffer_length(&fc41d->priv_data->nova_cmd.rb_data.ctx);
		if (len[0] > 0)
		{
			len[1] = rbuffer_get_bytes(&fc41d->priv_data->nova_cmd.rb_data.ctx, 
									   (uint8_t *)buf, sizeof(buf));
			if (len[1] < sizeof(UniformProtocolBodyHead_t)) 
			{
				break;
			}
			body =  (UniformProtocolBodyHead_t *)buf;
		}
		else
		{
			body = &uniform->body;
			fin = 1;
		}

		body_len = sizeof(UniformProtocolBodyHead_t) + body->tlv.data_len;
		if (len[0] - body_len <= 0)
		{
			fin = 2;
		}
		write_ctx->finish = fin;

		flag = _DevFc41dNovaCmd(fc41d, upload_type, uniform, body);
		if (flag == -1)
		{
			if (len[0] == 0)
			{
				break;
			}
		}
		if (flag != 0)
		{
			if (write_body)
			{
				write_body->tlv.err = 1;
			}
		}

		if (write_ctx->uniform_body != NULL)
		{
			if (write_body)
			{
				write_ctx->uniform_body += write_body->tlv.data_len 
					+ sizeof(UniformProtocolBodyHead_t);
			}
			// trim_body
			rbuffer_trim_idx(&fc41d->priv_data->nova_cmd.rb_data.ctx, 0, body_len);
		}
		else 
		{
			rbuffer_trim_idx(&fc41d->priv_data->nova_cmd.rb_data.ctx, 0, len[0]);
		}

		if (len[0] - body_len > 0)
		{
			if (flag == 0)
			{
				fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_NOVA_CMD, 
										FC41D_PROCESS_NOVA_CMD_INIT, 
										DEV_FC41D_TASK_STAT_WRITE, 0);
			}
		}

	}while(flag);

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	return 0;
}

