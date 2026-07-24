/*
 * dev_can_comm.h
 *
 *  Created on: 2025Äê12ÔÂ17ÈÕ
 *      Author: Nova008226
 */

#ifndef DEVICE_DEV_CAN_COMM_H_
#define DEVICE_DEV_CAN_COMM_H_


#include "stdint.h"
#include "stdbool.h"
#include "queue.h"



typedef enum
{
    EN_DEV_CAN_TX0 = 0,
    EN_DEV_CAN_TX1,
} EN_DEV_CAN_TX_INDEX;

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
    uint16_t sa;
    uint16_t ps;
    uint16_t pf;
    uint16_t dp:1;
    uint16_t r:1;
    uint16_t p:3;
}DevCanExtPdu_t;

typedef struct _std_pdu_stru_t
{
    uint16_t pf:6;
    uint16_t dp:1;
    uint16_t r:1;
    uint16_t p:3;
}DevCanStdPdu_t;


typedef struct _dev_can_rev_data_stru_t
{

    EN_DEV_CAN_TX_INDEX    e_can_tx_index;
    union
    {
        DevCanExtPdu_t  ext_pdu;
        DevCanStdPdu_t  std_pdu;
        uint32_t        id;
    }can_id;

    EN_DEV_CAN      canx;
    EN_DEV_CAN_TYPE can_type;
    uint16_t data_len;
    uint16_t data[8];

}DevCanCommData_t;


void DevCanCommInit(void);
bool DevCanSend(DevCanCommData_t *pDevCanComm);

bool CanRecv(void);

#endif /* DEVICE_DEV_CAN_COMM_H_ */
