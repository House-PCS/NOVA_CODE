/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_atmos_sk68xx.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-01-25
  * @brief       氛围灯设备驱动
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-01-25    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "dev_atmos_sk68xx.h"

DevAtmosCfg_t    gDevAtmosCfg[EN_ATMOS_CH_NUM];

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
        GpioBitReset(&AtmosChxHandle[ch]);
    }
    else
    {
        GpioBitSet(&AtmosChxHandle[ch]);
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
static void _IOLevel2Atmos(EN_ATMOS_CHx ch, uint8_t level)
{
    if(level == 0)
    {
        _AtmosChIOCtl(ch, 1);
        _HardDelay(6);//不同MCU频率或型号，对应值不同
        _AtmosChIOCtl(ch, 0);
        _HardDelay(25);//不同MCU频率或型号，对应值不同
    }
    else if(level == 1)
    {
        _AtmosChIOCtl(ch, 1);
        _HardDelay(20);//不同MCU频率或型号，对应值不同
        _AtmosChIOCtl(ch, 0);
        _HardDelay(8);//不同MCU频率或型号，对应值不同
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
    AtmosBspInit();
    
    /*上电初始化，默认所有灯都不亮*/
    for(uint8_t i = 0; i < 3; i++)
    {
        gDevAtmosCfg[i].effect_type = EN_ATMOS_EFFECT_TYPE_OFF;
        gDevAtmosCfg[i].led_num = 7;
        gDevAtmosCfg[i].color_index = 0;
        gDevAtmosCfg[i].color_change_num = 1;//默认只有一种颜色
        gDevAtmosCfg[i].effect_one_color_times = 1;//默认每种颜色执行一次
    }
    
    gDevAtmosCfg[EN_ATMOS_CH_4].effect_type = EN_ATMOS_EFFECT_TYPE_OFF;
    gDevAtmosCfg[EN_ATMOS_CH_4].led_num = 4;
    gDevAtmosCfg[EN_ATMOS_CH_4].color_index = 0;
    gDevAtmosCfg[EN_ATMOS_CH_4].color_change_num = 1;//默认只有一种颜色
    gDevAtmosCfg[EN_ATMOS_CH_4].effect_one_color_times = 1;//默认每种颜色执行一次
}


/**
  * @brief   氛围灯显示内容更新
  * @param   ch:通道索引
  * @param   level:要输出的电平，0为低电平，1为高电平
  * @return  None
  * @note    None
  */
static void _AtmosUpdate(EN_ATMOS_CHx ch, uint32_t *p_data, uint8_t num)
{
    for(uint8_t i = 0; i < num; i++)
    {
        for(uint8_t j = 0; j < 24; j++)
        {
            if((p_data[i] & (1 << (23 - j))) != 0)
            {
                _IOLevel2Atmos(ch, 1);
            }
            else
            {
                _IOLevel2Atmos(ch, 0);
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
    uint32_t data_temp = 0x0;
    for(uint8_t i = 0; i < gDevAtmosCfg[ch].led_num; i++)
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
    _AtmosUpdate(ch, gDevAtmosCfg[ch].data[color_index], gDevAtmosCfg[ch].led_num);
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
    uint8_t data_r, data_g, data_b;
    uint32_t data;
    for(uint8_t i = 0; i < gDevAtmosCfg[ch].led_num; i++)
    {
        data_g = (gDevAtmosCfg[ch].data[color_index][i] >> 16) & 0xFF;
        data_r = (gDevAtmosCfg[ch].data[color_index][i] >> 8) & 0xFF;
        data_b = (gDevAtmosCfg[ch].data[color_index][i] >> 0) & 0xFF;
        
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
    static uint8_t ch_index = 0;
	static uint32_t ch_index_loop_flag = 0;
	static uint32_t start_breath_flag = 1;

    uint16_t poll_period = TimerPollingHandle.timer_cfg->period / 1000 * EN_ATMOS_CH_NUM; //该轮询的执行周期，单位ms
    switch(gDevAtmosCfg[ch_index].effect_type)
    {
        case EN_ATMOS_EFFECT_TYPE_IDLE:
        {
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_OFF:
        {
            _AtmosLedOff((EN_ATMOS_CHx)ch_index);
            gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            break;
        }
		case EN_ATMOS_EFFECT_TYPE_ON:
        {
            if(gDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
                gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(gDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(gDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
                }
                gDevAtmosCfg[ch_index].effect_time_count++;
                if(gDevAtmosCfg[ch_index].effect_time_count * poll_period >= gDevAtmosCfg[ch_index].effect_period)
                {
                    gDevAtmosCfg[ch_index].effect_time_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                        if(gDevAtmosCfg[ch_index].effect_single_mode == true)//不循环，执行完停留在最后一帧
                        {
                            gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
                        }
                    }
                }
            }
            break;
        }
		case EN_ATMOS_EFFECT_TYPE_SINGLE_ON:
        {
			if(gDevAtmosCfg[ch_index].color_change_num == 1)
			{
				_AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
				gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
			}
			else if(gDevAtmosCfg[ch_index].color_change_num > 1)
			{
				if(gDevAtmosCfg[ch_index].effect_time_count == 0)
				{
					_AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
				}
				gDevAtmosCfg[ch_index].effect_time_count++;
				if(gDevAtmosCfg[ch_index].effect_time_count * poll_period >= gDevAtmosCfg[ch_index].effect_period)
				{
					gDevAtmosCfg[ch_index].effect_time_count = 0;
					gDevAtmosCfg[ch_index].color_index++;
					if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
					{
						gDevAtmosCfg[ch_index].color_index = gDevAtmosCfg[ch_index].color_change_num -1;
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
				for (int32_t k = 0; k < EN_ATMOS_CH_NUM; ++k)
				{
					gDevAtmosCfg[k].color_index = 0;
					gDevAtmosCfg[k].effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON;
				}
			}
			return;
		}
        case EN_ATMOS_EFFECT_TYPE_SWITCH_ON:
        {
            if(gDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
                gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(gDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(gDevAtmosCfg[ch_index].effect_time_count == 0)
                {
                    _AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
                }
                gDevAtmosCfg[ch_index].effect_time_count++;
				if(gDevAtmosCfg[ch_index].color_index >=  2  && gDevAtmosCfg[ch_index].color_index <=  4 )
				{
					gDevAtmosCfg[ch_index].effect_period = 150;
				}
				else
				{
					gDevAtmosCfg[ch_index].effect_period = 80;
				}
                if(gDevAtmosCfg[ch_index].effect_time_count * poll_period >= gDevAtmosCfg[ch_index].effect_period)
                {
                    gDevAtmosCfg[ch_index].effect_time_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
						ch_index++;
						if(ch_index >= EN_ATMOS_CH_NUM)
						{
							//reset all light
							++ch_index_loop_flag;
							ch_index = 0;
							for (int32_t k = 0; k < EN_ATMOS_CH_NUM; ++k)
							{
								gDevAtmosCfg[k].color_index = 0;
								_AtmosLedOff((EN_ATMOS_CHx)k);
								gDevAtmosCfg[k].effect_type = EN_ATMOS_EFFECT_TYPE_SWITCH_ON_RESTE;
							}
						}
                    }
                }
            }
			return;
        }
        case EN_ATMOS_EFFECT_TYPE_BLINK:
        {
            if(gDevAtmosCfg[ch_index].effect_time_count == 0)
            {
                _AtmosLedOn((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index);
            }
            if(gDevAtmosCfg[ch_index].effect_time_count == gDevAtmosCfg[ch_index].effect_period / poll_period / 2)
            {
                _AtmosLedOff((EN_ATMOS_CHx)ch_index);
            }
            gDevAtmosCfg[ch_index].effect_time_count++;
			
            if(gDevAtmosCfg[ch_index].effect_time_count * poll_period >= gDevAtmosCfg[ch_index].effect_period)
            {
                gDevAtmosCfg[ch_index].effect_time_count = 0;
                gDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(gDevAtmosCfg[ch_index].effect_one_color_times_count >= gDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    gDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
            break;
        }
        case EN_ATMOS_EFFECT_TYPE_BREATH:
        {
            uint32_t time = gDevAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t middile_time = time / 2;
            if(gDevAtmosCfg[ch_index].effect_time_count < middile_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * gDevAtmosCfg[ch_index].effect_time_count / middile_time);
            }
            if(gDevAtmosCfg[ch_index].effect_time_count >= middile_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - gDevAtmosCfg[ch_index].effect_time_count) / (time - 1 - middile_time));
            }
			gDevAtmosCfg[ch_index].effect_time_count++;
            if(gDevAtmosCfg[ch_index].effect_time_count >= time)
            {
                gDevAtmosCfg[ch_index].effect_time_count = 0;
                gDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(gDevAtmosCfg[ch_index].effect_one_color_times_count >= gDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    gDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
            break;
        }
		case EN_ATMOS_EFFECT_TYPE_START_BREATH:
		{
            uint32_t time = gDevAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t middile_time = (time * 8) / 11;
			uint32_t once_time = (time * 1) / 11;
            if(gDevAtmosCfg[ch_index].effect_time_count < middile_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * gDevAtmosCfg[ch_index].effect_time_count / middile_time);
            }
            if(gDevAtmosCfg[ch_index].effect_time_count >= middile_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - gDevAtmosCfg[ch_index].effect_time_count) / (time - 1 - middile_time));
            }
			gDevAtmosCfg[ch_index].effect_time_count++;
            if(gDevAtmosCfg[ch_index].effect_time_count >= time - 11)
            {
                gDevAtmosCfg[ch_index].effect_time_count = 355;
                gDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(gDevAtmosCfg[ch_index].effect_one_color_times_count >= gDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    gDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
			break;
		}
		case EN_ATMOS_EFFECT_TYPE_THIRD_BREATH:
        {
            uint32_t time = gDevAtmosCfg[ch_index].effect_period / poll_period;
			uint32_t once_time = (1 * time) / 13;
			uint32_t middile_time = (4 * time) / 13;
			uint32_t third_time = (8 * time / 13); 
			
            if(gDevAtmosCfg[ch_index].effect_time_count < 39)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * gDevAtmosCfg[ch_index].effect_time_count / third_time); 	
            }
            if(gDevAtmosCfg[ch_index].effect_time_count >= 39 && gDevAtmosCfg[ch_index].effect_time_count < third_time)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * gDevAtmosCfg[ch_index].effect_time_count / third_time);
            }
			if(gDevAtmosCfg[ch_index].effect_time_count >= third_time)
			{
				_AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)100 * (time - 1 - gDevAtmosCfg[ch_index].effect_time_count) / (middile_time + once_time));
			}
            gDevAtmosCfg[ch_index].effect_time_count++;
            if(gDevAtmosCfg[ch_index].effect_time_count >= time - 2)
            {
                gDevAtmosCfg[ch_index].effect_time_count = 3;
                gDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(gDevAtmosCfg[ch_index].effect_one_color_times_count >= gDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    gDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
            break;
        }
		case EN_ATMOS_EFFECT_TYPE_HALF_ON:
		{
            if(gDevAtmosCfg[ch_index].color_change_num == 1)
            {
                _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)gDevAtmosCfg[ch_index].effect_brightness_duty);
                gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
            }
            else if(gDevAtmosCfg[ch_index].color_change_num > 1)
            {
                if(gDevAtmosCfg[ch_index].effect_time_count == 0)
                {
					_AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)gDevAtmosCfg[ch_index].effect_brightness_duty);
                }
                gDevAtmosCfg[ch_index].effect_time_count++;
                if(gDevAtmosCfg[ch_index].effect_time_count * poll_period >= gDevAtmosCfg[ch_index].effect_period)
                {
                    gDevAtmosCfg[ch_index].effect_time_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                    }
                }
            }
			break;
		}
        case EN_ATMOS_EFFECT_TYPE_GRADIENT:
        {
            uint32_t time = gDevAtmosCfg[ch_index].effect_period / poll_period;
            uint32_t duty;
            
            if(gDevAtmosCfg[ch_index].effect_gradient_end_brightness_duty >= gDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty)
            {
                duty = gDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty + 
                       (gDevAtmosCfg[ch_index].effect_gradient_end_brightness_duty - gDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty) * 
                       gDevAtmosCfg[ch_index].effect_time_count / (time - 1);
            }
            else
            {
                duty = gDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty - 
                       (gDevAtmosCfg[ch_index].effect_gradient_start_brightness_duty - gDevAtmosCfg[ch_index].effect_gradient_end_brightness_duty) * 
                       gDevAtmosCfg[ch_index].effect_time_count / (time - 1);
            }
            
            duty = duty * gDevAtmosCfg[ch_index].effect_brightness_duty / 100;
            _AtmosLedGradient((EN_ATMOS_CHx)ch_index, gDevAtmosCfg[ch_index].color_index, (float)duty);
            gDevAtmosCfg[ch_index].effect_time_count++;
            if(gDevAtmosCfg[ch_index].effect_time_count >= time)
            {
                gDevAtmosCfg[ch_index].effect_time_count = 0;
                gDevAtmosCfg[ch_index].effect_one_color_times_count++;
                if(gDevAtmosCfg[ch_index].effect_one_color_times_count >= gDevAtmosCfg[ch_index].effect_one_color_times)
                {
                    gDevAtmosCfg[ch_index].effect_one_color_times_count = 0;
                    gDevAtmosCfg[ch_index].color_index++;
                    if(gDevAtmosCfg[ch_index].color_index >= gDevAtmosCfg[ch_index].color_change_num)
                    {
                        gDevAtmosCfg[ch_index].color_index = 0;
                        if(gDevAtmosCfg[ch_index].effect_single_mode == true)//不循环，执行完停留在最后一帧
                        {
                            gDevAtmosCfg[ch_index].effect_type = EN_ATMOS_EFFECT_TYPE_IDLE;
                        }
                    }
                }
            }
            
            break;
        }
        default: break;
    }
    
    ch_index++;
    if(ch_index >= EN_ATMOS_CH_NUM)
    {
        ch_index = 0;
    }
    
    
}

