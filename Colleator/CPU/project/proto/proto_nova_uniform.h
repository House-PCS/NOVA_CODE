#ifndef _PROTO_NOVA_UNIFORM_H
#define _PROTO_NOVA_UNIFORM_H

#include <stdint.h>
#include "kstar.h"

#define __LITTLE_ENDIAN 1

# if __BIG_ENDIAN
# define ntohl(x)    (x)
# define ntohs(x)    (x)
# define htonl(x)    (x)
# define htons(x)    (x)
# else
#  if __LITTLE_ENDIAN
#   define ntohl(x)    __builtin_bswap32 (x)
#   define ntohs(x)    __builtin_bswap16 (x)
#   define htonl(x)    __builtin_bswap32 (x)
#   define htons(x)    __builtin_bswap16 (x)
#  endif
# endif

#define PROTO_UNIFORM_RES				0
#define PROTO_UNIFORM_REQ				1

#define PROTO_UNIFORM_R_DEBUG_AES						0x12345676
#define PROTO_UNIFORM_R_DEBUG							0x12345677
#define PROTO_UNIFORM_R_DEBUG_GET_WIFI_INFO				0x12345678
#define PROTO_UNIFORM_R_DEBUG_GET_MQTT_INFO				0x12345679
#define PROTO_UNIFORM_R_HTTP							0x1234567A
#define PROTO_UNIFORM_R_DEBUG_GET_PROTO					0x1234567B
#define PROTO_UNIFORM_R_TEST_MQTT_SUB					0x12345680
#define PROTO_UNIFORM_R_TEST_UPGRADE_MD5				0x12345681
#define PROTO_UNIFORM_R_TEST_UPGRADE_FILE				0x12345682
#define PROTO_UNIFORM_R_DEBUG_GET_PCS_MODBUS			0x12345683
#define PROTO_UNIFORM_W_DEBUG_MODULE					0x12345684
#define PROTO_UNIFORM_W_DEBUG_WRITE_MONITOR_SN			0x12345685
#define PROTO_UNIFORM_W_DEBUG_MONITOR_AUTH				0x12345686  // 认证
#define PROTO_UNIFORM_W_DEBUG_GET_MONITOR_HARDFAULT		0x12345687  // 获取监控卡最后一次hardfault
#define PROTO_UNIFORM_W_DEBUG_CLS_MONITOR_TOTAL_FLASH	0x12345688  // 清空监控卡统计数据Flash
#define PROTO_UNIFORM_W_RESET_NOW						0x01234589  // 立即重启设备
#define PROTO_UNIFORM_W_DEBUG_OTA_FC41D					0x1234568A  // OTA FC41D模组

// 周边WIFI热点信息
#define PROTO_UNIFORM_R_WIFI_SCAN		0x00FF0000

// 要连接的WIFI热点信息
#define PROTO_UNIFORM_R_WIFI_INFO		0x00FF0001
#define PROTO_UNIFORM_W_WIFI_INFO		0x80FF0001

// 采集器设备信息
#define PROTO_UNIFORM_R_DEV_INFO		0x00FF0002
#define PROTO_UNIFORM_W_DEV_INFO		0x80FF0002

// 设置采集器MQTT连接的Server信息
#define PROTO_UNIFORM_R_DEV_MQTT_INFO		0x00FFA003
#define PROTO_UNIFORM_W_DEV_MQTT_INFO		0x80FFA003

// 设置采集器设备名称
#define PROTO_UNIFORM_R_DEV_NAME			0x00FF0004
#define PROTO_UNIFORM_W_DEV_NAME			0x80FF0004

// 采集器属性信息
#define PROTO_UNIFORM_R_DEV_PROPERTY		0x00FFA006
#define PROTO_UNIFORM_W_DEV_PROPERTY		0x80FFA006

#define PROTO_UNIFORM_R_MQTT_BY_HTTP		0x12341234

// 采集器升级
#define PROTO_UNIFORM_W_DEV_UPGRADE			0x80FFA00B
// 采集器升级状态上报
#define PROTO_UNIFORM_R_DEV_UPGRADE_STAT	0x00FFA00F

#define PROTO_UNIFORM_W_DEV_UPGRADE_PCS		0x80FFA00D

// 逆变器产品信息(不局限于科士达)
#define PROTO_UNIFORM_R_DEV_PRODUCT			0x00FFA00C

#define PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_START		0x80FFA0F0
#define PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_DOWN_FILE	0x80FFA0F1
#define PROTO_UNIFORM_W_DEV_LOCAL_UPGRADE_END		0x80FFA0FF

#pragma pack(1)

/**
 * @brief 最后一次 hardfault 信息(modbus)
 * 0x8200 —— 清除信息
 * 0x8201 —— 获取数据结构信息
 */
typedef union _hard_fault_info_stru_t
{
    struct
    {
        char hard_fault_info[120];
        uint32_t last_time;
        uint32_t count;
    } Para_t; 
    char info[128];    
} HardFaultInfo_t;

typedef struct _uniform_protocol_normalbody_head_stru_t 
{
	uint8_t data[8];
} UniformProtocolNormalBodyHead_t;

typedef struct _uniform_protocol_tlvbody_head_stru_t 
{
	int32_t cmd;
	uint16_t err;
	uint16_t data_len;
	uint8_t data[0];
} UniformProtocolTlvBodyHead_t;

typedef union _uniform_protocol_body_head_union_t 
{
	UniformProtocolNormalBodyHead_t normal;
	UniformProtocolTlvBodyHead_t tlv;
} UniformProtocolBodyHead_t;

typedef struct _uniform_protocol_frame_t 
{
	uint32_t				seq_id;
	uint16_t				packet_num;
	uint16_t				packet_index;
	uint8_t					request_body_type;
	uint8_t					reserved;
} UniformProtocolFrame_t;

typedef struct _uniform_protocol_head_stru_t
{
	uint32_t				delimiter;
	uint8_t					verion;
	uint8_t					frame_type;
	uint32_t				dst;
	uint32_t				src;
	uint16_t				tunnel_id;
	uint16_t				frame_Length;
	uint16_t				body_checksum;
	uint16_t				head_checksum;

	UniformProtocolFrame_t	frame;

	UniformProtocolBodyHead_t body;
} UniformProtocolHead_t;

#pragma pack()

unsigned char little_or_big_endian(void);

uint16_t calculateCRC16(const uint8_t* data, uint16_t length);

int32_t proto_decode_uniform(const uint8_t *data, int32_t len, 
							 UniformProtocolHead_t **head, int32_t *read_len);

int32_t proto_encode_uniform(int8_t reqtype, uint8_t *data, int32_t len, 
							 uint16_t pkg_num, uint16_t pkg_idx, int8_t uniform_type,
							 uint16_t body_total_len, int32_t cmd, uint16_t err);

#endif /* _PROTO_NOVA_UNIFORM_H */
