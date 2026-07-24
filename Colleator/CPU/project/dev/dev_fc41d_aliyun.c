#include "dev_fc41d_aliyun.h"
#include "dev_fc41d_common.h"
#include "dev_spiflash.h"
#include "nova_dev_upgrade.h"

#define SPI_FLASH_LOG_ADDR						0x3E8000+0x2000

#ifdef FC41D_HTTP_ALIYUN_OSS

#define FC41D_HTTP_ALIYUN_OSS_OUT_OF_RANGE		0

#define AT_FC41D_HTTP_REQ_H_DATE				"AT+QHTTPCFG=\"header\",\"Date\"%s\r\n"
#define AT_FC41D_HTTP_REQ_H_AUTH				"AT+QHTTPCFG=\"header\",\"Authorization\"%s\r\n"
#define AT_FC41D_HTTP_REQ_H_RANGE				"AT+QHTTPCFG=\"header\",\"Range\"%s\r\n"

#define AT_FC41D_HTTP_URC_RECV					"+QHTTPURC: \"recv\""

#define AT_FC41D_HTTP_ALIYUN_ETAG				"\r\nETag:"

extern int32_t string_to_bytes(char *s, int l, char *d);
extern int32_t _DevFc41dUrcHttpHeaderResultCode(char *data, int32_t len, int32_t *result);
extern int32_t _DevFc41dUrcHttpHeaderSkip(char *data, int32_t len);
extern int32_t _DevFc41dSetUpgradeFlashSectorFlag(void *fc41d);
extern void _DevFc41dResetUpgradeData(DevFc41d_t *fc41d);
static int32_t _DevFc41dAliyunUrc(void *fc41d, const char *data, int32_t len, int32_t *read);
extern int32_t CollectorSpiFlashWriteCfg();

static uint8_t _gUpgradeDataMd5MiddleBuf[2048] = {0};

#define PKG_MD5_DEBUG		1

#if PKG_MD5_DEBUG
unsigned char gMd5[16];
MD5_CTX gMdContext;
#endif 

void DevFc41dAliyunUninit(void *fc41d)
{
	if (fc41d == NULL) 
	{
		return;
	}
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
    
	if (f->priv_data->http.custom.ali_oss.date)
	{
		free(f->priv_data->http.custom.ali_oss.date);
		f->priv_data->http.custom.ali_oss.date = NULL;
	}

	if (f->priv_data->http.custom.ali_oss.auth)
	{
		free(f->priv_data->http.custom.ali_oss.auth);
		f->priv_data->http.custom.ali_oss.auth = NULL;
	}
}

int32_t DevFc41dAliyunInit(void *fc41d, uint16_t pkg_type, uint32_t size)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	f->priv_data->upgrade.retry_count = 0;
	uint32_t cur = tim_get_systime();
	f->priv_data->upgrade.time = cur;
	f->priv_data->upgrade.timeout = 0;
	if (size > 0) 
	{
		cur = 4000 * size/6144 + 120000; // 4s * pkg_size + 2s = 超时时间
		f->priv_data->upgrade.timeout = cur;
	}

	f->priv_data->http.custom.mqtt_info.http_ret_code = 0;

	memset((uint8_t *)&f->priv_data->http.custom.ali_oss, 0, 
		   sizeof(f->priv_data->http.custom.ali_oss));

	f->priv_data->http.custom.ali_oss.reboot_flag = 0;
	f->priv_data->http.custom.ali_oss.reboot_time = 0;

	f->priv_data->http.custom.ali_oss.pkg_type = pkg_type;
	f->priv_data->http.custom.ali_oss.fn_urc = _DevFc41dAliyunUrc;
	f->priv_data->http.custom.ali_oss.tmp_md5_size = 0;

	f->priv_data->http.custom.ali_oss.size = size;
	f->priv_data->http.custom.ali_oss.pkg_size = NOVA_UPGRADE_COLLECTOR_PKG_MAX_SIZE;
	f->priv_data->http.custom.ali_oss.recv_size = 0;
	f->priv_data->http.custom.ali_oss.pkg_num = 0;

	if (!f->priv_data->http.custom.ali_oss.date)
	{
		f->priv_data->http.custom.ali_oss.date = (char *)malloc(FC41D_HTTP_ALIYUN_OSS_DATE_LEN);
		if (!f->priv_data->http.custom.ali_oss.date)
		{
			return -2;
		}
	}

	if (!f->priv_data->http.custom.ali_oss.auth)
	{
		f->priv_data->http.custom.ali_oss.auth = (char *)malloc(FC41D_HTTP_ALIYUN_OSS_AUTH_LEN);
		if (!f->priv_data->http.custom.ali_oss.auth)
		{
			return -3;
		}
	}

	f->priv_data->http.custom.ali_oss.get_etag = 0;
	memset(f->priv_data->http.custom.ali_oss.etag, 0, sizeof(f->priv_data->http.custom.ali_oss.etag));

	MD5Init(&f->priv_data->http.custom.ali_oss.mdContext);
    
    return 0;
}

int32_t DevFc41dReadHttpCfgSetHeaderAliyun(void *fc41d, int32_t status)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	switch(status)
	{
	case FC41D_PROCESS_HTTP_SET_H_USER_AGENT:
		f->task_push_inhead(f, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_SET_H_DATE,
							DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_HTTP_SET_H_DATE:
		f->task_push_inhead(f, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_SET_H_AUTH,
							DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_HTTP_SET_H_AUTH:
		f->task_push_inhead(f, FC41D_PROCESS_STATUS_HTTP, 
							FC41D_PROCESS_HTTP_SET_H_RANGE,
							DEV_FC41D_TASK_STAT_WRITE, 0);
		break;
	case FC41D_PROCESS_HTTP_SET_H_RANGE:
		/*complete headers */
		f->priv_data->upgrade.flag = COLLECTOR_UPGRADE_PARSE;
		f->priv_data->http.custom.ali_oss.http_ret_code = 0;
		break;
	default:break;
	}
    
	return 0;
}

int32_t DevFc41dWriteHttpCfgSetHeaderAliyun(void *fc41d, int32_t status, 
												   char *cmd, int32_t cmd_size)
{
	if (fc41d == NULL) 
	{
		return -1;
	}

	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	switch(status)
	{
	case FC41D_PROCESS_HTTP_SET_H_DATE:
		{
			char tmp[128] = {0};
			snprintf(tmp, sizeof(tmp), ",\"%s\"",
					 f->priv_data->http.custom.ali_oss.date==NULL?
					 "":f->priv_data->http.custom.ali_oss.date);
			snprintf(cmd, cmd_size, AT_FC41D_HTTP_REQ_H_DATE, tmp);
		}
		break;
	case FC41D_PROCESS_HTTP_SET_H_AUTH:
		{
			char tmp[128] = {0};
			snprintf(tmp, sizeof(tmp), ",\"%s\"",
					 f->priv_data->http.custom.ali_oss.auth==NULL?
					 "":f->priv_data->http.custom.ali_oss.auth);
			snprintf(cmd, cmd_size, AT_FC41D_HTTP_REQ_H_AUTH, tmp);
		}
		break;
	case FC41D_PROCESS_HTTP_SET_H_RANGE:
		{
			char range[32] = {0};
			uint32_t pkg_size = 0;

			// 计算剩余待下载size
			if (!f->priv_data->http.custom.ali_oss.cur_pkg_err)
			{
				f->priv_data->http.custom.ali_oss.cur_recv_size 
					= f->priv_data->http.custom.ali_oss.recv_size;
			}
			else
			{
				f->priv_data->http.custom.ali_oss.recv_size 
					= f->priv_data->http.custom.ali_oss.cur_recv_size;
			}
			f->priv_data->http.custom.ali_oss.cur_pkg_err = 0;

			pkg_size = f->priv_data->http.custom.ali_oss.size
				- f->priv_data->http.custom.ali_oss.recv_size;

			if (f->priv_data->http.custom.ali_oss.pkg_size <= 0)
			{
				f->priv_data->http.custom.ali_oss.pkg_size 
					= NOVA_UPGRADE_COLLECTOR_PKG_MAX_SIZE;
			}

			// 根据当前rb_read剩余容量重新计算下一包数据size
			uint32_t unused_len = rbuffer_length(&f->priv_data->rb_read.ctx);
			unused_len = rbuffer_unused_length(&f->priv_data->rb_read.ctx);
			unused_len -= 2048;
			f->priv_data->http.custom.ali_oss.pkg_size = unused_len;

			if (pkg_size > f->priv_data->http.custom.ali_oss.pkg_size)
			{
				pkg_size = f->priv_data->http.custom.ali_oss.pkg_size;
			}
			else 
			{
				f->priv_data->http.custom.ali_oss.pkg_size = pkg_size;
			}

			f->priv_data->http.custom.ali_oss.cur_pkg_start 
				= f->priv_data->http.custom.ali_oss.recv_size;

#if FC41D_HTTP_ALIYUN_OSS_OUT_OF_RANGE
			f->priv_data->http.custom.ali_oss.cur_pkg_end 
				= f->priv_data->http.custom.ali_oss.recv_size+pkg_size-1+0xA0;
#else
			f->priv_data->http.custom.ali_oss.cur_pkg_end 
				= f->priv_data->http.custom.ali_oss.recv_size+pkg_size-1;
#endif 

			if (f->priv_data->http.custom.ali_oss.cur_pkg_end 
				> f->priv_data->http.custom.ali_oss.size)
			{
				f->priv_data->http.custom.ali_oss.cur_pkg_end 
					= f->priv_data->http.custom.ali_oss.recv_size+pkg_size-1;
			}

			snprintf(range, sizeof(range), ",\"bytes=%d-%d\"", 
					 f->priv_data->http.custom.ali_oss.cur_pkg_start, 
					 f->priv_data->http.custom.ali_oss.cur_pkg_end);
			snprintf(cmd, cmd_size, AT_FC41D_HTTP_REQ_H_RANGE, range);
			f->priv_data->http.custom.ali_oss.http_ret_code = 0;
			f->priv_data->http.custom.ali_oss.parse_len = 0;
			f->priv_data->http.custom.ali_oss.not_enough = 0;
		}
		break;
	default:break;
	}
    
	return 0;
}

static int32_t _DevFc41dAliyunUrcHttpHeaderEcCode(char *data, int32_t len, char *ec)
{
	int32_t ret = 0;

	return 0;
}

static int32_t _DevFc41dAliyunGetEtag(void *fc41d, char *data, int32_t len)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	char *find = NULL;
	char etag[33] = {0};

	if (f->priv_data->http.custom.ali_oss.get_etag)
	{
		return 0;
	}

	find = strstr(data, AT_FC41D_HTTP_ALIYUN_ETAG);
	if (find == NULL)
	{
		return -1;
	}

	find += (strlen(AT_FC41D_HTTP_ALIYUN_ETAG)+2);
	memcpy(etag, find, sizeof(etag)-1);

	int32_t ret = strlen(etag);
	ret = string_to_bytes(etag, ret, (char *)f->priv_data->http.custom.ali_oss.etag);
	f->priv_data->http.custom.ali_oss.get_etag = 1;
	return 0;
}

static int32_t _DevFc41dAliyunParseHttpCode(void *fc41d, const char *data, int32_t len)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t ret = 0, i = 0;
	char *p = (char *)data;

	// get etag
	_DevFc41dAliyunGetEtag(fc41d, p, len-ret);

	if (f->priv_data->http.custom.ali_oss.http_ret_code == 206)
	{
		return 0;
	}

	int32_t http_ret_code = 0;

	uint32_t real_pkg_size = 512;
	char real_pkg_size_buf[16] = {0};
	const char *find[2] = { strstr(p, AT_FC41D_HTTP_URC_RECV), NULL };
	if (find[0])
	{
		find[0] += strlen(AT_FC41D_HTTP_URC_RECV) + 1;
		find[1] = strstr(find[0], "\r\n");
		if (find[1] - find[0] > 0  && find[1] - find[0] < 16)
		{
			memcpy(real_pkg_size_buf, find[0], find[1]-find[0]);
		}
		real_pkg_size = atoi(real_pkg_size_buf);
	}

	if (len < real_pkg_size)
	{
		// not enough and continue recv http
		return -1;
	}

	// pasrse http ret code = 206
	i = _DevFc41dUrcHttpHeaderResultCode(p, len-ret, &http_ret_code);
	if (i < 0)
	{
		return -2;
	}

	f->priv_data->http.custom.ali_oss.http_ret_code = http_ret_code;
	ret += i;
	p += i;
	if (f->priv_data->http.custom.ali_oss.http_ret_code == 206)
	{

#if PKG_MD5_DEBUG
		MD5Init(&gMdContext);
#endif 

		f->priv_data->http.first_pkg = 1;
		f->priv_data->http.custom.ali_oss.parse_len = 0;
		f->priv_data->http.custom.ali_oss.not_enough 
			= f->priv_data->http.custom.ali_oss.pkg_size;
		f->priv_data->http.custom.ali_oss.recv_pkg_size = 0;

		// skip headers
		i = _DevFc41dUrcHttpHeaderSkip(p, len-ret);
		if (i > 0)
		{
			ret += i;
			p += i;
		}

		return ret;
	}

	// parse aliyun OSS error code(string)
	char ec[16] = {0};
	i = _DevFc41dAliyunUrcHttpHeaderEcCode(p, len-ret, ec);

	return -3;
}

int32_t _DevFc41dAliyunGetFlashDataMd5(void *fc41d, uint32_t spi_flash_addr, 
									   uint8_t md5[16], uint32_t len)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	int32_t count = 0;
#if 0
	uint8_t *buf= _gUpgradeDataMd5MiddleBuf;
	int32_t buf_len = sizeof(_gUpgradeDataMd5MiddleBuf);
#else
	uint8_t buf[512] = {0};
	int32_t buf_len = sizeof(buf);
#endif 
	int32_t total_len = 0, i = 0;

	MD5_CTX mdContext;

	MD5Init(&mdContext);
	total_len = f->priv_data->http.custom.ali_oss.size;
	if (len > 0)
	{
		total_len = len;
	}
	memset(buf, 0, buf_len);

	while (total_len > 0)
	{
		if (buf_len > total_len) 
		{
			buf_len = total_len;
		}
		SpiFlashReadData(spi_flash_addr + i, buf, buf_len);

		// calc md5
		MD5Update(&mdContext, buf, buf_len);

		total_len -= buf_len;
		i += buf_len;
	}

	MD5Final(md5, &mdContext);

	return 0;
}

static int32_t _DevFc41dAliyunSaveTmpMd5(void *fc41d)
{
	return 0;
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	uint32_t start_addr = SPI_FLASH_LOG_ADDR;
	SpiFlashEraseSector(start_addr);

    SpiFlashWriteBytesWithoutErasenoAligned(start_addr, 
		(uint8_t *)&f->priv_data->http.custom.ali_oss.tmp_md5_size, 
			sizeof(f->priv_data->http.custom.ali_oss.tmp_md5_size)
			+sizeof(f->priv_data->http.custom.ali_oss.tmp_md5));

	return 0;
}

static int32_t _DevFc41dAliyunSaveData(void *fc41d, const char *data, int32_t len)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	f->priv_data->upgrade.retry_count = 0;
	int32_t ret = rbuffer_write_bytes(&f->priv_data->upgrade.down.ctx, 
									  (const uint8_t *)data, len);
	return ret;
}

static int32_t _DevFc41dAliyunParseData(void *fc41d, const char *data, int32_t len, 
										int32_t first)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t ret = 0, i = 0, j = 0, k = 0, m = 0;
	char *p = (char *)data;
	char ret_str[8] = {0};

	if (f->priv_data->http.custom.ali_oss.not_enough == 0)
	{
		if (f->priv_data->http.custom.ali_oss.recv_pkg_size <= 0)
		{
			f->priv_data->http.first_pkg = 1;
		}
		return len;
	}

	if (f->priv_data->http.first_pkg == 1)
	{
		if (!strncmp(&data[4], AT_FC41D_HTTP_URC_RECV, strlen(AT_FC41D_HTTP_URC_RECV))) 
		{
			ret += strlen(AT_FC41D_HTTP_URC_RECV) + 4;
			p += ret;
		}
		for (i = ret; i < len && k < sizeof(ret_str); ++i, ++p)
		{
			if (*p == '\r' && *(p+1) == '\n' && j == 1)
			{
				ret = atoi(ret_str);
				j = 2;
				i += 2;
                p += 2;
				break;
			}
			if (*p == ',')
			{
				j = 1;
				continue;
			}

			if (j == 1)
			{
				ret_str[k++] = data[i];
				continue;
			}

		}

		if (j != 2)
		{
			return -1;
		}
		f->priv_data->http.first_pkg = 0;

		if (first)
		{
			m = _DevFc41dUrcHttpHeaderSkip(p, len-i);
			if (m <= 0)
			{
				return -2;
			}
			ret -= m;
		}

		j = len - i - m;
		f->priv_data->http.custom.ali_oss.recv_pkg_size = ret;
	}
	else
	{
		f->priv_data->http.first_pkg = 1;

		ret = f->priv_data->http.custom.ali_oss.recv_pkg_size;
		j = len - i - m;
	}

	if (j < ret)
	{
        f->priv_data->http._continue -= j;
		if (f->priv_data->http.custom.ali_oss.not_enough < j)
		{
			// out of range split 
			j = f->priv_data->http.custom.ali_oss.not_enough;
		}
		// not enough wait next pkg
		f->priv_data->http.custom.ali_oss.recv_size += j;
		f->priv_data->http.custom.ali_oss.recv_pkg_size -= j;
		f->priv_data->http.custom.ali_oss.not_enough -= j;
		f->priv_data->http.custom.ali_oss.parse_len += j;

		// save data
		_DevFc41dAliyunSaveData(fc41d, &p[m], j);

		return len;
	}

    // parse one pkg
    f->priv_data->http._continue -= ret;
	if (f->priv_data->http.custom.ali_oss.not_enough < ret)
	{
		// out of range split 
		ret = f->priv_data->http.custom.ali_oss.not_enough;
	}
	f->priv_data->http.custom.ali_oss.recv_size += ret;
	f->priv_data->http.custom.ali_oss.recv_pkg_size -= ret;
	f->priv_data->http.custom.ali_oss.not_enough -= ret;
	f->priv_data->http.custom.ali_oss.parse_len += ret;
	

	// save data
	_DevFc41dAliyunSaveData(fc41d, &p[m], ret);
    
    if (f->priv_data->http.custom.ali_oss.recv_pkg_size <= 0)
    {
        f->priv_data->http.first_pkg = 1;
    }
	
	return ret+i+m;
}

static int32_t _DevFc41dAliyunWriteDataInFlash(void *fc41d, uint8_t *data, int32_t len)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t spi_flash_addr = 0;
	// write to flash
	spi_flash_addr = NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR 
		+ f->priv_data->http.custom.ali_oss.recv_size - len;
	SpiFlashWriteBytesWithoutErasenoAligned(spi_flash_addr, (uint8_t *)data, len);

	return 0;
}

static int32_t _DevFc41dAliyunCalcMd5(void *fc41d)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	uint8_t *buf = _gUpgradeDataMd5MiddleBuf;
	int32_t total_len = rbuffer_length(&f->priv_data->upgrade.down.ctx);
	int32_t buf_len = sizeof(_gUpgradeDataMd5MiddleBuf);

	if (f->priv_data->http.custom.ali_oss.pkg_num == 9)
	{

		int32_t xx = 0;
		xx = 1;
	}

	memset(buf, 0, buf_len);

	while (1)
	{
		buf_len = rbuffer_get_bytes(&f->priv_data->upgrade.down.ctx, 
									(uint8_t *)buf, buf_len);
		if (buf_len <= 0)
		{
			break;
		}

#if PKG_MD5_DEBUG
		MD5Update(&gMdContext, buf, buf_len);
#endif 
		MD5Update(&f->priv_data->http.custom.ali_oss.mdContext, buf, buf_len);

		switch (f->priv_data->http.custom.ali_oss.pkg_type)
		{

		case EN_FILE_PKG_TYPE_NOVA_COLLECTOR:
			_DevFc41dAliyunWriteDataInFlash(fc41d, buf, buf_len);
			break;
		case EN_FILE_PKG_TYPE_NOVA_KSTAR_5KW:
		case EN_FILE_PKG_TYPE_NOVA_KSTAR_10KW:
		case EN_FILE_PKG_TYPE_NOVA_PCS_5KW:
        case EN_FILE_PKG_TYPE_NOVA_PCS_16KW:
			NovaDevUpgradeSpiFlashWrite(buf, buf_len, 0);
			break;
		}

		buf_len = rbuffer_trim_bytes(&f->priv_data->upgrade.down.ctx, buf_len);
	}

	return 0;
}

static int32_t _DevFc41dAliyunMd5(void *fc41d)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	/*
	uint32_t cur = tim_get_systime();

	cur = cur - f->priv_data->upgrade.time;
	f->priv_data->upgrade.time = cur;
	*/

	MD5Final(f->priv_data->http.custom.ali_oss._md5,
			 &f->priv_data->http.custom.ali_oss.mdContext);
	char buf_temp[3] = {0};
	for(int32_t i = 0; i < 16; i++)
	{
		sprintf(buf_temp,"%02x", f->priv_data->http.custom.ali_oss._md5[i]);
		strncat(f->priv_data->http.custom.ali_oss.md5, buf_temp, 2);
	}

	return 0;
}

static int32_t _DevFc41dAliyunCheckMd5(void *fc41d)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	// check flash md5
	uint8_t md5[16] = {0};

	switch (f->priv_data->http.custom.ali_oss.pkg_type)
	{
		case EN_FILE_PKG_TYPE_NOVA_COLLECTOR:
		{
			_DevFc41dAliyunGetFlashDataMd5(fc41d, NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR, md5, 0);
		}
		break;
		case EN_FILE_PKG_TYPE_NOVA_KSTAR_5KW:
		case EN_FILE_PKG_TYPE_NOVA_KSTAR_10KW:
		case EN_FILE_PKG_TYPE_NOVA_PCS_5KW:
        case EN_FILE_PKG_TYPE_NOVA_PCS_16KW:  
		{
			uint32_t spi_flash_addr = NovaDevUpgradeSpiFlashAddr();
			_DevFc41dAliyunGetFlashDataMd5(fc41d, spi_flash_addr, md5, 0);
			//_DevFc41dAliyunGetFlashDataMd5(fc41d, NovaDevUpgradeSpiFlashAddr(), md5, 0);
		}
		break;
	}

	if (memcmp(md5, f->priv_data->http.custom.ali_oss._md5, sizeof(md5)))
	{
		return -COLLECTOR_UPGRADE_ERR_FILE_MD5_DOWNLOAD;
	}

	if (memcmp(f->priv_data->http.custom.ali_oss.etag, md5, sizeof(md5)))
	{
		return -COLLECTOR_UPGRADE_ERR_FILE_MD5_CMP;
	}

	return 0;
}

static int32_t _DevFc41dAliyunCheckUpgradeStruct(void *fc41d)
{
	uint8_t *buf = _gUpgradeDataMd5MiddleBuf;
	int32_t buf_len = 0, total_len = 0, i = 0, j = 0, check_sum = 0;
	int32_t spi_flash_addr = NOVA_UPGRADE_COLLECTOR_FLASH_BASE_ADDR;
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	char firmware[2][64] = {0};

	total_len = f->priv_data->http.custom.ali_oss.size - sizeof(check_sum);
	buf_len = sizeof(_gUpgradeDataMd5MiddleBuf);
	memset(buf, 0, buf_len);

	while (total_len > 0)
	{
		if (buf_len > total_len) 
		{
			buf_len = total_len;
		}
		SpiFlashReadData(spi_flash_addr + i, buf, buf_len);

		for (j = 0; j < buf_len; ++j)
		{
			check_sum += buf[j];
		}

		if (i == 0)
		{
			// check header 
			if (!UPGRADE_DATA_STRUCT_CHECK(buf, f->priv_data->http.custom.ali_oss.size))
			{
				return -1;
			}
			memcpy(firmware[0], buf+sizeof(int32_t)*2+sizeof(uint16_t), 
				   sizeof(uint8_t)*3);
			snprintf(firmware[1], sizeof(firmware[1]), "%d.%d.%d", firmware[0][0], 
					 firmware[0][1], firmware[0][2]);
		}

		total_len -= buf_len;
		i += buf_len;
	}

	// check len
	if (total_len != 0)
	{
		return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE;
	}
	if (i+sizeof(check_sum) != f->priv_data->http.custom.ali_oss.size)
	{
		return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_SIZE2;
	}
	// read last check_sum
	uint32_t check_sum2 = 0;
	SpiFlashReadData(spi_flash_addr + i, (uint8_t *)&check_sum2, sizeof(check_sum2));
	if (check_sum != check_sum2)
	{
		return -COLLECTOR_LOCAL_UPGRADE_ERR_FILE_CHECK_SUM;
	}

	memcpy(gCollector->firmware, firmware[1], sizeof(firmware[1]));
	return 0;
}

static int32_t _DevFc41dAliyunCheckRecvFinish(void *fc41d)
{
	int32_t ret = 0;
	DevFc41d_t *f = (DevFc41d_t *)fc41d;

	static DevFc41d_t *f1 = NULL;
	if (f1 == NULL)
	{
		f1 = (DevFc41d_t *)fc41d;
	}

    if (f->priv_data->http.custom.ali_oss.recv_size >= f->priv_data->http.custom.ali_oss.size)
    {
		// recv complete
		f->priv_data->upgrade.flag = COLLECTOR_UPGRADE_NONE;

		// calc md5
		_DevFc41dAliyunMd5(fc41d);

		if (f->priv_data->http.custom.ali_oss.debug_type == 1)
		{
			return -COLLECTOR_UPGRADE_ERR_DEBUG;
		}

		// save tmp md5
		_DevFc41dAliyunSaveTmpMd5(fc41d);

		// check flash data's md5
		ret = _DevFc41dAliyunCheckMd5(fc41d);
		if (ret != 0)
		{
			return ret;
		}

		switch (f->priv_data->http.custom.ali_oss.pkg_type)
		{
		case EN_FILE_PKG_TYPE_NOVA_COLLECTOR:
			{
				// check upgrade data struct
				ret = _DevFc41dAliyunCheckUpgradeStruct(fc41d);
				if (ret != 0)
				{
					return ret;
				}

#if COLLECTOR_UPGRADE_TEST
				return 0;
#endif 
				// set upgrade flash flag
				_DevFc41dSetUpgradeFlashSectorFlag(fc41d);
				// update firmware ver
				CollectorSpiFlashWriteCfg();

				f->priv_data->http.custom.ali_oss.reboot_flag = 1;
				f->priv_data->http.custom.ali_oss.reboot_time = tim_get_systime();
			}
			break;
		case EN_FILE_PKG_TYPE_NOVA_KSTAR_5KW:
		case EN_FILE_PKG_TYPE_NOVA_KSTAR_10KW:
		case EN_FILE_PKG_TYPE_NOVA_PCS_5KW:
        case EN_FILE_PKG_TYPE_NOVA_PCS_16KW:
			{
				// save upgrade ctx
				NovaDevUpgradeSpiFlashSaveCtx(f->priv_data->http.custom.ali_oss.size, 
											  f->priv_data->http.custom.ali_oss._md5);
				NovaDevUpgradeStatSet(NOVA_DEV_UPGRADE_STAT_PREPARE);
			}
			break;
		}
        

		return 0;
	}

	if (f->priv_data->http._continue == 0)
	{
#if PKG_MD5_DEBUG
		MD5Final(gMd5, &gMdContext);
		/*
		memcpy(f->priv_data->http.custom.ali_oss.tmp_md5[
			   f->priv_data->http.custom.ali_oss.tmp_md5_size]._md5, gMd5, sizeof(gMd5));
			   */
		++f->priv_data->http.custom.ali_oss.tmp_md5_size;
#endif 
		// reset 
		f->priv_data->http.custom.ali_oss.http_ret_code = 0;
		f->priv_data->http.custom.ali_oss.parse_len = 0;
		f->priv_data->http.custom.ali_oss.not_enough = 0;
		f->priv_data->http.custom.ali_oss.recv_pkg_size = 0;

		++f->priv_data->http.custom.ali_oss.pkg_num;

		if (f->priv_data->http.custom.ali_oss.pkg_num > 17)
		{
			int32_t xx = 0;
		}
		return 2;
	}

	return 1;
}

static int32_t __DevFc41dAliyunUrc(void *fc41d, const char *data, int32_t len, int32_t *read)
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

	if (f->priv_data->http.custom.ali_oss.pkg_num > 32)
	{

		int32_t xxx = 0;
		xxx = 1;
	}

	i = _DevFc41dAliyunParseHttpCode(fc41d, data, len);
	if (i < 0)
	{
		if (i == -1)
		{
			return 1;
		}
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
			i = _DevFc41dAliyunParseData(fc41d, data, len, j);
			j = 0;
		}
		else
		{
			i = _DevFc41dAliyunParseData(fc41d, p, len-ret, j);
		}

		if (i == -2) 
		{
			f->priv_data->http.custom.ali_oss.http_ret_code = 0;
			*read = 0;
			return 1;
		}

		if (i < 0)
		{
			*read = len;
			return -COLLECTOR_UPGRADE_ERR_FILE_PARSE_FAILD;
		}

        // calc md5
		_DevFc41dAliyunCalcMd5(fc41d);

		ret += i;
		p += i;
	}

	*read = ret;

	j = _DevFc41dAliyunCheckRecvFinish(fc41d);

	return j;
}

static int32_t _DevFc41dAliyunUrc(void *fc41d, const char *data, int32_t len, int32_t *read)
{
	DevFc41d_t *f = (DevFc41d_t *)fc41d;
	int32_t ret = __DevFc41dAliyunUrc(fc41d, data, len, read);
	if (ret < 0 && ret > -COLLECTOR_UPGRADE_ERR_FILE_MD5_DOWNLOAD)
	{
		++f->priv_data->upgrade.retry_count;
		if (f->priv_data->upgrade.retry_count > NOVA_UPGRADE_COLLECTOR_RETRY_MAX)
		{
			_DevFc41dResetUpgradeData(f);
			return ret;
		}
		return 2; // continue
	}
	return ret;
}

#endif

