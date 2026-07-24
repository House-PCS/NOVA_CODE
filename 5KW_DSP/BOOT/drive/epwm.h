/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        epwm.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */


#ifndef __EPWM_H__
#define __EPWM_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "F28x_Project.h"     // Device Headerfile and Examples Include File

extern volatile struct EPWM_REGS *ePWM[];
void DisablePWMCLKCounting(void);
//TODO EnablePWMCLKCounting
void EnablePWMCLKCounting(void);
void SetupInverterPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks, uint16_t pwm_dbfed_ticks);
void SetupPVPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks, uint16_t pwm_dbfed_ticks);
void SetupLLCPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks, uint16_t phase_move);
void SetupBusPWM(uint16_t inv_pwm_no, uint16_t pwm_period_ticks, uint16_t pwm_dbred_ticks);
#ifdef __cplusplus
}
#endif                                  /* extern "C" */

#endif

//===========================================================================
// End of file.
//===========================================================================
