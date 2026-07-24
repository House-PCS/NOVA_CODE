/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_atmos_sk68xx.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-10-21
  * @brief       氛围灯开发
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-10-21    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_atmos_sk68xx.h"

DevAtmosCfg_t garrDevAtmosCfg[EN_ATMOS_CH_NUM];


/**
  * @brief   氛围灯通道对应的IO翻转
  * @param   ch:通道索引
  * @param   level:要输出的电平，0为低电平，1为高电平
  * @return  None
  * @note    None
  */
static void _AtmosChIOCtl(EN_ATMOS_CHx ch, uint8_t level)
{
    if(level == 0)
    {
        GpioBitReset((&AtmosLedHandle[ch]));
    }
    else
    {
        GpioBitSet((&AtmosLedHandle[ch]));
    }
}
/**
  * @brief   硬延时，适用精确到us级以下的延时，需要用示波器测试，不同MCU频率和型号，延时的输入值不同
  * @param   value:延时输入值
  * @return  None
  * @note    None
  */
static void _HardDelay(uint16_t value)
{
    while(value > 0)
    {
        value--;
    }
}

/**
  * @brief   将对应电平按照要求时序发送至氛围灯
  * @param   ch:通道索引
  * @param   level:要输出的电平，0为低电平，1为高电平
  * @return  None
  * @note    None
  */
static void _IOLevelToAtmos(EN_ATMOS_CHx ch, uint8_t level)
{
    if(level == 0)
    {
        _AtmosChIOCtl(ch, 1);
//        _HardDelay(1);//不同MCU频率或型号，对应值不同  6
        _AtmosChIOCtl(ch, 0);
        _HardDelay(3);//不同MCU频率或型号，对应值不同 26
    }
    else if(level == 1)
    {
        _AtmosChIOCtl(ch, 1);
        _HardDelay(8);//不同MCU频率或型号，对应值不同
        _AtmosChIOCtl(ch, 0);
        _HardDelay(1);//不同MCU频率或型号，对应值不同
    }
    else
    {
        return;
    }
}

/**
  * @brief   氛围灯dev初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevAtmosInit(void)
{
    uint8_t i = 0;
    //初始化氛围灯bsp
    AtmosLedBspInit();
    
    /*上电初始化，默认所有灯都不亮*/
    for(i = 0; i < 3; i++)
    {
        garrDevAtmosCfg[i].effect_type = EN_ATMOS_EFFECT_TYPE_OFF;
        garrDevAtmosCfg[i].led_num = ATMOS_CH_1_NUM;
        garrDevAtmosCfg[i].color_index = 0;
        garrDevAtmosCfg[i].color_change_num = 1;//默认只有一种颜色
        garrDevAtmosCfg[i].effect_one_color_times = 1;//默认每种颜色执行一次
    } 
    garrDevAtmosCfg[EN_ATMOS_CH_4].effect_type = EN_ATMOS_EFFECT_TYPE_OFF;
    garrDevAtmosCfg[EN_ATMOS_CH_4].led_num = ATMOS_CH_4_NUM;
    garrDevAtmosCfg[EN_ATMOS_CH_4].color_index = 0;
    garrDevAtmosCfg[EN_ATMOS_CH_4].color_change_num = 1;//默认只有一种颜色
    garrDevAtmosCfg[EN_ATMOS_CH_4].effect_one_color_times = 1;//默认每种颜色执行一次 
    
    garrDevAtmosCfg[EN_ATMOS_CH_5].effect_type = EN_ATMOS_EFFECT_TYPE_OFF;
    garrDevAtmosCfg[EN_ATMOS_CH_5].led_num = ATMOS_CH_5_NUM;
    garrDevAtmosCfg[EN_ATMOS_CH_5].color_index = 0;
    garrDevAtmosCfg[EN_ATMOS_CH_5].color_change_num = 1;//默认只有一种颜色
    garrDevAtmosCfg[EN_ATMOS_CH_5].effect_one_color_times = 1;//默认每种颜色执行一次 
    
    garrDevAtmosCfg[EN_ATMOS_CH_6].effect_type = EN_ATMOS_EFFECT_TYPE_OFF;
    garrDevAtmosCfg[EN_ATMOS_CH_6].led_num = ATMOS_CH_6_NUM;
    garrDevAtmosCfg[EN_ATMOS_CH_6].color_index = 0;
    garrDevAtmosCfg[EN_ATMOS_CH_6].color_change_num = 1;//默认只有一种颜色
    garrDevAtmosCfg[EN_ATMOS_CH_6].effect_one_color_times = 1;//默认每种颜色执行一次 
}

/**
  * @brief   氛围灯显示内容更新
  * @param   ch:通道索引
  * @param   level:要输出的电平，0为低电平，1为高电平
  * @return  None
  * @note    None
  */
static void _AtmosUpdate(EN_ATMOS_CHx ch, uint32_t* p_data, uint8_t num)
{
    uint8_t i = 0;
    uint8_t j = 0;
    for(i = 0; i < num; i++)
    {
        for(j = 0; j < 24; j++)
        {
            if((p_data[i] & (1 << (23 - j))) != 0)
            {
                _IOLevelToAtmos(ch, 1);
            }
            else
            {
                _IOLevelToAtmos(ch, 0);
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
static void _AtmosLedOff(EN_ATMOS_CHx ch)
{
    uint8_t i = 0;
    uint32_t data_temp = 0;
    for(i = 0; i < garrDevAtmosCfg[ch].led_num; i++)
    {
        _AtmosUpdate(ch, &data_temp, 1);
    }
}
/**
  * @brief   氛围灯点亮
  * @param   ch：通道索引
  * @param   color_index:颜色索引
  * @return  None
  * @note    None
  */
static void _AtmosLedOn(EN_ATMOS_CHx ch, uint8_t color_index)
{
    _AtmosUpdate(ch, garrDevAtmosCfg[ch].data[color_index], garrDevAtmosCfg[ch].led_num);
}

/**
  * @brief   氛围灯渐变点亮，用于呼吸
  * @param   ch：通道索引
  * @param   color_index:颜色索引
  * @param   duty:呼吸占空比，100表示系数为1，原样显示数组的值
  * @return  None
  * @note    None
  */
static void _AtmosLedGradient(EN_ATMOS_CHx ch, uint8_t color_index, float duty)
{
    uint8_t i = 0;
    uint8_t data_r, data_g, data_b;
    uint32_t data;
    for(i = 0; i < garrDevAtmosCfg[ch].led_num; i++)
    {
        data_g = (garrDevAtmosCfg[ch].data[color_index][i] >> 16) & 0xFF;
        data_r = (garrDevAtmosCfg[ch].data[color_index][i] >> 8) & 0xFF;
        data_b = (garrDevAtmosCfg[ch].data[color_index][i] >> 0) & 0xFF;

        data_g = data_g * duty / 100;
        data_r = data_r * duty / 100;
        data_b = data_b * duty / 100;

        data = (data_g << 16) | (data_r << 8) | data_b;
        _AtmosUpdate(ch, &data, 1);
    }
}

/**
  * @brief   氛围灯轮询执行函数
  * @param   None
  * @return  None
  * @note    None
  */
void DevAtmosLedProcess(void)
{
    int32_t k = 0;
    static uint8_t ch_index = 0;
    static uint32_t ch_index_loop_flag = 0;
    static uint32_t start_breath_flag = 1;

    uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 1000 * EN_ATMOS_CH_NUM; //该轮询的执行周期，单位ms

//    _AtmosLedOff((EN_ATMOS_CHx)3);

//    return;

    switch(garrDevAtmosCfg[ch_index].effect_type)
    {
        case EN_ATMOS_EFFECT_TYPE_IDLE:
        {
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_OFF:
        {
            _AtmosLedOff((EN_ATMOS_CHx)ch_index);
            garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_ON:
        {
            if(garrDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(garrDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(garrDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                }
                garrDevAtmosCfg[ch_index].effect_time_count++;
                if(garrDevAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevAtmosCfg[ch_index].effect_period)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevAtmosCfg[ch_index].color_index = 0;
                        if(garrDevAtmosCfg[ch_index].effect_single_mode == true)//不循环，执行完停留在最后一帧
                        {
                            garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
                        }
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_SINGLE_ON:
        {
            if(garrDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(garrDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(garrDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                }
                garrDevAtmosCfg[ch_index].effect_time_count++;
                if(garrDevAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevAtmosCfg[ch_index].effect_period)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevAtmosCfg[ch_index].color_index = garrDevAtmosCfg[ch_index].color_change_num - 1;
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_SWITCH_ON_RESTE:
        {
            ++ch_index_loop_flag;
            if (ch_index_loop_flag > 100)
            {
                ch_index_loop_flag = 0;
                for (k = 0; k < EN_ATMOS_CH_NUM - 3; ++k)
                {
                    garrDevAtmosCfg[k].color_index = 0;
                    garrDevAtmosCfg[k].effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON;
                }
            }
            return;
        }
        case EN_ATMOS_EFFECT_TYPE_SWITCH_ON:
        {
            if(garrDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(garrDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(garrDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                }
                garrDevAtmosCfg[ch_index].effect_time_count++;
                if(garrDevAtmosCfg[ch_index].color_index >= 2 && garrDevAtmosCfg[ch_index].color_index <=  4 )
                {
                    garrDevAtmosCfg[ch_index].effect_period = 150;
                }
                else
                {
                    garrDevAtmosCfg[ch_index].effect_period = 80;
                }
                if(garrDevAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevAtmosCfg[ch_index].effect_period)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        ch_index++;
                        if(ch_index >= EN_ATMOS_CH_NUM - 3)
                        {
                            //reset all light
                            ++ch_index_loop_flag;
                            ch_index = 0;
                            for (k = 0; k < EN_ATMOS_CH_NUM - 3; ++k)
                            {
                                garrDevAtmosCfg[k].color_index = 0;
                                _AtmosLedOff((EN_ATMOS_CHx)k);
                                garrDevAtmosCfg[k].effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON_RESTE;
                            }
                        }
                    }
                }
            }
            return;
        }
        case EN_ATMOS_EFFECT_TYPE_BLINK:
        {
            if(garrDevAtmosCfg[ch_index].effect_master_control_mode == 1)
            {//新加的代码
                if(garrDevAtmosCfg[ch_index].effect_period_status == 0)
                {
                    _AtmosLedOff((EN_ATMOS_CHx)ch_index);
                }
                else
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                }
            }
            else
            { 
                if(garrDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index);
                }
                if(garrDevAtmosCfg[ch_index].effect_time_count == garrDevAtmosCfg[ch_index].effect_period / poll_period / 2)
                {
                    _AtmosLedOff((EN_ATMOS_CHx)ch_index);
                }
                
                garrDevAtmosCfg[ch_index].effect_time_count++;
                if(garrDevAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevAtmosCfg[ch_index].effect_period)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevAtmosCfg[ch_index].effect_one_color_times_count++;
                    if(garrDevAtmosCfg[ch_index].effect_one_color_times_count >= garrDevAtmosCfg[ch_index].effect_one_color_times)
                    {
                        garrDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                        garrDevAtmosCfg[ch_index].color_index++;
                        if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                        {
                            garrDevAtmosCfg[ch_index].color_index = 0;
                        }
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_BREATH:
        {
            if(garrDevAtmosCfg[ch_index].effect_master_control_mode == 1)
            {//新加的代码
                uint32_t time = garrDevAtmosCfg[ch_index].effect_period / poll_period;
                uint32_t middile_time = time / 2;

                if(garrDevAtmosCfg[ch_index].effect_period_status == 0)
                {
                    if(garrDevAtmosCfg[ch_index].effect_time_count == 0)
                    {
                        _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 *  0);
                    }
                    else
                    {
                        garrDevAtmosCfg[ch_index].effect_time_count--;
                    }
                    _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 *  garrDevAtmosCfg[ch_index].effect_time_count / middile_time);
                }
                else if(garrDevAtmosCfg[ch_index].effect_period_status == 1)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count++;
                    if(garrDevAtmosCfg[ch_index].effect_time_count > middile_time)
                    {
                        garrDevAtmosCfg[ch_index].effect_time_count = middile_time;
                    } 
                    _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * garrDevAtmosCfg[ch_index].effect_time_count / middile_time);
                }
            }
            else
            {
                uint32_t time = garrDevAtmosCfg[ch_index].effect_period / poll_period;
                uint32_t middile_time = time / 2;
                if(garrDevAtmosCfg[ch_index].effect_time_count < middile_time)
                {
                    _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * garrDevAtmosCfg[ch_index].effect_time_count / middile_time);
                }
                if(garrDevAtmosCfg[ch_index].effect_time_count >= middile_time)
                {
                    _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - garrDevAtmosCfg[ch_index].effect_time_count) / (time - 1 - middile_time));
                }
                garrDevAtmosCfg[ch_index].effect_time_count++; 
                
                if(garrDevAtmosCfg[ch_index].effect_time_count >= time)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevAtmosCfg[ch_index].effect_one_color_times_count++;
                    if(garrDevAtmosCfg[ch_index].effect_one_color_times_count >= garrDevAtmosCfg[ch_index].effect_one_color_times)
                    {
                        garrDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                        garrDevAtmosCfg[ch_index].color_index++;
                        if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                        {
                            garrDevAtmosCfg[ch_index].color_index = 0;
                        }
                    }
                }
            }
           
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_START_BREATH:
        {
            uint32_t time = garrDevAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t middile_time = (time * 8) / 11;
            uint32_t once_time = (time * 1) / 11;
            if(garrDevAtmosCfg[ch_index].effect_time_count < middile_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * garrDevAtmosCfg[ch_index].effect_time_count / middile_time);
            }
            if(garrDevAtmosCfg[ch_index].effect_time_count >= middile_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - garrDevAtmosCfg[ch_index].effect_time_count) / (time - 1 - middile_time));
            }
            garrDevAtmosCfg[ch_index].effect_time_count++;
            if(garrDevAtmosCfg[ch_index].effect_time_count >= time - 11)
            {
                garrDevAtmosCfg[ch_index].effect_time_count = 355;
                garrDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(garrDevAtmosCfg[ch_index].effect_one_color_times_count >= garrDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    garrDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_THIRD_BREATH:
        {
            uint32_t time = garrDevAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t once_time = (1 * time) / 13;
            uint32_t middile_time = (4 * time) / 13;
            uint32_t third_time = (8 * time / 13);

            if(garrDevAtmosCfg[ch_index].effect_time_count < 39)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * garrDevAtmosCfg[ch_index].effect_time_count / third_time);
            }
            if(garrDevAtmosCfg[ch_index].effect_time_count >= 39 && garrDevAtmosCfg[ch_index].effect_time_count < third_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * garrDevAtmosCfg[ch_index].effect_time_count / third_time);
            }
            if(garrDevAtmosCfg[ch_index].effect_time_count >= third_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - garrDevAtmosCfg[ch_index].effect_time_count) / (middile_time + once_time));
            }
            garrDevAtmosCfg[ch_index].effect_time_count++;
            if(garrDevAtmosCfg[ch_index].effect_time_count >= time - 2)
            {
                garrDevAtmosCfg[ch_index].effect_time_count = 3;
                garrDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(garrDevAtmosCfg[ch_index].effect_one_color_times_count >= garrDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    garrDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_HALF_ON:
        {
            if(garrDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)garrDevAtmosCfg[ch_index].effect_brightness_duty);
                garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(garrDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(garrDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)garrDevAtmosCfg[ch_index].effect_brightness_duty);
                }
                garrDevAtmosCfg[ch_index].effect_time_count++;
                if(garrDevAtmosCfg[ch_index].effect_time_count * poll_period >= garrDevAtmosCfg[ch_index].effect_period)
                {
                    garrDevAtmosCfg[ch_index].effect_time_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_GRADIENT:
        {
            uint32_t time = garrDevAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t duty;

            if(garrDevAtmosCfg[ch_index].effect_gradient_end_brightness_duty >= garrDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty)
            {
                duty = garrDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty +
                       (garrDevAtmosCfg[ch_index].effect_gradient_end_brightness_duty - garrDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty) *
                       garrDevAtmosCfg[ch_index].effect_time_count / (time - 1);
            }
            else
            {
                duty = garrDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty -
                       (garrDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty - garrDevAtmosCfg[ch_index].effect_gradient_end_brightness_duty) *
                       garrDevAtmosCfg[ch_index].effect_time_count / (time - 1);
            }

            duty = duty * garrDevAtmosCfg[ch_index].effect_brightness_duty / 100;
            _AtmosLedGradient((EN_ATMOS_CHx)ch_index, garrDevAtmosCfg[ch_index].color_index, (float)duty);
            garrDevAtmosCfg[ch_index].effect_time_count++;
            if(garrDevAtmosCfg[ch_index].effect_time_count >= time)
            {
                garrDevAtmosCfg[ch_index].effect_time_count = 0;
                garrDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(garrDevAtmosCfg[ch_index].effect_one_color_times_count >= garrDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    garrDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    garrDevAtmosCfg[ch_index].color_index++;
                    if(garrDevAtmosCfg[ch_index].color_index >= garrDevAtmosCfg[ch_index].color_change_num)
                    {
                        garrDevAtmosCfg[ch_index].color_index = 0;
                        if(garrDevAtmosCfg[ch_index].effect_single_mode == true)//不循环，执行完停留在最后一帧
                        {
                            garrDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
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
    if(ch_index >= EN_ATMOS_CH_NUM)
    {
        ch_index = 0;
    }
}



