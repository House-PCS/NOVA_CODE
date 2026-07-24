/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        rs485_test.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-6-20
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-6-20      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "rs485_test.h"




static void _DataProcess(EN_DEV_UART_TYPE idx, uint8_t *p_buf, uint16_t len)
{
    if(len == 4 && p_buf[0] == 0x11 && p_buf[1] == 0x22 && p_buf[2] == 0x33 && p_buf[3] == 0x44)
    {
        if(idx == EN_DEV_UART_TYPE_485_1)
        {
			RS485TestCmdSend(EN_DEV_UART_TYPE_485_1);
        }
        else if(idx == EN_DEV_UART_TYPE_485_2)
        {
			RS485TestCmdSend(EN_DEV_UART_TYPE_485_2);
        }       
    }
}


/** 
* @brief    485数据接收处理
* @param    none
* @param    none
* @return   none
* @note: 
*/
void RS485RxProcess()
{
    DevUartRevData_t *p_queue_content;
    
    if(QueueGet(&gDevUartRevQueue[EN_DEV_UART_TYPE_485_1], (void *)&p_queue_content) == true)
    {
        _DataProcess(EN_DEV_UART_TYPE_485_1, p_queue_content->pData, p_queue_content->data_size);
    }
    if(QueueGet(&gDevUartRevQueue[EN_DEV_UART_TYPE_485_2], (void *)&p_queue_content) == true)
    {
        _DataProcess(EN_DEV_UART_TYPE_485_2, p_queue_content->pData, p_queue_content->data_size);
    }
}




static uint8_t _RS485TxBuf[4];
/** 
* @brief    485工装测试数据发送
* @param    idx 哪个485口,0代表485_1,1代表485_2
* @param    none
* @return   none
* @note: 
*/
void RS485TestCmdSend(uint8_t idx)
{
    _RS485TxBuf[0] = 0x12;
    _RS485TxBuf[1] = 0x23;
    _RS485TxBuf[2] = 0x34;
    _RS485TxBuf[3] = 0x45;
    
    DevUartCommSend(idx, _RS485TxBuf, 4);
}
