#ifndef _COLLECTOR_H
#define _COLLECTOR_H

#include <stdint.h>
#include <string.h>

#include "config.h"
#include "subdev.h"

/**
 * @brief 采集器属性列表
 */
typedef enum {
	COLLECTOR_PROPERTY_NONE = 0,
	COLLECTOR_PROPERTY_DATA_UPLOAD_PERIOD,			// 1. 数据上传周期
	COLLECTOR_PROPERTY_DATA_GET_PERIOD,				// 2. 数据采集周期
} EN_COLLECTOR_PROPERTY;

/**
 * @brief 升级状态
 */
typedef enum 
{
	COLLECTOR_UPGRADE_NONE = 0,
	COLLECTOR_UPGRADE_PAUSE,		// 1. 暂停升级
	COLLECTOR_UPGRADE_INIT,			// 2. 初始化HTTP/BLE请求
	COLLECTOR_UPGRADE_PARSE,		// 3. 解析下载的数据
	COLLECTOR_UPGRADE_SAVE,			// 4. 解析下载的数据
	COLLECTOR_UPGRADE_COMPLETE,		// 5. 下载完成未重启
} EN_COLLECTOR_UPGRADE;

/**
 * @brief 采集器MQTT Topic Type
 */
typedef enum {
	COLLECTOR_MQTT_TOPIC_TYPE_NONE = 0,
	COLLECTOR_MQTT_TOPIC_TYPE_SUB,
	COLLECTOR_MQTT_TOPIC_TYPE_PUB
} EN_COLLECTOR_MQTT_TOPIC_TYPE;

#pragma pack(1)
typedef struct _collector_mqtt_topic_t 
{
	int8_t type;		// pub | sub
	int8_t qos;
	char uri[64];
} CollectorMqttTopic_t;

typedef struct _collector_property_t
{
	uint8_t idx;			// 1 数据上传周期, 2 数据采集周期
	uint8_t len;
	uint8_t data;
} CollectorProperty_t;

typedef struct _collector_mqtt_t
{
	uint8_t type;
} CollectorMqtt_t;

typedef struct _collector_wifi_t
{
	uint8_t ssid[32];
	uint8_t pwd[64];
} CollectorWifi_t;

typedef struct _collector_crc_t
{
	int8_t change;
	uint16_t crc;
	uint32_t timeout_upload;
	int8_t force_upload;
	uint32_t upload_seq;

	uint32_t s_url_len;
	char *s_url;
} CollectorHead_t;

typedef struct _Collector_t
{
	CollectorHead_t head;

	uint16_t type;						// 采集器类型
	uint8_t sn[32];						// 采集器SN
	uint8_t name_flag;
	uint8_t name[64];					// 采集器name
	uint8_t firmware[64];				// 采集器固件版本
	uint16_t pcs_modelid;				// 逆变器类型
	uint8_t pcs_sn[64];				// 逆变器SN

	CollectorWifi_t wifi;
	CollectorMqtt_t mqtt;

	uint8_t property_size;
	CollectorProperty_t property[0];
} Collector_t;

#pragma pack()

extern const CollectorMqttTopic_t gCollectorMqttTopicSub[1];

extern const CollectorMqttTopic_t gCollectorMqttTopicPub[1];

extern Collector_t *gCollector;

int32_t CollectorSetProperty(uint8_t idx, const uint8_t *data,uint8_t len);
uint8_t* CollectorGetProperty(uint8_t idx, uint8_t *len);
int32_t CollectorUpdateSn(const uint8_t *sn, int32_t len);
int32_t CollectorUpdateName(const uint8_t *name, int32_t len);
int32_t CollectorSpiFlashEraseCfg();
int32_t CollectorSpiFlashWriteCfg();
int32_t CollectorUpdateCfgCheck();
int32_t CollectorProcess();
int32_t CollectorUploadRightNow();
int32_t CollectorInit();

#endif /* _COLLECTOR_H */
