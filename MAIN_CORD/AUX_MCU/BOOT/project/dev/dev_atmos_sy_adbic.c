/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_atmos_sy_adbic.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-10-28
  * @brief       new氛围灯开发
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-10-28    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_atmos_sy_adbic.h"


#define OUPUTGAIN                        28

//DevAtmosCfg_t garrDevAtmosCfg[EN_ATMOS_CH_NUM];

DevSyAtmosCfg_t garrDevSyAtmosCfg[EN_SY_ATMOS_NUM];

/**
  * @brief   硬延时，适用精确到us级以下的延时，需要用示波器测试，不同MCU频率和型号，延时的输入值不同
  * @param   value:延时输入值
  * @return  None
  * @note    None
  */
static void _HardDelay(uint16_t value)
{

    volatile uint32_t delay_count;
    delay_count = value;

    while(delay_count--)
    {
        __NOP();
    }
}

static void _IOHighLevelToAtmos(EN_SY_ATMOS_CHx ch)
{
    GpioBitSet((&AtmosLedHandle[ch]));
    _HardDelay(3);
    GpioBitReset((&AtmosLedHandle[ch]));
}

static void _IOLowLevelToAtmos(EN_SY_ATMOS_CHx ch)
{
    GpioBitSet((&AtmosLedHandle[ch]));
    GpioBitReset((&AtmosLedHandle[ch]));
}



/**
  * @brief   将对应电平进行增益
  * @param   ch:通道索引
  * @return  None
  * @note    None
  */
static void _OutPutGain(EN_SY_ATMOS_CHx ch, uint16_t red, uint16_t green, uint16_t blue)
{
    uint8_t i = 0;
    _IOLowLevelToAtmos(ch);   //输出1bit的0码
    //red
    for(i = 0; i < 5; i++)
    {
        if((red & (1 << (4 - i))) != 0)
        {
            _IOHighLevelToAtmos(ch);
        }
        else
        {
            _IOLowLevelToAtmos(ch);
        }
    }
    //green
    for(i = 0; i < 5; i++)
    {
        if((green & (1 << (4 - i))) != 0)
        {
            _IOHighLevelToAtmos(ch);
        }
        else
        {
            _IOLowLevelToAtmos(ch);
        }
    }
    //blue
    for(i = 0; i < 5; i++)
    {
        if((blue & (1 << (4 - i))) != 0)
        {
            _IOHighLevelToAtmos(ch);
        }
        else
        {
            _IOLowLevelToAtmos(ch);
        }
    }
}


/**
  * @brief   氛围灯显示内容更新
  * @param   ch:通道索引
  * @param   level:要输出的电平，0为低电平，1为高电平
  * @return  None
  * @note    None
  */
static void _AtmosUpdate(EN_SY_ATMOS_CHx ch, uint32_t* p_data, uint8_t num)
{
    uint8_t i = 0;
    uint8_t j = 0;

    uint8_t data_red = 0;
    uint8_t data_green = 0;
    uint8_t data_blue = 0;

    uint16_t red_16 = 0;
    uint16_t green_16 = 0;
    uint16_t blue_16 = 0;
    uint32_t data_color = *p_data;

    data_blue = data_color >> 16;
    data_green = data_color >> 8;
    data_red = data_color;

    red_16 = data_red * (65535 / 255);
    green_16 = data_green * (65535 / 255);
    blue_16 = data_blue * (65535 / 255);

    long long last_data = 0;

    last_data = ((long long)(blue_16) << 32) | ((long long)(green_16) << 16) | (red_16);

    for(i = 0; i < num; i++)
    {
        for(j = 0; j < 48; j++)
        {
            if(((last_data >> (47 - j)) & 0x01) != 0)
            {
                _IOHighLevelToAtmos(ch);
            }
            else
            {
                _IOLowLevelToAtmos(ch);
            }
        }
    }
}
/**
  * @brief   氛围灯关闭
  * @param   ch：通道索引
  * @return  None
  * @note    None
  */
static void _AtmosLedOff(EN_SY_ATMOS_CHx ch)
{
    uint8_t i = 0;
    uint32_t data_temp = 0;
    for(i = 0; i < garrDevSyAtmosCfg[ch].led_num; i++)
    {
        _AtmosUpdate(ch, &data_temp, 1);
    }
    _OutPutGain(ch, OUPUTGAIN, OUPUTGAIN, OUPUTGAIN);
}
/**
  * @brief   氛围灯点亮
  * @param   ch：通道索引
  * @param   color_index:颜色索引
  * @return  None
  * @note    None
  */
static void _AtmosLedOn(EN_SY_ATMOS_CHx ch, uint8_t color_index)
{
    _AtmosUpdate(ch, garrDevSyAtmosCfg[ch].data[color_index], garrDevSyAtmosCfg[ch].led_num);
    _OutPutGain(ch, OUPUTGAIN, OUPUTGAIN, OUPUTGAIN);
}

/**
  * @brief   氛围灯渐变点亮，用于呼吸
  * @param   ch：通道索引
  * @param   color_index:颜色索引
  * @param   duty:呼吸占空比，100表示系数为1，原样显示数组的值
  * @return  None
  * @note    None
  */
static void _AtmosLedGradient(EN_SY_ATMOS_CHx ch, uint8_t color_index, float duty)
{
    uint8_t i = 0;
    uint8_t data_r, data_g, data_b;
    uint32_t data;
    for(i = 0; i < garrDevSyAtmosCfg[ch].led_num; i++)
    {
        data_b = (garrDevSyAtmosCfg[ch].data[color_index][i] >> 16) & 0xFF;
        data_g = (garrDevSyAtmosCfg[ch].data[color_index][i] >> 8) & 0xFF;
        data_r = (garrDevSyAtmosCfg[ch].data[color_index][i] >> 0) & 0xFF;

        data_b = data_b * duty / 100;
        data_g = data_g * duty / 100;
        data_r = data_r * duty / 100;


        data = (data_b << 16) | (data_g << 8) | data_r;
        _AtmosUpdate(ch, &data, 1);
    }
    _OutPutGain(ch, OUPUTGAIN, OUPUTGAIN, OUPUTGAIN);
}

/**
  * @brief   氛围灯轮询执行函数
  * @param   None
  * @return  None
  * @note    None
  */
void DevSyAtmosLedProcess(void)
{
    int32_t k = 0;
    static uint8_t ch_index = 0;
    static uint32_t ch_index_loop_flag = 0;
    static uint32_t start_breath_flag = 1;

    uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 1000 * EN_SY_ATMOS_NUM; //该轮询的执行周期，单位ms

    switch(garrDevSyAtmosCfg[ch_index].effect_type)
    {
        case EN_SY_ATMOS_EFFECT_TYPE_IDLE:
        {
            break;
        }
        case EN_SY_ATMOS_EFFECT_TYPE_OFF:
        {
            _AtmosLedOff((EN_SY_ATMOS_CHx)ch_index);
            garrDevSyAtmosCfg[ch_index].effect_type = EN_SY_ATMOS_EFFECT_TYPE_IDLE;
            break;
        }
        case EN_SY_ATMOS_EFFECT_TYPE_ON:
        {
            if(garrDevSyAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedOn((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index);
                garrDevSyAtmosCfg[ch_index].effect_type = EN_SY_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(garrDevSyAtmosCfg[ch_index].color_change_num > 1)
            {
                if(garrDevSyAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index);
                }
                garrDevSyAtmosCfg[ch_index].effect_time_count++;
                if(garrDevSyAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevSyAtmosCfg[ch_index].effect_period)
                {
                    garrDevSyAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevSyAtmosCfg[ch_index].color_index++;
                    if(garrDevSyAtmosCfg[ch_index].color_index >= garrDevSyAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevSyAtmosCfg[ch_index].color_index = 0;
                        if(garrDevSyAtmosCfg[ch_index].effect_single_mode == true)//不循环，执行完停留在最后一帧
                        {
                            garrDevSyAtmosCfg[ch_index].effect_type = EN_SY_ATMOS_EFFECT_TYPE_IDLE;
                        }
                    }
                }
            }
            break;
        }
        case EN_SY_ATMOS_EFFECT_TYPE_BLINK:
        {
            if(garrDevSyAtmosCfg[ch_index].effect_master_control_mode == 1)
            {
                //新加的代码
                if(garrDevSyAtmosCfg[ch_index].effect_period_status == 0)
                {
                    _AtmosLedOff((EN_SY_ATMOS_CHx)ch_index);
                }
                else
                {
                    _AtmosLedOn((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index);
                }
            }
            else
            {
                if(garrDevSyAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index);
                }
                if(garrDevSyAtmosCfg[ch_index].effect_time_count == garrDevSyAtmosCfg[ch_index].effect_period / poll_period / 2)
                {
                    _AtmosLedOff((EN_SY_ATMOS_CHx)ch_index);
                }

                garrDevSyAtmosCfg[ch_index].effect_time_count++;
                if(garrDevSyAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevSyAtmosCfg[ch_index].effect_period)
                {
                    garrDevSyAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevSyAtmosCfg[ch_index].effect_one_color_times_count++;
                    if(garrDevSyAtmosCfg[ch_index].effect_one_color_times_count >= garrDevSyAtmosCfg[ch_index].effect_one_color_times)
                    {
                        garrDevSyAtmosCfg[ch_index].effect_one_color_times_count = 0;
                        garrDevSyAtmosCfg[ch_index].color_index++;
                        if(garrDevSyAtmosCfg[ch_index].color_index >= garrDevSyAtmosCfg[ch_index].color_change_num)
                        {
                            garrDevSyAtmosCfg[ch_index].color_index = 0;
                        }
                    }
                }
            }
            break;
        }
        case EN_SY_ATMOS_EFFECT_TYPE_BREATH:
        {
            if(garrDevSyAtmosCfg[ch_index].effect_master_control_mode == 1)
            {
                //新加的代码
                uint32_t time = garrDevSyAtmosCfg[ch_index].effect_period / poll_period;
                uint32_t middile_time = time / 2;

                if(garrDevSyAtmosCfg[ch_index].effect_period_status == 0)
                {
                    if(garrDevSyAtmosCfg[ch_index].effect_time_count == 0)
                    {
                        _AtmosLedGradient((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index, (float)100 *  0);
                    }
                    else
                    {
                        garrDevSyAtmosCfg[ch_index].effect_time_count--;
                    }
                    _AtmosLedGradient((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index, (float)100 *  garrDevSyAtmosCfg[ch_index].effect_time_count / middile_time);
                }
                else if(garrDevSyAtmosCfg[ch_index].effect_period_status == 1)
                {
                    garrDevSyAtmosCfg[ch_index].effect_time_count++;
                    if(garrDevSyAtmosCfg[ch_index].effect_time_count > middile_time)
                    {
                        garrDevSyAtmosCfg[ch_index].effect_time_count = middile_time;
                    }
                    _AtmosLedGradient((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index, (float)100 * garrDevSyAtmosCfg[ch_index].effect_time_count / middile_time);
                }
            }
            else
            {
                uint32_t time = garrDevSyAtmosCfg[ch_index].effect_period / poll_period;
                uint32_t middile_time = time / 2;
                if(garrDevSyAtmosCfg[ch_index].effect_time_count < middile_time)
                {
                    _AtmosLedGradient((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index, (float)100 * garrDevSyAtmosCfg[ch_index].effect_time_count / middile_time);
                }
                if(garrDevSyAtmosCfg[ch_index].effect_time_count >= middile_time)
                {
                    _AtmosLedGradient((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - garrDevSyAtmosCfg[ch_index].effect_time_count) / (time - 1 - middile_time));
                }
                garrDevSyAtmosCfg[ch_index].effect_time_count++;

                if(garrDevSyAtmosCfg[ch_index].effect_time_count >= time)
                {
                    garrDevSyAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevSyAtmosCfg[ch_index].effect_one_color_times_count++;
                    if(garrDevSyAtmosCfg[ch_index].effect_one_color_times_count >= garrDevSyAtmosCfg[ch_index].effect_one_color_times)
                    {
                        garrDevSyAtmosCfg[ch_index].effect_one_color_times_count = 0;
                        garrDevSyAtmosCfg[ch_index].color_index++;
                        if(garrDevSyAtmosCfg[ch_index].color_index >= garrDevSyAtmosCfg[ch_index].color_change_num)
                        {
                            garrDevSyAtmosCfg[ch_index].color_index = 0;
                        }
                    }
                }
            }

            break;
        }
        case EN_SY_ATMOS_EFFECT_TYPE_GRADIENT:
        {
            uint32_t time = garrDevSyAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t duty;

            if(garrDevSyAtmosCfg[ch_index].effect_gradient_end_brightness_duty >= garrDevSyAtmosCfg[ch_index].effect_gradient_start_brightness_duty)
            {
                duty = garrDevSyAtmosCfg[ch_index].effect_gradient_start_brightness_duty +
                       (garrDevSyAtmosCfg[ch_index].effect_gradient_end_brightness_duty - garrDevSyAtmosCfg[ch_index].effect_gradient_start_brightness_duty) *
                       garrDevSyAtmosCfg[ch_index].effect_time_count / (time - 1);
            }
            else
            {
                duty = garrDevSyAtmosCfg[ch_index].effect_gradient_start_brightness_duty -
                       (garrDevSyAtmosCfg[ch_index].effect_gradient_start_brightness_duty - garrDevSyAtmosCfg[ch_index].effect_gradient_end_brightness_duty) *
                       garrDevSyAtmosCfg[ch_index].effect_time_count / (time - 1);
            }

            duty = duty * garrDevSyAtmosCfg[ch_index].effect_brightness_duty / 100;
            _AtmosLedGradient((EN_SY_ATMOS_CHx)ch_index, garrDevSyAtmosCfg[ch_index].color_index, (float)duty);
            garrDevSyAtmosCfg[ch_index].effect_time_count++;
            if(garrDevSyAtmosCfg[ch_index].effect_time_count >= time)
            {
                garrDevSyAtmosCfg[ch_index].effect_time_count = 0;
                garrDevSyAtmosCfg[ch_index].effect_one_color_times_count++;
                if(garrDevSyAtmosCfg[ch_index].effect_one_color_times_count >= garrDevSyAtmosCfg[ch_index].effect_one_color_times)
                {
                    garrDevSyAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    garrDevSyAtmosCfg[ch_index].color_index++;
                    if(garrDevSyAtmosCfg[ch_index].color_index >= garrDevSyAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevSyAtmosCfg[ch_index].color_index = 0;
                        if(garrDevSyAtmosCfg[ch_index].effect_single_mode == true)//不循环，执行完停留在最后一帧
                        {
                            garrDevSyAtmosCfg[ch_index].effect_type = EN_SY_ATMOS_EFFECT_TYPE_IDLE;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    ch_index++;
    if(ch_index >= EN_SY_ATMOS_NUM)
    {
        ch_index = 0;
    }
}



