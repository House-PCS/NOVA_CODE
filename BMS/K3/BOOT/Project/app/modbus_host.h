/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_host.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-12-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-12-16    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#ifndef __MODBUS_SLAVE_H_
#define __MODBUS_SLAVE_H_
#include "bsp.h"
#include "uart.h"
#include "queue.h"

#define SLAVEADDR	            1
#define _MODBUS_RX_SIZE		    256
#define MODBUS_REV_QUEUE_DEPTH	4

/* RTU 应答代码 */
#define RSP_OK				0		/* 成功 */
#define RSP_ERR_CMD			0x01	/* 不支持的功能码 */
#define RSP_ERR_REG_ADDR	0x02	/* 寄存器地址错误 */
#define RSP_ERR_VALUE		0x03	/* 数据值域错误 */
#define RSP_ERR_WRITE		0x04	/* 写入失败 */

typedef enum
{
    MODBUS_STATE_TYPE_IDLE,
    MODBUS_STATE_TYPE_WAIT_ACK03H,
    MODBUS_STATE_TYPE_WAIT_ACK10H,
    MODBUS_STATE_TYPE_GOT_STAAP,
    MODBUS_STATE_TYPE_SET_TCP_SOCKET,
    MODBUS_STATE_TYPE_SET_UDP_SOCKET,
    MODBUS_STATE_TYPE_TCP_SOCKET_OPEN,
    MODBUS_STATE_TYPE_UDP_SOCKET_OPEN,
    MODBUS_STATE_TYPE_GET_STAAP,
    MODBUS_STATE_TYPE_UPLOAD_ACK,
}EN_MODBUS_STATE_TYPE;

typedef struct
{
	uint8_t RxBuf[_MODBUS_RX_SIZE];
	uint8_t RxCount;
	uint8_t RxStatus;
	uint8_t RxNewFlag;

	uint8_t RspCode;

	uint8_t TxBuf[_MODBUS_RX_SIZE];
	uint8_t TxCount;
	
	uint16_t Reg01H;		/* 保存主机发送的寄存器首地址 */
	uint16_t Reg02H;
	uint16_t Reg03H;		
	uint16_t Reg04H;

	uint8_t RegNum;			/* 寄存器个数 */

	uint8_t fAck03H;	    /* 应答命令标志 0 表示执行失败 1表示执行成功 */
	uint8_t fAck10H;
	
}MODH_T;

typedef struct _modbus_rev_data_stru_t
{
    uint16_t                     data_size;
	uint8_t				        *pData;
}ModbusRevData_t; 








bool ModHInit(void);
void ModHProcess(void);
#endif














