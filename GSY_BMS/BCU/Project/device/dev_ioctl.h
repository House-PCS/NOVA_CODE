
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_ioctl.h
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
  * <tr><td>2023-6-9      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#ifndef __DEVICE_IOCTL_H_
#define __DEVICE_IOCTL_H_

#include "mcu_bsp.h"


typedef enum
{
    EN_DEV_IOCTL_IDX_2WAY_OUT_1,//从控编址
    EN_DEV_IOCTL_IDX_2WAY_OUT_2,//从控编址备份
    EN_DEV_IOCTL_IDX_2WAY_OUT_3,//增加和华塑一样的双向IO,预留
    EN_DEV_IOCTL_IDX_2WAY_OUT_4,//增加和华塑一样的双向IO,预留
	EN_DEV_IOCTL_IDX_DRY_OUT_1,//预留
    EN_DEV_IOCTL_IDX_DRY_OUT_2,//预留
    EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_1,//断路器控制
    EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_2,//风扇继电器控制
    EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_3,//预留
    EN_DEV_IOCTL_IDX_LOW_SIDE_OUT_4,//预留
    EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_1,//总正继电器驱动
    EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_2,//总负继电器驱动
    EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_3,//预充继电器驱动
    EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_4,//预留
    EN_DEV_IOCTL_IDX_ISO_OUT_1,//控制绝缘阻抗上MOS
    EN_DEV_IOCTL_IDX_ISO_OUT_2,//控制绝缘阻抗下MOS
    EN_DEV_IOCTL_IDX_ISO_OUT_3,//控制绝缘阻抗继电器
   
    EN_DEV_IOCTL_IDX_OUT_NUM,
} EN_DEV_IOCTL_IDX_OUT;

typedef enum
{
    EN_DEV_IOCTL_IDX_2WAY_IN_1,//从控编址IO状态回读
    EN_DEV_IOCTL_IDX_2WAY_IN_2,//从控编址备份IO状态回读
    EN_DEV_IOCTL_IDX_2WAY_IN_3,//增加和华塑一样的双向IO,预留
    EN_DEV_IOCTL_IDX_2WAY_IN_4,//增加和华塑一样的双向IO,预留
    EN_DEV_IOCTL_IDX_ACT_LOW_IN_1,//断路器反馈
    EN_DEV_IOCTL_IDX_ACT_LOW_IN_2,//预留
    EN_DEV_IOCTL_IDX_ACT_LOW_IN_3,//预留
    EN_DEV_IOCTL_IDX_ACT_HIGH_IN_1,//总正继电器反馈
    EN_DEV_IOCTL_IDX_ACT_HIGH_IN_2,//总负继电器反馈
    EN_DEV_IOCTL_IDX_ACT_HIGH_IN_3,//预充继电器反馈
    EN_DEV_IOCTL_IDX_ACT_HIGH_IN_4,//预留
    EN_DEV_IOCTL_IDX_ISO_IN_1,//预留
    EN_DEV_IOCTL_IDX_IN_NUM,
} EN_DEV_IOCTL_IDX_IN;

void DevIOCtlInit(void);
void DevIOCtlOutSet(EN_DEV_IOCTL_IDX_OUT idx);
void DevIOCtlOutReset(EN_DEV_IOCTL_IDX_OUT idx);
uint8_t DevIOCtlInGet(EN_DEV_IOCTL_IDX_IN idx);
uint8_t DevIOCtlInGetInvert(EN_DEV_IOCTL_IDX_IN idx);

#endif








