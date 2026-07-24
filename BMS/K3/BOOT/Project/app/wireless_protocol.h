
#ifndef __WIRELESS_PROTOCOL_H_
#define __WIRELESS_PROTOCOL_H_

//#include "dev_fc41d.h"
#include "request_body.h"
#include "dev_communication.h"

#define MAX_CMD_NUM	 3






typedef enum
{
	UPPER_CMD_TYPE_READ_REQ,
	UPPER_CMD_TYPE_WRITE_REQ,
	UPPER_CMD_TYPE_REPORT_RESP,
}EN_UPPER_CMD_TYPE;


typedef enum
{
	EN_UPPER_ERROR_CODE_NULL,
}EN_UPPER_ERROR_CODE;





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


typedef struct _uniform_protocol_request_body_stru_t
{
	uint32_t		        cmd;
	EN_UPPER_ERROR_CODE		err_code;
	uint16_t				user_data_len;
	uint8_t					*p_user_data;
}UniformProtocolRequestBody_t;


typedef struct _upper_comm_frame_stru_t
{
	UniformProtocolHead_t   		upper_head;
	UniformProtocolRequestBody_t	*p_upper_request_body;
		
}UpperCommFrame_t;

void WirelessDataProcess(void);



typedef enum
{
    EN_UPLOAD_STATUS_IDLE,
    EN_UPLOAD_STATUS_SEND,
    EN_UPLOAD_STATUS_WAIT,
    EN_UPLOAD_STATUS_WAIT_LAST,
}EN_UPLOAD_STATUS;








typedef enum
{
    ACTIVE_REPORT_TYPE_DEVICE_PARA,
    ACTIVE_REPORT_TYPE_DEVICE_INTERFACE,
    ACTIVE_REPORT_TYPE_DEVICE_CELL_GROUP,
    ACTIVE_REPORT_TYPE_MAX_NUM
}EN_ACTIVE_REPORT_TYPE;





/*按照协议格式定义所有通信数据结构体*/










#endif














