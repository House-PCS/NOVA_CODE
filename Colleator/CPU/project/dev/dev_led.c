
#include "dev_led.h"



int8_t gDevLedProcessFlag = 0;
static LedInfo_t   _gLedInfo[LED_SRC_NUM];


void _LedOFF(EN_LED_SRC index)
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

void _LedON(EN_LED_SRC index)
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


static void _LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = _gLedInfo[index].led_status_period;

    if(first_flag == 1)/*??????'??*/
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








void LedCtlOn(EN_LED_SRC src)
{
    _gLedInfo[src].led_status = LED_STATUS_ON;
}

void LedCtlOff(EN_LED_SRC src)
{
    _gLedInfo[src].led_status = LED_STATUS_OFF;
}

void LedCtlBlink(EN_LED_SRC src, uint32_t period)
{
    _gLedInfo[src].led_status = LED_STATUS_BLINK;
    _gLedInfo[src].led_status_period = period;
}


void LedDirectOn(EN_LED_SRC src)
{
	_LedON(src);
}

void LedDirectOff(EN_LED_SRC src)
{
	_LedOFF(src);
}


void DevLedInit()
{
    _gLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gLedInfo[LED_SRC_SYS].led_status = LED_STATUS_IDLE;
    _gLedInfo[LED_SRC_SYS].led_status_period = 1000;//I???
    _gLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;
    _gLedInfo[LED_SRC_SYS].p_gpio_Handle = &LedSysHandle;

    _gLedInfo[LED_SRC_PCS].led_hw_mode = LED_HW_GPIO;
    _gLedInfo[LED_SRC_PCS].led_status = LED_STATUS_IDLE;
    _gLedInfo[LED_SRC_PCS].led_status_period = 1000;//I???
    _gLedInfo[LED_SRC_PCS].led_active_level = LED_HW_ACTIVE_LOW;
    _gLedInfo[LED_SRC_PCS].p_gpio_Handle = &LedPcsHandle;

    _gLedInfo[LED_SRC_CONN].led_hw_mode = LED_HW_GPIO;
    _gLedInfo[LED_SRC_CONN].led_status = LED_STATUS_IDLE;
    _gLedInfo[LED_SRC_CONN].led_status_period = 1000;//I???
    _gLedInfo[LED_SRC_CONN].led_active_level = LED_HW_ACTIVE_LOW;
    _gLedInfo[LED_SRC_CONN].p_gpio_Handle = &LedConnHandle;

    LedBspInit();

    _LedOFF(LED_SRC_SYS);
    _LedOFF(LED_SRC_PCS);
    _LedOFF(LED_SRC_CONN);
}


void DevLedProcess()
{
	if (gDevLedProcessFlag > 0)
	{
		static uint8_t flag = 0;
		static uint32_t led_time[2] = {0};
		static uint32_t led_step = 0;
		led_time[0] = tim_get_systime();
		switch (gDevLedProcessFlag)
		{
		case 1:
			if (led_time[0] - led_time[1] > 100)
			{
				led_time[1] = led_time[0];
				if ((++flag)%2 == 1)
				{
					_LedON(LED_SRC_SYS);
					_LedON(LED_SRC_PCS);
					_LedON(LED_SRC_CONN);
				}
				else
				{
					_LedOFF(LED_SRC_SYS);
					_LedOFF(LED_SRC_PCS);
					_LedOFF(LED_SRC_CONN);
				}
			}
			break;
		case 2:
			if (led_time[0] - led_time[1] > 900)
			{
				led_time[1] = led_time[0];
				if ((++flag)%2 == 1)
				{
					_LedON(LED_SRC_SYS);
					_LedON(LED_SRC_PCS);
					_LedON(LED_SRC_CONN);
				}
				else
				{
					_LedOFF(LED_SRC_SYS);
					_LedOFF(LED_SRC_PCS);
					_LedOFF(LED_SRC_CONN);
				}
			}
			break;
		case 3:
			if (led_time[0] - led_time[1] > 500)
			{
				led_time[1] = led_time[0];
				if ((++flag)%2 == 1)
				{
					_LedON(LED_SRC_SYS);
					_LedON(LED_SRC_PCS);
					_LedON(LED_SRC_CONN);
				}
				else
				{
					_LedOFF(LED_SRC_SYS);
					_LedOFF(LED_SRC_PCS);
					_LedOFF(LED_SRC_CONN);
				}
			}
			break;
		case 4:
			if (led_time[0] - led_time[1] > 300)
			{
				led_time[1] = led_time[0];
				if ((++flag)%2 == 1)
				{
					_LedON(LED_SRC_SYS);
					_LedON(LED_SRC_PCS);
					_LedON(LED_SRC_CONN);
				}
				else
				{
					_LedOFF(LED_SRC_SYS);
					_LedOFF(LED_SRC_PCS);
					_LedOFF(LED_SRC_CONN);
				}
			}
			break;
		case 5:
			_LedOFF(LED_SRC_SYS);
			_LedOFF(LED_SRC_PCS);
			if (led_time[0] - led_time[1] > 100)
			{
				_LedON(LED_SRC_SYS);
				_LedON(LED_SRC_PCS);
			}
			if (led_time[0] - led_time[1] > 500)
			{
				led_time[1] = led_time[0];
				++led_step;
				if ((++flag)%2 == 1)
				{
					_LedON(LED_SRC_SYS);
				}
				else
				{
					_LedOFF(LED_SRC_SYS);
				}
			}
			break;
		case 6:
			_LedOFF(LED_SRC_PCS);
			if (led_time[0] - led_time[1] > 100)
			{
				_LedON(LED_SRC_PCS);
			}
			if (led_time[0] - led_time[1] > 500)
			{
				led_time[1] = led_time[0];
				++led_step;
				if ((++flag)%2 == 1)
				{
					_LedON(LED_SRC_SYS);
					_LedON(LED_SRC_CONN);
				}
				else
				{
					_LedOFF(LED_SRC_SYS);
					_LedOFF(LED_SRC_CONN);
				}

			}
			break;
		};

		return;
	}
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

