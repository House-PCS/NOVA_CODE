#include "dev_fc41d_common.h"

#define MQTT_USING_NORMAL_AT_PUB		0
#define USE_STACK_OPEN					1

// AT MQTT
#define AT_FC41D_MQTT_MAX_TOPIC_SIZE	10
#define AT_FC41D_MQTT_CLIENT_ID			1
#define AT_FC41D_MQTT_VERSION			"3"
#define AT_FC41D_MQTT_DATATYPE			0
#define AT_FC41D_MQTT_SSL				0
#define AT_FC41D_MQTT_KEEPALIVE			60
#define AT_FC41D_MQTT_SESSION			1
#define AT_FC41D_MQTT_DELIVERY_TM		5
#define AT_FC41D_MQTT_DELIVERY_CNT		3
#define AT_FC41D_MQTT_TIMEOUT_REPORT	0
#define AT_FC41D_MQTT_IP				"172.16.22.3"
#define AT_FC41D_MQTT_PORT				"1883"
#define AT_FC41D_MQTT_USR				"test"
#define AT_FC41D_MQTT_PWD				"test"
#define AT_FC41D_MQTT_SET_CFG_VER		"AT+QMTCFG=\"version\",%d,%s\r\n"
#define AT_FC41D_MQTT_SET_CFG_DATATYPE	"AT+QMTCFG=\"datatype\",%d,%d\r\n"
#define AT_FC41D_MQTT_SET_CFG_SSL		"AT+QMTCFG=\"ssl\",%d,%d\r\n"
#define AT_FC41D_MQTT_SET_CFG_KEEPALIVE	"AT+QMTCFG=\"keepalive\",%d,%d\r\n"
#define AT_FC41D_MQTT_SET_CFG_TIMEOUT	"AT+QMTCFG=\"timeout\",%d,,,%d\r\n"
#define AT_FC41D_MQTT_SET_CFG_SESSION	"AT+QMTCFG=\"session\",%d,%d\r\n"
#define AT_FC41D_MQTT_SET_CFG_WILL		"AT+QMTCFG=\"will\",%d,1,0\r\n"
#define AT_FC41D_MQTT_CLOSE				"AT+QMTCLOSE=%d\r\n"
#define AT_FC41D_MQTT_CLOSE_RET			"OK\r\n\r\n+QMTCLOSE:"
#define AT_FC41D_MQTT_STAT				"AT+QMTCONN?\r\n"

#if USE_STACK_OPEN
#define AT_FC41D_MQTT_OPEN				"AT+QMTOPEN=%d,\"%s\",%s\r\n"
#else
#define AT_FC41D_MQTT_OPEN				"AT+QMTOPEN=1,\""
#endif 

#define AT_FC41D_MQTT_OPEN_RET			"+QMTOPEN:"
#define AT_FC41D_MQTT_CONN				"AT+QMTCONN=%d,\"%s\",\"%s\",\"%s\"\r\n"
#define AT_FC41D_MQTT_CONN_RET			"+QMTCONN:"
#define AT_FC41D_MQTT_DISCONN			"AT+QMTDISC=%d\r\n"
#define AT_FC41D_MQTT_STAT_RET			"+QMTCONN:"

#define AT_FC41D_MQTT_SUB_ONCE_MAX		5
#define AT_FC41D_MQTT_SUB				"AT+QMTSUB=%d,%d,\"%s%s\", %d\r\n"
#define AT_FC41D_MQTT_SUB_OK			"+QMTSUB:"
#define AT_FC41D_MQTT_PUB				"AT+QMTPUB=%d,%d,%d,%d,\"%s%s\",%d,\"%s\"\r\n"
#define AT_FC41D_MQTT_PUB_RAW			"AT+QMTPUBRAW=%d,%d,%d,%d,\"%s%s\",%d\r\n"
#define AT_FC41D_MQTT_PUB_RET			">\r\n"
#define AT_FC41D_MQTT_PUB_OK			"OK\r\n\r\n+QMTPUB:"

#define AT_FC41D_MQTT_URC_HEAD			"+QMT"
#define AT_FC41D_MQTT_URC_STAT_ERR		"STAT:"
#define AT_FC41D_MQTT_URC_STAT_OPEN		"OPEN:"
#define AT_FC41D_MQTT_URC_MQTT_RECV		"RECV:"
#define AT_FC41D_MQTT_URC_MQTT_SUB		"SUB:"
#define AT_FC41D_MQTT_URC_MQTT_PUB		"+QMTPUB:"

extern int32_t _DevFc41dUsartInit(DevFc41d_t *fc41d);
extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);
extern int32_t _DevFc41dUrcNovaDebug(DevFc41d_t *fc41d, const char *data, int32_t len);
extern int32_t _DevFc41dUrcNova(DevFc41d_t *fc41d, int8_t type, const char *data, int32_t len);
extern int32_t _DevFc41dHttpSubCmdGet(DevFc41d_t *fc41d);


static int32_t _gMqttSendLen[3] = {0};

static int32_t _DevFc41dMqttStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_MQTT_CLOSE
			:fc41d->priv_data->mqtt.status);
}

int32_t _DevFc41dMqttSendingStatus(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->mqtt.send_flag;
}

int32_t _DevFc41dMqttStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->mqtt.status = status;
	return 0;
}

static int32_t _DevFc41dReadMqttCfgComplete(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);

	fc41d->task_head_del(fc41d);
	return 0;
}

static int32_t _DevFc41dWriteMqttCfgComplete(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	return 0;
}

static int32_t _DevFc41dNextTasks(DevFc41d_t *fc41d, int32_t cmd)
{
	switch (cmd)
	{
	case PROTO_UNIFORM_W_DEV_UPGRADE:
	case PROTO_UNIFORM_W_DEV_UPGRADE_PCS:
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_DEV_UPGRADE_START,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case PROTO_UNIFORM_R_DEV_UPGRADE_STAT:
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_DEV_UPGRADE_DOWN,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	}

	return 0;
}

static int32_t _DevFc41dReadMqttPubLen(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len < 2) 
	{
		return 0;
	}

	if (_gMqttSendLen[0] < _gMqttSendLen[1])
	{
		++_gMqttSendLen[2];
	}

	if (_gMqttSendLen[2] > 10)
	{
		fc41d->priv_data->mqtt.conn_stat = 0;
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, FC41D_PROCESS_MQTT_CLOSE,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		return len;
	}

	int32_t i = 0, j = 0;
	for (; i < len; ++i)
	{
		if (data[i] == '>')
		{
			j = 1;
			break;
		}
	}
	if (j == 0)
	{
		// 数据异常，丢弃
		fc41d->task_head_del(fc41d);
		return i-1;
	}
	_gMqttSendLen[0] = _gMqttSendLen[1];
	_gMqttSendLen[2] = 0;

	fc41d->task_head_del(fc41d);

	fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
							FC41D_PROCESS_MQTT_PUB_SEND_DATA,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return i-1;
}

static int32_t _DevFc41dWriteMqttPubLen(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[FC41D_MQTT_MTU+sizeof(DevFc41dWriteCtx_t)] = {0};
	int32_t cmd_len = rbuffer_get_bytes(&fc41d->priv_data->rb_write.ctx, 
						(uint8_t *)cmd, sizeof(cmd)) - sizeof(DevFc41dWriteCtx_t);
	int32_t send_len = cmd_len;

	if (cmd_len < 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	if (send_len > fc41d->priv_data->mqtt.mtu) 
	{
		send_len = fc41d->priv_data->mqtt.mtu;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	int32_t pub_idx = write_ctx->mqtt_pub_idx;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
    
    if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_NAME)
	{
        int32_t xx = 0;
    }
	if (write_ctx->to_self)
	{
		int32_t xx = 0;
	}

	if (uniform->frame.packet_num == 0)
	{
		// debug
		if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEV_UPGRADE_STAT)
		{
			int32_t xx = 0;
		}
		// debug end

		memset(trim_len, 0, sizeof(int32_t));
		uint16_t pkg_num = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
			- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
		uint16_t tmp = fc41d->priv_data->mqtt.mtu - sizeof(UniformProtocolHead_t);
		uniform->frame.packet_num = (pkg_num/tmp) + (pkg_num%tmp>0?1:0);

		if (uniform->frame.packet_num == 0) 
		{
			uniform->frame.packet_num = 1;
		}

		uniform->frame.packet_index = -1;
	}
	if (*trim_len == 0)
	{
		write_ctx->try_times = 0;
		++uniform->frame.packet_index;
	}
	++write_ctx->try_times;
	if (write_ctx->try_times > 4)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->mqtt.send_flag = 0;
		fc41d->task_head_del(fc41d);
		return 0;
	}

	if (uniform->frame.packet_index >= uniform->frame.packet_num)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->mqtt.send_flag = 0;
		fc41d->task_head_del(fc41d);

		_DevFc41dNextTasks(fc41d, write_ctx->next_cmd);

		return 0;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->priv_data->mqtt.send_flag = 1;

	if (write_ctx->debug)
	{
		send_len -= sizeof(UniformProtocolHead_t);
		// 由于debug模式多添加了一个TLV头需要去掉
		send_len -= sizeof(UniformProtocolTlvBodyHead_t);
	}
	write_ctx->send_len = send_len;

	char topic[128] = {0};
	uint16_t modelid = gCollector->pcs_modelid;
	snprintf(topic, sizeof(topic), (const char *)gCollectorMqttTopicPub[pub_idx].uri, 
			 modelid);
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_PUB_RAW, 
			 AT_FC41D_MQTT_CLIENT_ID,
			 pub_idx+1, // msgid
			 gCollectorMqttTopicPub[pub_idx].qos, // qos
			 0, // retain
			 topic,
			 (const char *)gCollector->pcs_sn,
			 //(const char *)gCollector->sn,
			 send_len);

	if (write_ctx->to_self)
	{
		char topic[128] = {0};
		uint16_t modelid = gCollector->pcs_modelid;
		snprintf(topic, sizeof(topic), (const char *)gCollectorMqttTopicSub[pub_idx].uri, 
				 modelid);
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_PUB_RAW, 
				 AT_FC41D_MQTT_CLIENT_ID,
				 pub_idx+1, // msgid
				 1, // qos
				 0, // retain
				 topic,
				 (const char *)gCollector->pcs_sn,
				 //(const char *)gCollector->sn,
				 send_len);
	}

	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	++_gMqttSendLen[1];

	return 0;
}

static int32_t _DevFc41dReadMqttPubData(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = NULL;

	find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		char *tmp_find = strstr(find + strlen(AT_FC41D_MQTT_PUB_OK), "\r\n");
		if (tmp_find)
		{
			uint8_t tmp_end = tmp_find[1];
			tmp_find[1] = 0;
			read = strlen((const char *)data) + 1;
			tmp_find[1] = tmp_end;
		}
		else
		{
			find[strlen(AT_OK)-1] = 0;
			read = strlen((const char *)data)+1;
		}

		// trim old data
		uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		int32_t ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
		memcpy(&ptr_len, trim_len, sizeof(int32_t));
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);

		if (uniform->frame.packet_index == 0)
		{
			rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
							 sizeof(DevFc41dWriteCtx_t)
							 + sizeof(UniformProtocolHead_t)
							 - sizeof(UniformProtocolBodyHead_t),
							 ptr_len);
		}
		else
		{
			rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
							 sizeof(DevFc41dWriteCtx_t)
							 + sizeof(UniformProtocolHead_t)
							 - sizeof(UniformProtocolBodyHead_t),
							 ptr_len);
		}

		ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		write_ctx = (DevFc41dWriteCtx_t *)ptr;
        uniform = (UniformProtocolHead_t *)(write_ctx->data);
		trim_len = (uint8_t *)(&write_ctx->trim_send_len);
        memset(trim_len, 0, sizeof(int32_t));

		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		// check lose data
		int32_t loass_data = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttPubData(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t cmd_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	if (cmd_len < 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	int32_t send_len = cmd_len - sizeof(DevFc41dWriteCtx_t);
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	int32_t pub_idx = write_ctx->mqtt_pub_idx;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);

	send_len = write_ctx->send_len;
	if (send_len > fc41d->priv_data->mqtt.mtu) 
	{
		send_len = fc41d->priv_data->mqtt.mtu;
	}

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
		- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
	int32_t _send = 0;
	if (uniform->frame.packet_index == 0)
	{
		_send = send_len-sizeof(UniformProtocolHead_t);
	}
	else
	{
		_send = send_len-sizeof(UniformProtocolHead_t)
			+sizeof(UniformProtocolBodyHead_t);
	}

	uint8_t *tmp_ptr = (uint8_t *)(write_ctx->data+sizeof(UniformProtocolHead_t));
	if (write_ctx->debug)
	{
		// 去掉TLV 头信息
		tmp_ptr += sizeof(UniformProtocolTlvBodyHead_t);
		// write->send_len 已经在SendLen中已经去掉了多余的长度了
		// send_len -= sizeof(UniformProtocolTlvBodyHead_t);
	}
	else
	{
		tmp_ptr = (uint8_t *)(write_ctx->data);
		send_len = proto_encode_uniform(write_ctx->req_type, 
										tmp_ptr,
										_send,
										uniform->frame.packet_num,
										uniform->frame.packet_index,
										write_ctx->uniform_type,
										body_total_len,
										uniform->body.tlv.cmd, uniform->body.tlv.err);
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->priv_data->mqtt.try_send_count = 0;
	fc41d->priv_data->mqtt.send_flag = 2;
	fc41d->write(fc41d, (const uint8_t *)(tmp_ptr), send_len);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
							FC41D_PROCESS_MQTT_PUB_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	if (write_ctx->to_self)
	{
		int32_t xx = 0;
	}
	if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_NAME)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_INIT, 0,
								DEV_FC41D_TASK_STAT_WRITE, 0);
	}

	// debug for ble
	if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_KSTAR_REGISTER)
	{
		int32_t xx = 0;
	}
	else if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEV_NAME)
	{
		int32_t xx = 0;
	}
	// debug for ble end 

	if (uniform->frame.packet_index == 0)
	{
		if (uniform->frame.packet_num > 1)
		{
			ptr_len = send_len-sizeof(UniformProtocolHead_t) 
				+ sizeof(UniformProtocolBodyHead_t);
		}
		else 
		{
			ptr_len = 0;
		}
		memcpy(trim_len, &ptr_len, sizeof(int32_t));

	}
	else
	{
		ptr_len = send_len-sizeof(UniformProtocolHead_t) 
			+ sizeof(UniformProtocolBodyHead_t);
		memcpy(trim_len, &ptr_len, sizeof(int32_t));
	}

	return 0;
}

static int32_t _DevFc41dReadMqttPubNormalData(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = NULL;

	find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		char *tmp_find = strstr(find + strlen(AT_FC41D_MQTT_PUB_OK), "\r\n");
		if (tmp_find)
		{
			uint8_t tmp_end = tmp_find[1];
			tmp_find[1] = 0;
			read = strlen((const char *)data) + 1;
			tmp_find[1] = tmp_end;
		}
		else
		{
			find[strlen(AT_OK)-1] = 0;
			read = strlen((const char *)data)+1;
		}

		// trim old data
		uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		int32_t ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
		memcpy(&ptr_len, trim_len, sizeof(int32_t));
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);

		if (uniform->frame.packet_index == 0)
		{
			rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
							 sizeof(DevFc41dWriteCtx_t)
							 + sizeof(UniformProtocolHead_t)
							 - sizeof(UniformProtocolBodyHead_t),
							 ptr_len);
		}
		else
		{
			rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
							 sizeof(DevFc41dWriteCtx_t)
							 + sizeof(UniformProtocolHead_t)
							 - sizeof(UniformProtocolBodyHead_t),
							 ptr_len);
		}

		ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		write_ctx = (DevFc41dWriteCtx_t *)ptr;
        uniform = (UniformProtocolHead_t *)(write_ctx->data);
		trim_len = (uint8_t *)(&write_ctx->trim_send_len);
        memset(trim_len, 0, sizeof(int32_t));

		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		// check lose data
		int32_t loass_data = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttPubNormalData(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[FC41D_MQTT_MTU+sizeof(DevFc41dWriteCtx_t)] = {0};
	int32_t cmd_len = rbuffer_get_bytes(&fc41d->priv_data->rb_write.ctx, 
						(uint8_t *)cmd, sizeof(cmd)) - sizeof(DevFc41dWriteCtx_t);
	int32_t send_len = cmd_len;

	if (cmd_len < 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	if (send_len > fc41d->priv_data->mqtt.mtu) 
	{
		send_len = fc41d->priv_data->mqtt.mtu;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	int32_t pub_idx = write_ctx->mqtt_pub_idx;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
    
    if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_NAME)
	{
        int32_t xx = 0;
    }
	if (write_ctx->to_self)
	{
		int32_t xx = 0;
	}

	if (uniform->frame.packet_num == 0)
	{
		// debug
		if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEV_UPGRADE_STAT)
		{
			int32_t xx = 0;
		}
		// debug end

		memset(trim_len, 0, sizeof(int32_t));
		uint16_t pkg_num = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
			- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
		uint16_t tmp = fc41d->priv_data->mqtt.mtu - sizeof(UniformProtocolHead_t);
		uniform->frame.packet_num = (pkg_num/tmp) + (pkg_num%tmp>0?1:0);

		if (uniform->frame.packet_num == 0) 
		{
			uniform->frame.packet_num = 1;
		}

		uniform->frame.packet_index = -1;
	}
	if (*trim_len == 0)
	{
		write_ctx->try_times = 0;
		++uniform->frame.packet_index;
	}
	++write_ctx->try_times;
	if (write_ctx->try_times > 4)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->mqtt.send_flag = 0;
		fc41d->task_head_del(fc41d);
		return 0;
	}

	if (uniform->frame.packet_index >= uniform->frame.packet_num)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->mqtt.send_flag = 0;
		fc41d->task_head_del(fc41d);

		_DevFc41dNextTasks(fc41d, write_ctx->next_cmd);

		return 0;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->priv_data->mqtt.send_flag = 1;

	if (write_ctx->debug)
	{
		send_len -= sizeof(UniformProtocolHead_t);
		// 由于debug模式多添加了一个TLV头需要去掉
		send_len -= sizeof(UniformProtocolTlvBodyHead_t);
	}
	write_ctx->send_len = send_len;

	char topic[128] = {0};
	char test_pub[32] = {0};
	snprintf(test_pub, sizeof(test_pub), "%04d", uniform->body.tlv.cmd);
	uint16_t modelid = gCollector->pcs_modelid;
	snprintf(topic, sizeof(topic), (const char *)gCollectorMqttTopicPub[pub_idx].uri, 
			 modelid);
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_PUB, 
			 AT_FC41D_MQTT_CLIENT_ID,
			 // pub_idx+1, // msgid
			 // gCollectorMqttTopicPub[pub_idx].qos, // qos
			 0,
			 0,
			 0, // retain
			 topic,
			 (const char *)gCollector->pcs_sn,
			 //(const char *)gCollector->sn,
			 strlen(test_pub), test_pub);

	if (write_ctx->to_self)
	{
		char topic[128] = {0};
		uint16_t modelid = gCollector->pcs_modelid;
		snprintf(topic, sizeof(topic), (const char *)gCollectorMqttTopicSub[pub_idx].uri, 
				 modelid);
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_PUB, 
				 AT_FC41D_MQTT_CLIENT_ID,
				 0, // msgid
				 0, // qos 至多发送一次
				 0, // retain
				 topic,
				 (const char *)gCollector->pcs_sn,
				 //(const char *)gCollector->sn,
				 strlen(test_pub), test_pub);
	}

	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	++_gMqttSendLen[1];

	return 0;
}

static int32_t _DevFc41dReadMqttClose(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find = strstr((const char *)data, AT_FC41D_MQTT_CLOSE_RET);
		if (find == NULL) 
		{
			// not enough
			return 0;
		}
		find[strlen(AT_FC41D_MQTT_CLOSE_RET)-1] = 0;
		read = strlen((const char *)data)+1;
        fc41d->priv_data->mqtt.conn_stat = 0;

		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->mqtt.conn_stat = 0;
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttClose(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_CLOSE, AT_FC41D_MQTT_CLIENT_ID);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadMqttStat(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->task_head_del(fc41d);

		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		find = strstr((const char *)data, AT_FC41D_MQTT_STAT_RET);
		if (find)
		{
			int32_t i = 0, j = 0;
			for (find += strlen(AT_FC41D_MQTT_STAT_RET); *find != 0; ++find)
			{
				if (*find == (AT_FC41D_MQTT_CLIENT_ID + 0x30)) 
				{
					++i;
					continue;
				}
				if (*find == ',')
				{
					continue;
				}

				if (i == 1 && *find > '0' && *find < '5')
				{
					j = (*find-0x30);
					if (j == 3)
					{
						if (fc41d->priv_data->mqtt.conn_stat == 0)
						{
							fc41d->priv_data->mqtt.conn_stat = 1;
						}
					}
					else
					{
						fc41d->priv_data->mqtt.conn_stat = 0;
					}
					break;
				}
			}
			return read;
		}

		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttStat(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_MQTT_STAT, strlen(AT_FC41D_MQTT_STAT));
	return 0;
}

static int32_t _DevFc41dReadMqttSetCfg(DevFc41d_t *fc41d, int32_t status, 
											  const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int8_t flag = 0;
	int32_t read = 0;
	char *find = NULL;

	do 
	{
		find = strstr((const char *)data, AT_OK);
		if (find) 
		{
			flag = 1;
			find[strlen(AT_OK)-1] = 0;
			read = strlen((const char *)data)+1;

			fc41d->task_head_del(fc41d);
			break;
		}

		find = strstr((const char *)data, AT_ERR);
		if (find) 
		{
			flag = 1;
			find[strlen(AT_ERR)-1] = 0;
			read = strlen((const char *)data)+1;

			fc41d->task_head_del(fc41d);
			break;
		}
	} while(0);

	if (!flag)
	{
		return read;
	}

	switch(status)
	{
	case FC41D_PROCESS_MQTT_SET_CFG_VER:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_SET_CFG_DATATYPE,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_DATATYPE:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_SET_CFG_SSL,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_SSL:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_SET_CFG_KEEPALIVE,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_KEEPALIVE:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_SET_CFG_TIMEOUT,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_TIMEOUT:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_SET_CFG_SESSION,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_SESSION:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								FC41D_PROCESS_MQTT_SET_CFG_WILL,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_WILL:
		break;
	default:break;
	}

	return read;
}

static int32_t _DevFc41dWriteMqttSetCfg(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	char cmd[128] = {0};
	switch (status)
	{
	case FC41D_PROCESS_MQTT_SET_CFG_VER:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_VER, 
				 AT_FC41D_MQTT_CLIENT_ID, AT_FC41D_MQTT_VERSION	// VERSION
				);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_DATATYPE:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_DATATYPE, 
				 AT_FC41D_MQTT_CLIENT_ID, AT_FC41D_MQTT_DATATYPE	// Datatype
				);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_SSL:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_SSL, 
				 AT_FC41D_MQTT_CLIENT_ID, AT_FC41D_MQTT_SSL		// SSL
				);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_KEEPALIVE:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_KEEPALIVE, 
				 AT_FC41D_MQTT_CLIENT_ID, AT_FC41D_MQTT_KEEPALIVE	// KEEPALIVE
				);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_TIMEOUT:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_TIMEOUT, 
				 AT_FC41D_MQTT_CLIENT_ID, AT_FC41D_MQTT_TIMEOUT_REPORT // TIMEOUT
				);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_SESSION:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_SESSION, 
				 AT_FC41D_MQTT_CLIENT_ID, AT_FC41D_MQTT_SESSION	// CLEAN_SESSION(断开移除Topic)
				);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_WILL:
		snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SET_CFG_WILL, 
				 AT_FC41D_MQTT_CLIENT_ID							// Will(Topic)
				);
		break;
	default:
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		return 0;
	}

	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadMqttOpen(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = NULL;
	uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	int32_t tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	int32_t ptr_len = 0;
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
	memcpy(&ptr_len, trim_len, sizeof(int32_t));
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);

	if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t) 
		&& write_ctx->split_idx[0] == 1)
	{
		++write_ctx->split_idx[1];
		if (write_ctx->split_idx[1] > 30) // timeout > 3s
		{
			phead->body.tlv.err = FC41D_MQTT_ERR_AT_OPEN_TIMEOUT;

			fc41d->task_head_del(fc41d);
			return len;
		}
		delay_1ms(100);
	}

	find = strstr((const char *)data, AT_FC41D_MQTT_OPEN_RET);
	if (find)
	{
		find += strlen(AT_FC41D_MQTT_OPEN_RET);
		int32_t tmp_len = len - strlen(find);
		char tmp_ret[4] = {0};
		int8_t client_id = 0;
		int8_t err = 0;
		int32_t i = 0, j = 0, k = 0;
		for (; i < tmp_len; ++i)
		{
			if (find[i] == '\r' || find[i] == '\n')
			{
				break;
			}

			if (find[i] == ' ')
			{
				continue;
			}

			if (find[i] == ',')
			{
				tmp_ret[j] = 0;
				client_id = atoi(tmp_ret);
				j = 0;
				++k;
				continue;
			}

			tmp_ret[j++] = find[i];
		}

		tmp_ret[j] = 0;
		err = atoi(tmp_ret);

		if (client_id != AT_FC41D_MQTT_CLIENT_ID)
		{
			if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
			{
				phead->body.tlv.err = FC41D_MQTT_ERR_CLIENT_ID;
			}

			fc41d->task_head_del(fc41d);
			return len;
		}

		// check errcode
		int8_t uniform_err = 0;
		switch (err) 
		{
		case -1:	// Socket 连接失败
			uniform_err = FC41D_MQTT_ERR_SOCK;
			fc41d->task_head_del(fc41d);
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
									FC41D_PROCESS_MQTT_CLOSE,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			return len;
			break;
		case 0:	// Open mqtt 成功,进行 conn 过程
			fc41d->task_head_del(fc41d);
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, FC41D_PROCESS_MQTT_CONN,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			break;
		case 1:	// 参数错误
			uniform_err = FC41D_MQTT_ERR_PARAM;
			break;
		case 2:		// 标识符被占用
			uniform_err = FC41D_MQTT_ERR_OCCUPY;
			break;
		case 3:	// 内存不足
			uniform_err = FC41D_MQTT_ERR_MEM;
			break;
		case 4:	// DNS解析失败
			uniform_err = FC41D_MQTT_ERR_DNS;
			break;
		case 5:	// Socket 异常关闭
			uniform_err = FC41D_MQTT_ERR_SOCK_DUMP;
			break;
		}

		if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
		{
			phead->body.tlv.err = uniform_err;
		}

		if (err > 0) 
		{
			fc41d->task_head_del(fc41d);
		}
		return len;
	}

	find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
		{
			write_ctx->split_idx[0] = 1; // open at is ok, wait for real open with server
			write_ctx->split_idx[1] = 0; // timeout > 3
		}

		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;


		if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
		{
			phead->body.tlv.err = FC41D_MQTT_ERR_UNKNOWN_OPEN;
		}

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttOpen(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

#if COLLECTOR_DBG_MQTT
	DevFc41dSetMqttServerInfo(fc41d, 0, AT_FC41D_MQTT_IP, AT_FC41D_MQTT_PORT);
#endif 

	if (fc41d->priv_data->mqtt.server_ip[0] == 0 
		|| fc41d->priv_data->mqtt.server_port[0] == 0)
	{
		fc41d->task_head_del(fc41d);
		return 0;
	}

	// 由于url长度不定, 采用拼接发送办法
#if USE_STACK_OPEN
	char cmd[1024] = {0};
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_OPEN, 
			 AT_FC41D_MQTT_CLIENT_ID,				// CLIENT_ID
			 fc41d->priv_data->mqtt.server_ip,		// SERVER IP
			 fc41d->priv_data->mqtt.server_port		// SERVER PORT
			 );
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
#else 
	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_MQTT_OPEN, 
				 strlen(AT_FC41D_MQTT_OPEN));
	fc41d->write(fc41d, (const uint8_t *)fc41d->priv_data->mqtt.server_ip, 
				strlen((const char *)fc41d->priv_data->mqtt.server_ip));
	fc41d->write(fc41d, (const uint8_t *)"\",", strlen("\","));
	fc41d->write(fc41d, (const uint8_t *)fc41d->priv_data->mqtt.server_port, 
				strlen((const char *)fc41d->priv_data->mqtt.server_port));
	fc41d->write(fc41d, (const uint8_t *)"\r\n", strlen("\r\n"));
#endif 
	return 0;
}

static int32_t _DevFc41dReadMqttConn(DevFc41d_t *fc41d, const uint8_t
											*data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = NULL;
	uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	int32_t tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);

	if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t) 
		&& write_ctx->split_idx[0] == 2)
	{
		++write_ctx->split_idx[1];
		if (write_ctx->split_idx[1] > 30) // timeout > 3s
		{
			// 去掉状态判断临时占用字节
			phead->body.tlv.err = FC41D_MQTT_ERR_AT_CONN_TIMEOUT;
			fc41d->task_head_del(fc41d);
			return len;
		}
		delay_1ms(100);
	}

	find = strstr((const char *)data, AT_FC41D_MQTT_CONN_RET);
	if (find)
	{
		find += strlen(AT_FC41D_MQTT_CONN_RET);
		int32_t tmp_len = len - strlen(find);
		char tmp_ret[4] = {0};
		int8_t client_id = 0;
		int8_t result = 0;
		int8_t rescode = 0;
		int32_t i = 0, j = 0, k = 0;
		for (; i < tmp_len; ++i)
		{
			if (find[i] == '\r' || find[i] == '\n')
			{
				break;
			}

			if (find[i] == ' ')
			{
				continue;
			}

			if (find[i] == ',')
			{
				tmp_ret[j] = 0;
				if (k == 0)
				{
					client_id = atoi(tmp_ret);
				}
				else if (k == 1)
				{
					result = atoi(tmp_ret);
				}
				j = 0;
				++k;
				continue;
			}

			tmp_ret[j++] = find[i];
		}

		tmp_ret[j] = 0;
		if (k == 1)
		{
			result = atoi(tmp_ret);
		}
		else if (k == 2)
		{
			rescode = atoi(tmp_ret);
		}

		if (client_id != AT_FC41D_MQTT_CLIENT_ID)
		{
			if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
			{
				phead->body.tlv.err = FC41D_MQTT_ERR_CLIENT_ID;
			}

			fc41d->task_head_del(fc41d);
			return len;
		}

		// check errcode
		int8_t uniform_err = 0;
		switch (result) 
		{
		case 0:	// conn mqtt 成功
			write_ctx->split_idx[0] = 0;
			fc41d->task_head_del(fc41d);
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								  FC41D_PROCESS_MQTT_INIT_TOPIC,
								  DEV_FC41D_TASK_STAT_WRITE, 0);
			break;
		case 1:	// CONN 未得到ACK
			uniform_err = FC41D_MQTT_ERR_CONNACK;
			break;
		case 2:	// CONN失败
			uniform_err = FC41D_MQTT_ERR_CONN;
			break;
		}

		if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
		{
			phead->body.tlv.err = uniform_err;
		}

		if (result > 0)
		{
			fc41d->task_head_del(fc41d);
		}
		return len;
	}

	find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
		{
			write_ctx->split_idx[0] = 2; // open at is ok, wait for real open with server
			write_ctx->split_idx[1] = 0; // timeout > 3
		}

		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		if (fc41d->priv_data->mqtt.conn_stat > 0)
		{
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
									FC41D_PROCESS_MQTT_CLOSE,
									DEV_FC41D_TASK_STAT_WRITE, 0);
		}

		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
		{
			phead->body.tlv.err = FC41D_MQTT_ERR_UNKNOWN_CONN;
		}

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttConn(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	// debug 
#if COLLECTOR_DBG_MQTT
	fc41d->priv_data->mqtt.username[0] = 0;
	fc41d->priv_data->mqtt.passwd[0] = 0;
#endif 


	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_CONN, 
			 AT_FC41D_MQTT_CLIENT_ID,				// CLIENT_ID
			 fc41d->priv_data->mqtt.clientname[0]==0?
			 "nova_sn_unkown":fc41d->priv_data->mqtt.clientname,
			 fc41d->priv_data->mqtt.username[0]==0?
			 AT_FC41D_MQTT_USR:fc41d->priv_data->mqtt.username,
			 fc41d->priv_data->mqtt.passwd[0]==0?
			 AT_FC41D_MQTT_PWD:fc41d->priv_data->mqtt.passwd
			 );
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadMqttDisconn(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttDisconn(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_DISCONN, AT_FC41D_MQTT_CLIENT_ID);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadMqttInitTopic(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return 0;
	}

	uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	int32_t tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);

	if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
	{
		if (write_ctx->split_idx[0] 
			< sizeof(gCollectorMqttTopicSub)/sizeof(gCollectorMqttTopicSub[0]))
		{
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_MQTT, 
								  FC41D_PROCESS_MQTT_INIT_TOPIC,
								  DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_head_del(fc41d);
			return len;
		}
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_FC41D_MQTT_SUB_OK);
	if (find) 
	{
		find += strlen(AT_FC41D_MQTT_SUB_OK);
		find = strstr((const char *)find, "\r\n");

		if (find)
		{
			find[1] = 0;
		}

		int32_t write_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		if (write_len > 0)
		{
			int32_t yy = 0;
		}


		if (fc41d->priv_data->mqtt.conn_stat == 0)
		{
			fc41d->priv_data->mqtt.conn_stat = 1;
		}
		fc41d->priv_data->mqtt.test_sub = FC41D_MQTT_SUB_TEST_SIZE;
		read = strlen((const char *)data)+1;

		_gMqttSendLen[0] = _gMqttSendLen[1] = _gMqttSendLen[2] = 0;

		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteMqttInitTopic(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	int32_t tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t topic_idx = 0;

	if (tx_len - sizeof(DevFc41dWriteCtx_t) == sizeof(UniformProtocolHead_t))
	{
		topic_idx = write_ctx->split_idx[0];
		++write_ctx->split_idx[0];
	}

	char cmd[256] = {0};
	char topic[128] = {0};
	uint16_t modelid = gCollector->pcs_modelid;
	snprintf(topic, sizeof(topic), (const char *)gCollectorMqttTopicSub[topic_idx].uri, 
			modelid);
	snprintf(cmd, sizeof(cmd), AT_FC41D_MQTT_SUB, 
			AT_FC41D_MQTT_CLIENT_ID,
			topic_idx+1, topic,
			(const char *)gCollector->pcs_sn,
			//(const char *)gCollector->sn,
			gCollectorMqttTopicSub[topic_idx].qos);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

int32_t _DevFc41dReadMqtt(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dMqttStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_MQTT_CLOSE:
		return _DevFc41dReadMqttClose(fc41d, data, len);
	case FC41D_PROCESS_MQTT_STAT:
		return _DevFc41dReadMqttStat(fc41d, data, len);
	case FC41D_PROCESS_MQTT_SET_CFG_VER:
	case FC41D_PROCESS_MQTT_SET_CFG_DATATYPE:
	case FC41D_PROCESS_MQTT_SET_CFG_SSL:
	case FC41D_PROCESS_MQTT_SET_CFG_KEEPALIVE:
	case FC41D_PROCESS_MQTT_SET_CFG_TIMEOUT:
	case FC41D_PROCESS_MQTT_SET_CFG_SESSION:
	case FC41D_PROCESS_MQTT_SET_CFG_WILL:
		return _DevFc41dReadMqttSetCfg(fc41d, status, data, len);
	case FC41D_PROCESS_MQTT_OPEN:
		return _DevFc41dReadMqttOpen(fc41d, data, len);
	case FC41D_PROCESS_MQTT_CONN:
		return _DevFc41dReadMqttConn(fc41d, data, len);
	case FC41D_PROCESS_MQTT_DISCONN:
		return _DevFc41dReadMqttDisconn(fc41d, data, len);
	case FC41D_PROCESS_MQTT_INIT_TOPIC:
		return _DevFc41dReadMqttInitTopic(fc41d, data, len);
	case FC41D_PROCESS_MQTT_PUB_SEND_LEN:
		return _DevFc41dReadMqttPubLen(fc41d, data, len, status);
	case FC41D_PROCESS_MQTT_PUB_SEND_DATA:
		return _DevFc41dReadMqttPubData(fc41d, data, len, status);
	case FC41D_PROCESS_MQTT_CFG_COMPLETE:
		return _DevFc41dReadMqttCfgComplete(fc41d);
	case FC41D_PROCESS_MQTT_PUB_NORMAL_DATA:
		return _DevFc41dReadMqttPubNormalData(fc41d, data, len, status);
	}

	return 0;
}

int32_t _DevFc41dWriteMqtt(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dMqttStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_MQTT_CLOSE:
		_DevFc41dWriteMqttClose(fc41d);
		break;
	case FC41D_PROCESS_MQTT_STAT:
		_DevFc41dWriteMqttStat(fc41d);
		break;
	case FC41D_PROCESS_MQTT_SET_CFG_VER:
	case FC41D_PROCESS_MQTT_SET_CFG_DATATYPE:
	case FC41D_PROCESS_MQTT_SET_CFG_SSL:
	case FC41D_PROCESS_MQTT_SET_CFG_KEEPALIVE:
	case FC41D_PROCESS_MQTT_SET_CFG_TIMEOUT:
	case FC41D_PROCESS_MQTT_SET_CFG_SESSION:
	case FC41D_PROCESS_MQTT_SET_CFG_WILL:
		_DevFc41dWriteMqttSetCfg(fc41d, status);
		break;
	case FC41D_PROCESS_MQTT_OPEN:
		_DevFc41dWriteMqttOpen(fc41d);
		break;
	case FC41D_PROCESS_MQTT_CONN:
		_DevFc41dWriteMqttConn(fc41d);
		break;
	case FC41D_PROCESS_MQTT_DISCONN:
		_DevFc41dWriteMqttDisconn(fc41d);
		break;
	case FC41D_PROCESS_MQTT_INIT_TOPIC:
		_DevFc41dWriteMqttInitTopic(fc41d);
        break;
	case FC41D_PROCESS_MQTT_PUB_SEND_LEN:
		_DevFc41dWriteMqttPubLen(fc41d, status);
		//delay_1ms(250);
		break;
	case FC41D_PROCESS_MQTT_PUB_SEND_DATA:
		_DevFc41dWriteMqttPubData(fc41d, status);
		delay_1ms(500);
		break;
	case FC41D_PROCESS_MQTT_CFG_COMPLETE:
		 _DevFc41dWriteMqttCfgComplete(fc41d);
		//delay_1ms(250);
		break;
	case FC41D_PROCESS_MQTT_PUB_NORMAL_DATA:
		_DevFc41dWriteMqttPubNormalData(fc41d, status);
		//delay_1ms(250);
		break;
	}

	return 0;
}

#if MQTT_USING_NORMAL_AT_PUB

// 使用通用接口上报
int32_t _DevFc41dPubMqttResp(DevFc41d_t *fc41d, void *uniform, 
									int32_t pub_idx, int32_t cmd, uint16_t err, 
									const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	int32_t send_len = 0;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;

	write_ctx->cmd = cmd;
	write_ctx->next_cmd = cmd;
	write_ctx->uniform_type = 0;
	write_ctx->mqtt_pub_idx = pub_idx;

	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	UniformProtocolHead_t *u0 = (UniformProtocolHead_t *)(uniform);
	UniformProtocolHead_t *u = (UniformProtocolHead_t *)(write_ctx->data);
	u->src = (0x40);
	u->dst = (0x02);
	u->body.tlv.cmd = cmd;
	u->body.tlv.err = err;
	u->frame.seq_id = u0->frame.seq_id;

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add scan wifi, ble send task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_NORMAL_DATA,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dPubMqttMeta(DevFc41d_t *fc41d, int32_t pub_idx, 
										  const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	write_ctx.uniform_type = 1;
	write_ctx.mqtt_pub_idx = pub_idx;
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)meta_uniform, 
						sizeof(UniformProtocolHead_t)-sizeof(UniformProtocolTlvBodyHead_t));
	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	uniform->src = (0x40);
	uniform->dst = (0x02);
	uniform->frame.seq_id = ++fc41d->priv_data->ack_seq;

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add scan wifi, ble send task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_NORMAL_DATA,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dPubMqtt(DevFc41d_t *fc41d, int32_t pub_idx, 
										  int32_t cmd, uint16_t err,
										  const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.cmd = cmd;
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	write_ctx.next_cmd = cmd;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	write_ctx.mqtt_pub_idx = pub_idx;
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));

	uint8_t *ptr = (uint8_t *)meta_uniform;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(ptr);
	uniform->src = (0x40);
	uniform->dst = (0x02);
	uniform->body.tlv.cmd = cmd;
	uniform->body.tlv.err = err;
	uniform->frame.seq_id = ++fc41d->priv_data->ack_seq;


	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)meta_uniform, sizeof(UniformProtocolHead_t));
	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	uniform = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_NORMAL_DATA,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dPubMqtt2Self(DevFc41d_t *fc41d, int32_t pub_idx, 
										  const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	fc41d->priv_data->mqtt.test_sub_process = 1;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	write_ctx.to_self = 1;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	write_ctx.uniform_type = 1;
	write_ctx.mqtt_pub_idx = pub_idx;
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)meta_uniform, 
						sizeof(UniformProtocolHead_t));
	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	uniform->src = (0xFF);
	uniform->dst = (0xFF);
	uniform->frame.seq_id = ++fc41d->priv_data->ack_seq;
	uniform->body.tlv.cmd = PROTO_UNIFORM_R_TEST_MQTT_SUB;
	uniform->body.tlv.err = 0x00;
	uniform->body.tlv.data_len = len;

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add scan wifi, ble send task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_NORMAL_DATA,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	--fc41d->priv_data->mqtt.test_sub;
	if (fc41d->priv_data->mqtt.test_sub <= 0)
	{
		fc41d->priv_data->mqtt.conn_stat = 0;
		fc41d->priv_data->mqtt.test_sub = FC41D_MQTT_SUB_TEST_SIZE;
	}
	return 0;
}

#else

// 使用定制接口上报
int32_t _DevFc41dPubMqttResp(DevFc41d_t *fc41d, void *uniform, 
									int32_t pub_idx, int32_t cmd, uint16_t err, 
									const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	int32_t send_len = 0;
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;

	write_ctx->cmd = cmd;
	write_ctx->next_cmd = cmd;
	write_ctx->uniform_type = 0;
	write_ctx->mqtt_pub_idx = pub_idx;

	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	UniformProtocolHead_t *u0 = (UniformProtocolHead_t *)(uniform);
	UniformProtocolHead_t *u = (UniformProtocolHead_t *)(write_ctx->data);
	u->src = (0x40);
	u->dst = (0x02);
	u->body.tlv.cmd = cmd;
	u->body.tlv.err = err;
	u->frame.seq_id = u0->frame.seq_id;

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add scan wifi, ble send task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_SEND_LEN,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dPubMqttMeta(DevFc41d_t *fc41d, int32_t pub_idx, 
										  const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	write_ctx.uniform_type = 1;
	write_ctx.mqtt_pub_idx = pub_idx;
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)meta_uniform, 
						sizeof(UniformProtocolHead_t)-sizeof(UniformProtocolTlvBodyHead_t));
	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	uniform->src = (0x40);
	uniform->dst = (0x02);
	uniform->frame.seq_id = ++fc41d->priv_data->ack_seq;

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add scan wifi, ble send task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_SEND_LEN,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dPubMqtt(DevFc41d_t *fc41d, int32_t pub_idx, 
										  int32_t cmd, uint16_t err,
										  const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.cmd = cmd;
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	write_ctx.next_cmd = cmd;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	write_ctx.mqtt_pub_idx = pub_idx;
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));

	uint8_t *ptr = (uint8_t *)meta_uniform;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(ptr);
	uniform->src = (0x40);
	uniform->dst = (0x02);
	uniform->body.tlv.cmd = cmd;
	uniform->body.tlv.err = err;
	uniform->frame.seq_id = ++fc41d->priv_data->ack_seq;


	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)meta_uniform, sizeof(UniformProtocolHead_t));
	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	uniform = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_SEND_LEN,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dPubMqtt2Self(DevFc41d_t *fc41d, int32_t pub_idx, 
										  const uint8_t *data, int32_t len)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_MQTT;
	fc41d->priv_data->mqtt.test_sub_process = 1;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	write_ctx.req_type = PROTO_UNIFORM_REQ;
	write_ctx.to_self = 1;
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	write_ctx.uniform_type = 1;
	write_ctx.mqtt_pub_idx = pub_idx;
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)meta_uniform, 
						sizeof(UniformProtocolHead_t));
	// append data
	if (data != NULL &&  len > 0)
	{
		send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
									   (const uint8_t *)data, len);
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
	uniform->src = (0xFF);
	uniform->dst = (0xFF);
	uniform->frame.seq_id = ++fc41d->priv_data->ack_seq;
	uniform->body.tlv.cmd = PROTO_UNIFORM_R_TEST_MQTT_SUB;
	uniform->body.tlv.err = 0x00;
	uniform->body.tlv.data_len = len;

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);

	// add scan wifi, ble send task
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, 
						  FC41D_PROCESS_MQTT_PUB_SEND_LEN,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	--fc41d->priv_data->mqtt.test_sub;
	if (fc41d->priv_data->mqtt.test_sub <= 0)
	{
		fc41d->priv_data->mqtt.conn_stat = 0;
		fc41d->priv_data->mqtt.test_sub = FC41D_MQTT_SUB_TEST_SIZE;
	}
	return 0;
}

#endif 

int32_t _DevFc41dUrcMqtt(DevFc41d_t *fc41d, int8_t type, 
									   const char *data, int32_t len)
{
	int32_t ret = 0, i = 0;
	char *p = (char *)data;
    
    if (type != 0)
    {
        return ret;
    }

	if (strncmp(data, AT_FC41D_MQTT_URC_HEAD, strlen(AT_FC41D_MQTT_URC_HEAD))) 
	{
		return ret;
	}

	if (!strncmp(data, AT_FC41D_MQTT_URC_MQTT_PUB, strlen(AT_FC41D_MQTT_URC_MQTT_PUB)))
	{
		char *find = strstr(data, "\n");
		if (find == NULL)
		{
			return strlen(AT_FC41D_MQTT_URC_MQTT_PUB);
		}

		find[0] = 0;
		return strlen(data) + 1;
	}

	ret += strlen(AT_FC41D_MQTT_URC_HEAD);
	p += ret;

	while (ret < len) 
	{
		if (p[0] == '\r' || p[0] == '\n')
		{
			break;
		}

		if (strlen(p) > strlen(AT_FC41D_MQTT_URC_STAT_ERR)
			&& strncmp(p, AT_FC41D_MQTT_URC_STAT_ERR, 
					   strlen(AT_FC41D_MQTT_URC_STAT_ERR)) == 0) 
		{
            fc41d->priv_data->mqtt.conn_stat = 0;
			i = strlen(AT_FC41D_MQTT_URC_STAT_ERR);
			ret += i;
			p += i;

			continue;
		}

		if (strlen(p) > strlen(AT_FC41D_MQTT_URC_STAT_OPEN)
			&& strncmp(p, AT_FC41D_MQTT_URC_STAT_OPEN, 
					   strlen(AT_FC41D_MQTT_URC_STAT_OPEN)) == 0) 
		{
			if (fc41d->priv_data->mqtt.conn_stat == 0)
			{
				fc41d->priv_data->mqtt.conn_stat = 1;
			}
			fc41d->priv_data->mqtt.test_sub = FC41D_MQTT_SUB_TEST_SIZE;
			i = strlen(AT_FC41D_MQTT_URC_STAT_OPEN);
			ret += i;
			p += i;

			continue;
		}

		if (strlen(p) > strlen(AT_FC41D_MQTT_URC_MQTT_RECV)
			&& strncmp(p, AT_FC41D_MQTT_URC_MQTT_RECV, 
					   strlen(AT_FC41D_MQTT_URC_MQTT_RECV)) == 0) 
		{
            fc41d->priv_data->wifi.info.station.got_ip_stat = WIFI_CONN_STAT_CONN;
			i = strlen(AT_FC41D_MQTT_URC_MQTT_RECV);
			ret += i;
			p += i;

			int32_t i = ret, j = 0, k = 0, m = 0;
			char tmp_mqtt_sub[128] = {0};
			int32_t urc_data_len = 0;
			for (; i < len && k < 4 && m < 3; ++p, ++i)
			{
				if (data[i] == ' ')
				{
					continue;
				}
				if (data[i] == ',')
				{
					tmp_mqtt_sub[j] = 0;

					if (k == 2) {
						// uri
					} else if (k == 3) {
						// urc data len
						urc_data_len = atoi(tmp_mqtt_sub);
					} 

					j = 0;
					++k;
					continue;
				}

				tmp_mqtt_sub[j++] = data[i];

			}

			ret = i+1;
			p = (char*)data+i+1;

			int32_t ret1 = _DevFc41dUrcNovaDebug(fc41d, (const char *)p, len - ret);
			if (ret1 > 0) 
			{
				ret1 = i+1+urc_data_len+1+2; // i + 0x22(") + data_len + 0x22(") + 0x0D0A
				ret = ret1;
				p = (char*)data+ret1;
				continue;
			}

			ret1 = _DevFc41dUrcNova(fc41d, 0, (const char *)p, len - ret);
			if (ret1 > 0) 
			{
				ret1 = i+1+urc_data_len+1+2; // i + 0x22(") + data_len + 0x22(") + 0x0D0A
				ret = ret1;
				p = (char*)data+ret1;
				continue;
			}

			continue;
		}

		++p;
        ++ret;
	}

	return ret;
}

static int32_t _DevFc41dReconnect(DevFc41d_t *fc41d)
{
	DevFc41dWriteCtx_t write_ctx = {0};
	UniformProtocolHead_t uniform = {0};

	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&uniform, sizeof(UniformProtocolHead_t));

	// add task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, FC41D_PROCESS_MQTT_SET_CFG_VER,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, FC41D_PROCESS_MQTT_CLOSE,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, FC41D_PROCESS_MQTT_OPEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_MQTT, FC41D_PROCESS_MQTT_CFG_COMPLETE,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dCfgMqttAutoConn(DevFc41d_t *fc41d)
{
	static int32_t try_count = 0;

	if (fc41d->priv_data->http.ota_fc41d)
	{
		return -1;
	}

	if (gCollector->pcs_sn[0] == 0)
	{
		return -1;
	}

	if (fc41d->priv_data->wifi.info.station.got_ip_stat == 0
		|| fc41d->priv_data->wifi.info.station.conn_stat == 0)
	{
		gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_GRID;
		fc41d->priv_data->mqtt.conn_stat = 0;
		return -1;
	}

	if (!fc41d->priv_data->cfg.mqtt_auto_conn)
	{
		return -2;
	}

	if (fc41d->priv_data->mqtt.conn_stat > 0)
	{
		try_count = 0;
		gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OK;
		return -3;
	}

	gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_CLOUD;
	if (fc41d->priv_data->mqtt.server_type == 2)
	{
		if (fc41d->priv_data->mqtt.clientname[0] == 0
			|| gCollector->head.s_url_len == 0
			|| gCollector->head.s_url[0] == 0
			|| gCollector->pcs_sn[0] == 0)
		{
			return -4;
		}
	} 
	else
	{
		if (fc41d->priv_data->mqtt.clientname[0] == 0
			|| fc41d->priv_data->mqtt.server_ip[0] == 0
			|| fc41d->priv_data->mqtt.server_port[0] == 0)
		{
			return -4;
		}

	}

	if (fc41d->priv_data->wifi.type != DEV_FC41D_WIFI_TYPE_STATION)
	{
		gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_GRID;
		return -5;
	}

	if (fc41d->priv_data->cfg.mqtt_auto_conn_time > 0
		&& (uint32_t)(tim_get_systime() - fc41d->priv_data->cfg.mqtt_auto_conn_time) 
		< fc41d->priv_data->cfg.mqtt_auto_conn_timeout)
	{
		return -6;
	}

	fc41d->priv_data->cfg.mqtt_auto_conn_time = tim_get_systime();
	int32_t write_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	if (write_len >= sizeof(DevFc41dWriteCtx_t) + sizeof(UniformProtocolHead_t))
	{
		uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		int32_t ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
		memcpy(&ptr_len, trim_len, sizeof(int32_t));
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
//		return -7;
	}

	++try_count;
	if (try_count > 10)
	{
		// reset fc41d
        try_count = 0;
		fc41d->priv_data->mqtt.server_ip[0] = 0;
		fc41d->priv_data->mqtt.server_port[0] = 0;
		fc41d->priv_data->mqtt.username[0] = 0;
		fc41d->priv_data->mqtt.passwd[0] = 0;
		_DevFc41dUsartInit(fc41d);
	}

	gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_CLOUD;

#if !COLLECTOR_DBG_MQTT
	
	if (fc41d->priv_data->mqtt.server_type == 2)
	{
		if (fc41d->priv_data->mqtt.clientname[0] == 0
			|| fc41d->priv_data->mqtt.server_ip[0] == 0
			|| fc41d->priv_data->mqtt.server_port[0] == 0
			|| fc41d->priv_data->mqtt.username[0] == 0
			|| fc41d->priv_data->mqtt.passwd[0] == 0)
		{
			snprintf(fc41d->priv_data->http.url, FC41D_HTTP_URL_LEN, "%s", 
					 (const char *)&gCollector->head.s_url[1]?&gCollector->head.s_url[1]:"");
			fc41d->priv_data->http.custom_type = DEV_FC41D_HTTP_CUSTOM_MQTT_INFO;
			fc41d->http_req(fc41d, NULL, PROTO_UNIFORM_R_MQTT_BY_HTTP, 0, 
							(const uint8_t *)"1", 1);
			return 0;
		}
	}
#endif 

	_DevFc41dReconnect(fc41d);
	return 0;
}

int32_t DevFc41dSetMqttServerInfo(DevFc41d_t *fc41d, int8_t type, const char *ip, 
								  const char *port)
{
	if (fc41d == NULL)
	{
		return -1;
	}

	if (ip == NULL)
	{
		return -2;
	}

	if (port == NULL)
	{
		return -3;
	}

	if (type != 0 && type != 1)
	{
		return -4;
	}

	fc41d->priv_data->mqtt.server_type = type;

	snprintf(fc41d->priv_data->mqtt.server_ip, sizeof(fc41d->priv_data->mqtt.server_ip), 
			 "%s", ip);

	snprintf(fc41d->priv_data->mqtt.server_port, sizeof(fc41d->priv_data->mqtt.server_port), 
			 "%s", port);

	return 0;
}

int32_t DevFc41dCheckMqttSubTopicStatus(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return -1;
	}
	int32_t flag = 0xFFDEEDFF;
	return _DevFc41dPubMqtt2Self(fc41d, 0, (const uint8_t *)&flag, sizeof(flag));
}

int8_t DevFc41dGetMqttStat(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return -1;
	}
	return (int8_t)fc41d->priv_data->mqtt.conn_stat;
}

