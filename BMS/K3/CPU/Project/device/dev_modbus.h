
#ifndef __DEV_MODBUS_H_
#define __DEV_MODBUS_H_


#include "uart.h"
#include "queue.h"
#include "bsp.h"

#define MODBUS_DEVICE_ADDR	    0x04

typedef enum
{
	TYPE_MODBUS_DATA,
	TYPE_COM_DATA,
    TYPE_MPPT_DATA,
}EN_UART_DATA_TYPE;

typedef struct _comm_host_rev_data_stru_t
{
    uint8_t                     slave_addr;
    EN_UART_DATA_TYPE           data_type;    
	uint16_t			        data_size;
	uint8_t				        *pData;
}ModbusRTURevData_t; 



void DevModbusRTUInit(void);
void UART1ToFPGASoftcoreInit(void);
void ModbusRTUSend(uint8_t *p, uint16_t size);
bool DevModbusSendComplete(void);
void DevModbusDirCtlProcess(void);
uint16_t BEBufToUint16(uint8_t *_pBuf);
void TabLsbToMsb(uint8_t *str, uint16_t len);
void UartSendTest(void);
extern Queue_t	gModbusRTURevQueue;
extern uint8_t gBMSSleepFlag;
#endif








