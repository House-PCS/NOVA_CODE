
#ifndef __DEV_MODBUS_S_1_H_
#define __DEV_MODBUS_S_1_H_


#include "mcu_bsp.h"
#include "uart.h"
#include "queue.h"
#include "systick.h"


typedef enum
{
    EN_DEV_UART_TYPE_485_1 = 0,
    EN_DEV_UART_TYPE_485_2,
    EN_DEV_UART_TYPE_TTL,
    EN_DEV_UART_TYPE_NUM
}EN_DEV_UART_TYPE;


typedef struct _comm_host_rev_data_1_stru_t
{
    uint16_t                data_size;
    uint8_t                 *pData;
}DevUartRevData_t; 



void DevUartCommInit(void);
void DevUartCommSend(EN_DEV_UART_TYPE type, uint8_t *p, uint16_t size);
bool Dev485SendCompleteJudge(EN_DEV_UART_TYPE type);
void DevUartCommDirCtlProcess(void);

extern Queue_t	gDevUartRevQueue[];

#endif








