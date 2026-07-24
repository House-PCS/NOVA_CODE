/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
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
  * <tr><td>2024-2-4    <td>V1.0.1    <td>xingxiao    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "dev_led.h"

#define LED_IO          (*(volatile int *)0x40000000)


static LedInfo_t   _gstLedInfo[LED_SRC_NUM];


static void _LedOFF(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            LED_IO = 0;
        }
        else
        {
            LED_IO = 1;
        }
    }
    else if(_gstLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
//        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
//        }
//        else
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
//        }
//        _gstLedInfo[index].p_timer_Handle->current_chl = _gstLedInfo[index].tim_ch;
//        TimerPWMDutySet(_gstLedInfo[index].p_timer_Handle, 100);
    }
}

static void _LedON(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            LED_IO = 1;
        }
        else
        {
            LED_IO = 0;
        }
    }
    else if(_gstLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
//        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
//
//        }
//        else
//        {
//            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
//        }
//        _gstLedInfo[index].p_timer_Handle->current_chl = _gstLedInfo[index].tim_ch;
//        TimerPWMDutySet(_gstLedInfo[index].p_timer_Handle, 100);
    }
}

void LedBlinkTime()
{
    _gstLedInfo[0].led_status_period = 10000;
    _gstLedInfo[1].led_status_period = 10000;
}
static void _LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = _gstLedInfo[index].led_status_period;
    if(comm_fail_flag)
    {
		period = 100;
    }
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= period / 2)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= period)
    {
        timecount[index] = TimGetSystime();
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 0;
        }
    }
}

void LedCtlOn(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_ON;
}

void LedCtlOff(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_OFF;
}

void LedCtlBlink(EN_LED_SRC src, uint32_t period)
{
    _gstLedInfo[src].led_status = LED_STATUS_BLINK;
    _gstLedInfo[src].led_status_period = period;
}

void DevLedInit(void)
{
    _gstLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gstLedInfo[LED_SRC_SYS].led_status = LED_STATUS_BLINK;
    _gstLedInfo[LED_SRC_SYS].led_status_period = 1000;
    _gstLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;
}

void DevLedInitSuccess(void)
{
    _gstLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gstLedInfo[LED_SRC_SYS].led_status = LED_STATUS_BLINK;
    _gstLedInfo[LED_SRC_SYS].led_status_period = 1000;
    _gstLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;
}

void DevLedInitFailure(void)
{
    _gstLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gstLedInfo[LED_SRC_SYS].led_status = LED_STATUS_BLINK;
    _gstLedInfo[LED_SRC_SYS].led_status_period = 5000;
    _gstLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;
}

void DevLedProcess(void)
{
    static EN_LED_STATUS pre_status[LED_SRC_NUM];
    for(uint8_t i = 0; i < LED_SRC_NUM; i++)
    {
        switch(_gstLedInfo[i].led_status)
        {
            case LED_STATUS_BLINK:
            {
                if(pre_status[i] != LED_STATUS_BLINK)
                {
                    _LedBlink((EN_LED_SRC)i, 1);
                    pre_status[i] = LED_STATUS_BLINK;
                }
                else
                {
                    _LedBlink((EN_LED_SRC)i, 0);
                }
                break;
            }
            default:
                break;
        }
    }
}

