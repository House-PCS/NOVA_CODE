/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        rs485_test.h
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
#ifndef __RS485_TEST_H_
#define __RS485_TEST_H_

#include "dev_uart_comm.h"
//#include "sys_info.h"


void RS485TestCmdSend(uint8_t idx);
void RS485RxProcess(void);

#endif





















