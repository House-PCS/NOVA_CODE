
#ifndef __DEV_MODBUS_H_H_
#define __DEV_MODBUS_H_H_


#include "bsp.h"
#include "queue.h"
#include "systick.h"

typedef struct _comm_host_rev_data_stru_t
{
	uint16_t			        data_size;
	uint8_t				        *p_data;
}ModbusRTURevData_t; 


typedef struct _dev_modbus_host_stru_t
{
    bool                        dir_manual;//485方向是否手动控制
    uint8_t                     *p_rxbuf;
    bool                        flag_send_complete;//只在dir_manual=1时有用
    uint32_t                    send_time_out;
    uint32_t                    send_max_time;
    void                        (*send)(Uart_HandleType *);
}DevModbusHost_t;



extern Queue_t	gModbusMTURevQueue;


void DevModbusHostRTUInit();
void DevModbusDirCtlProcess();
void ModbusRTUSend(uint8_t *p_data, uint16_t len);

#endif





















