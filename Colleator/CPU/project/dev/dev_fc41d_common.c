#include "dev_fc41d_common.h"
#include "dev_spiflash.h"
#include "pcs_op_megarevo.h"

static int32_t remote_upgrade_flag = 0;

static uint8_t _wireless_rx_buf[][WIRELESS_DATA_RX_SIZE] = {{0}};
static uint8_t _gFc41dCtx[FC41D_CTX_SIZE]= {0};
static uint8_t _gCommBuf[4096] = {0};
static int32_t _DevFc41dInitTask(DevFc41d_t *fc41d);

extern void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d);

// 外部引用
extern int32_t _DevFc41dUrcBle(DevFc41d_t *fc41d, const char *data, int32_t len);
extern int32_t _DevFc41dUrcWifiStat(DevFc41d_t *fc41d, const char *data, int32_t len);
extern int32_t _DevFc41dUrcMqtt(DevFc41d_t *fc41d, int8_t type, const char *data, int32_t len);
//extern int32_t _DevFc41dUrcSocket(DevFc41d_t *fc41d, const char *data, int32_t len);

int32_t _DevFc41dUrcHttp(DevFc41d_t *fc41d, const char *data, int32_t len);
extern int32_t _DevFc41dUrcNovaDebug(DevFc41d_t *fc41d, const char *data, int32_t len);
extern int32_t _DevFc41dUrcNova(DevFc41d_t *fc41d, int8_t type, const char *data, int32_t len);

extern int32_t _DevFc41dWifiAutoConn(DevFc41d_t *fc41d);
extern int32_t _DevFc41dCfgMqttAutoConn(DevFc41d_t *fc41d);

extern int32_t _DevFc41dReadBle(DevFc41d_t *fc41d, const uint8_t *data, int32_t len);
extern int32_t _DevFc41dReadWifi(DevFc41d_t *fc41d, const uint8_t *data, int32_t len);
extern int32_t _DevFc41dReadMqtt(DevFc41d_t *fc41d, const uint8_t *data, int32_t len);
extern int32_t _DevFc41dReadHttp(DevFc41d_t *fc41d, const uint8_t *data, int32_t len);
//extern int32_t _DevFc41dReadSocket(DevFc41d_t *fc41d, int32_t socket_id, const uint8_t *data, 
								   //int32_t len);
extern int32_t _DevFc41dReadNovaCmd(DevFc41d_t *fc41d, const uint8_t *data, int32_t len);

extern int32_t _DevFc41dWriteBle(DevFc41d_t *fc41d);
extern int32_t _DevFc41dWriteWifi(DevFc41d_t *fc41d);
extern int32_t _DevFc41dWriteMqtt(DevFc41d_t *fc41d);
extern int32_t _DevFc41dWriteHttp(DevFc41d_t *fc41d);
//extern int32_t _DevFc41dWriteSocket(DevFc41d_t *fc41d, int32_t socket_id);
extern int32_t _DevFc41dWriteNovaCmd(DevFc41d_t *fc41d);

extern int32_t _DevFc41dBleStatusSet(DevFc41d_t *fc41d, int32_t status);
extern int32_t _DevFc41dWifiStatusSet(DevFc41d_t *fc41d, int32_t status);
extern int32_t _DevFc41dMqttStatusSet(DevFc41d_t *fc41d, int32_t status);
extern int32_t _DevFc41dHttpStatusSet(DevFc41d_t *fc41d, int32_t status);
//extern int32_t _DevFc41dSocketStatusSet(DevFc41d_t *fc41d, int32_t status, int32_t socket_id);
extern int32_t _DevFc41dNovaCmdStatusSet(DevFc41d_t *fc41d, int32_t status);

extern int32_t _DevFc41dPubMqttResp(DevFc41d_t *fc41d, void *uniform, 
									int32_t pub_idx, int32_t cmd, uint16_t err, 
									const uint8_t *data, int32_t len);
extern int32_t _DevFc41dPubMqttMeta(DevFc41d_t *fc41d, int32_t pub_idx, 
										  const uint8_t *data, int32_t len);
extern int32_t _DevFc41dPubMqtt(DevFc41d_t *fc41d, int32_t pub_idx, 
										  int32_t cmd, uint16_t err,
										  const uint8_t *data, int32_t len);

extern int32_t _DevFc41dHttpReq(DevFc41d_t *fc41d, void *uniform, int32_t cmd, 
								uint16_t err, const uint8_t *data, int32_t len);
extern int32_t DevFc41dHttpGetContinue(DevFc41d_t *fc41d);

extern int32_t _DevFc41dHttpGetUpgrade2Ack(DevFc41d_t *fc41d, int8_t stat);
extern int32_t _DevFc41dUrcHttpResult(DevFc41d_t *fc41d, int8_t stat);


int32_t StrEncode(const char *src, char *dest, int32_t dest_size)
{

	return 0;
}

int32_t StrDecode(const char *src, char *dest, int32_t dest_size)
{

	return 0;
}

int32_t string_to_bytes(char *s, int l, char *d)
{
	int32_t ret = l/2;
    while(l--)
    {
        char* p = (char *)(s+l);
        char* p2 = (char *)(p-1);
        *(d+l/2) =
        ( (*p>'9'? *p+9 : *p) & 0x0f ) |
        ( (*p2>'9'? *p2+9 : *p2) << 4 );
        l--;
    }
	return ret;
}

void _DevFc41dTaskClean(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return;
	}

	DevFc41dTaskList_t *pnode = NULL;
	xlist_head_t *pos = NULL, *n = NULL;
	xlist_for_each_safe(pos, n, &fc41d->priv_data->task.l) 
	{
		pnode = xlist_entry(pos, DevFc41dTaskList_t, l);
        xlist_del(pos);
#if USE_TASK_HEAP
        free(pnode);
#else
		pnode->t.use = 0;
#endif 
		--fc41d->priv_data->task_size;
	}
	fc41d->priv_data->task_size = 0;
}

DevFc41dTask_t* _DevFc41dTaskGetHeadPtr(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return NULL;
	}

	xlist_head_t *pos = NULL, *n = NULL;
	DevFc41dTaskList_t *tmp = NULL;
	xlist_for_each_safe(pos, n, &fc41d->priv_data->task.l) 
	{
		tmp = xlist_entry(pos, DevFc41dTaskList_t, l);
		return &tmp->t;
	}

    return NULL;
}

static int32_t _DevFc41dTaskGetHead(DevFc41d_t *fc41d, DevFc41dTask_t *t)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	xlist_head_t *pos = NULL, *n = NULL;
	DevFc41dTaskList_t *tmp = NULL;
	xlist_for_each_safe(pos, n, &fc41d->priv_data->task.l) 
	{
		tmp = xlist_entry(pos, DevFc41dTaskList_t, l);
		*t = tmp->t;
		return 0;
	}

    return -1;
}

static DevFc41dTaskList_t* _DevFc41dTaskEmptyGet(DevFc41d_t *fc41d)
{
	DevFc41dTaskList_t *tmp = NULL;
	for (int32_t i = 0; i < FC41D_TASK_MAX_SIZE; ++i)
	{
		tmp = &fc41d->priv_data->tl[i];
		if (tmp->t.use == 0)
		{
			return tmp;
		}
	}
	return NULL;
}

static int32_t _DevFc41dTaskPop(DevFc41d_t *fc41d, DevFc41dTask_t *t)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	xlist_head_t *pos = NULL, *n = NULL;
	DevFc41dTaskList_t *tmp = NULL;
	xlist_for_each_safe(pos, n, &fc41d->priv_data->task.l) 
	{
		tmp = xlist_entry(pos, DevFc41dTaskList_t, l);
		if (t != NULL)
		{
			*t = tmp->t;
		}

        xlist_del(pos);
#if USE_TASK_HEAP
        free(tmp);
#else 
        tmp->t.use = 0;
#endif 

		--fc41d->priv_data->task_size;
		return 0;
	}

    return -1;
}

static int32_t _DevFc41dTaskPush(DevFc41d_t *fc41d, uint8_t type, int32_t process_status,
								   int32_t sub_status, int8_t stat, int32_t socket)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (fc41d->priv_data->task_size >= FC41D_TASK_MAX_SIZE) 
	{
		return -1;
	}

#if USE_TASK_HEAP
	DevFc41dTaskList_t *node = (DevFc41dTaskList_t *)malloc(sizeof(DevFc41dTaskList_t));
    if (node == NULL) 
	{
		return -1;
    }

#else 
	DevFc41dTaskList_t *node = _DevFc41dTaskEmptyGet(fc41d);

#endif 

	node->t.use = 1;
	node->t.process_status = process_status;
	node->t.sub_status = sub_status;
	node->t.timeout = 0;
	node->t.stat = stat;
	node->t.socket_id = socket;
	if (type == 0) 
	{
		xlist_add_tail(&node->l, &fc41d->priv_data->task.l);
	} 
	else 
	{
		xlist_add(&node->l, &fc41d->priv_data->task.l);
	}
	++fc41d->priv_data->task_size;

    return 0;
}

static int32_t _DevFc41dTaskHeadDel(DevFc41d_t *fc41d)
{
	return _DevFc41dTaskPop(fc41d, NULL);
}

int32_t _DevFc41dTaskInhead(DevFc41d_t *fc41d, int32_t process_status,
								   int32_t sub_status, int8_t stat, int32_t socket)
{
	return _DevFc41dTaskPush(fc41d, 1, process_status, sub_status, stat, socket);
}

int32_t _DevFc41dTaskAppend(DevFc41d_t *fc41d, int32_t process_status,
								   int32_t sub_status, int8_t stat, int32_t socket)
{
	return _DevFc41dTaskPush(fc41d, 0, process_status, sub_status, stat, socket);
}

int8_t _DevFc41dTaskHeadGetStat(DevFc41d_t *fc41d)
{
	DevFc41dTask_t *t = _DevFc41dTaskGetHeadPtr(fc41d);
	if (t == NULL) 
	{
		return -1;
	}

	return t->stat;
}

int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat)
{
	DevFc41dTask_t *t = _DevFc41dTaskGetHeadPtr(fc41d);
	if (t == NULL) 
	{
		return -1;
	}
	t->stat = stat;
	return 0;
}

static int32_t _DevFc41dDel(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return 0;
	}
	_DevFc41dTaskClean(fc41d);
	free(fc41d);
	return 0;
}

static void _DevFc41dInitConf(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return ;
	}

	fc41d->priv_data->cfg.wifi_auto_conn = FC41D_WIFI_AUTO_CONN;
	fc41d->priv_data->cfg.wifi_auto_conn_timeout = FC41D_WIFI_AUTO_CONN_TIMEOU;
	fc41d->priv_data->cfg.wifi_auto_conn_time = 0;

	fc41d->priv_data->cfg.mqtt_auto_conn = FC41D_MQTT_AUTO_CONN;
	fc41d->priv_data->cfg.mqtt_auto_conn_timeout = FC41D_MQTT_AUTO_CONN_TIMEOU;
	fc41d->priv_data->cfg.mqtt_auto_conn_time = 0;
	fc41d->priv_data->mqtt.s_url = gCollector->head.s_url;
}

static int32_t _DevFc41dUsartStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_COMM_STATUS_INIT:fc41d->priv_data->usart_status);
}

static int32_t _DevFc41dUsartStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	
	if (fc41d == NULL) {
		return -1;
	}

	fc41d->priv_data->usart_status = status;
	return 0;
}

static int32_t _DevFc41dStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_STATUS_INIT:fc41d->priv_data->process_status);
}

static int32_t _DevFc41dStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	fc41d->priv_data->process_status = status;
	return 0;
}

static int32_t _DevFc41dUrcStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_URC_STATUS_OFF
			:fc41d->priv_data->urc_status);
}

static int32_t _DevFc41dUrcStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->urc_status = status;
	return 0;
}

int32_t _DevFc41dSend(DevFc41d_t *fc41d, const uint8_t *p_data, int32_t len)
{
//	if (UartGetSendFlag(&WirelessUartHandle))
//	{
//		return -1;
//	}
    WirelessUartHandle.pTxBuf = (uint8_t *)p_data;
    WirelessUartHandle.tx_size = (uint16_t)len;
    UartSend(&WirelessUartHandle);  

	// 开始计时(发送超时)
	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	return 0;
}

static void _DevFc41dReset()
{
    GpioBitReset(&WirelessResetIOHandle);
    GpioBitSet(&WirelessPowerEnIOHandle);
    delay_1ms(200);
    GpioBitSet(&WirelessResetIOHandle);
}

static int32_t _DevFc41dDataReset(DevFc41d_t *fc41d, int32_t clean_data_flag)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	uint32_t cur = tim_get_systime();
	fc41d->priv_data->stat_timeout = cur;

	fc41d->priv_data->stat = 0;
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	fc41d->priv_data->wifi.info.station.got_ip_stat = WIFI_CONN_STAT_DISCONN;
	fc41d->priv_data->wifi.info.station.conn_stat = WIFI_CONN_STAT_DISCONN;
	fc41d->priv_data->mqtt.conn_stat = 0;

	rbuffer_clear(&fc41d->priv_data->rb_read.ctx);
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	rbuffer_clear(&fc41d->priv_data->rb_busy.ctx);
	_DevFc41dTaskClean(fc41d);
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);

	if (clean_data_flag)
	{
		_DevFc41dResetUpgradeData(fc41d);
	}

    return 0;
}

int32_t _DevFc41dUsartInit(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	_DevFc41dDataReset(fc41d, 1);
	_DevFc41dReset();

	return 1;
}

static int32_t _DevFc41dInitTimeout(DevFc41d_t *fc41d)
{
	uint32_t cur = tim_get_systime();
	if (cur - fc41d->priv_data->stat_timeout > 5000 && fc41d->priv_data->stat < 2)
	{
#if NOVA_DEBUG_NO_RESET_FC41D

#else
		_DevFc41dUsartInit(fc41d);
#endif 
		return 0;
	}
	return -1;
}

static int32_t _DevFc41dReadUrc(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{ 
		// data not enough
		return 0;
	} 

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		_DevFc41dTaskPop(fc41d, NULL);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_WRITE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		_DevFc41dTaskPop(fc41d, NULL);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteUrc(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	int32_t status = _DevFc41dUrcStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_URC_STATUS_ON:
		fc41d->write(fc41d, (const uint8_t *)AT_FC41D_URC_ON, strlen(AT_FC41D_URC_ON));
		break;
	case FC41D_PROCESS_URC_STATUS_OFF:
		fc41d->write(fc41d, (const uint8_t *)AT_FC41D_URC_OFF, strlen(AT_FC41D_URC_OFF));
		break;
	}

	return 0;
}

static int32_t _DevFc41dReadDisableWebCfg(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{ // data not enough
		return 0;
	} 

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		find[strlen(AT_OK)-1] = 0;
		read = strlen((const char *)data)+1;

		_DevFc41dTaskPop(fc41d, NULL);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		_DevFc41dTaskPop(fc41d, NULL);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteDisableWebCfg(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_DISABLE_WEB_CFG, 
				 strlen(AT_FC41D_DISABLE_WEB_CFG));
	return 0;
}

static int32_t _DevFc41dReadVer(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len <= 0) 
	{ // data not enough
		return 0;
	} 

	int32_t read = 0;
	char *find = strstr((const char *)data, AT_OK);
	if (find) 
	{
		find[0] = 0;
		find = strstr((const char *)data, "\r\n");
		if (find)
		{
			find[0] = 0;
		}
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		read = strlen((const char *)data)+1;

		if (read >= sizeof(fc41d->priv_data->com_info.ver))
		{
			read = sizeof(fc41d->priv_data->com_info.ver)-1;
		}
		memset(fc41d->priv_data->com_info.ver, 0, sizeof(fc41d->priv_data->com_info.ver));
		memcpy(fc41d->priv_data->com_info.ver, data, read);

		_DevFc41dTaskPop(fc41d, NULL);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_WRITE);
		// wait timeout and retry
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		_DevFc41dTaskPop(fc41d, NULL);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteVer(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)AT_FC41D_VER, strlen(AT_FC41D_VER));
	return 0;
}

static int32_t _DevFc41dReadInit(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len < 11) 
	{ // data not enough
		return 0;
	} 

	char *p = (char *)&data[3];
	if (data[0] == 0x00 && data[1] == 0x0D && data[2] == 0x0A 
		&& strncmp(p, AT_READY, strlen(AT_READY)) == 0)
	{
		fc41d->priv_data->stat = 1;
		_DevFc41dTaskPop(fc41d, NULL);
		_DevFc41dInitTask(fc41d);
		return 10;
	}

	return 0;
}

static int32_t _DevFc41dWriteInit(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_UNDETECTED;
	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	_DevFc41dUsartInit(fc41d);
	return 0;
}

static int32_t _DevFc41dUpgrade(DevFc41d_t *fc41d)
{
	uint32_t cur = tim_get_systime();
	uint8_t flag = 0;
	if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_NONE)
	{
		_DevFc41dResetUpgradeData(fc41d);
		return 0;
	}

	if (fc41d->priv_data->upgrade.flag > COLLECTOR_UPGRADE_NONE)
	{
		if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_LOCAL)
		{
			if (cur - fc41d->priv_data->upgrade.time > 1000*10)
			{
				flag = 3;
			}
		}
		else
		{
			if (fc41d->priv_data->http.custom.ali_oss.pkg_type == EN_FILE_PKG_TYPE_NOVA_COLLECTOR)
			{
				if (cur - fc41d->priv_data->upgrade.time > NOVA_UPGRADE_COLLECTOR_TIMEOUT)
				{
					flag = 1;
				}
			}
			else if (fc41d->priv_data->http.custom.ali_oss.pkg_type == gCollector->pcs_modelid)
			{
				if (fc41d->priv_data->upgrade.timeout > 0)
				{
					if (cur - fc41d->priv_data->upgrade.time > 
						fc41d->priv_data->upgrade.timeout) 
					{
						flag = 2;
					}
				}
				else if (cur - fc41d->priv_data->upgrade.time > NOVA_UPGRADE_PCS_TIMEOUT) 
				{
					flag = 2;
				}
			}
		}

		if (flag > 0)
		{
			// cancel upgrade
			if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_REMOTE)
			{
				_DevFc41dHttpGetUpgrade2Ack(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_5);
			}
			else 
			{
				_DevFc41dResetUpgradeData(fc41d);
			}
			fc41d->priv_data->upgrade.time = cur;
		}
	}
	return fc41d->priv_data->upgrade.flag;
}

static int32_t _DevFc41dUpgradeTimeout2(DevFc41d_t *fc41d)
{
	if (fc41d->priv_data->upgrade.flag < COLLECTOR_UPGRADE_INIT)
	{
		return 0;
	}

	uint32_t cur = tim_get_systime();
	uint8_t flag = 0;
	if (fc41d->priv_data->upgrade.timeout > 0)
	{
		if (cur - fc41d->priv_data->upgrade.time > 
			fc41d->priv_data->upgrade.timeout) 
		{
			flag = 2;
		}
	}
	else if (cur - fc41d->priv_data->upgrade.time > NOVA_UPGRADE_COLLECTOR_TIMEOUT) 
	{
		flag = 2;
	}
	if (flag <= 0)
	{
		return fc41d->priv_data->upgrade.flag;
	}

	++fc41d->priv_data->upgrade.retry_count;
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
	_DevFc41dTaskClean(fc41d);
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	fc41d->priv_data->ble.send_flag =0;
	fc41d->priv_data->mqtt.send_flag = 0;
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);

	if (fc41d->priv_data->upgrade.retry_count >= NOVA_UPGRADE_COLLECTOR_RETRY_MAX)
	{
		if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_REMOTE)
		{
			_DevFc41dHttpGetUpgrade2Ack(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_5);
		}
		_DevFc41dResetUpgradeData(fc41d);
		/*
		   _DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_HTTP, 
		   FC41D_PROCESS_HTTP_DEV_UPGRADE_RETRY,
		   DEV_FC41D_TASK_STAT_READ, 0);
		   */
		return 0;
	}
	if (fc41d->priv_data->http.cmd != PROTO_UNIFORM_W_DEV_UPGRADE)
	{
		_DevFc41dResetUpgradeData(fc41d);
		return 0;
	}
	++fc41d->priv_data->http.custom.ali_oss.cur_pkg_err;
	DevFc41dHttpGetContinue(fc41d);

	return 1;
}

#if 1
static int32_t _DevFc41dUpgradeTimeout(DevFc41d_t *fc41d)
{
	if (fc41d->priv_data->upgrade.flag < COLLECTOR_UPGRADE_INIT)
	{
		return 0;
	}
	if (_DevFc41dUpgrade(fc41d) == 0)
	{
		return 0;
	}

	++fc41d->priv_data->upgrade.retry_count;
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
	_DevFc41dTaskClean(fc41d);
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	fc41d->priv_data->ble.send_flag =0;
	fc41d->priv_data->mqtt.send_flag = 0;
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);

	if (fc41d->priv_data->upgrade.retry_count >= NOVA_UPGRADE_COLLECTOR_RETRY_MAX)
	{
		if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_REMOTE)
		{
			_DevFc41dHttpGetUpgrade2Ack(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_5);
		}
		_DevFc41dResetUpgradeData(fc41d);
		/*
		_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_DEV_UPGRADE_RETRY,
							DEV_FC41D_TASK_STAT_READ, 0);
							*/
		return 0;
	}
	if (fc41d->priv_data->http.cmd != PROTO_UNIFORM_W_DEV_UPGRADE)
	{
		_DevFc41dResetUpgradeData(fc41d);
		return 0;
	}
	++fc41d->priv_data->http.custom.ali_oss.cur_pkg_err;
	DevFc41dHttpGetContinue(fc41d);

	return 1;
}

#else
static int32_t _DevFc41dUpgradeTimeout(DevFc41d_t *fc41d)
{
#if 0
	if (fc41d->priv_data->write_stat.status !=  DEV_FC41D_WRITE_HTTP
		|| !fc41d->priv_data->upgrade.flag
		|| fc41d->priv_data->upgrade.retry_count > NOVA_UPGRADE_COLLECTOR_RETRY_MAX)
#endif 
	if (fc41d->priv_data->upgrade.flag < COLLECTOR_UPGRADE_INIT)
	{
		return 0;
	}
	if (_DevFc41dUpgrade(fc41d) == 0)
	{
		return 0;
	}
	if (fc41d->priv_data->upgrade.retry_count < NOVA_UPGRADE_COLLECTOR_RETRY_MAX)
	{
		_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_DEV_UPGRADE_RETRY,
							DEV_FC41D_TASK_STAT_READ, 0);

#if 0
		fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_PAUSE;
		fc41d->priv_data->upgrade.retry_count = 0;
		DevFc41dReset(fc41d, 0);
		remote_upgrade_flag = fc41d->priv_data->upgrade.flag;
#endif 
		return 0;
	}
	++fc41d->priv_data->upgrade.retry_count;
	if (fc41d->priv_data->http.cmd != PROTO_UNIFORM_W_DEV_UPGRADE
		&& fc41d->priv_data->http.cmd != PROTO_UNIFORM_W_DEV_UPGRADE_PCS)
	{
		return 0;
	}
	++fc41d->priv_data->http.custom.ali_oss.cur_pkg_err;
	DevFc41dHttpGetContinue(fc41d);

	return 1;
}
#endif 

static int32_t _DevFc41dWriteTimeout(DevFc41d_t *fc41d)
{
	uint32_t cur = tim_get_systime();
	if ((uint32_t)(cur - fc41d->priv_data->write_stat.timeout) < FC41D_WRITE_TIMEOU)
	{
		return -1;
	}

	if (!fc41d->priv_data->write_stat.status)
	{
		return -1;
	}

	// check timeout model
	switch (fc41d->priv_data->write_stat.status)
	{
	case DEV_FC41D_WRITE_NOVA:
		break;
	case DEV_FC41D_WRITE_MQTT:
		{
			++fc41d->priv_data->mqtt.try_send_count;
			if (fc41d->priv_data->mqtt.try_send_count > 3)
			{
				// reconn mqtt
				fc41d->priv_data->mqtt.conn_stat = 0;
				fc41d->priv_data->mqtt.try_send_count = 0;
			}
		}
		break;
	case DEV_FC41D_WRITE_HTTP:
		{
			int32_t status = fc41d->priv_data->http.status;
			if (status == FC41D_PROCESS_HTTP_OTA_FC41D)
			{
				if ((uint32_t)(cur - fc41d->priv_data->write_stat.timeout) < 15*1000)
				{
					return -1;
				}
				break;
			}
			if (fc41d->priv_data->upgrade.flag > COLLECTOR_UPGRADE_PAUSE)
			{
				_DevFc41dUrcHttpResult(fc41d, -COLLECTOR_UPGRADE_ERR_TIMEOUT);
				return 1;
			}
		}
		break;
	}

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	return 0;
}

static int32_t _DevFc41dUpgrade1(DevFc41d_t *fc41d)
{
	if (fc41d->priv_data->upgrade.type == EN_UPGRADE_TYPE_LOCAL)
	{
		return 0;
	}
	return fc41d->priv_data->upgrade.flag;
}

static void _DevFc41dTaskCleanExceptNovaCmd(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return;
	}

	DevFc41dTaskList_t *pnode = NULL;
	xlist_head_t *pos = NULL, *n = NULL;
	xlist_for_each_safe(pos, n, &fc41d->priv_data->task.l) 
	{
		pnode = xlist_entry(pos, DevFc41dTaskList_t, l);
		if (pnode->t.process_status == FC41D_PROCESS_STATUS_NOVA_CMD)
		{
			if (fc41d->priv_data->fn.fn_usart_stat_get(fc41d) == FC41D_COMM_STATUS_READ)
			{
				continue;
			}
		}
        xlist_del(pos);
#if USE_TASK_HEAP
        free(pnode);
#else
		pnode->t.use = 0;
#endif 
		--fc41d->priv_data->task_size;
	}
	//fc41d->priv_data->task_size = 0;
}

static int32_t _DevFc41dFixReadTimeout(DevFc41d_t *fc41d)
{
	if (_DevFc41dUpgradeTimeout(fc41d))
	{
		return 0;
	}

	if (_DevFc41dWriteTimeout(fc41d) == 1)
	{
		return 0;
	}

	_DevFc41dTaskCleanExceptNovaCmd(fc41d);
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
	fc41d->priv_data->ble.send_flag =0;
	fc41d->priv_data->mqtt.send_flag = 0;
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);

	if (fc41d->priv_data->stat < 2)
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_INIT);
		return 0;
	}

	return 0;
}

static int32_t _DevFc41dUrcSkip(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	int32_t ret = 0;

	if (strncmp(data, AT_READY, strlen(AT_READY)) == 0)
	{
		_DevFc41dInitTask(fc41d);
		return strlen(AT_READY);
	}

	if (strncmp(data, AT_OK, strlen(AT_OK)) == 0) 
	{
		return strlen(AT_OK);
	}

	if (strncmp(data, AT_ERR, strlen(AT_ERR)) == 0) 
	{
		_DevFc41dFixReadTimeout(fc41d);

#if USE_LED_DEBUG
#if USE_GD_LED
		gd_eval_led_on(LED1);
		gd_eval_led_on(LED2);
		gd_eval_led_on(LED3);
		gd_eval_led_on(LED4);
#else
		LedDirectOn(LED1);
		LedDirectOn(LED2);
		LedDirectOn(LED3);
#endif /*USE_GD_LED*/
#endif 
		return strlen(AT_ERR);
	}

	return ret;
}

static int32_t _DevFc41dCheckUpgrade(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return 0;
	}

	if (fc41d->priv_data->http.ota_fc41d)
	{
		return 1;
	}

	return fc41d->priv_data->upgrade.flag;
}

static int32_t _DevFc41dCheckNetSetting(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return 0;
	}

	return fc41d->priv_data->wifi.info.station.net_setting_flag;
}


static int32_t _DevFc41dProcessing(DevFc41d_t *fc41d)
{
	if (fc41d == NULL)
	{
		return 0;
	}

	uint32_t cur_time = tim_get_systime();
	if (fc41d->priv_data->http.custom.ali_oss.reboot_flag == 1)
	{
		if (cur_time - fc41d->priv_data->http.custom.ali_oss.reboot_time > 10*1000)
		{
			delay_1ms(1000);

			// reset
			__set_FAULTMASK(1);
			NVIC_SystemReset();
		}
	}

	int32_t flag = fc41d->priv_data->write_stat.status
		|| (cur_time - fc41d->priv_data->write_stat.employ_time > 0?0:1);
	return flag;
}

static int32_t _DevFc41dUrcRecv(DevFc41d_t *fc41d, int8_t type, 
										 const uint8_t *data, int32_t len)
{
	int32_t ret = 0;
	char *tmp_ptr = (char *)data;
	int32_t tmp_len = len;
	int32_t process_len = 0, read_len = 0, urc_flag = 0;

	if (tmp_len <= 0) 
	{
		return ret;
	}

	while (read_len < tmp_len)
	{
		if (fc41d->priv_data->upgrade.flag <= COLLECTOR_UPGRADE_NONE)
		{
			if (tmp_ptr[0] == '\r' || tmp_ptr[0] == '\n') 
			{
				urc_flag = 0;
				++tmp_ptr;
				++process_len;
				++read_len;
				continue;
			}

			if (type != 1) // skip error
			{
				if (tmp_ptr[0] < 0x21 || tmp_ptr[0] > 0x7E) 
				{
					urc_flag = 0;
					++tmp_ptr;
					++process_len;
					++read_len;
					continue;
				}

				ret = _DevFc41dUrcSkip(fc41d, tmp_ptr, tmp_len-process_len);
				if (ret > 0) 
				{
					++urc_flag;
					process_len += ret;
					tmp_ptr += ret;
					read_len += ret;
					continue;
				}
			}
		}

		if (fc41d->priv_data->upgrade.type != EN_UPGRADE_TYPE_REMOTE)
		{
			ret = _DevFc41dUrcNovaDebug(fc41d, tmp_ptr, tmp_len-process_len);
			if (ret > 0) 
			{
				++urc_flag;
				process_len += ret;
				tmp_ptr += ret;
				read_len += ret;
				continue;
			}

			ret = _DevFc41dUrcNova(fc41d, type, tmp_ptr, tmp_len-process_len);
			if (ret > 0) 
			{
				++urc_flag;
				process_len += ret;
				tmp_ptr += ret;
				read_len += ret;
				continue;
			}
		}

		// analysis urc 
#if 0
		ret = _DevFc41dUrcSocket(fc41d, tmp_ptr, tmp_len-process_len);
		if (ret > 0) 
		{
			++urc_flag;
			process_len += ret;
			tmp_ptr += ret;
			read_len += ret;
			continue;
		}
#endif 
		ret = _DevFc41dUrcBle(fc41d, tmp_ptr, tmp_len-process_len);
		if (ret > 0) 
		{
			++urc_flag;
			process_len += ret;
			tmp_ptr += ret;
			read_len += ret;
			continue;
		}

		ret = _DevFc41dUrcWifiStat(fc41d, tmp_ptr, tmp_len-process_len);
		if (ret > 0) 
		{
			++urc_flag;
			process_len += ret;
			tmp_ptr += ret;
			read_len += ret;
			continue;
		}

		ret = _DevFc41dUrcMqtt(fc41d, type, tmp_ptr, tmp_len-process_len);
		if (ret > 0) 
		{
			++urc_flag;
			process_len += ret;
			tmp_ptr += ret;
			read_len += ret;
			continue;
		}

		ret = _DevFc41dUrcHttp(fc41d, tmp_ptr, tmp_len-process_len);
		if (ret > 0) 
		{
			++urc_flag;
			process_len += ret;
			tmp_ptr += ret;
			read_len += ret;
			continue;
		}
        else 
        {
            if (fc41d->priv_data->upgrade.flag > COLLECTOR_UPGRADE_PAUSE)
			{
				break;
			}
        }

		if (urc_flag == 0)
		{
#if 0
            if (type == 0)
            {
                ++tmp_ptr;
                ++read_len;
                continue;
            }
#endif 
			break;
		}

		// not found key
		++tmp_ptr;
		++read_len;
        
        if (process_len < read_len)
		{
			break;
		}
	}

	return process_len;
}

static int32_t _DevFc41dBusy(DevFc41d_t *fc41d)
{
	int32_t busy_len = rbuffer_length(&fc41d->priv_data->rb_busy.ctx);
	if (busy_len <= 0)
	{
		return -1;
	}

	int32_t buf_len = 0, trim_len = 0, read_bytes = 0;
	uint8_t buf[512] = {0};
	
	while (1) 
	{
		buf_len = rbuffer_get_bytes(&fc41d->priv_data->rb_busy.ctx, (uint8_t *)buf, 
									sizeof(buf));
		if (buf_len == 0)
		{
			break;
		}
		trim_len = rbuffer_trim_bytes(&fc41d->priv_data->rb_busy.ctx, buf_len);

		// do busy task
		read_bytes = _DevFc41dUrcRecv(fc41d, 0, buf, buf_len);
		if (read_bytes == 0)
		{
			// 非法指令，直接清空
			read_bytes = buf_len;
		}
		break;
	}


	return 0;
}

static int32_t _DevFc41dCfg(DevFc41d_t *fc41d)
{
	int32_t ret = 0;

	if (fc41d == NULL)
	{
		return -1;
	}
	ret = _DevFc41dWifiAutoConn(fc41d);
	if (ret == 0)
	{
		return ret;
	}
	ret = _DevFc41dCfgMqttAutoConn(fc41d);

	return ret;
}

static int32_t _DevFc41dPopTask(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

#if 1
	static uint32_t err_time = 0;
	uint32_t cur = tim_get_systime();

	if (fc41d->priv_data->task_size == 0 )
		//&& len <= 0)
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		if (fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
		{
			fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
			_DevFc41dTaskClean(fc41d);
			fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;
			fc41d->priv_data->ble.send_flag =0;
			fc41d->priv_data->mqtt.send_flag = 0;
			rbuffer_clear(&fc41d->priv_data->rb_write.ctx);

			if (err_time == 0)
			{
				err_time = cur;
			}
			else if (cur - err_time > AT_CMD_TIMEOUT)
			{
				err_time = 0;
				return -2;
			}

			return 0;
		}
		else
		{
			fc41d->priv_data->write_stat.employ_time = 0;
			// 无任务，无上报
		}
	}

	err_time = 0;
#endif 

	DevFc41dTask_t *t = _DevFc41dTaskGetHeadPtr(fc41d);

	if (t == NULL) 
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
		return 0;
	}

	if (t->stat == DEV_FC41D_TASK_STAT_WRITE && len > 0)
	{
		if (fc41d->priv_data->ble.send_flag != 0 || fc41d->priv_data->mqtt.send_flag != 0)
		{
			// urc not process complete
			fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
			return 1;
		}
	}

	_DevFc41dStatusSet(fc41d, t->process_status);
	switch (t->process_status) 
	{
	case FC41D_PROCESS_STATUS_URC:
		_DevFc41dUrcStatusSet(fc41d, t->sub_status);
		break;
	case FC41D_PROCESS_STATUS_BLE:
		_DevFc41dBleStatusSet(fc41d, t->sub_status);
		break;
	case FC41D_PROCESS_STATUS_WIFI:
		_DevFc41dWifiStatusSet(fc41d, t->sub_status);
		break;
	case FC41D_PROCESS_STATUS_MQTT:
		_DevFc41dMqttStatusSet(fc41d, t->sub_status);
		break;
	case FC41D_PROCESS_STATUS_HTTP:
		_DevFc41dHttpStatusSet(fc41d, t->sub_status);
		break;
	case FC41D_PROCESS_STATUS_SOCKET:
		//_DevFc41dSocketStatusSet(fc41d, t->sub_status, t->socket_id);
		break;
	case FC41D_PROCESS_STATUS_NOVA_CMD:
		_DevFc41dNovaCmdStatusSet(fc41d, t->sub_status);
		break;
	}

	switch (t->stat)
	{
	case DEV_FC41D_TASK_STAT_WRITE:
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_WRITE);
		break;
	case DEV_FC41D_TASK_STAT_READ:
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_READ);
		break;
	}
	return 2;
}

static int32_t _DevFc41dProcessRead(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL)
	{
		return -1;
	}

#if USE_LED_DEBUG
#if USE_GD_LED
	gd_eval_led_on(LED2);
	gd_eval_led_off(LED4);
#else
	LedDirectOn(LED2);
	LedDirectOff(LED3);
#endif /*USE_GD_LED*/
#endif 

	int32_t ret = _DevFc41dUrcRecv(fc41d, 1, data, len);
	if (ret > 0)
	{
		ret = rbuffer_trim_bytes(&fc41d->priv_data->rb_read.ctx, ret);
		return 0;
	}

	int32_t status = fc41d->priv_data->fn.fn_process_stat_get(fc41d);
	int32_t socket_id = 0;
	DevFc41dTask_t *t = _DevFc41dTaskGetHeadPtr(fc41d);
	if (t)
	{
		socket_id = t->socket_id;
	}
	switch (status) 
	{
	case FC41D_PROCESS_STATUS_INIT:
		return _DevFc41dReadInit(fc41d, data, len);
	case FC41D_PROCESS_STATUS_VER:
		return _DevFc41dReadVer(fc41d, data, len);
	case FC41D_PROCESS_STATUS_DISABLE_WEB_CFG:
		return _DevFc41dReadDisableWebCfg(fc41d, data, len);
	case FC41D_PROCESS_STATUS_URC:
		return _DevFc41dReadUrc(fc41d, data, len);
	case FC41D_PROCESS_STATUS_BLE:
		return _DevFc41dReadBle(fc41d, data, len);
	case FC41D_PROCESS_STATUS_WIFI:
		return _DevFc41dReadWifi(fc41d, data, len);
	case FC41D_PROCESS_STATUS_MQTT:
		return _DevFc41dReadMqtt(fc41d, data, len);
	case FC41D_PROCESS_STATUS_HTTP:
		return _DevFc41dReadHttp(fc41d, data, len);
	case FC41D_PROCESS_STATUS_SOCKET:
		//return _DevFc41dReadSocket(fc41d, socket_id, data, len);
	case FC41D_PROCESS_STATUS_NOVA_CMD:
		return _DevFc41dReadNovaCmd(fc41d, data, len);
	default:break;
	}

	return -1;
}

static int32_t _DevFc41dProcessWrite(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

#if USE_LED_DEBUG
#if USE_GD_LED
	gd_eval_led_off(LED1);
	gd_eval_led_on(LED2);
	gd_eval_led_off(LED4);
#else
	LedDirectOff(LED1);
	LedDirectOn(LED2);
	LedDirectOff(LED3);
#endif /*USE_GD_LED*/
#endif 

	int32_t status = fc41d->priv_data->fn.fn_process_stat_get(fc41d);
	int32_t socket_id = 0;
	DevFc41dTask_t *t = _DevFc41dTaskGetHeadPtr(fc41d);
	if (t)
	{
		socket_id = t->socket_id;
	}
	fc41d->priv_data->fn.fn_process_stat_get(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_STATUS_INIT:
		_DevFc41dWriteInit(fc41d);
		break;
	case FC41D_PROCESS_STATUS_VER:
		_DevFc41dWriteVer(fc41d);
		break;
	case FC41D_PROCESS_STATUS_DISABLE_WEB_CFG:
		_DevFc41dWriteDisableWebCfg(fc41d);
		break;
	case FC41D_PROCESS_STATUS_URC:
		_DevFc41dWriteUrc(fc41d);
		break;
	case FC41D_PROCESS_STATUS_BLE:
		_DevFc41dWriteBle(fc41d);
		break;
	case FC41D_PROCESS_STATUS_WIFI:
		_DevFc41dWriteWifi(fc41d);
		delay_1ms(50);
		break;
	case FC41D_PROCESS_STATUS_MQTT:
		_DevFc41dWriteMqtt(fc41d);
		delay_1ms(50);
		break;
	case FC41D_PROCESS_STATUS_HTTP:
		_DevFc41dWriteHttp(fc41d);
		break;
	case FC41D_PROCESS_STATUS_SOCKET:
		//_DevFc41dWriteSocket(fc41d, socket_id);
		break;
	case FC41D_PROCESS_STATUS_NOVA_CMD:
		_DevFc41dWriteNovaCmd(fc41d);
		break;
	default:break;
	}
	return 0;
}

void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d)
{
	fc41d->priv_data->upgrade.type = EN_UPGRADE_TYPE_NONE;
	fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
	fc41d->priv_data->upgrade.stat = 0;
	fc41d->priv_data->upgrade.time = 0;
	fc41d->priv_data->upgrade.retry_count = 0;
	rbuffer_clear(&fc41d->priv_data->upgrade.down.ctx);
}

static int32_t _DevFc41dCheckBusy(DevFc41d_t *fc41d)
{
    int32_t busy_len = rbuffer_length(&fc41d->priv_data->rb_busy.ctx);
	if (busy_len > 0)
	{
		int32_t xx = 0;
	}

	if (fc41d->priv_data->stat < 2
		|| fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
	{
		
		return 0;
	}

	return -1;
}

static int32_t _DevFc41dProcessIdle(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

#if USE_LED_DEBUG
#if USE_GD_LED
	gd_eval_led_on(LED1);
	gd_eval_led_on(LED2);
	gd_eval_led_off(LED4);
#else
	LedDirectOn(LED1);
	LedDirectOn(LED2);
	LedDirectOff(LED3);
#endif /*USE_GD_LED*/
#endif

	int32_t read_bytes = _DevFc41dUrcRecv(fc41d, 0, data, len);
	if (read_bytes > 0)
	{
		return read_bytes;
	}

	if (_DevFc41dInitTimeout(fc41d) == 0)
	{
		return 0;
	}

	if (_DevFc41dUpgradeTimeout2(fc41d))
	{
		return 0;
	}

	if (_DevFc41dCheckBusy(fc41d) == 0)
	{
		return 0;
	}

	// check nova busy cmd
	if (_DevFc41dBusy(fc41d) == 0)
	{
		return 0;
	}

	// check cfg
	if (_DevFc41dCfg(fc41d) == 0)
	{
		return 0;
	}

	if (fc41d->priv_data->mqtt.conn_stat > 0)
	{
		if (remote_upgrade_flag != 0)
		{
			remote_upgrade_flag = 0;
			_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_DEV_UPGRADE_RETRY,
								DEV_FC41D_TASK_STAT_READ, 0);
		}
	}

	return 0;
}

static int32_t _DevFc41dOtaReboot(DevFc41d_t *fc41d)
{
	uint32_t cur_time = tim_get_systime();
	if (fc41d->priv_data->http.ota_fc41d == 1)
	{
		gDevLedProcessFlag = 4;
		if (cur_time - fc41d->priv_data->http.ota_fc41d_reboot_time > 180 * 1000)
		{
			__set_FAULTMASK(1);
			NVIC_SystemReset();
			return 0;
		}
	}
	else if (fc41d->priv_data->http.ota_fc41d == 2)
	{
		gDevLedProcessFlag = 4;
		if (cur_time - fc41d->priv_data->http.ota_fc41d_reboot_time > 30 * 1000)
		{
			__set_FAULTMASK(1);
			NVIC_SystemReset();
			return 0;
		}
		return 1;
	}

	return -1;
}

static int32_t _DevFc41dCollectorUpgradeReboot(DevFc41d_t *fc41d)
{
#if 0
	if (fc41d->priv_data->upgrade.need_upgrade_cur_type == 1
		&& (fc41d->priv_data->upgrade.need_upgrade_type & 1))
	{
		gDevLedProcessFlag = 1;
		static uint32_t cur_time = 0;
		if (cur_time == 0)
		{
			cur_time = tim_get_systime();
		}
		if (tim_get_systime() - cur_time > 60*1000)
		{
			__set_FAULTMASK(1);
			NVIC_SystemReset();
		}
		return 0;
	}

#endif 
	return -1;
}

static int32_t _DevFc41dProcess(DevFc41d_t *fc41d)
{

	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dOtaReboot(fc41d) >= 0)
	{
		return 0;
	}

	_DevFc41dCollectorUpgradeReboot(fc41d);

	int32_t ret = 0;
	int32_t task = 0;
	int32_t read_bytes = 0, try_read = 0;
	static uint32_t read_timeout = 0, idle_timeout = 0;
	uint32_t cur_time = 0;
	uint8_t *buf = _gCommBuf;
	//uint8_t buf[4096] = {0};
	int32_t buf_len = 0, total_len = 0;
	int32_t status =  0, task_ret = 0;
	static int32_t precmd_size = 0, precmd_not_work_count = 0;
	do 
	{
		memset(_gCommBuf, 0, sizeof(_gCommBuf));
		total_len = rbuffer_length(&fc41d->priv_data->rb_read.ctx);
		buf_len = rbuffer_get_bytes(&fc41d->priv_data->rb_read.ctx, 
									(uint8_t *)buf, sizeof(_gCommBuf)-1);
		buf[buf_len] = 0;

		int32_t busy_len = rbuffer_length(&fc41d->priv_data->rb_busy.ctx);

		task_ret = _DevFc41dPopTask(fc41d, buf, buf_len);
		if (task_ret < 0)
		{
			if (task_ret == -2)
			{
				_DevFc41dFixReadTimeout(fc41d);
			}
			return -1;
		}

		cur_time = tim_get_systime();
		status = fc41d->priv_data->fn.fn_usart_stat_get(fc41d);

		if (buf_len > 0)
		{
			char *f = strstr((const char *)buf, "NOVA");
			if (f && (f[40] != 0xFF && f[41] != 0xED && f[42] != 0xDE && f[43] != 0xFF))
			{
				int32_t xx = 0;
				xx = 1;

			}
			if (strstr((const char *)buf, "+QIURC:"))
			{
				int32_t xx = 0;
				xx = 1;

			}
			int32_t x = 0;
            x = 1;
		}

		switch (status) 
		{
		case FC41D_COMM_STATUS_IDLE:
			read_bytes = _DevFc41dProcessIdle(fc41d, (const uint8_t *)buf, buf_len);
			if (read_bytes <= 0)
			{
				if (buf_len > 0 && task_ret == 1)
				{
					// 有任务，但仍有未解析的数据
					rbuffer_clear(&fc41d->priv_data->rb_read.ctx);
					break;
				}

				if (buf_len > 0)
				{
					if (cur_time - idle_timeout > AT_CMD_TIMEOUT)
					{
						try_read = 1;
						idle_timeout = tim_get_systime();
					}
					if (precmd_size == 0)
					{
						precmd_size = buf_len;
						precmd_not_work_count = 0;
					}
					else
					{
						if (precmd_size == buf_len)
						{
							++precmd_not_work_count;
						}
					}
                    break;;
				}
			}
//			else 
			{
				idle_timeout = tim_get_systime();
			}
			break;
		case FC41D_COMM_STATUS_READ:
			read_bytes = _DevFc41dProcessRead(fc41d, (const uint8_t *)buf, buf_len);
			if (cur_time - read_timeout > AT_CMD_TIMEOUT)
			{
				try_read = 1;
			}
			break;
		case FC41D_COMM_STATUS_WRITE:
			_DevFc41dProcessWrite(fc41d);
			read_timeout = tim_get_systime();
			break;
        default:
            read_bytes = 0;
            break;
		}
        
        if (try_read > 0)
		{
			try_read = 0;
			if (fc41d->priv_data->http.ota_fc41d == 1)
			{
				break;
			}
			_DevFc41dFixReadTimeout(fc41d);
			if (precmd_not_work_count > 100)
			{
				rbuffer_clear(&fc41d->priv_data->rb_read.ctx);
			}
			precmd_size = precmd_not_work_count = 0;
			break;
		}
        
        if (read_bytes == 0)
        {
            break;
        }

		if (read_bytes > 0)
		{
			precmd_size = precmd_not_work_count = 0;
			buf_len = rbuffer_trim_bytes(&fc41d->priv_data->rb_read.ctx, read_bytes);
			buf_len = rbuffer_length(&fc41d->priv_data->rb_read.ctx);
            if (buf_len > 0) 
			{
                read_bytes = 0;
            }
		}

		if (buf_len <= 0 || read_bytes >= buf_len)
		{
			break;
		}
	} while (1);

	return ret;
}

static int32_t _DevFc41dRead(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	int32_t ret = 0, read_bytes = 0;

	if (fc41d == NULL) 
	{
		return -1;
	}

	if (len > 0) 
	{
		const char tmp_debug[][32] = {
			{"\r\n+QMTRECV: 1,"},
		};
		int32_t i = 0, tmp_debug_len = 0;
		for (i = 0; i < sizeof(tmp_debug)/sizeof(tmp_debug[0]); ++i)
		{
			tmp_debug_len = strlen(tmp_debug[i]);
			if (strncmp((const char *)data, tmp_debug[i], strlen(tmp_debug[i])) == 0)
			{
				int32_t tmp_len = len - tmp_debug_len;
				const uint8_t *p_tmp_debug = &data[tmp_debug_len];
				char *find = strstr((const char *)p_tmp_debug, "NOVA");
				if (find != NULL)
				{
					UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)&find[0];
					if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_TEST_MQTT_SUB)
					{
						int32_t xx = 0;
					}
					else if (uniform->body.tlv.cmd == PROTO_UNIFORM_W_DEV_UPGRADE)
					{
						int32_t xx = 0;
					}

					if (_DevFc41dUpgrade1(fc41d) > COLLECTOR_UPGRADE_PAUSE)
					{
						return -2;
					}
				}
			}
		}

		if (strncmp((const char *)&data[0], "NOVA", sizeof(char)*4) == 0)
		{
			UniformProtocolHead_t *uniform = (UniformProtocolHead_t *)&data[0];
			if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_TEST_MQTT_SUB)
			{
				int32_t xx = 0;
			}
			else if (uniform->body.tlv.cmd == PROTO_UNIFORM_R_DEV_PRODUCT)
			{
				int32_t xx = 0;
			}

			if(fc41d->priv_data->task_size > 0 
			   && fc41d->priv_data->write_stat.status > DEV_FC41D_WRITE_NONE)
			{
				if (_DevFc41dUpgrade1(fc41d))
				{
					return -3;
				}
			}
		}

		ret += rbuffer_write_bytes(&fc41d->priv_data->rb_read.ctx, data+ret, len-ret);
	}

	return ret;
}

static int32_t _DevFc41dInitTask(DevFc41d_t *fc41d)
{
	// init fc41d 
	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_VER, 0, 
						DEV_FC41D_TASK_STAT_WRITE, 0);

	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_BLE, 
						FC41D_PROCESS_BLE_STATUS_ADDR,
						DEV_FC41D_TASK_STAT_WRITE, 0);

	// disable web config
	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_DISABLE_WEB_CFG, 0, 
						DEV_FC41D_TASK_STAT_WRITE, 0);

	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						FC41D_PROCESS_WIFI_STATION_STOP_DEF, DEV_FC41D_TASK_STAT_WRITE, 0);

	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_WIFI, 
						FC41D_PROCESS_WIFI_STATION_STOP, DEV_FC41D_TASK_STAT_WRITE, 0);
	// open URC
	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_URC, 
						FC41D_PROCESS_URC_STATUS_ON, 
						DEV_FC41D_TASK_STAT_WRITE, 0);

	// init ble
	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_BLE, 
						FC41D_PROCESS_BLE_STATUS_INIT,
						DEV_FC41D_TASK_STAT_WRITE, 0);

	/*
	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_BLE, 
							FC41D_PROCESS_BLE_STATUS_MTU,
							DEV_FC41D_TASK_STAT_WRITE, 0);
							*/

	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_BLE, 
						FC41D_PROCESS_BLE_STATUS_ADDR,
						DEV_FC41D_TASK_STAT_WRITE, 0);
	_DevFc41dTaskAppend(fc41d, FC41D_PROCESS_STATUS_BLE, 
						FC41D_PROCESS_BLE_STATUS_STAT,
						DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

static void _RevDataAnalysis(void *userdata, uint8_t *p_data, uint16_t len)
{
	DevFc41d_t *fc41d = (DevFc41d_t *)userdata;
	fc41d->read(fc41d, (const uint8_t *)p_data, (int32_t)len);
}

static void _IrqRxProcess()
{
    static uint8_t index = 0;
    uint16_t len;
	/*DMA方式，空闲中断机制*/
	UartDMARxDisable(&WirelessUartHandle);
	len = UartDMARxDataLength(&WirelessUartHandle);
    _RevDataAnalysis(WirelessUartHandle.userdata, WirelessUartHandle.pRxBuf, len);
	UartDMARxEnable(&WirelessUartHandle);
}

static bool _DevFc41dDmaInit(DevFc41d_t *fc41d)
{
    WirelessUartHandle.pRxBuf = _wireless_rx_buf[0];
    WirelessUartHandle.rx_size = WIRELESS_DATA_RX_SIZE;/*最大接收长度，不是实际长度*/
    WirelessUartHandle.irq_rx_process = _IrqRxProcess;
	WirelessUartHandle.userdata = (void *)fc41d;
    
	WirelessBspInit();
    return true;
}

static void _DevFc41dDataInit(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return;
	}

	memset(fc41d->priv_data, 0, sizeof(DevFc41dCtx_t));
	fc41d->priv_data->usart_status = FC41D_COMM_STATUS_INIT;
	fc41d->priv_data->process_status = FC41D_PROCESS_STATUS_INIT;

	// 设置Wifi模式[AP | STATION]
	fc41d->priv_data->wifi.type = DEV_FC41D_WIFI_TYPE_STATION;

	memcpy(fc41d->priv_data->mqtt.clientname, gCollector->pcs_sn, 
		   sizeof(gCollector->pcs_sn));

	// 设置BLE, MQTT MTU
	fc41d->priv_data->ble.mtu = FC41D_BLE_MTU;
	fc41d->priv_data->mqtt.mtu = FC41D_MQTT_MTU;

	int32_t len = sizeof(fc41d->priv_data->rb_read.arr);
	// 初始化接收、 发送发缓冲区
	rbuffer_init(&fc41d->priv_data->rb_read.ctx, fc41d->priv_data->rb_read.arr, 
					 sizeof(fc41d->priv_data->rb_read.arr));

	rbuffer_init(&fc41d->priv_data->rb_write.ctx, fc41d->priv_data->rb_write.arr, 
					 sizeof(fc41d->priv_data->rb_write.arr));

	rbuffer_init(&fc41d->priv_data->rb_busy.ctx, fc41d->priv_data->rb_busy.arr, 
					 sizeof(fc41d->priv_data->rb_busy.arr));

	rbuffer_init(&fc41d->priv_data->nova_cmd.rb_data.ctx, 
				 fc41d->priv_data->nova_cmd.rb_data.arr, 
					 sizeof(fc41d->priv_data->nova_cmd.rb_data.arr));

	// 初始化 task list
	INIT_XLIST_HEAD(&fc41d->priv_data->task.l);

	// 初始化 http
	fc41d->priv_data->http.status = 0;
	fc41d->priv_data->http.resp_type = 1;		// URC 方式响应
	fc41d->priv_data->http.resp_header = 0; // 响应消息体不包含header
	fc41d->priv_data->http.timeout = 60;
	fc41d->priv_data->http.url = (char *)malloc(FC41D_HTTP_URL_LEN);
	fc41d->priv_data->http.url[0] = 0;
	fc41d->priv_data->http.custom_type = DEV_FC41D_HTTP_CUSTOM_NONE;

	// upgrade
	rbuffer_init(&fc41d->priv_data->upgrade.down.ctx, fc41d->priv_data->upgrade.down.arr, 
					 sizeof(fc41d->priv_data->upgrade.down.arr));

	// set callback
	fc41d->priv_data->fn.fn_usart_stat_get = _DevFc41dUsartStatusGet;
	fc41d->priv_data->fn.fn_usart_stat_set = _DevFc41dUsartStatusSet;
	fc41d->priv_data->fn.fn_process_stat_get = _DevFc41dStatusGet;

	gCollectorDevInfo.wifi_module_flag = EN_WIFI_MODULE_OFF_GRID;
}

int32_t _DevFc41dSetUpgradeFlashSectorFlag(void *fc41d)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t i = 0;
	uint8_t buf[1024] = {0};

	memset(buf, 0x0A, sizeof(buf));

	while(i < 0x001000) 
	{
		SpiFlashWriteBytesWithErase(0x00, buf, sizeof(buf));
		i += sizeof(buf);
	}

	return 0;
}

int32_t _DevFc41dCleanUpgradeFlashSector()
{
	int32_t count = 0;
    uint32_t addr = 0x1000 * 8; // 32KB
	while (count < 32*2) // 2MB
	{
		//SpiFlashEraseBlock64K(addr*count);
		SpiFlashEraseBlock32K(addr*count);
		++count;
	}
	return 0;
}

int32_t DevFc41dCheckUpgrade(DevFc41d_t *fc41d)
{
	return _DevFc41dCheckUpgrade(fc41d);
}

int32_t DevFc41dCheckNetSetting(DevFc41d_t *fc41d)
{
	return _DevFc41dCheckNetSetting(fc41d);
}

int32_t DevFc41dProcessing(DevFc41d_t *fc41d)
{
	return _DevFc41dProcessing(fc41d);
}

int32_t DevFc41dReset(DevFc41d_t *fc41d, int32_t clean_data_flag)
{

	_DevFc41dDataReset(fc41d, clean_data_flag);
	return _DevFc41dTaskInhead(fc41d, FC41D_PROCESS_STATUS_INIT, 0, DEV_FC41D_TASK_STAT_WRITE, 0);
}

int32_t DevFc41dGetBleMac(DevFc41d_t *fc41d, uint8_t *mac, int32_t len)
{
	memcpy(mac, fc41d->priv_data->ble.mac, 
		   len<sizeof(fc41d->priv_data->ble.mac)?
		   len:sizeof(fc41d->priv_data->ble.mac));
	return sizeof(fc41d->priv_data->ble.mac);
}

int32_t DevFc41dGetWifiSsid(DevFc41d_t *fc41d, uint8_t *ssid, int32_t len)
{
	memcpy(ssid, fc41d->priv_data->wifi.info.station.ssid, 
		   len<sizeof(fc41d->priv_data->wifi.info.station.ssid)?
		   len:sizeof(fc41d->priv_data->wifi.info.station.ssid));
	return sizeof(fc41d->priv_data->wifi.info.station.ssid);
}

int32_t DevFc41dGetWifiPwd(DevFc41d_t *fc41d, uint8_t *pwd, int32_t len)
{
	memcpy(pwd, fc41d->priv_data->wifi.info.station.pwd,
		   len<sizeof(fc41d->priv_data->wifi.info.station.pwd)?
		   len:sizeof(fc41d->priv_data->wifi.info.station.pwd));
	return sizeof(fc41d->priv_data->wifi.info.station.pwd);
}

int32_t DevFc41dGetWifiRssi(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->wifi.info.station.rssi;
}

int32_t DevFc41dWifiSet(DevFc41d_t *fc41d, const char *ssid, const char *pwd)
{
    int32_t len[2] = {strlen(ssid), strlen(pwd)};
	memcpy(fc41d->priv_data->wifi.info.station.ssid, ssid,
		   len[0]<sizeof(fc41d->priv_data->wifi.info.station.ssid)?
		   len[0]:sizeof(fc41d->priv_data->wifi.info.station.ssid));
	memcpy(fc41d->priv_data->wifi.info.station.pwd, pwd,
		   len[1]<sizeof(fc41d->priv_data->wifi.info.station.pwd)?
		   len[1]:sizeof(fc41d->priv_data->wifi.info.station.pwd));
	return 0;
}

int32_t DevFc41dGetBleName(DevFc41d_t *fc41d, uint8_t *ble, int32_t len)
{
	memcpy(ble, fc41d->priv_data->ble.name,
		   len<sizeof(fc41d->priv_data->ble.name)?
		   len:sizeof(fc41d->priv_data->ble.name));
	return sizeof(fc41d->priv_data->ble.name);
}

int32_t DevFc41dGetMqttServerIp(DevFc41d_t *fc41d, uint8_t *ip, int32_t len)
{
	memcpy(ip, fc41d->priv_data->mqtt.server_ip,
		   len<sizeof(fc41d->priv_data->mqtt.server_ip)?
		   len:sizeof(fc41d->priv_data->mqtt.server_ip));
	return sizeof(fc41d->priv_data->mqtt.server_ip);
}

int32_t DevFc41dGetMqttServerPort(DevFc41d_t *fc41d, uint8_t *port, int32_t len)
{
	memcpy(port, fc41d->priv_data->mqtt.server_port, 
		   len<sizeof(fc41d->priv_data->mqtt.server_port)?
		   len:sizeof(fc41d->priv_data->mqtt.server_port));
	return sizeof(fc41d->priv_data->mqtt.server_port);
}

int8_t DevFc41dGetMqttServerType(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->mqtt.server_type;
}

int32_t DevFc41dMqttSet(DevFc41d_t *fc41d, int8_t type, const char *ip, const char *port)
{
	fc41d->priv_data->mqtt.server_type = type;
	memcpy(fc41d->priv_data->mqtt.clientname, gCollector->pcs_sn, 
		   sizeof(gCollector->pcs_sn));
	if (ip != NULL)
	{
		memcpy(fc41d->priv_data->mqtt.server_ip, ip,
			   strlen(ip)<sizeof(fc41d->priv_data->mqtt.server_ip)?
			   strlen(ip):sizeof(fc41d->priv_data->mqtt.server_ip));
	}

	if (port != NULL)
	{
		memcpy(fc41d->priv_data->mqtt.server_port, port,
			   strlen(port)<sizeof(fc41d->priv_data->mqtt.server_port)?
			   strlen(port):sizeof(fc41d->priv_data->mqtt.server_port));
	}
	return 0;
}

int32_t DevFc41dIsBusy(DevFc41d_t *fc41d)
{
	int32_t busy_len = rbuffer_length(&fc41d->priv_data->rb_busy.ctx);

	if (fc41d->priv_data->write_stat.employ_time > 0)
	{
		uint32_t cur = tim_get_systime();
		if (cur < fc41d->priv_data->write_stat.employ_time + 10000)
		{
			busy_len = 1;
		}
	}
	return busy_len;
}

int32_t DevFc41dGetTaskSize(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->task_size;
}

int8_t DevFc41dGetStat(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->stat;
}

char* DevFc41dGetFc41dVer(DevFc41d_t *fc41d)
{
	return (char *)fc41d->priv_data->com_info.ver;
}

int32_t DevFc41dGetBleConnStat(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->ble.conn;
}

int32_t DevFc41dInit(DevFc41d_t **fc41d)
{
	DevFc41d_t *p = NULL;
	if (fc41d == NULL) 
	{
		return -1;
	}
    
#if 0
    int32_t malloc_size = sizeof(DevFc41d_t) + sizeof(DevFc41dCtx_t);
	p = (DevFc41d_t *)malloc(sizeof(DevFc41d_t) + sizeof(DevFc41dCtx_t));

	if (p == NULL) 
	{
		return -1;
	}
#else 
    int32_t malloc_size = sizeof(DevFc41dTaskList_t) * FC41D_TASK_MAX_SIZE;
    malloc_size = sizeof(DevFc41d_t);
    malloc_size = sizeof(DevFc41dCtx_t);
    malloc_size = sizeof(DevFc41d_t) + sizeof(DevFc41dCtx_t);
	p = (DevFc41d_t *)_gFc41dCtx;
#endif 

	p->priv_data = (DevFc41dCtx_t*)(p+1);
	p->del = _DevFc41dDel;
	p->read = _DevFc41dRead;
	p->write = _DevFc41dSend;
	p->mqtt_pub_resp = _DevFc41dPubMqttResp;
	p->mqtt_pub_meta = _DevFc41dPubMqttMeta;
	p->mqtt_pub = _DevFc41dPubMqtt;
	p->http_req = _DevFc41dHttpReq;
	p->process = _DevFc41dProcess;
	p->task_head_del = _DevFc41dTaskHeadDel;
	p->task_push_inhead = _DevFc41dTaskInhead;
	p->task_push_append = _DevFc41dTaskAppend;

	*fc41d = p;
	_DevFc41dDataInit(p);
	_DevFc41dInitConf(p);
	_DevFc41dDmaInit(p);

	// init fc41d 
	_DevFc41dTaskAppend(p, FC41D_PROCESS_STATUS_INIT, 0, DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

