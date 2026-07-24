/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-1-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-1-16    <td>V1.0.0    <td>             <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "F28x_Project.h"
#include "dev_led.h"
#include "gpio.h"
#include "cpu_timers.h"

void LED_100ms(void)
{
    static uint16_t leddelay = 0;

    leddelay++;
    if(leddelay == 5)
    {
        LedGpioToggle();
        leddelay = 0;
    }
}

static LedInfo_t   _gstLedInfo[LED_SRC_NUM];


static void _LedOFF(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            SetLedGpioState(0);
        }
        else
        {
            SetLedGpioState(1);
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
            SetLedGpioState(1);
        }
        else
        {
            SetLedGpioState(0);
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


static void _LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = _gstLedInfo[index].led_status_period;
    
    if(first_flag == 1)/*?*/
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


static void _LedStandby(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 2000;
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if(TimDifftimeFromNow(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
    }
}

static void _LedFailed(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 3000;
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 400)
    {
        if(led_flag[index] == 2)
        {
            _LedON(index);
            led_flag[index] = 3;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 500)
    {
        if(led_flag[index] == 3)
        {
            _LedOFF(index);
            led_flag[index] = 4;
        }
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 700)
    {
        if(led_flag[index] == 4)
        {
            _LedON(index);
            led_flag[index] = 5;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 800)
    {
        if(led_flag[index] == 5)
        {
            _LedOFF(index);
            led_flag[index] = 6;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 1000)
    {
        if(led_flag[index] == 6)
        {
            _LedON(index);
            led_flag[index] = 7;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 1100)
    {
        if(led_flag[index] == 7)
        {
            _LedOFF(index);
            led_flag[index] = 8;
        }
    }
    
    if(TimDifftimeFromNow(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
    }
}

static void _LedSucceed(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 3000;
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if((uint32_t)(TimGetSystime() - timecount[index]) >= 400)
    {
        if(led_flag[index] == 2)
        {
            _LedON(index);
            led_flag[index] = 3;
        }
    }

    if((uint32_t)(TimGetSystime() - timecount[index]) >= 500)
    {
        if(led_flag[index] == 3)
        {
            _LedOFF(index);
            led_flag[index] = 4;
        }
    }
    
    
    if(TimDifftimeFromNow(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = TimGetSystime();
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


void LedCtlStandby(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_STANDBY;
}

void LedCtlSucceed(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_SUCCEED;
}

void LedCtlFailed(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_FAILED;
}




void DevLedInit(void)
{    
    _gstLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gstLedInfo[LED_SRC_SYS].led_status = LED_STATUS_IDLE;
    _gstLedInfo[LED_SRC_SYS].led_status_period = RUN_LED_BRINK_PRIED;
    _gstLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;

    LedCtlBlink(LED_SRC_SYS, RUN_LED_BRINK_PRIED);
}


void DevLedProcess(void)
{
    uint8_t i;
    static EN_LED_STATUS pre_status[LED_SRC_NUM];

    for(i = 0; i < LED_SRC_NUM; i++)
    {
        switch(_gstLedInfo[i].led_status)
        {
            case LED_STATUS_IDLE:
            {
                
                break;
            }
            case LED_STATUS_OFF:
            {
                _LedOFF((EN_LED_SRC)i);
                _gstLedInfo[i].led_status = LED_STATUS_IDLE;
                break;
            }
            case LED_STATUS_ON:
            {
                _LedON((EN_LED_SRC)i);
                _gstLedInfo[i].led_status = LED_STATUS_IDLE;
                break;
            }
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
            
            case LED_STATUS_STANDBY:
                        {
                            if(pre_status[i] != LED_STATUS_STANDBY)
                            {
                                _LedStandby((EN_LED_SRC)i, 1);
                                pre_status[i] = LED_STATUS_STANDBY;
                            }
                            else
                            {
                                _LedStandby((EN_LED_SRC)i, 0);
                            }
                            break;
                        }
                        case LED_STATUS_SUCCEED:
                        {
                            if(pre_status[i] != LED_STATUS_SUCCEED)
                            {
                                _LedSucceed((EN_LED_SRC)i, 1);
                                pre_status[i] = LED_STATUS_SUCCEED;
                            }
                            else
                            {
                                _LedSucceed((EN_LED_SRC)i, 0);
                            }
                            break;
                        }
                        case LED_STATUS_FAILED:
                        {
                            if(pre_status[i] != LED_STATUS_FAILED)
                            {
                                _LedFailed((EN_LED_SRC)i, 1);
                                pre_status[i] = LED_STATUS_FAILED;
                            }
                            else
                            {
                                _LedFailed((EN_LED_SRC)i, 0);
                            }
                            break;
                        }
            default:break;
        }
    }
}

