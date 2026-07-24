#include "dev_led.h"




static LedInfo_t   _gstLedInfo[LED_SRC_NUM];


static void _LedOFF(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitReset(_gstLedInfo[index].p_gpio_Handle);
        }
        else
        {
            GpioBitSet(_gstLedInfo[index].p_gpio_Handle);
        }
    }
    else if(_gstLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
        }
        else
        {
            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
        }
        _gstLedInfo[index].p_timer_Handle->current_chl = _gstLedInfo[index].tim_ch;
        TimerPWMDutySet(_gstLedInfo[index].p_timer_Handle, 100);
    }
}

static void _LedON(EN_LED_SRC index)
{
    if(_gstLedInfo[index].led_hw_mode == LED_HW_GPIO)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            GpioBitSet(_gstLedInfo[index].p_gpio_Handle);
        }
        else
        {
            GpioBitReset(_gstLedInfo[index].p_gpio_Handle);
        }
    }
    else if(_gstLedInfo[index].led_hw_mode == LED_HW_PWM)
    {
        if(_gstLedInfo[index].led_active_level == LED_HW_ACTIVE_HIGH)
        {
            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 100;
            
        }
        else
        {
            _gstLedInfo[index].p_timer_Handle->oc_pwm_duty = 0;
        }
        _gstLedInfo[index].p_timer_Handle->current_chl = _gstLedInfo[index].tim_ch;
        TimerPWMDutySet(_gstLedInfo[index].p_timer_Handle, 100);
    }
}


static void _LedBlink(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = _gstLedInfo[index].led_status_period;
    
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

static void _LedStandby(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];
    static uint32_t timecount[LED_SRC_NUM];
    uint32_t period = 2000;
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index] = 0;
        timecount[index] = tim_get_systime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if(tim_difftime_from_now(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = tim_get_systime();
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
        timecount[index] = tim_get_systime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= 400)
    {
        if(led_flag[index] == 2)
        {
            _LedON(index);
            led_flag[index] = 3;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 500)
    {
        if(led_flag[index] == 3)
        {
            _LedOFF(index);
            led_flag[index] = 4;
        }
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= 700)
    {
        if(led_flag[index] == 4)
        {
            _LedON(index);
            led_flag[index] = 5;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 800)
    {
        if(led_flag[index] == 5)
        {
            _LedOFF(index);
            led_flag[index] = 6;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 1000)
    {
        if(led_flag[index] == 6)
        {
            _LedON(index);
            led_flag[index] = 7;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 1100)
    {
        if(led_flag[index] == 7)
        {
            _LedOFF(index);
            led_flag[index] = 8;
        }
    }
    
    if(tim_difftime_from_now(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = tim_get_systime();
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
        timecount[index] = tim_get_systime();
        _LedOFF(index);
        return;
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= 100)
    {
        if(led_flag[index] == 0)
        {
            _LedON(index);
            led_flag[index] = 1;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 200)
    {
        if(led_flag[index] == 1)
        {
            _LedOFF(index);
            led_flag[index] = 2;
        }
    }
    
    if((uint32_t)(tim_get_systime() - timecount[index]) >= 400)
    {
        if(led_flag[index] == 2)
        {
            _LedON(index);
            led_flag[index] = 3;
        }
    }

    if((uint32_t)(tim_get_systime() - timecount[index]) >= 500)
    {
        if(led_flag[index] == 3)
        {
            _LedOFF(index);
            led_flag[index] = 4;
        }
    }
    
    
    if(tim_difftime_from_now(timecount[index]) > period)
    {
        led_flag[index] = 0;
        timecount[index] = tim_get_systime();
    }
}

/**
 * @brief  : LED四次闪烁模式实现
 * @para   : index : LED类型
             first_flag : 是否是第一次进入
 * @note   : None
 * @return : None
 */
static void _LedBlinkFour(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];          //led状态标志
    static uint32_t period_time[LED_SRC_NUM];       //整体周期时间计时
    static uint8_t  blink_count[LED_SRC_NUM];       //闪烁次数
    static uint32_t  blink_period[LED_SRC_NUM];      //闪烁周期
    static uint32_t blink_time[LED_SRC_NUM];        //闪烁时间计时
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index]     = 0;
        blink_count[index]  = 0;
        period_time[index]  = tim_get_systime();
        blink_time[index]   = tim_get_systime();
        blink_period[index] = _gstLedInfo[index].led_status_period / 8; //闪烁周期（闪烁总时长控制在整体周期的一半）
        _LedOFF(index);
        return;
    }
    //闪烁四次
    if(blink_count[index] < 4)
    {
        //亮
        if((tim_difftime_from_now(blink_time[index]) >= (blink_period[index]) >> 1))
        {
            if(led_flag[index] == 0)
            {
                _LedON(index);
                led_flag[index] = 1;
            }
        }
        //暗
        if((tim_difftime_from_now(blink_time[index]) >= blink_period[index]))
        {
            blink_time[index]   = tim_get_systime();
            if(led_flag[index] == 1)
            {
                _LedOFF(index);
                led_flag[index] = 0;
                blink_count[index]++;
            }
        }
    }
    //剩余时间均为暗，等待时间更新
    if(tim_difftime_from_now(period_time[index]) > _gstLedInfo[index].led_status_period)
    {
        led_flag[index]     = 0;
        blink_count[index]  = 0;
        period_time[index]  = tim_get_systime();
        blink_time[index]   = tim_get_systime();
    }
}

/**
 * @brief  : LED工装模式实现
 * @para   : index : LED类型
             first_flag : 是否是第一次进入
 * @note   : None
 * @return : None
 */
static void _LedTool(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];          //led状态标志
    static uint32_t period_time[LED_SRC_NUM];       //整体周期时间计时
    static uint8_t  blink_count[LED_SRC_NUM];       //闪烁次数
    static uint32_t  blink_period[LED_SRC_NUM];      //闪烁周期
    static uint32_t blink_time[LED_SRC_NUM];        //闪烁时间计时
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index]     = 0;
        blink_count[index]  = 0;
        period_time[index]  = tim_get_systime();
        blink_time[index]   = tim_get_systime();
        blink_period[index] = _gstLedInfo[index].led_status_period / 6; //闪烁周期（闪烁总时长控制在整体周期的一半）
        _LedOFF(index);
        return;
    }
    //闪烁三次
    if(blink_count[index] < 3)
    {
        //亮
        if((tim_difftime_from_now(blink_time[index]) >= (blink_period[index]) >> 1))
        {
            if(led_flag[index] == 0)
            {
                _LedON(index);
                led_flag[index] = 1;
            }
        }
        //暗
        if((tim_difftime_from_now(blink_time[index]) >= blink_period[index]))
        {
            blink_time[index]   = tim_get_systime();
            if(led_flag[index] == 1)
            {
                _LedOFF(index);
                led_flag[index] = 0;
                blink_count[index]++;
            }
        }
    }
    //剩余时间均为暗，等待时间更新
    if(tim_difftime_from_now(period_time[index]) > _gstLedInfo[index].led_status_period)
    {
        led_flag[index]     = 0;
        blink_count[index]  = 0;
        period_time[index]  = tim_get_systime();
        blink_time[index]   = tim_get_systime();
    }
}

/**
 * @brief  : LED老化模式实现
 * @para   : index : LED类型
             first_flag : 是否是第一次进入
 * @note   : None
 * @return : None
 */
static void _LedAging(EN_LED_SRC index, uint8_t first_flag)
{
    static uint32_t led_flag[LED_SRC_NUM];          //led状态标志
    static uint32_t period_time[LED_SRC_NUM];       //整体周期时间计时
    static uint8_t  blink_count[LED_SRC_NUM];       //闪烁次数
    static uint32_t  blink_period[LED_SRC_NUM];      //闪烁周期
    static uint32_t blink_time[LED_SRC_NUM];        //闪烁时间计时
    
    if(first_flag == 1)/*计数初始化*/
    {
        led_flag[index]     = 0;
        blink_count[index]  = 0;
        period_time[index]  = tim_get_systime();
        blink_time[index]   = tim_get_systime();
        blink_period[index] = _gstLedInfo[index].led_status_period / 4; //闪烁周期（闪烁总时长控制在整体周期的三分之一）
        _LedOFF(index);
        return;
    }
    //闪烁2次
    if(blink_count[index] < 2)
    {
        //亮
        if((tim_difftime_from_now(blink_time[index]) >= (blink_period[index]) >> 1))
        {
            if(led_flag[index] == 0)
            {
                _LedON(index);
                led_flag[index] = 1;
            }
        }
        //暗
        if((tim_difftime_from_now(blink_time[index]) >= blink_period[index]))
        {
            blink_time[index]   = tim_get_systime();
            if(led_flag[index] == 1)
            {
                _LedOFF(index);
                led_flag[index] = 0;
                blink_count[index]++;
            }
        }
    }
    //剩余时间均为暗，等待时间更新
    if(tim_difftime_from_now(period_time[index]) > _gstLedInfo[index].led_status_period)
    {
        led_flag[index]     = 0;
        blink_count[index]  = 0;
        period_time[index]  = tim_get_systime();
        blink_time[index]   = tim_get_systime();
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

/**
 * @brief  : 闪烁设置
 * @para   : src : LED类型
 * @note   : None
 * @return : None
 */
void LedBlinkSet(EN_LED_SRC src, uint8_t blink_count ,uint16_t period)
{
    //设置闪烁模式
    switch(blink_count)
    {
        case 2:
        {
            _gstLedInfo[src].led_status = LED_STATUS_AGING;
            break;
        }
        case 3:
        {
            _gstLedInfo[src].led_status = LED_STATUS_TOOL;
            break;
        }
        case 4:
        {
            _gstLedInfo[src].led_status = LED_STATUS_BLINK_FOUR;
            break;
        }
        default:
        {
            _gstLedInfo[src].led_status = LED_STATUS_AGING;
            break;
        }
    }
    //设置周期
    _gstLedInfo[src].led_status_period = period;
}

/**
 * @brief  : LED工装模式设置
 * @para   : src : LED类型
 * @note   : None
 * @return : None
 */
void LedCtlTool(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_TOOL;
    _gstLedInfo[src].led_status_period = LED_BLINK_TIME_TOOL;
}

/**
 * @brief  : LED老化模式设置
 * @para   : src : LED类型
 * @note   : None
 * @return : None
 */
void LedCtlAging(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_AGING;
    _gstLedInfo[src].led_status_period = LED_BLINK_TIME_AGING;
}


/**
 * @brief  : 耐久度测试故障LED设置
 * @para   : src : LED类型
 * @note   : None
 * @return : None
 */
void LedCtlEnduranceTest(EN_LED_SRC src)
{
    _gstLedInfo[src].led_status = LED_STATUS_AGING;
    _gstLedInfo[src].led_status_period = LED_BLINK_TIME_ENDURANCE_ERROR;
}
/**
 * @brief  : LED初始化
 * @para   : None
 * @note   : None
 * @return : None
 */
void DevLedInit(void)
{    
    _gstLedInfo[LED_SRC_SYS].led_hw_mode = LED_HW_GPIO;
    _gstLedInfo[LED_SRC_SYS].led_status = LED_STATUS_BLINK;
    _gstLedInfo[LED_SRC_SYS].led_status_period = LED_BLINK_TIME_NORMAL;//默认值
    _gstLedInfo[LED_SRC_SYS].led_active_level = LED_HW_ACTIVE_LOW;
    _gstLedInfo[LED_SRC_SYS].p_gpio_Handle = &McuLedHandle;
    
    McuLedBspInit();
    RunLedEnInit();
    
    _LedOFF(LED_SRC_SYS);

    
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
            /* 老化LED模式 */
            case LED_STATUS_AGING:
            {
                if(pre_status[i] != LED_STATUS_AGING)
                {
                    _LedAging((EN_LED_SRC)i, 1);
                    pre_status[i] = LED_STATUS_AGING;
                }
                else
                {
                    _LedAging((EN_LED_SRC)i, 0);
                }
                break;
            } 
            /* 工装LED模式 */
            case LED_STATUS_TOOL:
            {
                if(pre_status[i] != LED_STATUS_TOOL)
                {
                    _LedTool((EN_LED_SRC)i, 1);
                    pre_status[i] = LED_STATUS_TOOL;
                }
                else
                {
                    _LedTool((EN_LED_SRC)i, 0);
                }
                break;
            }      
            /* 连续闪烁四次模式 */
            case LED_STATUS_BLINK_FOUR:
            {
                if(pre_status[i] != LED_STATUS_BLINK_FOUR)
                {
                    _LedBlinkFour((EN_LED_SRC)i, 1);
                    pre_status[i] = LED_STATUS_BLINK_FOUR;
                }
                else
                {
                    _LedBlinkFour((EN_LED_SRC)i, 0);
                }
                break;
            }    
            default:break;
        }
    }
}

