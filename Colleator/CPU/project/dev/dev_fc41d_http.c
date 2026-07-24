#include "dev_fc41d_common.h"
#include "dev_fc41d_aliyun.h"
#include "dev_spiflash.h"
#include "nova_dev_upgrade.h"

// AT HTTP CFG
#define HTTP_H_USER_AGENT						",\"NovaStar Collector_FC41D\""
#define HTTP_H_RES_URC							1
#define HTTP_H_RES_NONE							1
#define HTTP_REQ_TIMEOUT						60

#define AT_FC41D_HTTP_REQ_URL					"AT+QHTTPCFG=\"url\",\"%s\"\r\n" // HTTP URL
#define AT_FC41D_HTTP_REQ_H_USER_AGENT			"AT+QHTTPCFG=\"header\",\"User-Agent\"%s\r\n"
#define AT_FC41D_HTTP_RES_URC					"AT+QHTTPCFG=\"response/output\",%d\r\n"
#define AT_FC41D_HTTP_RES_H_NONE				"AT+QHTTPCFG=\"response/header\",%d\r\n"

#define AT_FC41D_HTTP_GET_RES					"+QHTTPGET:"
#define AT_FC41D_HTTP_REQ_GET					"AT+QHTTPGET=%d\r\n"

#define AT_FC41D_HTTP_URC_RECV					"+QHTTPURC: \"recv\""

extern void _DevFc41dTaskClean(DevFc41d_t *fc41d);
extern int32_t _DevFc41dTaskHeadSetStat(DevFc41d_t *fc41d, int8_t stat);
extern int32_t _DevFc41dCleanUpgradeFlashSector();
extern void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d);
static int32_t _DevFc41dHttpGetUpgrade(DevFc41d_t *fc41d, int8_t first);
static int32_t _DevFc41dHttpUpgradeReboot(DevFc41d_t *fc41d);

static int32_t _DevFc41dHttpStatusGet(DevFc41d_t *fc41d)
{
	return (fc41d==NULL?FC41D_PROCESS_HTTP_STATUS_OFF
			:fc41d->priv_data->http.status);
}

int32_t _DevFc41dHttpStatusSet(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}
	fc41d->priv_data->http.status = status;
	return 0;
}

static int32_t _DevFc41dHttpSubCmdSet(DevFc41d_t *fc41d, int32_t cmd)
{
	fc41d->priv_data->http.sub_cmd = cmd;
	return 0;
}

int32_t _DevFc41dHttpSubCmdGet(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->http.sub_cmd;
}

static int32_t _DevFc41dHttpCmdSet(DevFc41d_t *fc41d, int32_t cmd)
{
	fc41d->priv_data->http.cmd = cmd;
	return 0;
}

static int32_t _DevFc41dHttpCmdGet(DevFc41d_t *fc41d)
{
	return fc41d->priv_data->http.cmd;
}

static int32_t _DevFc41dReadHttpCfgSetHeaderCustom(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d->priv_data->http.custom_type == DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS)
	{
#ifdef	FC41D_HTTP_ALIYUN_OSS
		return DevFc41dReadHttpCfgSetHeaderAliyun(fc41d, status);
#endif /*#ifdef	FC41D_HTTP_ALIYUN_OSS*/
	}
	return 0;
}

static int32_t _DevFc41dWriteHttpCfgSetHeaderCustom(DevFc41d_t *fc41d, int32_t status,
														   char *cmd, int32_t cmd_size)
{
	if (fc41d->priv_data->http.custom_type == DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS)
	{
#ifdef	FC41D_HTTP_ALIYUN_OSS
		return DevFc41dWriteHttpCfgSetHeaderAliyun(fc41d, status, cmd, cmd_size);
#endif /*#ifdef	FC41D_HTTP_ALIYUN_OSS*/
	}
	return 0;
}

static int32_t _DevFc41dReadHttpCfg(DevFc41d_t *fc41d, int32_t status, 
										   const uint8_t *data, int32_t len)
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
	case FC41D_PROCESS_HTTP_SET_USE_URC:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_SET_URC_SKIP_HEADER,
							DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_HTTP_SET_URC_SKIP_HEADER:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_SET_URL,
							DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_HTTP_SET_URL:
		fc41d->task_push_inhead(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_SET_H_USER_AGENT,
								DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	default:
		_DevFc41dReadHttpCfgSetHeaderCustom(fc41d, status);
		break;
	}

	return read;
}

static int32_t _DevFc41dWriteHttpCfg(DevFc41d_t *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	char cmd[1024] = {0};
	switch(status)
	{
	case FC41D_PROCESS_HTTP_SET_USE_URC:
		snprintf(cmd, sizeof(cmd), AT_FC41D_HTTP_RES_URC, HTTP_H_RES_URC);
		break;
	case FC41D_PROCESS_HTTP_SET_URC_SKIP_HEADER:
		snprintf(cmd, sizeof(cmd), AT_FC41D_HTTP_RES_H_NONE, HTTP_H_RES_NONE);
		break;
	case FC41D_PROCESS_HTTP_SET_URL:
		snprintf(cmd, sizeof(cmd), AT_FC41D_HTTP_REQ_URL, 
				 fc41d->priv_data->http.url==NULL?"":fc41d->priv_data->http.url);
		break;
	case FC41D_PROCESS_HTTP_SET_H_USER_AGENT:
		snprintf(cmd, sizeof(cmd), AT_FC41D_HTTP_REQ_H_USER_AGENT, HTTP_H_USER_AGENT);
		break;
	default:
		_DevFc41dWriteHttpCfgSetHeaderCustom(fc41d, status, cmd, sizeof(cmd));
		break;
	}

	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadHttpTest(DevFc41d_t *fc41d, const uint8_t *data, 
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

static int32_t _DevFc41dWriteHttpTest(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	char cmd[] = {"AT\r\n"};
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadHttpGet(DevFc41d_t *fc41d, const uint8_t *data, 
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
		find[strlen(AT_OK)-1] = '\n';

		find = strstr((const char *)data, AT_FC41D_HTTP_GET_RES);
		if (find == NULL)
		{
			// not enough
			return 0;
		}

		char tmp[64] = {0};
		int32_t tmp_len = strlen(find), result = 0, http_res_code = 0, content_lenth = 0;

		find += strlen(AT_FC41D_HTTP_GET_RES);
		tmp[0] = find[0];
		find[0] = 0;
		read = strlen((const char *)data);
		find[0] = tmp[0];

		for (int32_t i = 0, j = 0, k = 0; *find != 0 && i < tmp_len; ++find,++read)
		{
			if (*find == '\r' && *(find+1) == '\n')
			{
				tmp[j] = 0;
				content_lenth = atoi(tmp);
				break;
			}
			if (*find == ',') 
			{
				tmp[j] = 0;
				j = 0;
				++i;
				++k;
				if (k == 1)
				{
					result = atoi(tmp);
				} 
				else if (k == 2)
				{
					http_res_code = atoi(tmp);
				}
				continue;
			}

			tmp[j++] = *find;
		}
		fc41d->priv_data->http._continue = content_lenth;

		fc41d->task_head_del(fc41d);
		return read;
	}

	find = strstr((const char *)data, AT_ERR);
	if (find) 
	{
		// write result to txbuffer
		rbuffer_write_byte(&fc41d->priv_data->rb_write.ctx, 0x00);
		find[strlen(AT_ERR)-1] = 0;
		read = strlen((const char *)data)+1;

		fc41d->task_head_del(fc41d);
		return read;
	}

	return 0;
}

static int32_t _DevFc41dWriteHttpGet(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	char cmd[32] = {0};
	snprintf(cmd, sizeof(cmd), AT_FC41D_HTTP_REQ_GET, HTTP_REQ_TIMEOUT);
	fc41d->write(fc41d, (const uint8_t *)cmd, strlen(cmd));
	return 0;
}

static int32_t _DevFc41dReadOtaFc41d(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
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

		fc41d->priv_data->http.ota_fc41d = 2;
		fc41d->priv_data->http.ota_fc41d_reboot_time = tim_get_systime();
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

	return len;
}

static int32_t _DevFc41dWriteOtaFc41d(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	if (_DevFc41dTaskHeadSetStat(fc41d, DEV_FC41D_TASK_STAT_READ) != 0)
	{
		return -2;
	}

	fc41d->write(fc41d, (const uint8_t *)fc41d->priv_data->http.url, 
				 strlen(fc41d->priv_data->http.url));
	return 0;
}

int32_t _DevFc41dReadHttp(DevFc41d_t *fc41d, const uint8_t *data, int32_t len)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dHttpStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_HTTP_SET_USE_URC:
	case FC41D_PROCESS_HTTP_SET_URC_SKIP_HEADER:
	case FC41D_PROCESS_HTTP_SET_URL:
	case FC41D_PROCESS_HTTP_SET_H_USER_AGENT:
#ifdef FC41D_HTTP_ALIYUN_OSS
	case FC41D_PROCESS_HTTP_SET_H_DATE:
	case FC41D_PROCESS_HTTP_SET_H_AUTH:
	case FC41D_PROCESS_HTTP_SET_H_RANGE:
#endif /*FC41D_HTTP_ALIYUN_OSS*/
		return _DevFc41dReadHttpCfg(fc41d, status, data, len);
	case FC41D_PROCESS_HTTP_TEST:
		return _DevFc41dReadHttpTest(fc41d, data, len);
	case FC41D_PROCESS_HTTP_GET:
		return _DevFc41dReadHttpGet(fc41d, data, len);
	case FC41D_PROCESS_HTTP_DEV_UPGRADE_RETRY:
		return _DevFc41dHttpGetUpgrade(fc41d, 1);
	case FC41D_PROCESS_HTTP_OTA_FC41D:
		return _DevFc41dReadOtaFc41d(fc41d, data, len);
	}

	return 0;
}

int32_t _DevFc41dWriteHttp(DevFc41d_t *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	int32_t status = _DevFc41dHttpStatusGet(fc41d);
	switch (status) 
	{
	case FC41D_PROCESS_HTTP_SET_USE_URC:
	case FC41D_PROCESS_HTTP_SET_URC_SKIP_HEADER:
	case FC41D_PROCESS_HTTP_SET_URL:
	case FC41D_PROCESS_HTTP_SET_H_USER_AGENT:
#ifdef FC41D_HTTP_ALIYUN_OSS
	case FC41D_PROCESS_HTTP_SET_H_DATE:
	case FC41D_PROCESS_HTTP_SET_H_AUTH:
	case FC41D_PROCESS_HTTP_SET_H_RANGE:
#endif /*FC41D_HTTP_ALIYUN_OSS*/
		_DevFc41dWriteHttpCfg(fc41d, status);
		break;
	case FC41D_PROCESS_HTTP_TEST:
		_DevFc41dWriteHttpTest(fc41d);
		break;
	case FC41D_PROCESS_HTTP_GET:
		_DevFc41dWriteHttpGet(fc41d);
		break;
	case FC41D_PROCESS_HTTP_DEV_UPGRADE_START:
		_DevFc41dHttpGetUpgrade(fc41d, 1);
		break;
	case FC41D_PROCESS_HTTP_DEV_UPGRADE_DOWN:
		_DevFc41dHttpUpgradeReboot(fc41d);
		break;
	case FC41D_PROCESS_HTTP_OTA_FC41D:
		_DevFc41dWriteOtaFc41d(fc41d);
		break;
	}

	return 0;
}

static int32_t _DevFc41dHttpGetUpgrade1Ack(DevFc41d_t *fc41d, void *uniform, int32_t cmd)
{
	_DevFc41dHttpSubCmdSet(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_1);

	if (cmd == PROTO_UNIFORM_W_DEV_UPGRADE) // 采集器自身升级
	{
		// clean flsh 0 ~ 63 sector
		_DevFc41dCleanUpgradeFlashSector();
	}
	else if (cmd == PROTO_UNIFORM_W_DEV_UPGRADE_PCS) // PCS升级
	{
		// clean flash 64 ~ 5111 sector
		NovaDevCleanUpgradeFlashSector();
		NovaDevUpgradeCtxReset();
	}

	const char mqtt_info[] = {"ok"};

	// ack starting 
	if (uniform)
	{
		fc41d->mqtt_pub_resp(fc41d, uniform, 0, cmd, 0, (const uint8_t *)mqtt_info, 
							 strlen(mqtt_info));
	}
	else
	{
		fc41d->mqtt_pub(fc41d, 0, cmd, 0, (const uint8_t *)mqtt_info, strlen(mqtt_info));
	}

	// add download file 
	fc41d->priv_data->upgrade.type = EN_UPGRADE_TYPE_REMOTE;
	fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_INIT;
	fc41d->priv_data->upgrade.time = tim_get_systime();
	fc41d->priv_data->upgrade.retry_count = 0;
	return 0;
}

int32_t _DevFc41dHttpGetUpgrade2Ack(DevFc41d_t *fc41d, int8_t stat)
{
	if (stat == 0)
	{
		_DevFc41dHttpSubCmdSet(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_3);
	}
	else
	{
		_DevFc41dHttpSubCmdSet(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_5);
	}

	uint16_t modelid = gCollector->type;
	switch (_DevFc41dHttpCmdGet(fc41d))
	{
	case PROTO_UNIFORM_W_DEV_UPGRADE_PCS:
		modelid = gCollector->pcs_modelid;
		_DevFc41dHttpSubCmdSet(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_6);
		break;
	}


	uint8_t mqtt_info[128] = {0};
	uint8_t split_char[2] = {0x0D, 0x0A};
	int32_t mqtt_len = 0;
	mqtt_len = sizeof(gCollector->pcs_modelid) + sizeof(uint8_t);
	memcpy(mqtt_info, &modelid, sizeof(modelid));
	memcpy(mqtt_info+sizeof(modelid), &stat, sizeof(stat));

	// 0x0D 0x0A
	memcpy(mqtt_info+mqtt_len, split_char,  sizeof(split_char));
	mqtt_len += sizeof(split_char);

	memcpy(mqtt_info+mqtt_len, fc41d->priv_data->http.custom.ali_oss.etag, 
		   sizeof(fc41d->priv_data->http.custom.ali_oss.etag));
	mqtt_len += sizeof(fc41d->priv_data->http.custom.ali_oss.etag);

	// 0x0D 0x0A
	memcpy(mqtt_info+mqtt_len, split_char,  sizeof(split_char));
	mqtt_len += sizeof(split_char);

	memcpy(mqtt_info+mqtt_len, fc41d->priv_data->http.custom.ali_oss._md5, 
		   sizeof(fc41d->priv_data->http.custom.ali_oss._md5));
	mqtt_len += sizeof(fc41d->priv_data->http.custom.ali_oss._md5);

	_DevFc41dTaskClean(fc41d);
    fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;

	// ack starting 
	fc41d->mqtt_pub(fc41d, 0, PROTO_UNIFORM_R_DEV_UPGRADE_STAT, stat, 
					(const uint8_t *)mqtt_info, mqtt_len);
	
	// fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_COMPLETE;
	_DevFc41dResetUpgradeData(fc41d);
	return 0;
}

static int32_t _DevFc41dHttpGetUpgrade(DevFc41d_t *fc41d, int8_t first)
{
	// start upgrade
	fc41d->priv_data->upgrade.type = EN_UPGRADE_TYPE_REMOTE;
	fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_INIT;
	fc41d->priv_data->http.first_pkg = 0;
	fc41d->priv_data->http._continue = 0;

	fc41d->task_head_del(fc41d);
	if (first)
	{
		fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);

		_DevFc41dHttpSubCmdSet(fc41d, HTTP_SUB_CMD_UPGRADE_COLLECTOR_2);

		fc41d->priv_data->upgrade.time = tim_get_systime();
	}

	rbuffer_clear(&fc41d->priv_data->rb_read.ctx);
	rbuffer_clear(&fc41d->priv_data->rb_busy.ctx);

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_HTTP;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));

	fc41d->priv_data->write_stat.timeout = tim_get_systime();

	// set http cfg
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, FC41D_PROCESS_HTTP_TEST,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, FC41D_PROCESS_HTTP_TEST,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	if (first)
	{
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_SET_USE_URC,
								DEV_FC41D_TASK_STAT_WRITE, 0);
	}
	else
	{
#ifdef FC41D_HTTP_ALIYUN_OSS
		fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
								FC41D_PROCESS_HTTP_SET_H_RANGE,
								DEV_FC41D_TASK_STAT_WRITE, 0);
#endif 
	}
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_GET,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

static int32_t _DevFc41dHttpUpgradeReboot(DevFc41d_t *fc41d)
{
	fc41d->priv_data->upgrade.type = EN_UPGRADE_TYPE_NONE;
	fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
	fc41d->priv_data->fn.fn_usart_stat_set(fc41d, FC41D_COMM_STATUS_IDLE);
	fc41d->priv_data->upgrade.stat = 0;
	fc41d->priv_data->upgrade.time = 0;
	rbuffer_clear(&fc41d->priv_data->upgrade.down.ctx);


	int32_t status = _DevFc41dHttpSubCmdGet(fc41d);
	if (status == HTTP_SUB_CMD_UPGRADE_COLLECTOR_3)
	{
		delay_1ms(1000);

		// reset
		__set_FAULTMASK(1);
		NVIC_SystemReset();
		return 0;
	}
	else if (status == HTTP_SUB_CMD_UPGRADE_COLLECTOR_6)
	{
		// 升级下位机不做其他动作

	}

	fc41d->task_head_del(fc41d);
	return 0;
}

int32_t DevFc41dHttpGetContinue(DevFc41d_t *fc41d)
{
	return  _DevFc41dHttpGetUpgrade(fc41d, 1);
}

int32_t _DevFc41dUrcHttpHeaderResultCode(char *data, int32_t len, int32_t *result)
{
	int32_t ret = 0;
	char *p = data;
	char code[8] = {0};

	for (int32_t i = 0, j = 0; ret < len; ++ret, ++p)
	{
		if (p[0] == '\r' && p[1] == '\n' && p[2] == 'H' && p[3] == 'T' && p[4] == 'T'
			&& p[5] == 'P' && p[6] == '/' && p[7] == '1' && p[8] == '.' && p[9] == '1'
			&& p[10] == ' ')
		{
			i = 1;
            p += 10;
			ret += 10;
			continue;
		}
		if (i == 1) 
		{
			if (*p == ' ' || j > 6)
			{
				if (result != NULL)
				{
					*result = atoi(code);
				}
				break;
			}
			code[j++] = *p;
		}

	}

	return ret;
}

int32_t _DevFc41dUrcHttpHeaderSkip(char *data, int32_t len)
{
	int32_t ret = 0;
	char *p = data;
	int32_t i = 0;

	for (; ret < len; ++ret, ++p)
	{
		if (p[0] == '\r' && p[1] == '\n' && p[2] == '\r' && p[3] == '\n')
		{
			i = 1;
			ret += 4;
			break;
		}
	}

	if (i == 0) 
	{
		return -1;
	}

	return ret;
}

int32_t _DevFc41dUrcHttpResult(DevFc41d_t *fc41d, int8_t stat)
{
	fc41d->priv_data->http.first_pkg = 0;
	fc41d->priv_data->http._continue = 0;
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_NONE;

	switch (_DevFc41dHttpCmdGet(fc41d))
	{
	case PROTO_UNIFORM_W_DEV_UPGRADE:
		_DevFc41dHttpGetUpgrade2Ack(fc41d, stat);
		break;
	case PROTO_UNIFORM_W_DEV_UPGRADE_PCS:
		_DevFc41dHttpGetUpgrade2Ack(fc41d, stat);
		fc41d->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;
		break;
	}
	_DevFc41dHttpCmdSet(fc41d, 0);

	return 0;
}

int32_t _DevFc41dUrcHttp(DevFc41d_t *fc41d, const char *data, int32_t len)
{
	int32_t ret = 0;
	int32_t i = -1, j = 0;
	int32_t http_ret_code = 0;
	char tmp_split = 0;
	char *p = (char *)data;
    
	if (fc41d->priv_data->http._continue == 0)
	{
        if (fc41d->priv_data->task_size != 0)
        {
            return ret;
        }

		char *find = strstr((const char *)data, AT_FC41D_HTTP_URC_RECV);
		if (!find)
		{
			return ret;
		}

		if (strncmp(data, AT_FC41D_HTTP_URC_RECV, strlen(AT_FC41D_HTTP_URC_RECV))) 
		{
			int32_t xx = 0;
		}
		tmp_split = find[strlen(AT_FC41D_HTTP_URC_RECV)-1];
		find[strlen(AT_FC41D_HTTP_URC_RECV)-1] = 0;
		find[strlen(AT_FC41D_HTTP_URC_RECV)-1] = tmp_split;
		ret += strlen(data);
		p += ret;

	}

	switch (fc41d->priv_data->http.custom_type)
	{
	case DEV_FC41D_HTTP_CUSTOM_ALIYUNOSS:
		{
		static void *ptr = NULL;
		if (ptr != fc41d->priv_data->http.custom.ali_oss.fn_urc)
		{
			ptr = fc41d->priv_data->http.custom.ali_oss.fn_urc;
		}
#ifdef FC41D_HTTP_ALIYUN_OSS
		if (fc41d->priv_data->http.custom.ali_oss.fn_urc)
		{
			i = fc41d->priv_data->http.custom.ali_oss.fn_urc((void *)fc41d, p, len-ret, &j);
		}
#endif 
		break;
		}
	case DEV_FC41D_HTTP_CUSTOM_MQTT_INFO:
		if (fc41d->priv_data->http.custom.mqtt_info.fn_urc)
		{
			i = fc41d->priv_data->http.custom.mqtt_info.fn_urc((void *)fc41d, p, len-ret, &j);
		}
		break;
	}

	if (i < 0)
	{
		// recv error & reset
		_DevFc41dUrcHttpResult(fc41d, -i);
		return len;
	}
	ret += j;
    
    if (i == 1)
	{
		// 1 pkg not enough
		return ret;
	}

	if (i == 0) 
	{
		// recv complete 
		_DevFc41dUrcHttpResult(fc41d, 0);
		return len;
	}

	// req next pkg
	switch (_DevFc41dHttpCmdGet(fc41d))
	{
	case PROTO_UNIFORM_W_DEV_UPGRADE:
	case PROTO_UNIFORM_W_DEV_UPGRADE_PCS:
		_DevFc41dHttpGetUpgrade(fc41d, 0);
		break;
	}

	return len;
}

static int32_t _DevFc41dHttpGet1(DevFc41d_t *fc41d)
{
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_HTTP;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));

	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, FC41D_PROCESS_HTTP_TEST,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	return 0;
}

static int32_t _DevFc41dHttpGet2(DevFc41d_t *fc41d)
{
	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_HTTP;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));

	fc41d->priv_data->write_stat.timeout = tim_get_systime();
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, FC41D_PROCESS_HTTP_GET,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

static int32_t _DevFc41dHttpGetMqtt(DevFc41d_t *fc41d)
{

	DevFc41dHttpMqttInfoInit((void *)fc41d);

	fc41d->priv_data->write_stat.status = DEV_FC41D_WRITE_HTTP;
	int32_t send_len = 0;
	uint8_t meta_uniform[64] = {0};
	DevFc41dWriteCtx_t write_ctx = {0};
	// clear tx buffer
	rbuffer_clear(&fc41d->priv_data->rb_write.ctx);
	// write pub_idx->msgid, qos
	send_len = rbuffer_write_bytes(&fc41d->priv_data->rb_write.ctx, 
						(const uint8_t *)&write_ctx, sizeof(DevFc41dWriteCtx_t));

	fc41d->priv_data->write_stat.timeout = tim_get_systime();

	// set http cfg
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, FC41D_PROCESS_HTTP_TEST,
							DEV_FC41D_TASK_STAT_WRITE, 0);
	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, FC41D_PROCESS_HTTP_TEST,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_SET_USE_URC,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	fc41d->task_push_append(fc41d, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_GET,
							DEV_FC41D_TASK_STAT_WRITE, 0);

	return 0;
}

int32_t _DevFc41dHttpReq(DevFc41d_t *fc41d, void *uniform, int32_t cmd, 
								uint16_t err, const uint8_t *data, int32_t len)
{
	UniformProtocolHead_t *u = (UniformProtocolHead_t *)uniform;

	if (data == NULL || len <= 0)
	{
		return -1;
	}

	switch (cmd)
	{
	case PROTO_UNIFORM_W_DEV_UPGRADE:
	case PROTO_UNIFORM_W_DEV_UPGRADE_PCS:
		_DevFc41dHttpCmdSet(fc41d, cmd);
		_DevFc41dHttpGetUpgrade1Ack(fc41d, uniform, cmd);
		break;
	case PROTO_UNIFORM_W_DEV_UPGRADE+100:
		_DevFc41dHttpGet1(fc41d);
		break;
	case PROTO_UNIFORM_W_DEV_UPGRADE+200:
		_DevFc41dHttpGet2(fc41d);
		break;
	case PROTO_UNIFORM_R_MQTT_BY_HTTP:
		_DevFc41dHttpGetMqtt(fc41d);
		break;
	}

	return 0;
}

