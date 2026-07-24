
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_pwm_detect_cfg.c
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
#include "dev_pwm_detect_cfg.h"



/** 
* @brief    PWM输入检测及输出初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevPwmInOutInit()
{
    PWMFanRdInputBspInit();
    PWMFanOutputBspInit();
    DevPwmOutDutySet(0);
}




/** 
* @brief    PWM FAN RD输出占空比设置
* @param    duty:占空比的值，0~100
* @return   none
* @note:
*/
void DevPwmOutDutySet(uint8_t duty)
{
    gTimer2PwmFanHandle.current_chl = 0;
    gTimer2PwmFanHandle.oc_pwm_duty = duty;
    TimerPWMDutySet(&gTimer2PwmFanHandle, 100);
}





/** 
* @brief    PWM FAN输入频率和占空比回读
* @param    p_freq:频率的值回读
* @param    p_duty:占空比的值回读，0~100
* @return   none
* @note:
*/
void DevPwmInGet(uint32_t *p_freq, uint8_t *p_duty)
{
    gTimer0PwmFanRdHandle.current_chl = 0;
    TimerNoCapEventDetect(&gTimer0PwmFanRdHandle);
    *p_freq = gTimer0PwmFanRdHandle.pwm_capture_freq[gTimer0PwmFanRdHandle.current_chl];
    *p_duty = gTimer0PwmFanRdHandle.pwm_capture_duty[gTimer0PwmFanRdHandle.current_chl];
}





















