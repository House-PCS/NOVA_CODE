
#ifndef __DEV_MODBUS_H_H_
#define __DEV_MODBUS_H_H_


#include "bsp.h"
#include "queue.h"
#include "systick.h"


#define MODBUS_RTU_REV_QUEUE_DEPTH   2
#define MODBUS_RTU_RX_SIZE           2068


typedef struct _comm_host_rev_data_stru_t
{
    uint16_t                    data_size;
    uint8_t                     *p_data;
} ModbusRTURevData_t;


typedef enum
{
    EN_MODBUS_MODE_HOST_MAIN,
    EN_MODBUS_MODE_HOST_DBG,
} EN_MODBUS_MODE;

typedef struct _dev_modbus_host_stru_t
{
    EN_MODBUS_MODE              mode;
    bool                        dir_manual;//485方向是否手动控制
    uint8_t                     rxbuf[MODBUS_RTU_RX_SIZE];
    uint8_t                     rx_queue_buf[MODBUS_RTU_REV_QUEUE_DEPTH][MODBUS_RTU_RX_SIZE];
    uint16_t                    rev_index;
    uint16_t                    rev_len;
    
    Queue_t                     rev_queue;
    bool                        flag_send_complete;//只在dir_manual=1时有用
    uint32_t                    send_time_out;
    uint32_t                    send_max_time;
    void                        (*send)(Uart_HandleType *);
} DevModbusHost_t;

extern DevModbusHost_t gDevModbus;
extern DevModbusHost_t gDbgUart1;

void DevModbusHostRTUInit(void);
void DevModbusDirCtlProcess(void);
void ModbusRTUSend(uint8_t *p_data, uint16_t len);
void ModbusRTUSendWithoutCheckSum(uint8_t *p_data, uint16_t len);
void ModbusRTUSendData(uint8_t *p_data, uint16_t len);

void DbgModbusRTUSend(uint8_t *p_data, uint16_t len);
void DbgModbusRTUSendWithoutCheckSum(uint8_t *p_data, uint16_t len);
void DbgModbusRTUSendData(uint8_t *p_data, uint16_t len);


typedef uint8_t (*SlaveUpdataCallFun)(uint8_t *p, uint16_t data_size);
void RegisterSlaveUpdateFunPoint(SlaveUpdataCallFun call_back_fun);
#endif





















