/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_pwm_detect_cfg.h
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
#ifndef __DEV_PWM_DETECT_CFG_H_
#define __DEV_PWM_DETECT_CFG_H_


#include "mcu_bsp.h"

void DevPwmOutDutySet(uint8_t duty);
void DevPwmInGet(uint32_t *p_freq, uint8_t *p_duty);
void DevPwmInOutInit(void);


#endif










