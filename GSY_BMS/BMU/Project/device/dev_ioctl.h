/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ioctl.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-7-26
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-7-26      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "mcu_bsp.h"
   

typedef enum
{
    EN_DEV_IOCTL_IDX_JACK_OUT_1,
    EN_DEV_IOCTL_IDX_JACK_OUT_2,
//    EN_DEV_IOCTL_IDX_LED_OUT,
    
} EN_DEV_IOCTL_IDX_OUT;
  
  
  
typedef enum
{
    EN_DEV_IOCTL_IDX_JACK_IN_1,
    EN_DEV_IOCTL_IDX_JACK_IN_2
} EN_DEV_IOCTL_IDX_IN;
  
void DevIOCtlInit(void);
void DevIOCtlOutSet(EN_DEV_IOCTL_IDX_OUT idx);
void DevIOCtlOutReset(EN_DEV_IOCTL_IDX_OUT idx);
uint8_t DevIOCtlInGet(EN_DEV_IOCTL_IDX_IN idx);
uint8_t DevIOCtlInGetInvert(EN_DEV_IOCTL_IDX_IN idx);






