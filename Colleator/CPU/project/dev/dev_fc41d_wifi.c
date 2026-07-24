#include "dev_fc41d_common.h"

// AT WIFI
#define AT_FC41D_STATION_OFF1				"AT+QSTAAPINFODEF=hellow,12345678\r\n"
#define AT_FC41D_STATION_OFF				"AT+QSTASTOP\r\n"
#define AT_FC41D_WIFI_STATION_WSCAN			"AT+QWSCAN\r\n"				// 搜索AP
#define AT_FC41D_WIFI_STATION_CONN1			"AT+QSTAAPINFODEF=" 		// 连接AP
#define AT_FC41D_WIFI_STATION_CONN			"AT+QSTAAPINFO="			// 连接AP
#define AT_FC41D_WIFI_STATION_CONN_STAT		"AT+QGETWIFISTATE\r\n" 		// 查询连接WIFI状态
#define AT_FC41D_WIFI_STATION_STAT			"AT+QGETWIFISTATE\r\n"		// 查询连接AP的信息
#define AT_FC41D_WIFI_STATION_STAT_RET		"+QGETWIFISTATE:"			// 查询连接AP的信息
#define AT_FC41D_WIFI_STATION_GET_IP		"AT+QGETIP=station\r\n"		// 查询Station模式获取的IP
#define AT_FC41D_WIFI_STATION_GET_IP_RET	"+QGETIP:"		// 查询Station模式获取的IP

// URC wifi
#define AT_FC41D_WIFI_URC_STAT_HEAD					"+QSTASTAT:"
#define AT_FC41D_WIFI_URC_STAT_GOT_IP				"GOT_IP"
#define AT_FC41D_WIFI_URC_STAT_SCAN_NO_AP			"SCAN_NO_AP"
#define AT_FC41D_WIFI_URC_STAT_WLAN_CONN			"WLAN_CONNECTED"
#define AT_FC41D_WIFI_URC_STAT_WLAN_DISCONN			"WLAN_DISCONNECTED"

#define AT_FC41D_WIFI_URC_WSCAN						"+QWSCAN:"

#define AT_FC41D_WIFI_URC_GET_WIFI_STAT_HEAD		"+QGETWIFISTATE:"
#define AT_FC41D_WIFI_URC_GET_WIFI_STAT_SSID		"ssid="
#define AT_FC41D_WIFI_URC_GET_WIFI_STAT_BSSID		"bssid="
#define AT_FC41D_WIFI_URC_GET_WIFI_STAT_RSSI		"rssi="
#define AT_FC41D_WIFI_URC_GET_WIFI_STAT_CHANNEL		"channel="
#define AT_FC41D_WIFI_URC_GET_WIFI_STAT_CIPHER		"cipher_type="

extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);
extern DevFc41dTask_t* _DevFc41dTaskGetHeadPtr(DevFc41d_t *fc41d);

static int32_t _DevFc41dWifiStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_WIFI_STATUS_INIT
			:fc41d->priv_data->wifi.info.station.status);
}

int32_t _DevFc41dWifiStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->wifi.info.station.status = status;
	return 0;
}

static int32_t _DevFc41dReadWifiWscan(DevFc41d_t *fc41d, const uint8_t *data, 
											   int32_t len)
{
	int32_t ret = 0;

	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{
		return ret;
	}

	// not enough
	uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
	DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
	int32_t tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
	int32_t wscan_total_len = rbuffer_length(&fc41d->priv_data->rb_read.ctx);
	UniformProtocolHead_t *phead = (UniformProtocolHead_t *)(write_ctx->data);

	// 去掉多余字符
	char wscan_buf[1024] = {0};
	int32_t i = 0, j = 0, m = len - strlen(AT_FC41D_WIFI_URC_WSCAN);
	char *p = (char *)data;
	for (; i < m && i < sizeof(wscan_buf); ++i, ++p) 
	{
		if (p[0] == '\n' && i > len - strlen(AT_FC41D_WIFI_URC_WSCAN)) 
		{
			// not enough next loop process
			break;
		}

		if (p[0] == '+'
			&& (p[1] == 'Q' || p[1] == 'q')
			&& (p[2] == 'W' || p[2] == 'w')
			&& (p[3] == 'S' || p[3] == 's')
			&& (p[4] == 'C' || p[4] == 'c')
			&& (p[5] == 'A' || p[5] == 'a')
			&& (p[6] == 'N' || p[6] == 'n')
			&& p[7] == ':') 
		{
			i += 7;
			p += 7;
			write_ctx->split_idx[0] = 1;
			write_ctx->split_idx[1] = 0;
			continue;
		}
		if(data[i] != '"' && data[i] != '\n')
		{
			wscan_buf[j++] = data[i];
			++write_ctx->split_idx[1];
			continue;
		}

		if(data[i] == '\n')
		{
			wscan_buf[j++] = data[i];
			write_ctx->split_idx[0] = 2;
			++write_ctx->split_idx[1];
			continue;
		}
	}

	if (i < len) 
	{
		ret = i;
	}

	tx_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
							(const uint8_t *)wscan_buf, j);

	if (wscan_total_len == len) 
	{
		// complete
		if (write_ctx->split_idx[0] != 2)
		{
			// maybe last data not enough
			tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
			tx_len = tx_len - write_ctx->split_idx[1];
			rbuffer_trim_idx(&fc41d->priv_data->rb_write.ctx, tx_len, write_ctx->split_idx[1]);
		}
		// rbuffer_trim_bytes(&fc41d->priv_data->rb_write.ctx, sizeof(uint16_t));
		rbuffer_write_byte(&fc41d->priv_data->rb_write.ctx, 0x00);
		tx_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
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

static int32_t _DevFc41dWriteWifiWscan(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_WIFI_STATION_WSCAN, 
				 strlen(AT_FC41D_WIFI_STATION_WSCAN));
	return 0;
}

static int32_t _DevFc41dReadWifiStat(DevFc41d_t *fc41d, const uint8_t *data, 
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

	if (status == FC41D_PROCESS_WIFI_STATION_STAT_UPDATE)
	{
		fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->task_head_del(fc41d);

		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		find = strstr((const char *)data, AT_FC41D_WIFI_STATION_STAT_RET);
		if (find == NULL)
		{
			return read;
		}

		find += strlen(AT_FC41D_WIFI_STATION_STAT_RET);
		char tmp[64] = {0};

		for (int32_t i = 0, j = 0, k = 0; *find != 0 && i < 6; ++find)
		{
			if (*find == '=') 
			{
				j = 1;
				continue;
			} 

			if (*find == ',' || *find == '\r' || *find == '\n') 
			{
				j = 2;
				++i;
			}

			if (j == 1) 
			{
				tmp[k++] = *find;
				continue;
			} 
			else if (j == 2) 
			{
                j = 0;
                tmp[k] = '\0';
                k = 0;
				switch (i) 
				{
				case 1:	// ssid
					{
					memset(fc41d->priv_data->wifi.info.station.ssid, 0, 
						   sizeof(fc41d->priv_data->wifi.info.station.ssid));
					memcpy(fc41d->priv_data->wifi.info.station.ssid, tmp, 
						   sizeof(fc41d->priv_data->wifi.info.station.ssid) 
						   < strlen(tmp)?
						   sizeof(fc41d->priv_data->wifi.info.station.ssid)
						   :strlen(tmp));
					break;
					}
				case 2:	// bssid
					{
					memcpy(fc41d->priv_data->wifi.info.station.bssid, tmp, 
						   sizeof(fc41d->priv_data->wifi.info.station.bssid) 
						   < strlen(tmp)?
						   sizeof(fc41d->priv_data->wifi.info.station.bssid)
						   :strlen(tmp));
					break;
					}
				case 3:	// rssi
					{
					fc41d->priv_data->wifi.info.station.rssi = atoi(tmp);
					break;
					}
				case 4:	// channel
					{
					fc41d->priv_data->wifi.info.station.channel = atoi(tmp);
					break;
					}
				case 5:	// cipher_type
					{
						memcpy(fc41d->priv_data->wifi.info.station.cipher_type, tmp, 
							   sizeof(fc41d->priv_data->wifi.info.station.cipher_type) 
							   < strlen(tmp)?
							   sizeof(fc41d->priv_data->wifi.info.station.cipher_type)
							   :strlen(tmp));
					break;
					}
				}
			}
		}

		if (status != FC41D_PROCESS_WIFI_STATION_STAT_UPDATE)
		{
			snprintf(tmp, sizeof(tmp), "%s,%d", 
					 (const char *)fc41d->priv_data->wifi.info.station.ssid,
					 fc41d->priv_data->wifi.info.station.rssi);
			int32_t tmp_len = strlen(tmp);
			// write result to txbuffer
			rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)tmp, 
								strlen(tmp));
		}

		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		// write result to txbuffer
		if (status != FC41D_PROCESS_WIFI_STATION_STAT_UPDATE)
		{
			rbuffer_write_byte(&fc41d->priv_data->rb_write.ctx, 0x00);
		}

		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteWifiStat(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_WIFI_STATION_STAT, 
				 strlen(AT_FC41D_WIFI_STATION_STAT));
	return 0;
}

static int32_t _DevFc41dReadWifiStatStopDef(DevFc41d_t *fc41d, const uint8_t *data, 
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

static int32_t _DevFc41dWriteWifiStatStopDef(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_STATION_OFF1, strlen(AT_FC41D_STATION_OFF1));
	return 0;
}

static int32_t _DevFc41dReadWifiStatStop(DevFc41d_t *fc41d, const uint8_t *data, 
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

static int32_t _DevFc41dWriteWifiStatStop(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_STATION_OFF, strlen(AT_FC41D_STATION_OFF));
	return 0;
}

static int32_t _DevFc41dReadWifiConn(DevFc41d_t *fc41d, const uint8_t *data, 
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

static int32_t _DevFc41dWriteWifiConn(DevFc41d_t *fc41d)
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
	snprintf(cmd, sizeof(cmd), "%s%s%s%s\r\n", AT_FC41D_WIFI_STATION_CONN,
			 (char*)fc41d->priv_data->wifi.info.station.ssid, 
			 fc41d->priv_data->wifi.info.station.pwd[0]==0?"":",", 
			 (char*)fc41d->priv_data->wifi.info.station.pwd);

	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadWifiConnStat(DevFc41d_t *fc41d, const uint8_t *data, 
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

	if (fc41d->priv_data->wifi.info.station.got_ip_stat == 1) 
	{
		fc41d->priv_data->wifi.info.station.net_setting_flag = 0;
		fc41d->priv_data->wifi.info.station.conn_stat = 1;
		// urc already recv status
		int32_t uniform_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		if (uniform_len > 0)
		{
			uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
			UniformProtocolHead_t *uniform = 
				(UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
			uniform->body.tlv.err = FC41D_WIFI_STATION_ERR_NONE;
		}

		fc41d->task_head_del(fc41d);
		return len;
	}

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->priv_data->wifi.info.station.conn_stat = WIFI_CONN_STAT_CONN;
		find = strstr((const char *)data, AT_FC41D_WIFI_STATION_STAT_RET);
		if (find != NULL)
		{
			fc41d->priv_data->wifi.info.station.got_ip_stat = WIFI_CONN_STAT_CONN;

			find += strlen(AT_FC41D_WIFI_STATION_STAT_RET);
			char tmp[64] = {0};

			for (int32_t i = 0, j = 0, k = 0; *find != 0 && i < 6; ++find)
			{
				if (*find == '=') 
				{
					j = 1;
					continue;
				} 

				if (*find == ',' || *find == '\r' || *find == '\n') 
				{
					j = 2;
					++i;
				}

				if (j == 1) 
				{
					tmp[k++] = *find;
					continue;
				} 
				else if (j == 2) 
				{
					j = 0;
					tmp[k] = '\0';
					k = 0;
					switch (i) 
					{
					case 1:	// ssid
						{
							memset(fc41d->priv_data->wifi.info.station.ssid, 0, 
								   sizeof(fc41d->priv_data->wifi.info.station.ssid));
							memcpy(fc41d->priv_data->wifi.info.station.ssid, tmp, 
								   sizeof(fc41d->priv_data->wifi.info.station.ssid) 
								   < strlen(tmp)?
								   sizeof(fc41d->priv_data->wifi.info.station.ssid)
								   :strlen(tmp));
							break;
						}
					case 2:	// bssid
						{
							memcpy(fc41d->priv_data->wifi.info.station.bssid, tmp, 
								   sizeof(fc41d->priv_data->wifi.info.station.bssid) 
								   < strlen(tmp)?
								   sizeof(fc41d->priv_data->wifi.info.station.bssid)
								   :strlen(tmp));
							break;
						}
					case 3:	// rssi
						{
							fc41d->priv_data->wifi.info.station.rssi = atoi(tmp);
							break;
						}
					case 4:	// channel
						{
							fc41d->priv_data->wifi.info.station.channel = atoi(tmp);
							break;
						}
					case 5:	// cipher_type
						{
							memcpy(fc41d->priv_data->wifi.info.station.cipher_type, tmp, 
								   sizeof(fc41d->priv_data->wifi.info.station.cipher_type) 
								   < strlen(tmp)?
								   sizeof(fc41d->priv_data->wifi.info.station.cipher_type)
								   :strlen(tmp));
							break;
						}
					}
				}
			}
		}

		int32_t uniform_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		if (uniform_len > 0)
		{
			uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
			UniformProtocolHead_t *uniform = 
				(UniformProtocolHead_t *)(ptr+sizeof(DevFc41dWriteCtx_t));
			uniform->body.tlv.err = FC41D_WIFI_STATION_ERR_NONE;
		}

		fc41d->priv_data->wifi.info.station.net_setting_flag = 0;
		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->wifi.info.station.net_setting_flag = 0;
		fc41d->task_head_del(fc41d);
		// stop sta
		fc41d->priv_data->wifi.info.station.conn_stat = WIFI_CONN_STAT_DISCONN;

		int32_t uniform_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		if (uniform_len > 0)
		{
			uint8_t *ptr = (uint8_t *)rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
			DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
			UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
			int32_t ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
			uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
			memcpy(&ptr_len, trim_len, sizeof(int32_t));

			/*
            if ((uint32_t)(tim_get_systime() - (uint32_t)write_ctx->trim_send_len) > 8000)
			{
				uniform->body.tlv.err = FC41D_WIFI_STATION_ERR_CONN;
				fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_WIFI, 
									FC41D_PROCESS_WIFI_STATION_STOP, 
									DEV_FC41D_TASK_STAT_WRITE, 0);
			}
            else 
            {
                fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_WIFI, 
									FC41D_PROCESS_WIFI_STATION_CONN_STAT,
									DEV_FC41D_TASK_STAT_WRITE, 0);
            }
			*/
		}

		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteWifiConnStat(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_WIFI_STATION_CONN_STAT, 
				 strlen(AT_FC41D_WIFI_STATION_CONN_STAT));
	return 0;
}

static int32_t _DevFc41dReadWifiInit(DevFc41d_t *fc41d, const uint8_t *data, 
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

static int32_t _DevFc41dWriteWifiInit(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_STATION_OFF, strlen(AT_FC41D_STATION_OFF));
	return 0;
}

static int32_t _DevFc41dReadWifiGetIpDebug(DevFc41d_t *fc41d)
{
	DevFc41dTask_t *task = _DevFc41dTaskGetHeadPtr(fc41d);
	char buf[512] = {0};
	snprintf(buf, sizeof(buf), 
			 "type:%02X,SN:%s,dc|ac_sn:%s,firmware:%s,\r\n"
			 "fc41d_ver:%s\r\n"
			 "wifi:[\r\n"
			 "mac:%s,\r\n"
			 "ssid:%s,\r\n"
			 "pwd:%s,\r\n"
			 "conn:%d,\r\n"
			 "signal:%d,\r\n"
			 "got_ip:%d,\r\n"
			 "ip:%s,\r\n"
			 "gw:%s,\r\n"
			 "mask:%s,\r\n"
			 "dns:%s,\r\n"
			 "cipher_type:%s,\r\n"
			 "channel:%d,\r\n"
			 "],\r\n"
			 "mqtt:[%d:%s,type:%d,stat:%d,mtu:%d],\r\n"
			 "task:[size:%d,cur:%d,%d]", 
			 gCollector->type,
			 (const char *)gCollector->sn,
			 (const char *)gCollector->pcs_sn,
			 (const char *)gCollector->firmware,
			 (const char *)fc41d->priv_data->com_info.ver,
			 (const char *)fc41d->priv_data->wifi.info.station.bssid,
			 (const char *)fc41d->priv_data->wifi.info.station.ssid,
			 (const char *)fc41d->priv_data->wifi.info.station.pwd,
			 fc41d->priv_data->wifi.info.station.conn_stat,
			 fc41d->priv_data->wifi.info.station.rssi,
			 fc41d->priv_data->wifi.info.station.got_ip_stat,
			 (const char *)fc41d->priv_data->wifi.info.station.ip,
			 (const char *)fc41d->priv_data->wifi.info.station.gate,
			 (const char *)fc41d->priv_data->wifi.info.station.mask,
			 (const char *)fc41d->priv_data->wifi.info.station.dns,
			 (const char *)fc41d->priv_data->wifi.info.station.cipher_type,
			 fc41d->priv_data->wifi.info.station.channel,
			 gCollector->head.s_url_len,
			 (const char *)&gCollector->head.s_url[1],
			 //(const char *)fc41d->priv_data->mqtt.server_ip,
			 //(const char *)fc41d->priv_data->mqtt.server_port,
			 fc41d->priv_data->mqtt.server_type,
			 fc41d->priv_data->mqtt.conn_stat,
			 fc41d->priv_data->mqtt.mtu,
			 fc41d->priv_data->task_size,
			 task==NULL?-1:task->process_status,
			 task==NULL?-1:task->sub_status
				 );

			 int32_t buf_len = strlen(buf);
			 rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf, strlen(buf));
			 return 0;
}

static int32_t _DevFc41dReadWifiGetIpDebugGetWifiInfo(DevFc41d_t *fc41d)
{
	DevFc41dTask_t *task = _DevFc41dTaskGetHeadPtr(fc41d);
	char buf[512] = {0};
	snprintf(buf, sizeof(buf), 
			 "type:%02X,SN:%s,dc|ac_sn:%s,firmware:%s,\r\n"
			 "fc41d_ver:%s\r\n"
			 "wifi:[\r\n"
			 "mac:%s,\r\n"
			 "ssid:%s,\r\n"
			 "pwd:%s,\r\n"
			 "conn:%d,\r\n"
			 "signal:%d,\r\n"
			 "got_ip:%d,\r\n"
			 "ip:%s,\r\n"
			 "gw:%s,\r\n"
			 "mask:%s,\r\n"
			 "dns:%s,\r\n"
			 "cipher_type:%s,\r\n"
			 "channel:%d,\r\n"
			 "],\r\n"
			 "task:[size:%d,cur:%d,%d]", 
			 gCollector->type,
			 (const char *)gCollector->sn,
			 (const char *)gCollector->pcs_sn,
			 (const char *)gCollector->firmware,
			 (const char *)fc41d->priv_data->com_info.ver,
			 (const char *)fc41d->priv_data->wifi.info.station.bssid,
			 (const char *)fc41d->priv_data->wifi.info.station.ssid,
			 (const char *)fc41d->priv_data->wifi.info.station.pwd,
			 fc41d->priv_data->wifi.info.station.conn_stat,
			 fc41d->priv_data->wifi.info.station.rssi,
			 fc41d->priv_data->wifi.info.station.got_ip_stat,
			 (const char *)fc41d->priv_data->wifi.info.station.ip,
			 (const char *)fc41d->priv_data->wifi.info.station.gate,
			 (const char *)fc41d->priv_data->wifi.info.station.mask,
			 (const char *)fc41d->priv_data->wifi.info.station.dns,
			 (const char *)fc41d->priv_data->wifi.info.station.cipher_type,
			 fc41d->priv_data->wifi.info.station.channel,
			 fc41d->priv_data->task_size,
			 task==NULL?-1:task->process_status,
			 task==NULL?-1:task->sub_status
				 );

			 int32_t buf_len = strlen(buf);
			 rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf, strlen(buf));
			 return 0;
}

static int32_t _DevFc41dReadWifiGetIpDebugGetMqttInfo(DevFc41d_t *fc41d)
{
	DevFc41dTask_t *task = _DevFc41dTaskGetHeadPtr(fc41d);
	char buf[512] = {0};
	snprintf(buf, sizeof(buf), 
			 "type:%02X,SN:%s,dc|ac_sn:%s,fw:%s,\r\n"
			 "fc41d_ver:%s\r\n"
			 "wifi:[%s,%s,%s]\r\n"
			 "mqtt:[%d:%s,type:%d,stat:%d,mtu:%d,<%s,%s,%s,%s,%s>],\r\n"
			 "task:[size:%d,cur:%d,%d]", 
			 gCollector->type,
			 (const char *)gCollector->sn,
			 (const char *)gCollector->pcs_sn,
			 (const char *)gCollector->firmware,
			 (const char *)fc41d->priv_data->com_info.ver,
			 (const char *)fc41d->priv_data->wifi.info.station.bssid,
			 (const char *)fc41d->priv_data->wifi.info.station.ssid,
			 (const char *)fc41d->priv_data->wifi.info.station.pwd,
			 gCollector->head.s_url_len,
			 (const char *)&gCollector->head.s_url[1],
			 fc41d->priv_data->mqtt.server_type,
			 fc41d->priv_data->mqtt.conn_stat,
			 fc41d->priv_data->mqtt.mtu,
			 (const char *)fc41d->priv_data->mqtt.server_ip,
			 (const char *)fc41d->priv_data->mqtt.server_port,
			 (const char *)fc41d->priv_data->mqtt.clientname,
			 (const char *)fc41d->priv_data->mqtt.username,
			 (const char *)fc41d->priv_data->mqtt.passwd,
			 fc41d->priv_data->task_size,
			 task==NULL?-1:task->process_status,
			 task==NULL?-1:task->sub_status
			);

	int32_t buf_len = strlen(buf);
	rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, (const uint8_t *)buf, strlen(buf));
	return 0;
}

static int32_t _DevFc41dReadWifiGetIp(DevFc41d_t *fc41d, const uint8_t *data, 
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
        
        // 解析IP
		find = strstr((const char *)data, AT_FC41D_WIFI_STATION_GET_IP_RET);
		if (find)
		{
			find += strlen(AT_FC41D_WIFI_STATION_GET_IP_RET);
			char tmp[32] = {0};
			for (int32_t i = 0, j = 0, k = 0, m = 0; i < strlen(find); ++i)
			{
				if (find[i] == ',') 
				{
					tmp[k] = 0;
					switch (j)
					{
					case 0: // ip
						memcpy(fc41d->priv_data->wifi.info.station.ip, tmp, 
							   sizeof(fc41d->priv_data->wifi.info.station.ip));
						break;
					case 1: // gateway
						memcpy(fc41d->priv_data->wifi.info.station.gate, tmp, 
							   sizeof(fc41d->priv_data->wifi.info.station.gate));
						break;
					case 2: // mask
						memcpy(fc41d->priv_data->wifi.info.station.mask, tmp, 
							   sizeof(fc41d->priv_data->wifi.info.station.mask));
						break;
					case 3: // dns
						memcpy(fc41d->priv_data->wifi.info.station.dns, tmp, 
							   sizeof(fc41d->priv_data->wifi.info.station.dns));
						break;
					}

					++j;
					k = 0;
					m = 0;
					continue;
				}

				if (m == 0)
				{
					if (find[i] == ':')
					{
						m = 1;
					}
					continue;
				}
				else
				{
					tmp[k++] = find[i];
				}
			}
		}

		fc41d->task_head_del(fc41d);

		uint8_t *ptr = rbuffer_get_ptr(&fc41d->priv_data->rb_write.ctx);
		int32_t ptr_len = rbuffer_length(&fc41d->priv_data->rb_write.ctx);
		if (ptr_len >= sizeof(UniformProtocolHead_t))
		{
			DevFc41dWriteCtx_t *write_ctx = (DevFc41dWriteCtx_t *)ptr;
			uint8_t *trim_len = (uint8_t *)(&write_ctx->trim_send_len);
			memcpy(&ptr_len, trim_len, sizeof(int32_t));
			UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)(write_ctx->data);
			if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEBUG)
			{
				_DevFc41dReadWifiGetIpDebug(fc41d);
			}
			else if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEBUG_GET_WIFI_INFO)
			{
				_DevFc41dReadWifiGetIpDebugGetWifiInfo(fc41d);
			}
			if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEBUG_GET_MQTT_INFO)
			{
				_DevFc41dReadWifiGetIpDebugGetMqttInfo(fc41d);
			}
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

static int32_t _DevFc41dWriteWifiGetIp(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_WIFI_STATION_GET_IP, 
				 strlen(AT_FC41D_WIFI_STATION_GET_IP));
	return 0;
}

int32_t _DevFc41dReadWifi(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dWifiStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_WIFI_STATUS_INIT:
		return _DevFc41dReadWifiInit(fc41d, data, len);
	case FC41D_PROCESS_WIFI_STATION_WSCAN:
		return _DevFc41dReadWifiWscan(fc41d, data, len);
	case FC41D_PROCESS_WIFI_STATION_CONN:
		return _DevFc41dReadWifiConn(fc41d, data, len);
	case FC41D_PROCESS_WIFI_STATION_CONN_STAT:
		return _DevFc41dReadWifiConnStat(fc41d, data, len);
	case FC41D_PROCESS_WIFI_STATION_STAT:
	case FC41D_PROCESS_WIFI_STATION_STAT_UPDATE:
		return _DevFc41dReadWifiStat(fc41d, data, len, status);
	case FC41D_PROCESS_WIFI_STATION_STOP_DEF:
		return _DevFc41dReadWifiStatStopDef(fc41d, data, len);
	case FC41D_PROCESS_WIFI_STATION_STOP:
		return _DevFc41dReadWifiStatStop(fc41d, data, len);
	case FC41D_PROCESS_WIFI_STATION_GET_IP:
		return _DevFc41dReadWifiGetIp(fc41d, data, len);
	}

	return 0;
}

int32_t _DevFc41dWriteWifi(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dWifiStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_WIFI_STATUS_INIT:
		_DevFc41dWriteWifiInit(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_WSCAN:
		_DevFc41dWriteWifiWscan(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_CONN:
		_DevFc41dWriteWifiConn(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_CONN_STAT:
		_DevFc41dWriteWifiConnStat(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_STAT:
	case FC41D_PROCESS_WIFI_STATION_STAT_UPDATE:
		_DevFc41dWriteWifiStat(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_STOP_DEF:
		_DevFc41dWriteWifiStatStopDef(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_STOP:
		_DevFc41dWriteWifiStatStop(fc41d);
		break;
	case FC41D_PROCESS_WIFI_STATION_GET_IP:
		_DevFc41dWriteWifiGetIp(fc41d);
		break;
	}

	return 0;
}

int32_t _DevFc41dUrcWifiStat(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	int32_t ret = 0, i = 0;
	char *p = (char *)data;

	if (strncmp(data, AT_FC41D_WIFI_URC_STAT_HEAD, strlen(AT_FC41D_WIFI_URC_STAT_HEAD))) 
	{
		return ret;
	}
	ret += strlen(AT_FC41D_WIFI_URC_STAT_HEAD);
	p += ret;

	int32_t p_len = 0;

	while (ret < len) 
	{
		p_len = strlen(p);

		if (p_len > strlen(AT_FC41D_WIFI_URC_STAT_GOT_IP)
			&& strncmp(p, AT_FC41D_WIFI_URC_STAT_GOT_IP, 
					   strlen(AT_FC41D_WIFI_URC_STAT_GOT_IP)) == 0) 
		{
			fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_WIFI, 
									FC41D_PROCESS_WIFI_STATION_GET_IP,
									DEV_FC41D_TASK_STAT_WRITE, 0);
            fc41d->priv_data->wifi.info.station.got_ip_stat = WIFI_CONN_STAT_CONN;
			gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_CLOUD;
			i = strlen(AT_FC41D_WIFI_URC_STAT_GOT_IP);
			ret += i;
			p += i;
			break;
		}

		if (p_len > strlen(AT_FC41D_WIFI_URC_STAT_SCAN_NO_AP) 
			&& strncmp(p, AT_FC41D_WIFI_URC_STAT_SCAN_NO_AP, 
					   strlen(AT_FC41D_WIFI_URC_STAT_SCAN_NO_AP)) == 0) 
		{
            fc41d->priv_data->wifi.info.station.got_ip_stat = WIFI_CONN_STAT_DISCONN;
            fc41d->priv_data->mqtt.conn_stat = 0;
			gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_GRID;
			i = strlen(AT_FC41D_WIFI_URC_STAT_SCAN_NO_AP);
			ret += i;
			p += i;
			break;
		}

		if (p_len > strlen(AT_FC41D_WIFI_URC_STAT_WLAN_CONN)
			&& strncmp(p, AT_FC41D_WIFI_URC_STAT_WLAN_CONN, 
					  strlen(AT_FC41D_WIFI_URC_STAT_WLAN_CONN)) == 0) 
		{
            fc41d->priv_data->wifi.info.station.conn_stat = WIFI_CONN_STAT_CONN;
			gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_CLOUD;
			i = strlen(AT_FC41D_WIFI_URC_STAT_WLAN_CONN);
			ret += i;
			p += i;
			break;
		}

		if (p_len > strlen(AT_FC41D_WIFI_URC_STAT_WLAN_DISCONN) 
			&& strncmp(p, AT_FC41D_WIFI_URC_STAT_WLAN_DISCONN, 
					   strlen(AT_FC41D_WIFI_URC_STAT_WLAN_DISCONN)) == 0) 
		{
            fc41d->priv_data->wifi.info.station.conn_stat = WIFI_CONN_STAT_DISCONN;
            fc41d->priv_data->mqtt.conn_stat = 0;
			gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_GRID;
			i = strlen(AT_FC41D_WIFI_URC_STAT_WLAN_DISCONN);
			ret += i;
			p += i;
			break;
		}
        break;
	}

	return ret;
}

static int32_t _DevFc41dUrcWscan(DevFc41d_t *fc41d, const char *data, int32_t len)
{
    return 0;
	int32_t ret = 0;
	int32_t i = 0;
	char wscan_buf[128] = {0};
	char *p = (char *)data;

	if (strncmp(data, AT_FC41D_WIFI_URC_WSCAN, strlen(AT_FC41D_WIFI_URC_WSCAN))) 
	{
		return ret;
	}
	ret += strlen(AT_FC41D_WIFI_URC_WSCAN);
	p += ret;

	while (ret < len) 
	{
		if(*p != '"' && *p != '\n')
		{
			wscan_buf[i++] = *p;
		}
		else if(*p == '\n')
		{
			wscan_buf[i++] = *p;
			p += strlen(AT_FC41D_WIFI_URC_WSCAN)-1;
		}
		else if(*p == '"')
		{
		}

		++p;
	}

	return ret;
}

int32_t _DevFc41dWifiAutoConn(DevFc41d_t *fc41d)
{
	if (fc41d->priv_data->wifi.info.station.got_ip_stat != 0
		&& fc41d->priv_data->wifi.info.station.conn_stat != 0)
	{
		fc41d->priv_data->cfg.wifi_auto_conn_time = tim_get_systime();
		return -1;
	}

	if ((uint32_t)(tim_get_systime() - fc41d->priv_data->cfg.wifi_auto_conn_time) 
		< fc41d->priv_data->cfg.wifi_auto_conn_timeout)
	{
		return -2;
	}
    
	fc41d->priv_data->cfg.wifi_auto_conn_time = tim_get_systime();
    if (fc41d->priv_data->wifi.info.station.ssid[0] == 0)
	{
		return -3;
	}

	// reset wifi station status
	fc41d->priv_data->wifi.info.station.got_ip_stat = 0;
	fc41d->priv_data->wifi.info.station.conn_stat = 0;
	fc41d->priv_data->mqtt.conn_stat = 0;

	// add connect wifi, ble send task
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						  FC41D_PROCESS_WIFI_STATION_CONN,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						  FC41D_PROCESS_WIFI_STATION_CONN_STAT,
						  DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t DevFc41dUpdateWifiInfo(DevFc41d_t *fc41d)
{
	if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		return -1;
	}
	
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_WIFI;
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_WIFI, 
							FC41D_PROCESS_WIFI_STATION_STAT_UPDATE,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	return 0;
}

int8_t DevFc41dGetWifiStat(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return -1;
	}
	return (int8_t)fc41d->priv_data->wifi.info.station.conn_stat;
}
