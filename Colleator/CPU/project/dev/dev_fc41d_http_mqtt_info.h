#ifndef _DEV_FC41D_HTTP_MQTT_INFO__H
#define _DEV_FC41D_HTTP_MQTT_INFO__H

#include "config.h"

#include <stdint.h>
#include "md5.h"

#define FC41D_HTTP_URL_LEN					1024
#define FC41D_HTTP_ALIYUN_OSS_DATE_LEN		64
#define FC41D_HTTP_ALIYUN_OSS_AUTH_LEN		128
#define FC41D_HTTP_ALIYUN_OSS_ETAG_LEN		64

#pragma pack(1)
typedef struct _dev_fc41d_http_mqtt_info_encrypt_aes_cbc256_t
{
	char time[64];
	uint8_t iv[16];
	uint8_t key[32];
} DevFc41dHttpMqttInfoEncryptAesCbc256_t;

typedef union _union_DevFc41dHttpMqttInfoEncrypt_t 
{
	DevFc41dHttpMqttInfoEncryptAesCbc256_t aes_cbc_256;

} UnionDevFc41dHttpMqttInfoEncrypt_t;

typedef struct _dev_fc41d_http_mqtt_info_encrypt_t
{
	int32_t type;
	UnionDevFc41dHttpMqttInfoEncrypt_t union_type;
} DevFc41dHttpMqttInfoEncrypt_t;

typedef struct _dev_fc41d_http_mqtt_info_t
{
	int8_t debug_type;			// 1时，不升级只做升级包校验

	char *url;

	uint32_t size;				// 结果大小

	DevFc41dHttpMqttInfoEncrypt_t encrypt;

	int32_t http_ret_code;		// 实际解析HTTP code

	int32_t (*fn_urc)(void*, const char*, int32_t, int32_t*);

} DevFc41dHttpMqttInfo_t;
#pragma pack()

void DevFc41dHttpMqttInfoUninit(void *fc41d);
int32_t DevFc41dHttpMqttInfoInit(void *fc41d);
int32_t DevFc41dReadHttpCfgSetHeaderMqttInfo(void *fc41d, int32_t status);
int32_t DevFc41dWriteHttpCfgSetHeaderMqttInfo(void *fc41d, int32_t status, 
												   char *cmd, int32_t cmd_size);

#endif /*_DEV_FC41D_HTTP_MQTT_INFO__H*/
