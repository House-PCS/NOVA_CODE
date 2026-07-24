#ifndef _DEV_FC41D_ALIYUN_H
#define _DEV_FC41D_ALIYUN_H

#include "config.h"

#ifdef	FC41D_HTTP_ALIYUN_OSS

#include <stdint.h>
#include "md5.h"

#define FC41D_HTTP_URL_LEN					1024
#define FC41D_HTTP_ALIYUN_OSS_DATE_LEN		64
#define FC41D_HTTP_ALIYUN_OSS_AUTH_LEN		128
#define FC41D_HTTP_ALIYUN_OSS_ETAG_LEN		64
#define FC41D_HTTP_ALIYUN_OSS_TMP_MD5_LEN	32

#pragma pack(1)

typedef struct _dev_fc41d_md5_t
{
	//MD5_CTX mdContext;
	unsigned char _md5[16];
	//char md5[33];
} DevFc41dMd5_t;
	
typedef struct _dev_fc41d_http_aliyun_oss_t
{
	int8_t debug_type;			// 1时，不升级只做升级包校验
	int8_t get_etag;
	int8_t reboot_flag;			// 1时，强制重启
	uint32_t reboot_time;		// 1时，强制重启

	uint16_t pkg_type;			// 升级包类型
	uint32_t size;				// 文件总大小
	uint32_t pkg_size;			// 一包数据大小
	uint32_t recv_size;			// 已经接收的size
	uint32_t pkg_num;

	int8_t cur_pkg_err;
	uint32_t cur_recv_size;
	uint32_t cur_pkg_start;
	uint32_t cur_pkg_end;

	char *date;					// ali_Oss 请求时间(做校验用)
	char *auth;					// ali_oss 认证
	uint8_t etag[16];			// ali_oss file md5

	MD5_CTX mdContext;
	unsigned char _md5[16];
	char md5[33];

	uint8_t tmp_md5_size;
	DevFc41dMd5_t tmp_md5[FC41D_HTTP_ALIYUN_OSS_TMP_MD5_LEN];

	int32_t http_ret_code;		// 实际解析HTTP code
	int32_t parse_len;			// 实际解析到的数据长度
	int32_t not_enough;			// 拆包剩余未解析长度
	int32_t recv_pkg_size;		// 1包数据长度

	int32_t (*fn_urc)(void*, const char*, int32_t, int32_t*);

} DevFc41dHttpAliyunOss_t;
#pragma pack()

void DevFc41dAliyunUninit(void *fc41d);
int32_t DevFc41dAliyunInit(void *fc41d, uint16_t pkg_type, uint32_t size);
int32_t DevFc41dReadHttpCfgSetHeaderAliyun(void *fc41d, int32_t status);
int32_t DevFc41dWriteHttpCfgSetHeaderAliyun(void *fc41d, int32_t status, 
												   char *cmd, int32_t cmd_size);

#endif /*FC41D_HTTP_ALIYUN_OSS*/
#endif /*_DEV_FC41D_ALIYUN_H*/
