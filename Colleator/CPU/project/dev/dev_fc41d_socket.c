//#include "cJSON.h"
#include "dev_fc41d_common.h"

#define FC41D_SOCKET_USING_STR				1

#define FC41D_SOCKET_MTU					1024
#define DEFAULT_UDP_PORT                    6666

#define AT_FC41D_SOCKET_ACCEPT				"AT+QICFG=\"accept/mode\",0\r\n"
#define AT_FC41D_SOCKET_PORT_RELTYPE		"AT+QICFG=\"tcp/tw_cycle\",0\r\n"
#define AT_FC41D_SOCKET_DATA_TYPE_STR		"AT+QICFG=\"datatype\",0\r\n"
#define AT_FC41D_SOCKET_DATA_TYPE_BIN		"AT+QICFG=\"datatype\",2\r\n"

// socket_id, service_type<TCP LISTENER>, remote ip, remote port, local port, access_mode
#define AT_FC41D_SOCKET_OPEN				"AT+QIOPEN=%d,\"%s\",\"%s\",%d,%d,1\r\n"
#define AT_FC41D_SOCKET_OPEN_RET			"+QIOPEN: "

// socket_id, send_len, [UDP SERVER参数]
#define AT_FC41D_SOCKET_SEND				"AT+QISEND=%d,%d%s\r\n"
// socket_id, len, data, remote_ip, remote_port [当为UDP SERVER时生效]
#define AT_FC41D_SOCKET_SEND_STR			"AT+QISEND=%d,%d,\"%s\",\"%s\",%d\r\n"
// socket_id, len, remote_ip, remote_port [当为UDP SERVER时生效]
#define AT_FC41D_SOCKET_SEND_BIN			"AT+QISEND=%d,%d,\"%s\",%d\r\n"

// socket_id
#define AT_FC41D_SOCKET_CLOSE				"AT+QICLOSE=%d\r\n"

#define AT_FC41D_TCP_S_STAT					"AT+QISTATE"

// URC SOCKET
#define AT_FC41D_TCP_S_URC_HEAD				"+QIURC: "
#define AT_FC41D_TCP_S_URC_RECV				"\"recv\","
#define AT_FC41D_TCP_S_URC_CLOSE			"\"closed\","
#define AT_FC41D_TCP_S_URC_INCOMING			"\"incoming\","
#define AT_FC41D_TCP_S_URC_IN_FULL			"\"incoming full\","

static int32_t gShellyCtReq = 0;
static uint32_t gShellyCtJsonStrCount = 0;
static char gShellyCtJsonStr[2048] = {0};

extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);

static int32_t _DevFc41dSocketStatusGet(DevFc41d_t *fc41d, int32_t socket_id)
{
	return (fc41d==NULL?FC41D_PROCESS_SOCKET_STATUS_CLOSE
			:fc41d->priv_data->socket.sock[socket_id].status);
}

int32_t _DevFc41dSocketStatusSet(DevFc41d_t *fc41d, int32_t status, int32_t socket_id)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->socket.sock[socket_id].status = status;
	return 0;
}

static int32_t _DevFc41dSocketGetUnusedSocket(DevFc41d_t *fc41d)
{
	if (!fc41d)
	{
		return -1;
	}

	int32_t i = 0, j = sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]);

	for (; i < j; ++i) 
	{
		if (fc41d->priv_data->socket.sock[i].type == DEV_FC41D_SOCKET_NONE)
		{
			return i;
		}
	}
	return -2;
}

static int32_t _DevFc41dSocketGetUsing(DevFc41d_t *fc41d)
{
	if (!fc41d)
	{
		return -1;
	}

	int32_t i = 0, j = sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]);

	for (; i < j; ++i) 
	{
		if (fc41d->priv_data->socket.sock[i].type != DEV_FC41D_SOCKET_NONE)
		{
			return i;
		}
	}
	return -2;
}

int32_t _DevFc41dCloseAllSocket(DevFc41d_t *fc41d)
{
	if (!fc41d)
	{
		return -1;
	}

	int32_t task_size = 0;
	int32_t i = 0, j = sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]);

	for (; i < j; ++i) 
	{
		if (fc41d->priv_data->socket.sock[i].type != DEV_FC41D_SOCKET_NONE)
		{
			if (fc41d->priv_data->socket.sock[i].init_type != DEV_FC41D_SOCKET_CLOSING)
			{
				task_size = DevFc41dGetTaskSize(fc41d);
				if (task_size > 0)
				{
					continue;
				}
			}
			fc41d->priv_data->socket.sock[i].init_type = DEV_FC41D_SOCKET_CLOSING;
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
									FC41D_PROCESS_SOCKET_STATUS_CLOSE,
									DEV_FC41D_TASK_STAT_WRITE, i);
		}
	}
	return 0;
}

int32_t _DevFc41dSocketClose(DevFc41d_t *fc41d, int32_t socket_id)
{
	int32_t i = sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]);
	if (socket_id < 0 || socket_id >= i)
	{
		return -1;
	}
	// 在执行完AT指令后reset状态
	if (fc41d->priv_data->socket.sock[socket_id].init_type > DEV_FC41D_SOCKET_NONE)
	{
		fc41d->priv_data->socket.sock[socket_id].init_type = DEV_FC41D_SOCKET_CLOSING;
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
								FC41D_PROCESS_SOCKET_STATUS_CLOSE,
								DEV_FC41D_TASK_STAT_WRITE, socket_id);
	}

	return 0;
}

int32_t _DevFc41dSocketOpen(DevFc41d_t *fc41d, EN_DEV_FC41D_SOCKET_TYPE socket_type,
							char *remote_ip, uint16_t remote_port, uint16_t local_port)
{
	// get empty socket_id
	int32_t socket_id = _DevFc41dSocketGetUnusedSocket(fc41d);
	if (socket_id < 0)
	{
		return socket_id;
	}
    
	memset(&fc41d->priv_data->socket.sock[socket_id], 0, 
		   sizeof(fc41d->priv_data->socket.sock[socket_id]));
	// 设置初始化socket类型
	fc41d->priv_data->socket.sock[socket_id].init_type = (int32_t)socket_type;
	snprintf(fc41d->priv_data->socket.sock[socket_id].remote_ip, 
			 sizeof(fc41d->priv_data->socket.sock[socket_id].remote_ip), "%s", remote_ip);
	fc41d->priv_data->socket.sock[socket_id].remote_port = remote_port;
	fc41d->priv_data->socket.sock[socket_id].local_port = local_port;

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
							FC41D_PROCESS_SOCKET_STATUS_CFG_ACCEPT,
							DEV_FC41D_TASK_STAT_WRITE, socket_id);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
							FC41D_PROCESS_SOCKET_STATUS_CFG_PORT_RELTYPE,
							DEV_FC41D_TASK_STAT_WRITE, socket_id);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
							FC41D_PROCESS_SOCKET_STATUS_CFG_DATA_TYPE,
							DEV_FC41D_TASK_STAT_WRITE, socket_id);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
							FC41D_PROCESS_SOCKET_STATUS_OPEN,
							DEV_FC41D_TASK_STAT_WRITE, socket_id);


	return 0;
}


static int32_t _DevFc41dNextSocketTasks(DevFc41d_t *fc41d, int32_t cmd)
{
	switch (cmd)
	{
	case PROTO_UNIFORM_W_RESET_NOW:
		break;
	case PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END:
		break;
	}

	return 0;
}

static int32_t _DevFc41dReadSocketSendLen(DevFc41d_t *fc41d, const uint8_t *data, 
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
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
									FC41D_PROCESS_SOCKET_STATUS_SEND_DATA,
									DEV_FC41D_TASK_STAT_WRITE, 0);
			return len;
		}
	}

	fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
							FC41D_PROCESS_SOCKET_STATUS_SEND_DATA,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return len;
}

static int32_t _DevFc41dWriteSocketSendLen(DevFc41d_t *fc41d, int32_t socket_id)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);

	char cmd[1024 + 64] = {0};
	int32_t cmd_len = rbuffer_get_bytes(&fc41d->priv_data->rb_write.ctx, 
								(uint8_t *)cmd, sizeof(cmd)) - sizeof(DevFc41dWriteCtx_t);
	if (write_ctx->meta_send == 1)
	{
		cmd_len -= sizeof(UniformProtocolHead_t);
	}
	int32_t send_len = cmd_len;

	if (cmd_len <= 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	if (send_len > fc41d->priv_data->socket.mtu) 
	{
		send_len = fc41d->priv_data->socket.mtu;
	}
    
	if (uniform->frame.packet_num == 0)
	{
		memset(trim_len, 0, sizeof(int32_t));
		uint16_t pkg_num = rbuffer_length(&fc41d->priv_data->rb_write.ctx) 
			- sizeof(DevFc41dWriteCtx_t);
		int16_t tmp = fc41d->priv_data->socket.mtu;
		if (write_ctx->meta_send != 1)
		{
			pkg_num -= sizeof(UniformProtocolHead_t);
			tmp -= sizeof(UniformProtocolHead_t);
		}
		else 
		{
			pkg_num -= sizeof(UniformProtocolHead_t);
		}
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
		fc41d->priv_data->socket.send_flag = 0;
		fc41d->task_head_del(fc41d);
		return 0;
	}
	if (uniform->frame.packet_index >= uniform->frame.packet_num)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->socket.send_flag = 0;
		fc41d->task_head_del(fc41d);

		_DevFc41dNextSocketTasks(fc41d, write_ctx->next_cmd);
		return 0;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->priv_data->socket.send_flag = 1;

	if (write_ctx->debug && write_ctx->meta_send != 1)
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

	// get socket ip, port
	char *ip = fc41d->priv_data->socket.sock[socket_id].remote_ip;
	uint16_t port = fc41d->priv_data->socket.sock[socket_id].remote_port;
	//snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_SEND, socket_id, send_len, "");
	snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_SEND_BIN, socket_id, send_len, 
			 ip?ip:"255.255.255.255", port==0?DEFAULT_UDP_PORT:port);
	if (fc41d->priv_data->socket.sock[socket_id].type == DEV_FC41D_SOCKET_SERVER_UDP)
	{
		snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_SEND_BIN, socket_id, send_len, 
				 ip?ip:"255.255.255.255", port==0?DEFAULT_UDP_PORT:port);
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));

	if (write_ctx->cmd == PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START)
	{
		int32_t xx = 0;
	}

	return 0;
}

static int32_t _DevFc41dReadSocketSendData(DevFc41d_t *fc41d, const uint8_t *data, 
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
			if (write_ctx->meta_send == 1)
			{
				rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
								 sizeof(DevFc41dWriteCtx_t)
								 + sizeof(UniformProtocolHead_t),
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
		}
		else
		{
			if (write_ctx->meta_send == 1)
			{
				rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
								 sizeof(DevFc41dWriteCtx_t)
								 + sizeof(UniformProtocolHead_t),
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

static int32_t _DevFc41dWriteSocketSendData(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);

	int32_t cmd_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	if (cmd_len <= 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	int32_t send_len = cmd_len - sizeof(DevFc41dWriteCtx_t);
	if (write_ctx->meta_send == 1)
	{
		send_len -= sizeof(UniformProtocolHead_t);
	}
    
    send_len = write_ctx->send_len;
	if (send_len > fc41d->priv_data->socket.mtu) 
	{
		send_len = fc41d->priv_data->socket.mtu;
	}

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
		- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
	int32_t _send = send_len;
	if (write_ctx->meta_send != 1)
	{
		if (uniform->frame.packet_index == 0)
		{
			_send = send_len-sizeof(UniformProtocolHead_t);
		}
		else
		{
			_send = send_len-sizeof(UniformProtocolHead_t)
				+sizeof(UniformProtocolBodyHead_t);
		}
	}
	uint8_t *tmp_ptr = (uint8_t *)(write_ctx->data+sizeof(UniformProtocolHead_t));
	if (write_ctx->debug)
	{
		// 去掉TLV 头信息
		tmp_ptr += sizeof(UniformProtocolTlvBodyHead_t);
		// write->send_len 已经在SendLen中已经去掉了多余的长度了
		// send_len -= sizeof(UniformProtocolTlvBodyHead_t);
	}
	else if (write_ctx->meta_send)
	{

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

	fc41d->priv_data->socket.send_flag = 2;
	fc41d->write(fc41d, (const uint8_t *)(tmp_ptr), send_len);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
							FC41D_PROCESS_SOCKET_STATUS_SEND_LEN,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_NAME)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_INIT, 0, 
								DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	else if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEBUG_GET_PCS_MODBUS)
	{
		int32_t xx = 0;
	}

	delay_1ms(10);
	// debug for socket
	if (write_ctx->cmd == PROTO_UNIFORM_W_WIFI_INFO)
	{
		int32_t xx = 0;
	}
	// debug for socket end 

	if (uniform->frame.packet_index == 0)
	{
		if (uniform->frame.packet_num > 1)
		{
			ptr_len = send_len;
			if (write_ctx->meta_send != 1)
			{
				ptr_len -= sizeof(UniformProtocolHead_t);
				ptr_len += sizeof(UniformProtocolBodyHead_t);
			}
		}
		else 
		{
			ptr_len = 0;
		}
		memcpy(trim_len, &ptr_len, sizeof(int32_t));
	}
	else
	{
		ptr_len = send_len;
		if (write_ctx->meta_send != 1)
		{
			ptr_len -= sizeof(UniformProtocolHead_t);
			ptr_len += sizeof(UniformProtocolBodyHead_t);
		}
		memcpy(trim_len, &ptr_len, sizeof(int32_t));
	}

	return 0;
}

static int32_t _DevFc41dReadSocketSendNormalData(DevFc41d_t *fc41d, const uint8_t *data, 
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
			if (write_ctx->meta_send == 1)
			{
				rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
								 sizeof(DevFc41dWriteCtx_t)
								 + sizeof(UniformProtocolHead_t),
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
		}
		else
		{
			if (write_ctx->meta_send == 1)
			{
				rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, 
								 sizeof(DevFc41dWriteCtx_t)
								 + sizeof(UniformProtocolHead_t),
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

static int32_t _DevFc41dWriteSocketSendNormalData(DevFc41d_t *fc41d, int32_t socket_id)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
	int32_t ptr_len = 0;
	uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);

	char cmd[1024 + 64] = {0};
	int32_t cmd_len = rbuffer_get_bytes(&fc41d->priv_data->rb_write.ctx, 
								(uint8_t *)cmd, sizeof(cmd)) - sizeof(DevFc41dWriteCtx_t);
	if (write_ctx->meta_send == 1)
	{
		cmd_len -= sizeof(UniformProtocolHead_t);
	}
	int32_t send_len = cmd_len;

	if (cmd_len <= 0) 
	{
		fc41d->task_head_del(fc41d);
		return -1;
	}

	if (send_len > fc41d->priv_data->socket.mtu) 
	{
		send_len = fc41d->priv_data->socket.mtu;
	}
    
	if (uniform->frame.packet_num == 0)
	{
		memset(trim_len, 0, sizeof(int32_t));
		uint16_t pkg_num = rbuffer_length(&fc41d->priv_data->rb_write.ctx) 
			- sizeof(DevFc41dWriteCtx_t);
		int16_t tmp = fc41d->priv_data->socket.mtu;
		if (write_ctx->meta_send != 1)
		{
			pkg_num -= sizeof(UniformProtocolHead_t);
			tmp -= sizeof(UniformProtocolHead_t);
		}
		else 
		{
			pkg_num -= sizeof(UniformProtocolHead_t);
		}
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
		fc41d->priv_data->socket.send_flag = 0;
		fc41d->task_head_del(fc41d);
		return 0;
	}
	if (uniform->frame.packet_index >= uniform->frame.packet_num)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
		fc41d->priv_data->socket.send_flag = 0;
		fc41d->task_head_del(fc41d);

		_DevFc41dNextSocketTasks(fc41d, write_ctx->next_cmd);
		return 0;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->priv_data->socket.send_flag = 1;

	if (write_ctx->debug && write_ctx->meta_send != 1)
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

	int32_t body_total_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx)
		- sizeof(UniformProtocolHead_t) - sizeof(DevFc41dWriteCtx_t);
	int32_t _send = send_len;
	if (write_ctx->meta_send != 1)
	{
		if (uniform->frame.packet_index == 0)
		{
			_send = send_len-sizeof(UniformProtocolHead_t);
		}
		else
		{
			_send = send_len-sizeof(UniformProtocolHead_t)
				+sizeof(UniformProtocolBodyHead_t);
		}
	}
	uint8_t *tmp_ptr = (uint8_t *)(write_ctx->data+sizeof(UniformProtocolHead_t));
	if (write_ctx->debug)
	{
		// 去掉TLV 头信息
		tmp_ptr += sizeof(UniformProtocolTlvBodyHead_t);
		// write->send_len 已经在SendLen中已经去掉了多余的长度了
		// send_len -= sizeof(UniformProtocolTlvBodyHead_t);
	}
	else if (write_ctx->meta_send)
	{

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
	fc41d->priv_data->socket.send_flag = 2;

	// get socket ip, port
	char *ip = fc41d->priv_data->socket.sock[socket_id].remote_ip;
	uint16_t port = fc41d->priv_data->socket.sock[socket_id].remote_port;
	//snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_SEND, socket_id, send_len, "");
	snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_SEND_STR, socket_id, send_len, tmp_ptr,
			 ip?ip:"255.255.255.255", port==0?DEFAULT_UDP_PORT:port);
	if (fc41d->priv_data->socket.sock[socket_id].type == DEV_FC41D_SOCKET_SERVER_UDP)
	{
		snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_SEND_STR, socket_id, send_len, tmp_ptr,
				 ip?ip:"255.255.255.255", port==0?DEFAULT_UDP_PORT:port);
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));

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

	fc41d->task_head_del(fc41d);

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
	snprintf(cmd, sizeof(cmd), "%s", AT_FC41D_SOCKET_ACCEPT);

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadSocketCfgTcpTwCycle(DevFc41d_t *fc41d, const uint8_t *data, 
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

	fc41d->task_head_del(fc41d);

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

static int32_t _DevFc41dWriteSocketCfgTcpTwCycle(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), "%s", AT_FC41D_SOCKET_PORT_RELTYPE);
	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadSocketCfgDataType(DevFc41d_t *fc41d, const uint8_t *data, 
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

	fc41d->task_head_del(fc41d);

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

static int32_t _DevFc41dWriteSocketCfgDataType(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[64] = {0};
#if FC41D_SOCKET_USING_STR
	snprintf(cmd, sizeof(cmd), "%s", AT_FC41D_SOCKET_DATA_TYPE_STR);
#else 
	snprintf(cmd, sizeof(cmd), "%s", AT_FC41D_SOCKET_DATA_TYPE_BIN);
#endif 
	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadSocketClose(DevFc41d_t *fc41d, int32_t socket_id, 
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

	fc41d->task_head_del(fc41d);

	if (socket_id >= 0
		&& socket_id < sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]))
	{
		fc41d->priv_data->socket.sock[socket_id].init_type = DEV_FC41D_SOCKET_NONE;
		fc41d->priv_data->socket.sock[socket_id].type = DEV_FC41D_SOCKET_NONE;
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

static int32_t _DevFc41dWriteSocketClose(DevFc41d_t *fc41d, int32_t socket_id)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_CLOSE, socket_id);
	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	fc41d->priv_data->socket.sock[socket_id].init_type = DEV_FC41D_SOCKET_NONE;
	fc41d->priv_data->socket.sock[socket_id].type = DEV_FC41D_SOCKET_NONE;
	return 0;
}

static int32_t _DevFc41dReadSocketOpen(DevFc41d_t *fc41d, const uint8_t *data, 
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
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->task_head_del(fc41d);
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_WRITE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;
		return read;
	}

	char *find2 = strstr((const char *)data, AT_FC41D_SOCKET_OPEN_RET);
	if (!find2)
	{
		return read;
	}

	fc41d->task_head_del(fc41d);
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);

	find2 += strlen(AT_FC41D_SOCKET_OPEN_RET);
	char tmp_str[2][8] = {0};
	int32_t socket_id = -1, errcode = 0, i = 0, j = strlen(find2), k = 0, m = 0;
	for (i = 0; i < j; ++i)
	{
		if (find2[i] == ',')
		{
			m = 0;
			++k;
			continue;
		}
		else if (find2[i] == 0x0D || find2[i] == 0x0A)
		{
			break;
		}
		tmp_str[k][m++] = find2[i];
	}

	socket_id = atoi(tmp_str[0]);
	errcode = atoi(tmp_str[1]);
	if (errcode == 0)
	{
		fc41d->priv_data->socket.sock[socket_id].type = 
			fc41d->priv_data->socket.sock[socket_id].init_type;
		// 检测socket是否是CT广播
		if (fc41d->priv_data->socket.sock[socket_id].type == DEV_FC41D_SOCKET_SERVER_UDP
			&& fc41d->priv_data->socket.sock[socket_id].local_port 
			== fc41d->priv_data->ct.broadcast_port)
		{
			fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_BC;
			fc41d->priv_data->ct.socket_id = socket_id;
		}
		else if (fc41d->priv_data->socket.sock[socket_id].type == DEV_FC41D_SOCKET_CLIENT_UDP
			&& fc41d->priv_data->socket.sock[socket_id].remote_port
			== fc41d->priv_data->ct.remote_port)
		{
			fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_LISTEN;
		}

		find = strstr(find2, "\r\n");
		if (find)
		{
			find[1] = 0;
		}
		read = strlen((const char *)data)+1;
		return read;
	}
	else
	{

		int32_t j = sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]);
		if (socket_id >= 0 && socket_id < j)
		{
			fc41d->priv_data->socket.sock[socket_id].init_type = DEV_FC41D_SOCKET_CLOSING;
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
									FC41D_PROCESS_SOCKET_STATUS_CLOSE,
									DEV_FC41D_TASK_STAT_WRITE, socket_id);
		}
	}

	return read;
}

static int32_t _DevFc41dWriteSocketOpen(DevFc41d_t *fc41d, int32_t socket_id)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (fc41d->priv_data->socket.mtu <= 0)
	{
		fc41d->priv_data->socket.mtu = FC41D_SOCKET_MTU;
	}

	char cmd[64] = {0};
	char server_type[16] = {"TCP LISTENER"};
	switch (fc41d->priv_data->socket.sock[socket_id].init_type)
	{
	case DEV_FC41D_SOCKET_SERVER_TCP:
		snprintf(server_type, sizeof(server_type), "%s", "TCP LISTENER");
		break;
	case DEV_FC41D_SOCKET_CLIENT_TCP:
		snprintf(server_type, sizeof(server_type), "%s", "TCP");
		break;
	case DEV_FC41D_SOCKET_SERVER_UDP:
		snprintf(server_type, sizeof(server_type), "%s", "UDP SERVICE");
		break;
	case DEV_FC41D_SOCKET_CLIENT_UDP:
		snprintf(server_type, sizeof(server_type), "%s", "UDP");
		break;
	}
	snprintf(cmd, sizeof(cmd), AT_FC41D_SOCKET_OPEN, socket_id, server_type, 
			 fc41d->priv_data->socket.sock[socket_id].remote_ip,
			 fc41d->priv_data->socket.sock[socket_id].remote_port,
			 fc41d->priv_data->socket.sock[socket_id].local_port);

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

int32_t _DevFc41dReadSocket(DevFc41d_t *fc41d, int32_t socket_id, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dSocketStatusGet(fc41d, 0);
	switch (status) 
	{
	case FC41D_PROCESS_SOCKET_STATUS_CLOSE:
		return _DevFc41dReadSocketClose(fc41d, socket_id, data, len);
	case FC41D_PROCESS_SOCKET_STATUS_OPEN:
		return _DevFc41dReadSocketOpen(fc41d, data, len);
	case FC41D_PROCESS_SOCKET_STATUS_CFG_ACCEPT:
		return _DevFc41dReadTcpServerCfgAccept(fc41d, data, len);
	case FC41D_PROCESS_SOCKET_STATUS_CFG_PORT_RELTYPE:
		return _DevFc41dReadSocketCfgTcpTwCycle(fc41d, data, len);
	case FC41D_PROCESS_SOCKET_STATUS_CFG_DATA_TYPE:
		return _DevFc41dReadSocketCfgDataType(fc41d, data, len);
	case FC41D_PROCESS_SOCKET_STATUS_SEND_LEN:
		return _DevFc41dReadSocketSendLen(fc41d, data, len, status);
	case FC41D_PROCESS_SOCKET_STATUS_SEND_DATA:
		return _DevFc41dReadSocketSendData(fc41d, data, len, status);
	case FC41D_PROCESS_SOCKET_STATUS_SEND_NORMAL_DATA:
		return _DevFc41dReadSocketSendNormalData(fc41d, data, len, status);
	}

	return 0;
}

int32_t _DevFc41dWriteSocket(DevFc41d_t *fc41d, int32_t socket_id)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dSocketStatusGet(fc41d, socket_id);
	switch (status) 
	{
	case FC41D_PROCESS_SOCKET_STATUS_CLOSE:
		_DevFc41dWriteSocketClose(fc41d, socket_id);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_OPEN:
		_DevFc41dWriteSocketOpen(fc41d, socket_id);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_CFG_ACCEPT:
		_DevFc41dWriteTcpServerCfgAccept(fc41d);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_CFG_PORT_RELTYPE:
		_DevFc41dWriteSocketCfgTcpTwCycle(fc41d);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_CFG_DATA_TYPE:
		_DevFc41dWriteSocketCfgDataType(fc41d);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_SEND_LEN:
		_DevFc41dWriteSocketSendLen(fc41d, socket_id);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_SEND_DATA:
		_DevFc41dWriteSocketSendData(fc41d, status);
		break;
	case FC41D_PROCESS_SOCKET_STATUS_SEND_NORMAL_DATA:
		_DevFc41dWriteSocketSendNormalData(fc41d, socket_id);
		break;
	}

	return 0;
}

int32_t _DevFc41dUrcSocket(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	int32_t ret = 0;
	int32_t socket_id = -1;

	char *find = strstr(data, AT_FC41D_TCP_S_URC_HEAD);

	if (strncmp(data, AT_FC41D_TCP_S_URC_HEAD, strlen(AT_FC41D_TCP_S_URC_HEAD))) 
	{
		return ret;
	}
	const char *p = &data[strlen(AT_FC41D_TCP_S_URC_HEAD)-1];

	find = strstr(p, AT_FC41D_TCP_S_URC_RECV);
	if (find)
	{
		// +QIURC: "recv",0,201,"192.168.1.103",50810, \r\n <data>
		// +QIURC: "recv",0,201, \r\n <data> 单播
		find += strlen(AT_FC41D_TCP_S_URC_RECV);
		int32_t i = 0, j = strlen(find), k = 0, m = 0;
		char *tmp_recv[6] = { NULL };
		tmp_recv[k++] = find;
		for (i = 0; i < j; ++i)
		{
			if (find[i] == 0x0D && find[i+1] == 0x0A)
			{
				find[i] = 0;
				i += 2;
				tmp_recv[k++] = &find[i];
				break;
			}
			if (find[i] == ',')
			{
				find[i] = 0;
				tmp_recv[k++] = &find[i+1];
				continue;
			}
		}
		find[i-1] = 0;

		socket_id = atoi(tmp_recv[0]);
		int32_t data_len = atoi(tmp_recv[1]);
		char *remote_ip = tmp_recv[2];
		char *remote_port = tmp_recv[3];
		char *remote_data = tmp_recv[5];
		if (remote_data == NULL)
		{
			remote_data = tmp_recv[4];
		}

		switch (fc41d->priv_data->socket.sock[socket_id].type)
		{
		case DEV_FC41D_SOCKET_SERVER_TCP:
			break;
		case DEV_FC41D_SOCKET_CLIENT_TCP:
			remote_data = tmp_recv[2];
			break;
		case DEV_FC41D_SOCKET_SERVER_UDP:
			break;
		case DEV_FC41D_SOCKET_CLIENT_UDP:
			remote_data = tmp_recv[2];
			break;
		}

		find += i;
		char *find2 = strstr(remote_data, "\r\n");
		if (!find2)
		{
			ret = (int32_t)(find - data);
		}
		else
		{
			ret = (int32_t)(find2 - data + 2);
			find2[0] = 0;
		}

		return ret;
	}


	find = strstr(p, AT_FC41D_TCP_S_URC_CLOSE);
	if (find)
	{
		find += strlen(AT_FC41D_TCP_S_URC_CLOSE);
		char *find2 = strstr(find, "\r\n");
		if (!find2) 
		{
			ret = (int32_t)(find - data);
		}
		else
		{
			find2[0] = 0;
			ret = find2 - data + 2;
			socket_id = atoi(find);

			if (socket_id >= 0
				&& socket_id 
				< sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]))
			{
				memset(&fc41d->priv_data->socket.sock[socket_id], 0, 
					   sizeof(fc41d->priv_data->socket.sock[socket_id]));
			}

		}
		return ret;
	}

	return ret;
}

int32_t DevFc41dShellyCtProcess(DevFc41d_t *fc41d)
{
    
#if 0
	static int32_t shelly_ct_req_id[3] = {0};
	static int32_t socket_id = -1;
	static uint32_t req_time = 0;
	static uint32_t req_time2 = 0;
	uint32_t cur_time = 0;
	int32_t req_flag = 0;
	static uint32_t retry_count = 0;

	//return -1;
	if (fc41d->priv_data->http.ota_fc41d)
	{
		return -1;
	}
	cur_time = tim_get_systime();
	if (fc41d->priv_data->stat != 2
		|| fc41d->priv_data->wifi.info.station.got_ip_stat != WIFI_CONN_STAT_CONN
			|| fc41d->priv_data->wifi.info.station.conn_stat != WIFI_CONN_STAT_CONN)
    {
        fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_NONE;
        return -3;
    }

//     if ( fc41d->priv_data->ble.conn
// 		|| fc41d->priv_data->upgrade.flag > COLLECTOR_UPGRADE_NONE)
	if (fc41d->priv_data->upgrade.flag > COLLECTOR_UPGRADE_NONE)
	{
		return -4;
	}
        
	if (fc41d->priv_data->ct.status < EN_SHELLY_CT_STATUS_BC_INIT)
	{
		_DevFc41dShellyCtDataReset(fc41d);
		if (cur_time - req_time2 > 10*1000)
		{
			req_flag = 1;
			req_time2 = tim_get_systime();
		}

		if (req_flag == 0)
		{
			return -2;
		}

		_DevFc41dCloseAllSocket(fc41d);

		gShellyCtReq = -1;
		shelly_ct_req_id[0] = 0;
		shelly_ct_req_id[1] = 0;
		fc41d->priv_data->ct.socket_id = -1;
		if (fc41d->priv_data->ct.broadcast_port == 0)
		{
			fc41d->priv_data->ct.broadcast_port = SHELLEY_CT_BC_PORT;
		}
		fc41d->priv_data->ct.remote_port = 0;
		snprintf(fc41d->priv_data->ct.ip, sizeof(fc41d->priv_data->ct.ip), "%s", "0.0.0.0");
		int32_t socket_id = _DevFc41dSocketGetUnusedSocket(fc41d);
		if (socket_id != 0)
		{
			return 0;
		}
		if (_DevFc41dSocketOpen(fc41d, DEV_FC41D_SOCKET_SERVER_UDP, 
							fc41d->priv_data->ct.ip, 
							fc41d->priv_data->ct.remote_port, 
							fc41d->priv_data->ct.broadcast_port) == 0)
		{
			fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_BC_INIT;
		}
	}
	else if (fc41d->priv_data->ct.status == EN_SHELLY_CT_STATUS_BC_INIT)
	{
		_DevFc41dShellyCtDataReset(fc41d);
		if (cur_time - req_time2 > 30*1000)
		{
			req_flag = 1;
			req_time2 = tim_get_systime();
		}

		if (req_flag == 0)
		{
			return -2;
		}

		_DevFc41dCloseAllSocket(fc41d);
		fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_NONE;

		return 0;
	}
	else if (fc41d->priv_data->ct.status == EN_SHELLY_CT_STATUS_GOT_IP)
	{
		_DevFc41dShellyCtDataReset(fc41d);
		if (cur_time - req_time2 > 10*1000)
		{
			req_flag = 1;
			req_time2 = tim_get_systime();
		}

		if (req_flag == 0)
		{
			return -2;
		}

		gShellyCtReq = 0;
		shelly_ct_req_id[0] = 0;
		shelly_ct_req_id[1] = 0;
		// 发起UDP单播访问
		socket_id = _DevFc41dSocketGetUnusedSocket(fc41d);
		if (socket_id != 0)
		{
			// 证明广播还未完全关闭需要等待一段时间
			_DevFc41dSocketClose(fc41d, socket_id);
			return -3;
		}
		fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_START;

		return 0;
	}
	else if (fc41d->priv_data->ct.status == EN_SHELLY_CT_STATUS_START)
	{
		_DevFc41dShellyCtDataReset(fc41d);
		if (cur_time - req_time2 > 10*1000)
		{
			req_flag = 1;
			req_time2 = tim_get_systime();
		}

		if (req_flag == 0)
		{
			return -2;
		}

		socket_id = _DevFc41dSocketGetUnusedSocket(fc41d);
		if (socket_id != 0)
		{
			// 证明广播还未完全关闭需要等待一段时间
			//_DevFc41dSocketClose(fc41d, socket_id);
			return -3;
		}

		gShellyCtReq = 1;
		shelly_ct_req_id[0] = 0;
		shelly_ct_req_id[1] = 0;
		_DevFc41dSocketOpen(fc41d, DEV_FC41D_SOCKET_CLIENT_UDP, 
							fc41d->priv_data->ct.ip, 
							fc41d->priv_data->ct.remote_port, 
							fc41d->priv_data->ct.local_port);
	}
	else if (fc41d->priv_data->ct.status > EN_SHELLY_CT_STATUS_START)
	{
		if (socket_id < 0 || socket_id 
			> sizeof(fc41d->priv_data->socket.sock)/sizeof(fc41d->priv_data->socket.sock[0]))
		{
			return -3;
		}

		if (fc41d->priv_data->socket.sock[socket_id].type == DEV_FC41D_SOCKET_NONE)
		{
			fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_GOT_IP;
			return -4;
		}

		if (req_time == 0)
		{
			req_flag = 1;
		}
		else if (cur_time - req_time > 10*1000)
		{
			req_flag = 1;
			if (fc41d->priv_data->ble.conn && cur_time - req_time < 20*1000)
			{
				req_flag = 0;
			}
		}

		if (fc41d->priv_data->write_stat.status != DEV_FC41D_WRITE_NONE)
		{
			req_flag = 0;
		}

		if (req_flag == 0)
		{
			return 0;
		}

		if (shelly_ct_req_id[0] == 0)
		{
			shelly_ct_req_id[0] = gShellyCtReq;
		}

		if (shelly_ct_req_id[0] == gShellyCtReq)
		{
			++shelly_ct_req_id[1];
		}
		else 
		{
			shelly_ct_req_id[1] = 0;
		}

		if (shelly_ct_req_id[2] < shelly_ct_req_id[1])
		{
			// 用于统计udp收数据失败最大次数
			shelly_ct_req_id[2] = shelly_ct_req_id[1];
		}

		if (shelly_ct_req_id[1] > 16)
		{
			// 5次都没有请求到数据, 重置socket
			fc41d->priv_data->ct.status = EN_SHELLY_CT_STATUS_NONE;
			_DevFc41dCloseAllSocket(fc41d);
			gShellyCtReq = 0;
			++retry_count;
			return -5;
		}

		shelly_ct_req_id[0] = gShellyCtReq;

		req_time = tim_get_systime();
		// 主动请求数据
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_SOCKET;

		DevFc41dWriteCtx_t write_ctx = {0};
		DevFc41dWriteCtx_t *write_ctx_ptr = NULL;
		UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx.data);

		// clear tx buffer
		rbuffer_clear(&fc41d->priv_data->rb_write.ctx);

		write_ctx.meta_send = 1;
		write_ctx.finish = 1;

		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));
		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)uniform, sizeof(UniformProtocolHead_t)
							- sizeof(UniformProtocolBodyHead_t));
		uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		write_ctx_ptr = (DevFc41dWriteCtx_t *)ptr;
		UniformProtocolBodyHead_t *body = &uniform->body;
		body->tlv.err = 0;
		body->tlv.data_len = 0;

		fc41d->priv_data->ct.json_str = gShellyCtJsonStr;

		// example:
		// {"id":1,"src":"shellyemg3-8cbfea96d354","result":{"ble":{},"bthome":{"errors":["observer_disabled"]},"cloud":{"connected":true},"em1:0":{"id":0,"current":0.020,"voltage":231.0,"act_power":0.0,"aprt_power":4.7,"pf":0.00,"freq":50.0,"calibration":"factory"},"em1:1":{"id":1,"current":0.020,"voltage":231.0,"act_power":0.0,"aprt_power":4.7,"pf":0.00,"freq":50.0,"calibration":"factory"},"em1data:0":{"id":0,"total_act_energy":0.01,"total_act_ret_energy":0.29},"em1data:1":{"id":1,"total_act_energy":0.00,"total_act_ret_energy":0.00},"mqtt":{"connected":false},"switch:0":{"id":0, "source":"init", "output":false,"temperature":{"tC":50.3, "tF":122.6}},"sys":{"mac":"8CBFEA96D354","restart_required":false,"time":"11:10","unixtime":1647141041,"uptime":185537,"ram_size":259220,"ram_free":85744,"fs_size":786432,"fs_free":315392,"cfg_rev":23,"kvs_rev":0,"schedule_rev":0,"webhook_rev":0,"available_updates":{},"reset_reason":1},"wifi":{"sta_ip":"192.168.1.103","status":"got ip","ssid":"NovaStar_Energy","rssi":-57},"ws":{"connected":false}}}
		char dest_ptr[128] = {0};
		const char *dest_ptr2 = {"{\"id\":%d,\"method\":\"Shelly.GetStatus\"}"};
		snprintf(dest_ptr, sizeof(dest_ptr), dest_ptr2, tim_get_systime());
		body->tlv.data_len = strlen(dest_ptr);

		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
							(const uint8_t *)&body->tlv, 
							sizeof(UniformProtocolBodyHead_t));

		rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx,
							(const uint8_t *)dest_ptr, 
							body->tlv.data_len);

		if (write_ctx.finish)
		{
#if FC41D_SOCKET_USING_STR
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
									FC41D_PROCESS_SOCKET_STATUS_SEND_NORMAL_DATA,
									DEV_FC41D_TASK_STAT_WRITE, 
									fc41d->priv_data->ct.socket_id);
#else 
			fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_SOCKET, 
									FC41D_PROCESS_SOCKET_STATUS_SEND_LEN,
									DEV_FC41D_TASK_STAT_WRITE, 
									fc41d->priv_data->ct.socket_id);
#endif 
		}
		return 10;
	}
#endif 

	return 0;
}


