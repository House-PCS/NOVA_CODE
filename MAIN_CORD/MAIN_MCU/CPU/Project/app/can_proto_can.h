/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_can_test.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
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
  
#include "dev_can_comm.h"
uint8_t GetSystemPowerStatus(void);
void    CleanPowerStatus(void);
void    AckBmsRomotShutDownCtrl(uint8_t status);


