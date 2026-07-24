#include "dev_fc41d_http_mqtt_info.h"
#include "dev_fc41d_common.h"
#include "dev_spiflash.h"
#include "base64.h"
#include "aes2.h"

#define AT_FC41D_HTTP_REQ_H_DATE				"AT+QHTTPCFG=\"header\",\"Date\"%s\r\n"
#define AT_FC41D_HTTP_REQ_H_AUTH				"AT+QHTTPCFG=\"header\",\"Authorization\"%s\r\n"
#define AT_FC41D_HTTP_REQ_H_RANGE				"AT+QHTTPCFG=\"header\",\"Range\"%s\r\n"

#define AT_FC41D_HTTP_URC_RECV					"+QHTTPURC: \"recv\""

extern int32_t _DevFc41dUrcHttpHeaderResultCode(char *data, int32_t len, int32_t *result);
extern int32_t _DevFc41dUrcHttpHeaderSkip(char *data, int32_t len);
extern int32_t _DevFc41dSetUpgradeFlashSectorFlag(void *fc41d);
extern void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d);
static int32_t _DevFc41dMqttInfoUrc(void *fc41d, const char *data, int32_t len, int32_t *read);
extern int32_t CollectorSpiFlashWriteCfg();
extern int32_t string_to_bytes(char *s, int l, char *d);

void DevFc41dHttpMqttInfoUninit(void *fc41d)
{
	if (fc41d == NULL) 
	{
		return;
	}
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	memset(&f->priv_data->http.custom.mqtt_info, 0, sizeof(f->priv_data->http.custom.mqtt_info));
}

int32_t DevFc41dHttpMqttInfoInit(void *fc41d)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	//f->priv_data->http.custom_type = DEV_FC41D_HTTP_CUSTOM_MQTT_INFO;
	f->priv_data->http.custom.mqtt_info.http_ret_code = 0;
	f->priv_data->http.custom.mqtt_info.fn_urc = _DevFc41dMqttInfoUrc;

    return 0;
}

int32_t DevFc41dReadHttpCfgSetHeaderMqttInfo(void *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	switch(status)
	{
	default:break;
	}
    
	return 0;
}

int32_t DevFc41dWriteHttpCfgSetHeaderMqttInfo(void *fc41d, int32_t status, 
												   char *cmd, int32_t cmd_size)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	switch(status)
	{
	default:break;
	}
    
	return 0;
}

static int32_t _DevFc41dAliyunUrcHttpHeaderEcCode(char *data, int32_t len, char *ec)
{
	int32_t ret = 0;

	return 0;
}

static int32_t _DevFc41dParseHttpCode(void *fc41d, const char *data, int32_t len)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t ret = 0, i = 0;
	char *p = (char *)data;

	if (f->priv_data->http.custom.mqtt_info.http_ret_code == 200)
	{
		return 0;
	}

	int32_t http_ret_code = 0;

	if (len < 256)
	{
		return -1;
	}

	// pasrse http ret code = 200
	i = _DevFc41dUrcHttpHeaderResultCode(p, len-ret, &http_ret_code);
	if (i < 0)
	{
		return -2;
	}

	f->priv_data->http.custom.mqtt_info.http_ret_code = http_ret_code;
	ret += i;
	p += i;
	if (f->priv_data->http.custom.mqtt_info.http_ret_code == 200)
	{
		// skip headers
		i = _DevFc41dUrcHttpHeaderSkip(p, len-ret);
		if (i > 0)
		{
			ret += i;
			p += i;
		}
		return ret;
	}

	return -3;
}

static int32_t _DevFc41dParseData(void *fc41d, const char *data, int32_t len, 
										int32_t first)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t ret = first + (first>0?1:0);
	char *p[2] = {(char *)(data + ret), (char *)data};
	ret = len - 3 - first - 1; // 去掉 0x22, 0x0D, 0x0A 结束标记
	char buf[512] = {0};

	// 1. base64
	p[1][len-3] = 0x00;
	base64_decode((unsigned char *)p[0], ret, (unsigned char *)buf);

	// 2. decode crypt data
	ret = strlen(buf);
	int32_t i = 0, j = 0, k = 0;
	for (p[1] = buf; i < ret - 1; ++i)
	{
		if ((buf[i] == 0x0D && buf[i+1] == 0x0A)
			|| buf[i] == ':')
		{
			// parse data
			buf[i] = 0;
			++j;

			switch (j)
			{
				case 1:		// mqtt ip
					snprintf(f->priv_data->mqtt.server_ip, 
							 sizeof(f->priv_data->mqtt.server_ip), "%s", p[1]);
					++i;
					p[1] = &buf[i+1];
					break;	
				case 2:		// mqtt port
					snprintf(f->priv_data->mqtt.server_port, 
							 sizeof(f->priv_data->mqtt.server_port), "%s", p[1]);
					++i;
					p[1] = &buf[i+1];
					break;
				case 3:		// mqtt auth type(pwd)
					++i;
					p[1] = &buf[i+1];
					break;
				case 4:		// token encrypt type(AES CBC 256)
					f->priv_data->http.custom.mqtt_info.encrypt.type = p[1][0] - 0x30;
					p[1] = &buf[i+1];
					break;
				case 5:		// time
					snprintf(f->priv_data->http.custom.mqtt_info.encrypt.
							 union_type.aes_cbc_256.time, 
							 sizeof(f->priv_data->http.custom.mqtt_info.encrypt.
									union_type.aes_cbc_256.time), "%s", p[1]);
					p[1] = &buf[i+1];
					break;
				case 6:		// mqtt usr/pwd
					break;
			}

			continue;
		}
	}

	// set case 6
	k = 0;
	char reserv_time[64] = {0};
	char *t = f->priv_data->http.custom.mqtt_info.encrypt.union_type.aes_cbc_256.time;
	ret =  strlen(f->priv_data->http.custom.mqtt_info.encrypt.union_type.aes_cbc_256.time);
	while (k < ret)
	{
		reserv_time[k] = t[ret - 1 -k];
		++k;
	}

	// calc iv & key
	MD5_CTX mdContext;
	char tmp_md5[128] = {0};
	uint8_t *iv = f->priv_data->http.custom.mqtt_info.encrypt.union_type.aes_cbc_256.iv;

	snprintf(tmp_md5, sizeof(tmp_md5), "%s%s", t, reserv_time);
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *)tmp_md5, strlen(tmp_md5));
	MD5Final(iv, &mdContext);

	uint8_t *key = f->priv_data->http.custom.mqtt_info.encrypt.union_type.aes_cbc_256.key;
	snprintf(tmp_md5, sizeof(tmp_md5), "%s", t);
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *)tmp_md5, strlen(tmp_md5));
	MD5Final(key, &mdContext);

	snprintf(tmp_md5, sizeof(tmp_md5), "%s", reserv_time);
	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *)tmp_md5, strlen(tmp_md5));
	MD5Final(key+16, &mdContext);

	char auth[256] = {0};
	ret = strlen(p[1]);
	ret = string_to_bytes(p[1], ret, auth);
	ret = ret/AES_BLOCKLEN;
	if (ret <= 0)
	{
		return len;
	}
	memset(tmp_md5, 0, sizeof(tmp_md5));
	AES256_CBC_ctx ctx;
	AES256_CBC_init(&ctx, key, iv);
	AES256_CBC_decrypt(&ctx, ret, (unsigned char *)tmp_md5, (unsigned char *)auth);

	// parse len
	i = 0, j = 0, k = sizeof(tmp_md5);
	for (i = 0; i < k; ++i)
	{
		if (tmp_md5[i] == '#')
		{
			tmp_md5[i] = 0;
			++j;
			++i;

			switch(j)
			{
			case 1:				// len
				ret = atoi(tmp_md5);
				if (ret > 0)
				{
					k = ret + i + 1;
					tmp_md5[k] = 0;
				}
				break;
			case 2:				// usr
				snprintf(f->priv_data->mqtt.username, sizeof(f->priv_data->mqtt.username), 
						 "%s", p[1]);
				break;
			}

			p[1] = &tmp_md5[i+1];
		}
	}
	snprintf(f->priv_data->mqtt.passwd, sizeof(f->priv_data->mqtt.passwd), "%s", p[1]);

	f->priv_data->cfg.mqtt_auto_conn_time = 1; // 重置超时，立即尝试连接mqtt
	
	return len;
}

static int32_t __DevFc41dMqttInfoUrc(void *fc41d, const char *data, int32_t len, int32_t *read)
{
	if (fc41d == NULL || read == NULL) 
	{
		return -COLLECTOR_UPGRADE_ERR_INNER;
	}
	*read = 0;

	int32_t ret = 0;
	int32_t i = 0, j = 0;
	int32_t http_ret_code = 0;
	char *p = (char *)data;
	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	i = _DevFc41dParseHttpCode(fc41d, data, len);
	if (i < -1)
	{
		*read = len;
		return -COLLECTOR_UPGRADE_ERR_DOWLOAD_FAILD;
	}
	j = i;
	i = 0;

	// parse data
	while (ret < len)
	{
		if (j > 0)
		{
			i = _DevFc41dParseData(fc41d, data, len, j);
			j = 0;
		}
		else
		{
			i = _DevFc41dParseData(fc41d, p, len-ret, j);
		}

		if (i == -2) 
		{
			f->priv_data->http.custom.mqtt_info.http_ret_code = 0;
			*read = 0;
			return 1;
		}

		if (i < 0)
		{
			*read = len;
			return -COLLECTOR_UPGRADE_ERR_FILE_PARSE_FAILD;
		}

		ret += i;
		p += i;
	}

	*read = ret;

	return j;
}

static int32_t _DevFc41dMqttInfoUrc(void *fc41d, const char *data, int32_t len, int32_t *read)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t ret = __DevFc41dMqttInfoUrc(fc41d, data, len, read);
	if (ret < 0)
	{
		//_DevFc41dResetUpgradeData(f);
	}
	return ret;
}


