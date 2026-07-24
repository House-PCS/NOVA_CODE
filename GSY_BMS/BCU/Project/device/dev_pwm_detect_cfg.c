
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
    PwmInputCapBspInit();
//    PwmOutputBspInit();
    DevPwmOutDutySet(0);
//    DevPwmLedDclkDutySet(50);
}




/** 
* @brief    PWM FAN RD输出占空比设置
* @param    duty:占空比的值，0~100
* @return   none
* @note:
*/
void DevPwmOutDutySet(uint8_t duty)
{
    gTimer0PwmOutHandle.current_chl = 0;
    gTimer0PwmOutHandle.oc_pwm_duty = duty;
    TimerPWMDutySet(&gTimer0PwmOutHandle, 100);
}


/** 
* @brief    PWM LED BORAD DCLK输出占空比设置
* @param    duty:占空比的值，0~100
* @return   none
* @note:
*/
//void DevPwmLedDclkDutySet(uint8_t duty)
//{
//    gTimer10TestBoradOutHandle.current_chl = 0;
//    gTimer10TestBoradOutHandle.oc_pwm_duty = duty;
//    TimerPWMDutySet(&gTimer10TestBoradOutHandle, 100);
//}


/** 
* @brief    PWM FAN输入频率和占空比回读
* @param    p_freq:频率的值回读
* @param    p_duty:占空比的值回读，0~100
* @return   none
* @note:
*/
void DevPwmInGet(uint32_t *p_freq, uint8_t *p_duty)
{
    gTimer1PwmInHandle.current_chl = 0;
    TimerNoCapEventDetect(&gTimer1PwmInHandle);
    *p_freq = gTimer1PwmInHandle.pwm_capture_freq[gTimer1PwmInHandle.current_chl];
    *p_duty = gTimer1PwmInHandle.pwm_capture_duty[gTimer1PwmInHandle.current_chl];
}





















