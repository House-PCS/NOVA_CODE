
#ifndef __PROTO_UNIFORM_H_
#define __PROTO_UNIFORM_H_

#include "dev_rs485.h"


#define UPPER_CMD_EVENT_READ_SN                    0x00FF0002
#define UPPER_CMD_EVENT_WRITE_SN                   0x80FF0002

#pragma pack(1)
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
	
	uint32_t				seq_id;
	uint16_t				packet_num;
	uint16_t				packet_index;
	uint8_t					request_body_type;
	uint8_t					reserved;
}UniformProtocolHead_t;
#pragma pack()


void ProtoUniformRevProcess();

#endif






