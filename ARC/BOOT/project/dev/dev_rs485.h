
#ifndef _DEV_RS485_H_
#define _DEV_RS485_H_


#include "bsp.h"
#include "queue.h"
#include "systick.h"




typedef struct _comm_485_stru_t
{
    bool            data_received;
    uint8_t         *p_data;
    uint16_t        data_len;
    
    void            (*send)(Uart_HandleType *);
}DevComm485_t;


extern DevComm485_t gDevComm485;

void DevComm485Init();
void Comm485Send(uint8_t *p_data, uint16_t len);



#endif





