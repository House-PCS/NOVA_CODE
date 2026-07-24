
#ifndef __DEV_MODBUS_S_H_
#define __DEV_MODBUS_S_H_


#include "bsp.h"
#include "uart.h"
#include "queue.h"

#define MODBUS_DEVICE_ADDR 0x05             //´Ó»úµØÖ·

typedef enum
{
	UART_DATA_TYPE_NORMAL,
	UART_DATA_TYPE_UPGRADE,
}EN_UART_DATA_TYPE;

typedef struct _comm_host_rev_data_stru_t
{
	EN_UART_DATA_TYPE			data_type;
	uint16_t			        data_size;
	uint8_t				        *pData;
}ModbusRTURevData_t; 



void DevModbusRTUInit();
void ModbusRTUSend(uint8_t *p, uint16_t size);
bool DevModbusSendComplete();
void DevModbusDirCtlProcess();

extern Queue_t	gModbusMTURevQueue;
void TabLsbToMsb(uint8_t *str, uint16_t len);
uint16_t BEBufToUint16(uint8_t *_pBuf);

#endif








