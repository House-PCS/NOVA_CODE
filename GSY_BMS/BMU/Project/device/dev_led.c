/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-11-22
  * @brief       GD32F450
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-11-22    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_led.h"


static LedInfo_t   _gLedInfo[LED_SRC_NUM];


/** 
* @brief  LED灯灭IO控制
* @param  none
* @return none
* @note:
*/
static void _LedOFF(EN_LED_SRC index)
{
    if(_gLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitReset(_gLedInfo[index].p_gpio_Handle);
        }
        else
        {
            GpioBitSet(_gLedInfo[index].p_gpio_Handle);
        }
    }
    else if(_gLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
        if(_gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            _gLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
        }
        else
        {
            _gLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
        }
        _gLedInfo[index].p_timer_Handle->current_chl = _gLedInfo[index].tim_ch;
        TimerPWMDutySet(_gLedInfo[index].p_timer_Handle, 100);
    }
}


/** 
* @brief  LED灯亮IO控制
* @param  none
* @return none
* @note:
*/
static void _LedON(EN_LED_SRC index)
{
    if(_gLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitSet(_gLedInfo[index].p_gpio_Handle);
        }
        else
        {
            GpioBitReset(_gLedInfo[index].p_gpio_Handle);
        }
    }
    else if(_gLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
        if(_gLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            _gLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
            
        }
        else
        {
            _gLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
        }
        _gLedInfo[index].p_timer_Handle->current_chl = _gLedInfo[index].tim_ch;
        TimerPWMDutySet(_gLedInfo[index].p_timer_Handle, 100);
    }
}



/** 
* @brief  LED灯闪烁IO控制
* @param  none
* @return none
* @note:
*/
static void _LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = _gLedInfo[index].led_status_period;
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;
        timecount[index] = tim_get_systime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= period / 2)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= period)
    {
        timecount[index] = tim_get_systime();
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 0;
        }
    }
}


/** 
* @brief  LED灯长亮接口
* @param  none
* @return none
* @note:
*/
void LedCtlOn(EN_LED_SRC src)
{
    _gLedInfo[src].led_status = LED_STATUS_ON;
}

/** 
* @brief  LED灯长灭接口
* @param  none
* @return none
* @note:
*/
void LedCtlOff(EN_LED_SRC src)
{
    _gLedInfo[src].led_status = LED_STATUS_OFF;
}

/** 
* @brief  LED灯闪烁接口
* @param  period:闪烁周期，一亮一灭算一个周期
* @return none
* @note:
*/
void LedCtlBlink(EN_LED_SRC src, uint32_t period)
{
    _gLedInfo[src].led_status = LED_STATUS_BLINK;
    _gLedInfo[src].led_status_period = period;
}

/** 
* @brief  LED灯初始化
* @param  none
* @return none
* @note:
*/
void DevLedInit()
{
    _gLedInfo[LED_SRC_DEBUG].led_hw_mode = LED_HW_GPIO;
    _gLedInfo[LED_SRC_DEBUG].led_status = LED_STATUS_IDLE;
    _gLedInfo[LED_SRC_DEBUG].led_status_period = 1000;//默认值
    _gLedInfo[LED_SRC_DEBUG].led_active_level = LED_HW_ACTIVE_LOW;
    _gLedInfo[LED_SRC_DEBUG].p_gpio_Handle = &gGpioLedHandle;
    
    GpioLedBspInit();
    
    _LedOFF(LED_SRC_DEBUG);
}



/** 
* @brief  LED灯操作大轮询
* @param  none
* @return none
* @note:
*/

void DevLedProcess()
{
    uint8_t i;
    static EN_LED_STATUS pre_status[LED_SRC_NUM];

    for(i = 0; i < LED_SRC_NUM; i++)
    {
        switch(_gLedInfo[i].led_status)
        {
            case LED_STATUS_IDLE:
            {
                
                break;
            }
            case LED_STATUS_OFF:
            {
                _LedOFF(i);
                _gLedInfo[i].led_status = LED_STATUS_IDLE;
                break;
            }
            case LED_STATUS_ON:
            {
                _LedON(i);
                _gLedInfo[i].led_status = LED_STATUS_IDLE;
                break;
            }
            case LED_STATUS_BLINK:
            {
                if(pre_status[i] != LED_STATUS_BLINK)
                {
                    _LedBlink(i, 1);
                    pre_status[i] = LED_STATUS_BLINK;
                }
                else
                {
                    _LedBlink(i, 0);
                }
                break;
            }
            default:break;
        }
    }
}







