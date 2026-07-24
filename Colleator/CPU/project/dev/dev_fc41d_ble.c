#include "dev_fc41d_common.h"

// FC41D Ble name
#define FC41D_BLE_NAME				"NovaCollectorBle"
#define FC41D_BLE_MAX_MTU			512

// AT ble 
#define AT_QBLE_STAT_RET_HEAD		"+QBLESTAT:"
#define AT_FC41D_BLE_INIT			"AT+QBLEINIT=2\r\n" // 模块为外围设备进行 BLE 初始化
#define AT_FC41D_BLE_ADDR			"AT+QBLEADDR?\r\n"
#define AT_FC41D_BLE_STAT			"AT+QBLESTAT\r\n"
#define AT_FC41D_BLE_ADDR_RET_HEAD	"+QBLEADDR:"
#define AT_FC41D_BLE_MTU			"AT+QBLECFGMTU=512\r\n"
#define AT_FC41D_BLE_ADVDATA		"AT+QBLEADVDATA="
#define AT_FC41D_BLE_GATTSSRV		"AT+QBLEGATTSSRV=fff1\r\n"
#define AT_FC41D_BLE_GATTSCHAR		"AT+QBLEGATTSCHAR=fff2,1f\r\n"
#define AT_FC41D_BLE_GATTSCHAR1		"AT+QBLEGATTSCHAR=fff3\r\n"
#define AT_FC41D_BLE_ADVPARAM		"AT+QBLEADVPARAM=150,150\r\n"
#define AT_FC41D_BLE_ADVSTART		"AT+QBLEADVSTART\r\n"
#define AT_FC41D_BLE_ADVSTOP		"AT+QBLEADVSTOP\r\n"

// URC ble 
#define AT_FC41D_BLE_URC_HEAD		"+QBLE"
#define AT_FC41D_BLE_URC_CONN		"+QBLESTAT:CONNECTED"
#define AT_FC41D_BLE_URC_DISCONN	"+QBLESTAT:DISCONNECTED"
#define AT_FC41D_BLE_URC_MTU		"+QBLEMTU:"

extern void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d);
extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);
extern int32_t _LocalUpgradeStructCheck(DevFc41d_t *fc41d);
extern void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d);

static int32_t _Asc2Hex(char *ascii, uint8_t len, uint8_t *p_hex)
{
    int16_t i = 0;
    uint8_t n_temp = 0;
    uint32_t n_data = 0;

    //将ASCII码转化为16进制数
    for(i = 0; i < len; ++i) 
	{
        if((ascii[i]>='0')&&(ascii[i]<='9')) 
		{
            n_temp = ascii[i]-0x30;
        } 
		else if((ascii[i]>='A')&&(ascii[i]<='F')) 
		{
            n_temp = ascii[i]-'A'+10;
        } 
		else if((ascii[i]>='a')&&(ascii[i]<='f')) 
		{
            n_temp = ascii[i]-'a'+10;        
        } 
		else 
		{
            return -1;
        }
		n_data += (uint32_t)n_temp << (4*(len-1-i));
    }
    *p_hex = n_data;
    return 0;
}

static int32_t _DevFc41dBleStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_BLE_STATUS_INIT
			:fc41d->priv_data->ble.status);
}

int32_t _DevFc41dBleStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->ble.status = status;
	return 0;
}

int32_t _DevFc41dBleSendingStatus(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->ble.send_flag;
}

static int32_t _DevFc41dNextTasks(DevFc41d_t *fc41d, int32_t cmd)
{
	switch (cmd)
	{
	case PROTO_UNIFORM_W_DEBUG_OTA_FC41D:
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_OTA_FC41D,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case PROTO_UNIFORM_W_RESET_NOW:
		delay_1ms(1000);

		// reset
		__set_FAULTMASK(1);
		NVIC_SystemReset();
		break;
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END:
		if (_LocalUpgradeStructCheck(fc41d) == 0)
		{
#if 1
			fc41d->priv_data->upgrade.type = EN_UPGRADE_TYPE_NONE;
			fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
			fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
			fc41d->priv_data->upgrade.stat = 0;
			fc41d->priv_data->upgrade.time = 0;
			rbuffer_clear(&fc41d->priv_data->upgrade.down.ctx);

			delay_1ms(1000);

			// reset
			__set_FAULTMASK(1);
			NVIC_SystemReset();
#endif 
		}

		_DevFc41dResetUpgradeData(fc41d);
		break;
	}

	return 0;
}

static int32_t _DevFc41dReadBleSendLen(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len < 1) 
	{
		return 0;
	}

	static int32_t try_count = 3;

#if 1
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
		uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
		--uniform->frame.packet_index;
		_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_WRITE);
		return i-1;
	}


#else 

	if (data[0] != '>')
	{
		// 数据异常，丢弃
		--try_count;
		if (try_count < 0)
		{
			try_count = 3;
			fc41d->task_head_del(fc41d);
			return len;
		}
		uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
		--uniform->frame.packet_index;
		_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_WRITE);
		return len;
	}

	try_count = 3;
#endif 

	fc41d->task_head_del(fc41d);

	int32_t cmd_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	if (cmd_len >= sizeof(UniformProtocolHead_t))
	{
		uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
		if (write_ctx->cmd_proity) 
		{
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_SEND_DATA,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			return len;
		}
	}

	fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_BLE, 
							FC41D_PROCESS_BLE_STATUS_SEND_DATA,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return len;
}

static int32_t _DevFc41dWriteBleSendLen(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[FC41D_BLE_MTU + sizeof(DevFc41dWriteCtx_t)] = {0};
	int32_t cmd_len = rbuffer_get_bytes(&fc41d->priv_data->rb_write.ctx, 
								(uint8_t *)cmd, sizeof(cmd)) - sizeof(DevFc41dWriteCtx_t);
	int32_t send_len = cmd_len;

	if (cmd_len <= 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	if (send_len > fc41d->priv_data->ble.mtu) 
	{
		send_len = fc41d->priv_data->ble.mtu;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
    
	if (write_ctx->cmd == PROTO_UNIFORM_R_WIFI_SCAN)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_WIFI_INFO)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_MQTT_INFO)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_DEBUG_GET_PCS_MODBUS)
	{
		int32_t xx = 0;
	}

	if (uniform->frame.packet_num == 0)
	{
		memset(trim_len, 0, sizeof(int32_t));
		uint16_t pkg_num = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
			- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
		uint16_t tmp = fc41d->priv_data->ble.mtu - sizeof(UniformProtocolHead_t);
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
		fc41d->priv_data->ble.send_flag = 0;
		fc41d->task_head_del(fc41d);
		return 0;
	}
	if (uniform->frame.packet_index >= uniform->frame.packet_num)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->ble.send_flag = 0;
		fc41d->task_head_del(fc41d);

		_DevFc41dNextTasks(fc41d, write_ctx->next_cmd);
		return 0;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->priv_data->ble.send_flag = 1;

	if (write_ctx->debug)
	{
		send_len -= sizeof(UniformProtocolHead_t);
		// 由于debug模式多添加了一个TLV头需要去掉
		send_len -= sizeof(UniformProtocolTlvBodyHead_t);
	}
	write_ctx->send_len = send_len;
	if (send_len <= 0)
	{
		// 无数据可发，直接删除该任务
		fc41d->task_head_del(fc41d);
		return -1;
	}
	snprintf(cmd, sizeof(cmd), "AT+QBLEGATTSNTFY=fff2,H,%d\r\n", send_len);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));

	if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END)
	{
		int32_t xx = 0;
	}
	else 
	{
		if (write_ctx->cmd == PROTO_UNIFORM_R_DEV_PRODUCT)
		{
			int32_t xx = 0;
		}
		delay_1ms(100);
	}

	return 0;
}

static int32_t _DevFc41dReadBleSendData(DevFc41d_t *fc41d, const uint8_t *data, 
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
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

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

static int32_t _DevFc41dWriteBleSendData(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t cmd_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	if (cmd_len <= 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	int32_t send_len = cmd_len - sizeof(DevFc41dWriteCtx_t);
	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
    
    send_len = write_ctx->send_len;
	if (send_len > fc41d->priv_data->ble.mtu) 
	{
		send_len = fc41d->priv_data->ble.mtu;
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
		send_len = proto_encode_uniform(PROTO_UNIFORM_RES, 
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

	if (write_ctx->cmd == PROTO_UNIFORM_W_WIFI_INFO)
	{
		delay_1ms(100);
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_MQTT_INFO)
	{
		delay_1ms(100);
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_WIFI_INFO)
	{
		delay_1ms(100);
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_WIFI_SCAN)
	{
		delay_1ms(100);
	}


	fc41d->priv_data->ble.send_flag = 2;
	fc41d->write(fc41d, (const uint8_t *)(tmp_ptr), send_len);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
							FC41D_PROCESS_BLE_STATUS_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_NAME)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_INIT, 0, 
								DEV_FC41D_TASK_STAT_WRITE, 0);
		delay_1ms(2000);
	}
	else if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEBUG_GET_PCS_MODBUS)
	{
		int32_t xx = 0;
	}

	delay_1ms(10);
	// debug for ble
	if (write_ctx->cmd == PROTO_UNIFORM_W_WIFI_INFO)
	{
		int32_t xx = 0;
		delay_1ms(30);
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_MQTT_INFO)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_WIFI_INFO)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_WIFI_SCAN)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_DEV_INFO)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_DEV_NAME)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_DEV_PRODUCT)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START)
	{
		if (uniform->body.tlv.err == COLLECTOR_LOCAL_UPGRADE_ERR_ALREADY_DOWNLOAD 
			&& write_ctx->upgrade_reset != 0)
		{
			_DevFc41dNextTasks(fc41d, write_ctx->next_cmd);
		}
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END)
	{
		int32_t xx = 0;
	}
	else if (write_ctx->cmd == PROTO_UNIFORM_R_TEST_UPGRADE_MD5)
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

static int32_t _DevFc41dReadBleAdvstart(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

		fc41d->priv_data->stat = 2;

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

static int32_t _DevFc41dWriteBleAdvstart(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_ADVSTART, strlen(AT_FC41D_BLE_ADVSTART));
	return 0;
}

static int32_t _DevFc41dReadBleAdvstop(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

		fc41d->priv_data->stat = 1;

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

static int32_t _DevFc41dWriteBleAdvstop(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_ADVSTOP, strlen(AT_FC41D_BLE_ADVSTOP));
	return 0;
}

static int32_t _DevFc41dReadBleAdvparam(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

static int32_t _DevFc41dWriteBleAdvparam(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_ADVPARAM, strlen(AT_FC41D_BLE_ADVPARAM));
	return 0;
}

static int32_t _DevFc41dReadBleGattschar(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

static int32_t _DevFc41dWriteBleGattschar(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_GATTSCHAR, 
				 strlen(AT_FC41D_BLE_GATTSCHAR));
	return 0;
}

static int32_t _DevFc41dWriteBleGattschar1(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_GATTSCHAR1, 
				 strlen(AT_FC41D_BLE_GATTSCHAR1));
	return 0;
}

static int32_t _DevFc41dReadBleGattssrv(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_BLE, 
							  FC41D_PROCESS_BLE_STATUS_GATTSSRV,
							  DEV_FC41D_TASK_STAT_WRITE, 0);
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_BLE, 
							  FC41D_PROCESS_BLE_STATUS_STAT,
							  DEV_FC41D_TASK_STAT_WRITE, 0);

		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteBleGattssrv(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_GATTSSRV, strlen(AT_FC41D_BLE_GATTSSRV));
	return 0;
}

static int32_t _DevFc41dReadBleMtu(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

static int32_t _DevFc41dWriteBleMtu(DevFc41d_t *fc41d)
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
	char ble_name[21] = {0}; 
	int32_t content_len = 0;
	int32_t cmd_len = 0;

	//snprintf(cmd, sizeof(cmd), "%s%d\r\n", AT_FC41D_BLE_MTU, FC41D_BLE_MAX_MTU);
	snprintf(cmd, sizeof(cmd), "%s", AT_FC41D_BLE_MTU);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadBleAdvdata(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

static int32_t _DevFc41dWriteBleAdvdata(DevFc41d_t *fc41d)
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
	char ble_name[21] = {0}; 
	int32_t content_len = 0;
	int32_t cmd_len = 0;

	snprintf(ble_name, sizeof(ble_name), "%s", (const char *)gCollector->name);
	content_len = strlen(ble_name);
	if (content_len <= 0)
	{
		snprintf(ble_name, sizeof(ble_name), "%s", FC41D_BLE_NAME);
	}
	memcpy(fc41d->priv_data->ble.name, ble_name, sizeof(fc41d->priv_data->ble.name));
	snprintf(cmd, sizeof(cmd), "%s%02x%02x", 
			 AT_FC41D_BLE_ADVDATA, 
			 content_len+1, // ADVData len
			 0x09 // ADVData type
			 );
	cmd_len = strlen(cmd);
	for(int32_t i = 0; i < content_len; ++i) 
	{
		snprintf(cmd+cmd_len, sizeof(cmd)-cmd_len, "%02x", ble_name[i]);
		cmd_len += 2;
	}

	//manufactor
	content_len = 9;
	snprintf(cmd+cmd_len, sizeof(cmd)-cmd_len, "%02x%02x%02x%02x", 
			 content_len, //adv_data length
			 0xFF, //AD Type
#if 0
			 (uint8_t)*((uint8_t *)&(gCollector->type)+1),
			 (uint8_t)*(uint8_t *)&(gCollector->type) // model ID
#else
			 0xC1, 0x00 // 前两个字段-
#endif 
			);
	cmd_len = strlen(cmd);
	//MAC
	for(int i = 0; i < sizeof(fc41d->priv_data->ble.mac); ++i) 
	{
		snprintf(cmd+cmd_len, sizeof(cmd)-cmd_len, "%02x", fc41d->priv_data->ble.mac[i]);
		cmd_len += 2;
	}

	snprintf(cmd+cmd_len, sizeof(cmd)-cmd_len, "%s", "\r\n");
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadBleStat(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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
	const char *buf = (const char *)data;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		char *find = strstr(buf, AT_QBLE_STAT_RET_HEAD);
		if (find == NULL) 
		{
			return read;
		}
		find += strlen(AT_QBLE_STAT_RET_HEAD);
		fc41d->priv_data->stat = 0;
		switch (find[0]) 
		{
		case 'i':
		case 'I':
			// 初始化状态
			/*
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_MTU,
									DEV_FC41D_TASK_STAT_WRITE, 0);
									*/
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_ADVDATA,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_GATTSSRV,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_GATTSCHAR,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			/*
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_GATTSCHAR1,
									DEV_FC41D_TASK_STAT_WRITE, 0);
									*/
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_ADVPARAM,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_ADVSTART,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			break;
		case 'a':
		case 'A':
			// 广播状态
			fc41d->priv_data->stat = 2;
			break;
		case 'c':
		case 'C':
			// 连接状态
			break;
		case 'd':
		case 'D':
			// 断开状态
			break;
		case 'n':
		case 'N':
			if (find[2] == 'i' || find[2] == 'I') 
			{
				// 未初始化
				fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									  FC41D_PROCESS_BLE_STATUS_INIT,
									  DEV_FC41D_TASK_STAT_WRITE, 0);

			} 
			else if (find[2] == 'a' || find[2] == 'A') 
			{
				// 无广播状态
			}
			/*
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_MTU,
									DEV_FC41D_TASK_STAT_WRITE, 0);
									*/
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_ADVDATA,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_GATTSSRV,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_GATTSCHAR,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			/*
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_GATTSCHAR1,
									DEV_FC41D_TASK_STAT_WRITE, 0);
									*/
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_ADVPARAM,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_BLE, 
									FC41D_PROCESS_BLE_STATUS_ADVSTART,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			break;
		default:
			// 异常状态，需重置 FC41D
			break;

		}

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

static int32_t _DevFc41dWriteBleStat(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_STAT, strlen(AT_FC41D_BLE_STAT));
	return 0;
}

static int32_t _DevFc41dReadBleAddr(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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
	const char *buf = (const char *)data;

	char *find = strstr((const char *)buf, AT_OK);
	if (find) 
	{
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		// parse ble mac[6]
		find = strstr(buf, AT_FC41D_BLE_ADDR_RET_HEAD);
		if (find == NULL) 
		{
			return read;
		}

		find += strlen(AT_FC41D_BLE_ADDR_RET_HEAD);

		char mac[4] = {0};
		for (int32_t i = 0, j = 0, k = 0; i < strlen(find) 
			 && k < sizeof(fc41d->priv_data->ble.mac); ++i) 
		{
			if (find[i] == ' ') 
			{
				continue;
			}
			if (find[i] == ':' || find[i] == 0x0D || find[i] == 0x0A) 
			{
				mac[j] = 0;

				// get one mac
				_Asc2Hex(mac, strlen(mac), &fc41d->priv_data->ble.mac[k]);

				++k;
				j = 0;
				continue;
			}

			if (j > 2) 
			{
				continue;
			}
			mac[j++] = find[i];
		}

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

static int32_t _DevFc41dWriteBleAddr(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_ADDR, strlen(AT_FC41D_BLE_ADDR));
	return 0;
}

static int32_t _DevFc41dReadBleInit(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

static int32_t _DevFc41dWriteBleInit(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_BLE_INIT, strlen(AT_FC41D_BLE_INIT));
	return 0;
}

int32_t _DevFc41dReadBle(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dBleStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_BLE_STATUS_INIT:
		return _DevFc41dReadBleInit(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_ADDR:
		return _DevFc41dReadBleAddr(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_STAT:
		return _DevFc41dReadBleStat(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_MTU:
		return _DevFc41dReadBleMtu(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_ADVDATA:
		return _DevFc41dReadBleAdvdata(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_GATTSSRV:
		return _DevFc41dReadBleGattssrv(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_GATTSCHAR:
	case FC41D_PROCESS_BLE_STATUS_GATTSCHAR1:
		return _DevFc41dReadBleGattschar(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_ADVPARAM:
		return _DevFc41dReadBleAdvparam(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_ADVSTART:
		return _DevFc41dReadBleAdvstart(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_ADVSTOP:
		return _DevFc41dReadBleAdvstop(fc41d, data, len);
	case FC41D_PROCESS_BLE_STATUS_SEND_LEN:
		return _DevFc41dReadBleSendLen(fc41d, data, len, status);
	case FC41D_PROCESS_BLE_STATUS_SEND_DATA:
		return _DevFc41dReadBleSendData(fc41d, data, len, status);
	}

	return 0;
}

int32_t _DevFc41dWriteBle(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dBleStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_BLE_STATUS_INIT:
		_DevFc41dWriteBleInit(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_ADDR:
		_DevFc41dWriteBleAddr(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_STAT:
		_DevFc41dWriteBleStat(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_MTU:
		_DevFc41dWriteBleMtu(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_ADVDATA:
		_DevFc41dWriteBleAdvdata(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_GATTSSRV:
		_DevFc41dWriteBleGattssrv(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_GATTSCHAR:
		_DevFc41dWriteBleGattschar(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_GATTSCHAR1:
		_DevFc41dWriteBleGattschar1(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_ADVPARAM:
		_DevFc41dWriteBleAdvparam(fc41d);
		break;
	case FC41D_PROCESS_BLE_STATUS_ADVSTART:
		_DevFc41dWriteBleAdvstart(fc41d);
        break;
	case FC41D_PROCESS_BLE_STATUS_ADVSTOP:
		_DevFc41dWriteBleAdvstop(fc41d);
        break;
	case FC41D_PROCESS_BLE_STATUS_SEND_LEN:
		_DevFc41dWriteBleSendLen(fc41d, status);
		break;
	case FC41D_PROCESS_BLE_STATUS_SEND_DATA:
		_DevFc41dWriteBleSendData(fc41d, status);
		break;
	}

	return 0;
}

int32_t _DevFc41dUrcBle(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	int32_t ret = 0;
	if (strncmp(data, AT_FC41D_BLE_URC_HEAD, strlen(AT_FC41D_BLE_URC_HEAD))) 
	{
		return ret;
	}

	if (strncmp(data, AT_FC41D_BLE_URC_CONN, strlen(AT_FC41D_BLE_URC_CONN)) == 0) 
	{
		fc41d->priv_data->ble.conn = FC41D_BLE_CONN;

		// 初始化本地模式，接收到指定CMD才进行切换到本地模式
		fc41d->priv_data->ble.local_only = 0;

		return strlen(AT_FC41D_BLE_URC_CONN);
	}

	if (strncmp(data, AT_FC41D_BLE_URC_DISCONN, strlen(AT_FC41D_BLE_URC_DISCONN)) == 0) 
	{
		fc41d->priv_data->ble.conn = FC41D_BLE_DISCONN;

		// 初始化本地模式
		fc41d->priv_data->ble.local_only = 0;

		// 判断是否是本地升级模式
		if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_LOCAL)
		{
			fc41d->priv_data->upgrade.flag = 0;
		}
		return strlen(AT_FC41D_BLE_URC_DISCONN);
	}

	if (strncmp(data, AT_FC41D_BLE_URC_MTU, strlen(AT_FC41D_BLE_URC_MTU)) == 0) 
	{
		uint16_t mtu = 0;
		char mtu_char[6] = {0};
		uint8_t flag = 0;
		int32_t _len = strlen(AT_FC41D_BLE_URC_MTU);
		int32_t i = _len;

		for (int32_t j = 0; i < len; ++i) 
		{
			if (data[i] == '<') 
			{
				++flag;
				continue;
			}

			if (data[i] == '>')
			{
				++flag;
				if (flag == 2)
				{
					mtu = atoi(mtu_char);
					++i;
					break;
				}
				continue;
			}

			mtu_char[j++] = data[i];
		}

		if (mtu > 0) 
		{
			//fc41d->priv_data->ble.mtu = mtu;
		}

		return i;
	}

	return ret;
}

