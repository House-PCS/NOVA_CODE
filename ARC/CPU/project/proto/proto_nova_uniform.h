#ifndef _PROTO_NOVA_UNIFORM_H
#define _PROTO_NOVA_UNIFORM_H

#include <stdint.h>
#include "kstar.h"

#define UNIFORM_TYPE		1

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

#define PROTO_UNIFORM_R_DEBUG			0x12345677
#define PROTO_UNIFORM_R_HTTP			0x12345678

// 周边WIFI热点信息
#define PROTO_UNIFORM_R_WIFI_SCAN		0x00FF0000

// 要连接的WIFI热点信息
#define PROTO_UNIFORM_R_WIFI_INFO		0x00FF0001
#define PROTO_UNIFORM_W_WIFI_INFO		0x80FF0001

// 采集器设备信息
#define PROTO_UNIFORM_R_DEV_INFO		0x00FFA002
#define PROTO_UNIFORM_W_DEV_INFO		0x80FFA002

// 设置采集器MQTT连接的Server信息
#define PROTO_UNIFORM_R_DEV_MQTT_INFO		0x00FFA003
#define PROTO_UNIFORM_W_DEV_MQTT_INFO		0x80FFA003

// 设置采集器设备名称
#define PROTO_UNIFORM_R_DEV_NAME			0x00FF0004
#define PROTO_UNIFORM_W_DEV_NAME			0x80FF0004

// 采集器属性信息
#define PROTO_UNIFORM_R_DEV_PROPERTY		0x00FFA006
#define PROTO_UNIFORM_W_DEV_PROPERTY		0x80FFA006

// 采集器升级
#define PROTO_UNIFORM_W_DEV_UPGRADE			0x00FFA00B

#define PROTO_UNIFORM_R_DEV_PRODUCT			0x00FFA00C

#pragma pack(1)
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

int32_t proto_encode_uniform(int8_t reqtype, const uint8_t *data, int32_t len, 
							 uint16_t pkg_num, uint16_t pkg_idx, int8_t uniform_type,
							 uint16_t body_total_len, int32_t cmd, uint16_t err);

#endif /* _PROTO_NOVA_UNIFORM_H */
