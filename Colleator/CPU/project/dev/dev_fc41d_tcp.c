#include "dev_fc41d_common.h"

#define AT_FC41D_TCP_S_ACCEPT				"AT+QICFG=\"accept/mode\",0\r\n"
#define AT_FC41D_TCP_S_PORT_RELTYPE			"AT+QICFG=\"tcp/tw_cycle\",0\r\n"
#define AT_FC41D_TCP_S_DATA_TYPE			"AT+QICFG=\"datatype\",2\r\n"
#define AT_FC41D_TCP_S_START				"AT+QIOPEN=1,\"TCP LISTENER\",\"\",0,%d,1\r\n"
#define AT_FC41D_TCP_S_SEND					"AT+QISEND=%d,%d\r\n"
#define AT_FC41D_TCP_S_CLOSE				"AT+QICLOSE=%d\r\n"
#define AT_FC41D_TCP_S_STAT					"AT+QISTATE"

// URC TCP
#define AT_FC41D_TCP_S_URC_HEAD				"+QIURC: "
#define AT_FC41D_TCP_S_URC_RECV				"\"recv\","
#define AT_FC41D_TCP_S_URC_CLOSE			"\"closed\","
#define AT_FC41D_TCP_S_URC_INCOMING			"\"incoming\","
#define AT_FC41D_TCP_S_URC_IN_FULL			"\"incoming full\","

static int32_t _DevFc41dTcpServerStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_TCP_S_STATUS_STOP
			:fc41d->priv_data->tcp_s.status);
}

int32_t _DevFc41dTcpServerStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->tcp_s.status = status;
	return 0;
}

static int32_t _DevFc41dReadTcpServerSend(DevFc41d_t *fc41d, const uint8_t *data, 
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

	if (status != FC41D_PROCESS_TCP_S_STATUS_SEND_DATA) 
	{
		return 0;
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		return read;
	}

	return len;
}

static int32_t _DevFc41dWriteTcpServerSend(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[512] = {0};
	int32_t cmd_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	if (cmd_len <= 0) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		return -1;
	}

	int32_t send_len = cmd_len - sizeof(DevFc41dWriteCtx_t);
	if (send_len > fc41d->priv_data->tcp_s.mtu) 
	{
		send_len = fc41d->priv_data->tcp_s.mtu;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);

	if (status == FC41D_PROCESS_TCP_S_STATUS_SEND_LEN) 
	{
		if (uniform->frame.packet_num == 0)
		{
			memset(trim_len, 0, sizeof(int32_t));
			uint16_t pkg_num = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
				- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
			uint16_t tmp = fc41d->priv_data->tcp_s.mtu - sizeof(UniformProtocolHead_t);
			uniform->frame.packet_num = (pkg_num/tmp) + (pkg_num%tmp>0?1:0);
			if (uniform->frame.packet_num == 0) 
			{
				uniform->frame.packet_num = 1;
			}

			uniform->frame.packet_index = -1;
		}
		if (*trim_len == 0)
		{
			++uniform->frame.packet_index;
		}
		++write_ctx->try_times;
		if (write_ctx->try_times > 4)
		{
			fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
            fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
            rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
            return 0;
		}
        if (uniform->frame.packet_index >= uniform->frame.packet_num)
        {
			fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
            fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
            rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
            return 0;
        }

		snprintf(cmd, sizeof(cmd), AT_FC41D_TCP_S_SEND, fc41d->priv_data->tcp_s.s_id, send_len);
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	}
	else if (status == FC41D_PROCESS_TCP_S_STATUS_SEND_DATA)
	{
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

		uint8_t *tmp_ptr = (uint8_t *)(write_ctx->data);
		send_len = proto_encode_uniform(PROTO_UNIFORM_RES, 
							tmp_ptr,
							 _send,
							 uniform->frame.packet_num,
							 uniform->frame.packet_index,
							 write_ctx->uniform_type,
                             body_total_len,
							 uniform->body.tlv.cmd, uniform->body.tlv.err);


		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_READ);
		fc41d->write(fc41d, (const uint8_t *)(cmd+sizeof(DevFc41dWriteCtx_t)), send_len);
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_TCP_S, 
							FC41D_PROCESS_TCP_S_STATUS_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE);

		if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_NAME)
		{
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_INIT, 0, 
									DEV_FC41D_TASK_STAT_WRITE);
			delay_1ms(2000);
		}

		if (uniform->frame.packet_index == 0)
		{
			ptr_len = send_len-sizeof(UniformProtocolHead_t) 
				+ sizeof(UniformProtocolBodyHead_t);
			memcpy(trim_len, &ptr_len, sizeof(int32_t));
		}
		else
		{
			ptr_len = send_len-sizeof(UniformProtocolHead_t) 
				+ sizeof(UniformProtocolBodyHead_t);
			memcpy(trim_len, &ptr_len, sizeof(int32_t));
		}
	}

	return 0;
}

static int32_t _DevFc41dReadTcpServerCfgAccept(DevFc41d_t *fc41d, const uint8_t *data, 
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
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_WRITE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;
		return read;
	}
    
    read = strlen((const char *)data);

	return read;
}

static int32_t _DevFc41dWriteTcpServerCfgAccept(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), "%s", AT_FC41D_TCP_S_ACCEPT);
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_READ);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

int32_t _DevFc41dReadTcpServer(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dTcpServerStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_TCP_S_STATUS_STOP:
		return 0;
	case FC41D_PROCESS_TCP_S_STATUS_CFG_ACCEPT:
		return _DevFc41dReadTcpServerCfgAccept(fc41d, data, len);
	case FC41D_PROCESS_TCP_S_STATUS_SEND_LEN:
	case FC41D_PROCESS_TCP_S_STATUS_SEND_DATA:
		return _DevFc41dReadTcpServerSend(fc41d, data, len, status);
	}

	return 0;
}

int32_t _DevFc41dWriteTcpServer(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dTcpServerStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_TCP_S_STATUS_STOP:
		// _DevFc41dProcessWriteTcpServerStop(fc41d);
		break;
	case FC41D_PROCESS_TCP_S_STATUS_CFG_ACCEPT:
		_DevFc41dWriteTcpServerCfgAccept(fc41d);
		break;
	case FC41D_PROCESS_TCP_S_STATUS_SEND_LEN:
		_DevFc41dWriteTcpServerSend(fc41d, status);
		break;
	case FC41D_PROCESS_TCP_S_STATUS_SEND_DATA:
		_DevFc41dWriteTcpServerSend(fc41d, status);
		break;
	}

	return 0;
}

int32_t _DevFc41dUrcTcpServer(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	int32_t ret = 0;
	if (strncmp(data, AT_FC41D_TCP_S_URC_HEAD, strlen(AT_FC41D_TCP_S_URC_HEAD))) 
	{
		return ret;
	}

	if (strncmp(data, AT_FC41D_TCP_S_URC_RECV, strlen(AT_FC41D_TCP_S_URC_RECV)) == 0) 
	{
		return ret;
	}

	return ret;
}

