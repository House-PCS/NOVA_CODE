#ifndef PROGRAM_SN_HHH
#include "dev_modbus.h"





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


/*READ*/

#define UPPER_CMD_EVENT_READ_CURRENT_DEVICE_INFO                    0x00FF0002
#define UPPER_CMD_EVENT_READ_CALIBRATION_TIME                       0x00FF0003
#define UPPER_CMD_EVENT_READ_DEVICE_NAME                            0x00FF0004

/*WRITE*/
#define UPPER_CMD_EVENT_WRITE_DEVICE_INFO                           0x80FF0002
#define UPPER_CMD_EVENT_WRITE_DEVICE_NAME                           0x80FF0004
#define UPPER_CMD_EVENT_WRITE_CALIBRATION_TIME                      0x80FF0003


void ProgramFunInital(void);




#endif