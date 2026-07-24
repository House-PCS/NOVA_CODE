
#ifndef __DEV_CAN_COMM_H_
#define __DEV_CAN_COMM_H_


#include "bsp.h"
#include "queue.h"
//#define CAN_BPS_250K    24
//#define CAN_BPS_500K    12

typedef enum
{
	EN_DEV_CAN0 = 0,
	EN_DEV_CAN1,
	EN_DEV_CAN_NUM,
} EN_DEV_CAN;


typedef enum
{
    EN_DEV_CAN_TYPE_STD,
    EN_DEV_CAN_TYPE_EXT,
}EN_DEV_CAN_TYPE;




typedef struct _ext_pdu_stru_t
{
    uint8_t sa;
    uint8_t ps;
    uint8_t pf;
    uint8_t dp:1;
    uint8_t r:1;
    uint8_t p:3;
}DevCanExtPdu_t;

typedef struct _std_pdu_stru_t
{
    uint8_t pf:6;
    uint8_t dp:1;
    uint8_t r:1;
    uint8_t p:3;
}DevCanStdPdu_t;


typedef struct _dev_can_rev_data_stru_t
{
    union
    {
        DevCanExtPdu_t  ext_pdu;
        DevCanStdPdu_t  std_pdu;
        uint32_t        id;
    }can_id;
    
    EN_DEV_CAN      canx;
    EN_DEV_CAN_TYPE can_type;
    uint8_t data_len;
    uint8_t data[8];

}DevCanCommData_t;


void DevCanCommInit(void);
bool DevCanSend(DevCanCommData_t *pDevCanComm);

extern Queue_t	gDevCanRevQueue[];

#endif







