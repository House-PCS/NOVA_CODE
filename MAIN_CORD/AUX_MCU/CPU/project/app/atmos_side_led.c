/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        bsp.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-03-07
  * @brief       N32G4FR
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-03-07    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
#include "atmos_side_led.h"
#include "systick.h"
//#include "dev_atmos_sk68xx.h"
#include "dev_atmos_sy_adbic.h"
#include "slave_ledctl.h"

static const DevSyAtmosCfg_t _SideAtmosLedCfgOff = 
{
    .effect_type = EN_SY_ATMOS_EFFECT_TYPE_OFF,
    .led_num = SIDE_LED_NUM,   //21个
    .color_change_num = 1,
    .data[0] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static void _SetSideLedOn(EN_SY_ATMOS_CHx ch, uint32_t color, uint32_t led_num)
{
    uint8_t i = 0;
    for(i = 0; i < led_num; i++)
    {
        garrDevSyAtmosCfg[ch].data[0][i] = color;
    }
    garrDevSyAtmosCfg[ch].effect_type = EN_SY_ATMOS_EFFECT_TYPE_ON;
    garrDevSyAtmosCfg[ch].led_num = led_num;
    garrDevSyAtmosCfg[ch].color_change_num = 1;
    garrDevSyAtmosCfg[ch].effect_brightness_duty = 100;
}

static void _SetSideLedBlinkOrBreath(EN_SY_ATMOS_CHx ch, EN_SY_ATMOS_EFFECT_TYPE effect_type, uint32_t color, uint16_t led_period, uint16_t led_status, uint32_t led_num)
{
    uint8_t i = 0;
    for(i = 0; i < led_num; i++)
    {
        garrDevSyAtmosCfg[ch].data[0][i] = color;
    }
    garrDevSyAtmosCfg[ch].effect_type = effect_type,
    garrDevSyAtmosCfg[ch].led_num = led_num;
    garrDevSyAtmosCfg[ch].color_change_num = 1;
    garrDevSyAtmosCfg[ch].effect_one_color_times = 1;
    garrDevSyAtmosCfg[ch].effect_master_control_mode = 1;
    garrDevSyAtmosCfg[ch].effect_period_status = led_status;
    garrDevSyAtmosCfg[ch].effect_period = led_period;
}

static void _SetSideSocShowLedOn(EN_SY_ATMOS_CHx ch, uint32_t color, uint32_t led_num)
{
    uint8_t i = 0;
    if(led_num > SIDE_LED_NUM)
    {
        led_num = SIDE_LED_NUM;
    }
    
    for(i = 0; i < led_num; i++)
    {
        garrDevSyAtmosCfg[ch].data[0][SIDE_LED_NUM - i] = color;
    }
    
    for(i = led_num; i < SIDE_LED_NUM; i++)
    {
        garrDevSyAtmosCfg[ch].data[0][SIDE_LED_NUM - led_num] = 0;
    }
    
    
    garrDevSyAtmosCfg[ch].effect_type = EN_SY_ATMOS_EFFECT_TYPE_ON;
    garrDevSyAtmosCfg[ch].led_num = SIDE_LED_NUM;
    garrDevSyAtmosCfg[ch].color_change_num = 1;
    garrDevSyAtmosCfg[ch].effect_brightness_duty = 100;
}


/**
  * @brief   氛围灯dev初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevSyAdbicAtmosInit(void)
{
    uint8_t i = 0;
    //初始化氛围灯bsp
    AtmosLedBspInit();
    
    /*上电初始化，默认所有灯都不亮*/
    for(i = 0; i < 3; i++)
    {
        garrDevSyAtmosCfg[i].effect_type = EN_SY_ATMOS_EFFECT_TYPE_OFF;
        garrDevSyAtmosCfg[i].led_num = FRONT_ATMOS_NUM;
        garrDevSyAtmosCfg[i].color_index = 0;
        garrDevSyAtmosCfg[i].color_change_num = 1;//默认只有一种颜色
        garrDevSyAtmosCfg[i].effect_one_color_times = 1;//默认每种颜色执行一次
    }

    garrDevSyAtmosCfg[EN_SY_ATMOS_KEY].effect_type = EN_SY_ATMOS_EFFECT_TYPE_OFF;
    garrDevSyAtmosCfg[EN_SY_ATMOS_KEY].led_num = KEY_ATMOS_NUM;
    garrDevSyAtmosCfg[EN_SY_ATMOS_KEY].color_index = 0;
    garrDevSyAtmosCfg[EN_SY_ATMOS_KEY].color_change_num = 1;//默认只有一种颜色
    garrDevSyAtmosCfg[EN_SY_ATMOS_KEY].effect_one_color_times = 1;//默认每种颜色执行一次

    garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT].effect_type = EN_SY_ATMOS_EFFECT_TYPE_OFF;
    garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT].led_num = SIDE_LED_NUM;
    garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT].color_index = 0;
    garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT].color_change_num = 1;//默认只有一种颜色
    garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT].effect_one_color_times = 1;//默认每种颜色执行一次

    garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT].effect_type = EN_SY_ATMOS_EFFECT_TYPE_OFF;
    garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT].led_num = SIDE_LED_NUM;
    garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT].color_index = 0;
    garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT].color_change_num = 1;//默认只有一种颜色
    garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT].effect_one_color_times = 1;//默认每种颜色执行一次
}



void DevSideLedProcess(void)
{
    uint8_t  i = 0;
    uint8_t  led_status = gstMainCopParaData.ctrl_info.led_dir_r & 0xff;
    uint16_t led_period = gstMainCopParaData.ctrl_info.led_period;
    uint16_t led_mode = gstMainCopParaData.ctrl_info.led_mode;
   
    uint8_t r_value = (gstMainCopParaData.ctrl_info.led_dir_r) >> 8 & 0xff;
    uint8_t b_value = (gstMainCopParaData.ctrl_info.led_g_b >> 8 ) & 0xff;
    uint8_t g_value = gstMainCopParaData.ctrl_info.led_g_b & 0xff;
    
    uint32_t color_data = (b_value << 16) | (g_value << 8) | r_value;
    
    switch (led_mode)
    {
        case EN_ATMOS_SIDE_OFF:    //关闭
        {
            garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT]  = _SideAtmosLedCfgOff;
            garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT] = _SideAtmosLedCfgOff;
            break;
        }
        case EN_ATMOS_SIDE_ON:   //常亮
        {
            #if 0
            for(i = 0; i < SIDE_LED_NUM; i++)
            {
                garrDevAtmosCfg[EN_ATMOS_CH_5].data[0][i] = color_data;
                garrDevAtmosCfg[EN_ATMOS_CH_6].data[0][i] = color_data;
            }
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_type = EN_ATMOS_EFFECT_TYPE_ON;
            garrDevAtmosCfg[EN_ATMOS_CH_5].led_num = SIDE_LED_NUM;
            garrDevAtmosCfg[EN_ATMOS_CH_5].color_change_num = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_brightness_duty = 100;
            
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_type = EN_ATMOS_EFFECT_TYPE_ON;
            garrDevAtmosCfg[EN_ATMOS_CH_6].led_num = SIDE_LED_NUM;
            garrDevAtmosCfg[EN_ATMOS_CH_6].color_change_num = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_brightness_duty = 100;
            #else 
            _SetSideLedOn(EN_SY_ATMOS_LEFT,  color_data, SIDE_LED_NUM);
            _SetSideLedOn(EN_SY_ATMOS_RIGHT, color_data, SIDE_LED_NUM);
            #endif
            break;
        }
        case EN_ATMOS_SIDE_BLINK:   //闪烁
        {
            #if 0
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
            garrDevAtmosCfg[EN_ATMOS_CH_5].led_num = SIDE_LED_NUM;
            garrDevAtmosCfg[EN_ATMOS_CH_5].color_change_num = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_one_color_times = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_master_control_mode = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_period_status = led_status;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_period = led_period;
            
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_type = EN_ATMOS_EFFECT_TYPE_BLINK,
            garrDevAtmosCfg[EN_ATMOS_CH_6].led_num = SIDE_LED_NUM;
            garrDevAtmosCfg[EN_ATMOS_CH_6].color_change_num = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_one_color_times = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_master_control_mode = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_period_status = led_status;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_period = led_period;
            
            for(i = 0; i < SIDE_LED_NUM; i++)
            {
                garrDevAtmosCfg[EN_ATMOS_CH_5].data[0][i] = color_data;
                garrDevAtmosCfg[EN_ATMOS_CH_6].data[0][i] = color_data;
            }
            #else
            _SetSideLedBlinkOrBreath(EN_SY_ATMOS_LEFT,  EN_SY_ATMOS_EFFECT_TYPE_BLINK, color_data, led_period, led_status, SIDE_LED_NUM);
            _SetSideLedBlinkOrBreath(EN_SY_ATMOS_RIGHT, EN_SY_ATMOS_EFFECT_TYPE_BLINK, color_data, led_period, led_status, SIDE_LED_NUM);
            #endif
            break;
        }
        case EN_ATMOS_SIDE_BREATH:  //呼吸
        {
            #if 0
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_type = EN_ATMOS_EFFECT_TYPE_BREATH;
            garrDevAtmosCfg[EN_ATMOS_CH_5].led_num = SIDE_LED_NUM;
            garrDevAtmosCfg[EN_ATMOS_CH_5].color_change_num = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_one_color_times = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_master_control_mode = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_period_status = led_status;
            garrDevAtmosCfg[EN_ATMOS_CH_5].effect_period = led_period;
            
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_type = EN_ATMOS_EFFECT_TYPE_BREATH;
            garrDevAtmosCfg[EN_ATMOS_CH_6].led_num = SIDE_LED_NUM;
            garrDevAtmosCfg[EN_ATMOS_CH_6].color_change_num = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_one_color_times = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_master_control_mode = 1;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_period_status = led_status;
            garrDevAtmosCfg[EN_ATMOS_CH_6].effect_period = led_period;
            
            for(i = 0; i < SIDE_LED_NUM; i++)
            {
                garrDevAtmosCfg[EN_ATMOS_CH_5].data[0][i] = color_data;
                garrDevAtmosCfg[EN_ATMOS_CH_6].data[0][i] = color_data;
            }
            #else
            _SetSideLedBlinkOrBreath(EN_SY_ATMOS_LEFT,  EN_SY_ATMOS_EFFECT_TYPE_BREATH, color_data, led_period, led_status, SIDE_LED_NUM);
            _SetSideLedBlinkOrBreath(EN_SY_ATMOS_RIGHT, EN_SY_ATMOS_EFFECT_TYPE_BREATH, color_data, led_period, led_status, SIDE_LED_NUM);
            #endif
            break;
        }
        case EN_ATMOS_SIDE_SOC:
        {
            _SetSideLedOn(EN_SY_ATMOS_LEFT,  color_data, SIDE_LED_NUM);
            _SetSideLedOn(EN_SY_ATMOS_RIGHT, color_data, SIDE_LED_NUM);
            break;
        }
        default:
        {
            garrDevSyAtmosCfg[EN_SY_ATMOS_LEFT]  = _SideAtmosLedCfgOff;
            garrDevSyAtmosCfg[EN_SY_ATMOS_RIGHT] = _SideAtmosLedCfgOff;
            break;
        }
    }
}

