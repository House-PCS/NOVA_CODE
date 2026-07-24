/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        key_backlight_led.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-02-28
  * @brief       按键背光控制业务
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-02-28    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
 */
#include "key_backlight_led.h"

#define KEY_LED_THEME_COLOR                 0xE800CD   //0xBEB200
#define KEY_LED_STARTUP_TIME_1              9000
#define KEY_LED_STARTUP_TIME_2              3000
#define KEY_LED_TO_STANDBY_MODE_TIME        (5000 * 60)
#define KEY_LED_HIGH_LIGHT_HOLD_TIME        500

#define KEY_LED_BRIGHTNESS_HIGH             100
#define KEY_LED_BRIGHTNESS_NORMAL           40
#define KEY_LED_BRIGHTNESS_STANDBY          10



static EN_KEY_LED_STATUS _gKeyLedStatus = EN_KEY_LED_STATUS_START_UP_FIRST;
static uint8_t _gKeyLedRemap[] = {0, 1, 3, 2};//按键和灯的映射关系，数组值从左到右的顺序是EN_KEY_SRC_LEFT， EN_KEY_SRC_RIGHT， EN_KEY_SRC_CONFIRM， EN_KEY_SRC_ESC


static const DevAtmosCfg_t _KeyLedCfgStartUpFirstStep =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_GRADIENT,
    .led_num = 4,
    .color_change_num = 1,
    .data[0] = {KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR},
    .effect_brightness_duty = 100,
    .effect_period = 8000,
    .effect_single_mode = true,
    .effect_gradient_start_brightness_duty = 0,
    .effect_gradient_end_brightness_duty = 100,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _KeyLedCfgStartUpSecondStep =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_GRADIENT,
    .led_num = 4,
    .color_change_num = 1,
    .data[0] = {KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR},
    .effect_brightness_duty = 100,
    .effect_period = 3000,
    .effect_single_mode = true,
    .effect_gradient_start_brightness_duty = 100,
    .effect_gradient_end_brightness_duty = 40,
    .effect_one_color_times = 1,
};

static const DevAtmosCfg_t _KeyLedCfgNormal =
{
    .effect_type = EN_ATMOS_EFFECT_TYPE_ON,
    .led_num = 4,
    .color_change_num = 1,
    .data[0] = {KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR, KEY_LED_THEME_COLOR},
    .effect_brightness_duty = 40,
};

/**
  * @brief   颜色叠加亮度百分比转换
  * @param   origin_color:原始颜色
  * @param   duty:亮度百分比
  * @return  叠加亮度百分比后的颜色
  * @note    None
  */
static uint32_t _KeyLedColorInv(uint32_t origin_color, uint8_t duty)
{
    uint8_t data_r, data_g, data_b;

    data_g = (origin_color >> 16) & 0xFF;
    data_r = (origin_color >> 8) & 0xFF;
    data_b = (origin_color >> 0) & 0xFF;

    data_g = data_g * duty / 100;
    data_r = data_r * duty / 100;
    data_b = data_b * duty / 100;

    return (data_g << 16) | (data_r << 8) | data_b;
}



/**
  * @brief   按键灯进入唤醒模式
  * @param   None
  * @return  None
  * @note    None
  */
static void _KeyLedFuncWakeUpMode()
{
    *pKeyLedCfg = _KeyLedCfgNormal;
    for(uint8_t i = 0; i < EN_KEY_SRC_NUM; i++)
    {
        pKeyLedCfg->effect_type = EN_ATMOS_EFFECT_TYPE_ON;
        pKeyLedCfg->data[0][_gKeyLedRemap[i]] = _KeyLedColorInv(KEY_LED_THEME_COLOR, KEY_LED_BRIGHTNESS_NORMAL);
    }
}

/**
  * @brief   按键灯进入待机模式
  * @param   None
  * @return  None
  * @note    None
  */
static void _KeyLedFuncStandByMode()
{
    *pKeyLedCfg = _KeyLedCfgNormal;
    for(uint8_t i = 0; i < EN_KEY_SRC_NUM; i++)
    {
        pKeyLedCfg->effect_type = EN_ATMOS_EFFECT_TYPE_ON;
        pKeyLedCfg->data[0][_gKeyLedRemap[i]] = _KeyLedColorInv(KEY_LED_THEME_COLOR, KEY_LED_BRIGHTNESS_STANDBY);
    }
}



/**
  * @brief   按键灯轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
void KeyLedProcess(void)
{
    static uint32_t wait_time_count = 0;
    static uint32_t goto_standby_time_count = 0;
    static uint32_t goto_normal_time_count[EN_KEY_SRC_NUM];

    switch(_gKeyLedStatus)
    {
        case EN_KEY_LED_STATUS_START_UP_FIRST:
        {
            *pKeyLedCfg = _KeyLedCfgStartUpFirstStep;
            wait_time_count = GetSystick();
            _gKeyLedStatus = EN_KEY_LED_STATUS_START_UP_FIRST_WAIT;
            break;
        }
        case EN_KEY_LED_STATUS_START_UP_FIRST_WAIT:
        {
            if((uint32_t)(GetSystick() - wait_time_count) > KEY_LED_STARTUP_TIME_1)
            {
                _gKeyLedStatus = EN_KEY_LED_STATUS_START_UP_SECOND;
            }
            break;
        }
        case EN_KEY_LED_STATUS_START_UP_SECOND:
        {
            *pKeyLedCfg = _KeyLedCfgStartUpSecondStep;
            wait_time_count = GetSystick();
            _gKeyLedStatus = EN_KEY_LED_STATUS_START_UP_SECOND_WAIT;
            break;
        }
        case EN_KEY_LED_STATUS_START_UP_SECOND_WAIT:
        {
            if((uint32_t)(GetSystick() - wait_time_count) > KEY_LED_STARTUP_TIME_2)
            {
                _KeyLedFuncWakeUpMode();
                goto_standby_time_count = GetSystick();
                _gKeyLedStatus = EN_KEY_LED_STATUS_WAKE_UP;
            }
            break;
        }
        case EN_KEY_LED_STATUS_STAND_BY:
        {
            uint8_t i;
            for(i = 0; i < EN_KEY_SRC_NUM; i++)
            {
                if(DevKeyPressed((EN_KEY_SRC)i) == true)
                {
                    break;
                }
            }
            if(i < EN_KEY_SRC_NUM)//进入WakeUp模式
            {
                _KeyLedFuncWakeUpMode();
                goto_standby_time_count = GetSystick();
                _gKeyLedStatus = EN_KEY_LED_STATUS_WAKE_UP;
            }
            break;
        }
        case EN_KEY_LED_STATUS_WAKE_UP:
        {
            for(uint8_t i = 0; i < EN_KEY_SRC_NUM; i++)
            {
                if(DevKeyPressed((EN_KEY_SRC)i) == true)
                {
                    pKeyLedCfg->effect_type = EN_ATMOS_EFFECT_TYPE_ON;
                    pKeyLedCfg->data[0][_gKeyLedRemap[i]] = _KeyLedColorInv(KEY_LED_THEME_COLOR, KEY_LED_BRIGHTNESS_HIGH);
                    goto_normal_time_count[_gKeyLedRemap[i]] = GetSystick();
                    goto_standby_time_count = GetSystick();
                }
                else
                {
                    if((uint32_t)(GetSystick() - goto_normal_time_count[_gKeyLedRemap[i]]) >= KEY_LED_HIGH_LIGHT_HOLD_TIME)
                    {
                        pKeyLedCfg->effect_type = EN_ATMOS_EFFECT_TYPE_ON;
                        pKeyLedCfg->data[0][_gKeyLedRemap[i]] = _KeyLedColorInv(KEY_LED_THEME_COLOR, KEY_LED_BRIGHTNESS_NORMAL);
                    }
                }
            }
            if((uint32_t)(GetSystick() - goto_standby_time_count) >= KEY_LED_TO_STANDBY_MODE_TIME)//进入StandBy模式
            {
                _KeyLedFuncStandByMode();
                _gKeyLedStatus = EN_KEY_LED_STATUS_STAND_BY;
            }
            break;
        }
        default:
            break;
    }
}











