/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_process.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-18
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-18     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#ifndef __MODBUS_SLAVE_PROCESS_H_
#define __MODBUS_SLAVE_PROCESS_H_

#include "dev_uart_comm.h"


typedef enum
{
    EN_MODBUS_SLAVE_RTU,
    EN_MODBUS_SLAVE_TCP
}EN_MODBUS_SLAVE_TYPE;

typedef enum
{
    EN_MODBUS_SLAVE_REV_IDLE,
    EN_MODBUS_SLAVE_REV_ANALYSIS,
    EN_MODBUS_SLAVE_REV_ACK,
    EN_MODBUS_SLAVE_REV_ACK_WAIT,
}EN_MODBUS_SLAVE_REV_STATUS;

typedef enum
{
    EN_MODBUS_SLAVE_FUNC_CODE_READ_INPUT = 0x04,
    EN_MODBUS_SLAVE_FUNC_CODE_READ_REMAIN = 0x03,
    EN_MODBUS_SLAVE_FUNC_CODE_WRITE_SINGLE = 0x06,
    EN_MODBUS_SLAVE_FUNC_CODE_WRITE_MULTI = 0x10
}EN_MODBUS_SLAVE_FUNC_CODE;

typedef enum
{
    EN_MODBUS_SLAVE_ERROR_CODE_CORRECT = 0x00,
    EN_MODBUS_SLAVE_ERROR_CODE_INVALID_CMD = 0x01,
    EN_MODBUS_SLAVE_ERROR_CODE_INVALID_REG_ADDR = 0x02,
    EN_MODBUS_SLAVE_ERROR_CODE_INVALID_DATA = 0x03,
    EN_MODBUS_SLAVE_ERROR_CODE_DEV_FAULT = 0x04,
    EN_MODBUS_SLAVE_ERROR_CODE_SLAVE_BUSY = 0x06,
    EN_MODBUS_SLAVE_ERROR_CODE_INVLID_LEN = 0x0E,
}EN_MODBUS_SLAVE_ERROR_CODE;



//typedef struct _modbus_slave_reg_info_stru_t
//{
//    
//}ModBusSlaveRegInfo_t;



void ModbusSlaveRxProcess();


#endif


