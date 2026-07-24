#ifndef CONFIG_H
#define CONFIG_H


// 独立测试MQTT连接稳定性版本
#define ONLY_MQTT_TEST			0

#if ONLY_MQTT_TEST

#define NOVA_PCS_SN_DEBUG		1
#define LOCAL_PCS_UPGRADE		0
#define COLLECTOR_IWDG			0
#define COLLECTOR_PCS_TYPE_DEBUG
#define COLLECTOR_DBG_SN		'B'
#define COLLECTOR_UPLOAD_DATA	1

#else
// DEBUG_专用
#if 0
#define LOCAL_PCS_UPGRADE		1
//#define COLLECTOR_PCS_TYPE_DEBUG
#define _KSTAR_DEBUG_ERROR
#define COLLECTOR_DBG_MQTT		0 // 开启后连接指定server
#define COLLECTOR_DEBUG			1
#define USE_LED_DEBUG			0
#define COLLECTOR_UPGRADE_TEST	1
#define COLLECTOR_IWDG			0
#define COLLECTOR_DBG_SN		'B'
#endif 

#endif  /* ONLY_MQTT_TEST */

#ifndef NOVA_DEBUG_NO_RESET_FC41D		
#define NOVA_DEBUG_NO_RESET_FC41D		0
#endif 

#ifndef NOVA_PCS_SN_DEBUG		
#define NOVA_PCS_SN_DEBUG		0
#endif 

#ifndef LOCAL_PCS_UPGRADE		
#define LOCAL_PCS_UPGRADE		0
#endif 

#ifndef COLLECTOR_UPLOAD_DATA
#define COLLECTOR_UPLOAD_DATA	1
#endif 

#ifndef COLLECTOR_IWDG
#define COLLECTOR_IWDG			1
#endif 

#ifndef AES_BLOCKLEN
#define AES_BLOCKLEN			16
#endif 

#ifndef COLLECTOR_DEBUG
#define COLLECTOR_DEBUG			0
#endif 

#ifndef USE_LED_DEBUG
#define USE_LED_DEBUG			0
#endif 

#ifndef USE_GD_LED
#define USE_GD_LED				0
#endif 

#ifndef FC41D_HTTP_ALIYUN_OSS
#define FC41D_HTTP_ALIYUN_OSS	1
#endif 

#endif /* CONFIG_H */
