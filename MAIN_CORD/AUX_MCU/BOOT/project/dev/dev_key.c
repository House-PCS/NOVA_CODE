/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_key.c
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
#include "dev_key.h"

#define KEY_POLLING_PERIOD              20

#define KEY_FILTER_TIME                 20
#define KEY_PRESS_MIDDLE_TIME           300
#define KEY_PRESS_CONTINUOUS_PRE_TIME   1000
#define KEY_PRESS_CONTINUOUS_TIME       100
#define KEY_PRESS_LONG_TIME             2000
#define KEY_PRESS_VERY_LONG_TIME        3000
#define KEY_COMBINATION_PRESS_LONG_TIME 2000

#define KEY_PRESS_5S_TIME               5000
#define KEY_PRESS_10S_TIME              10000


static KeyInfo_t _gKeyInfo[EN_KEY_SRC_NUM];
static Gpio_HandleType *_pHandle[EN_KEY_SRC_NUM];

Queue_t gKeyMsgQueue;


static bool _KeyPress(EN_KEY_SRC src)
{
    if(_gKeyInfo[src].key_hw_mode == EN_KEY_HW_MODE_GPIO)
    {
        uint8_t value = GpioBitGet(_pHandle[src]);
        if(_gKeyInfo[src].key_press_volt == EN_KEY_PRESS_VOLTAGE_LOW)
        {
            return (bool)(!value);
        }
        else
        {
            return (bool)(value);
        }
    }
    else
    {
        /*后续增加ADC按键*/
        return false;
    }
}


/**
  * @brief   按键外设初始化
  * @param   None
  * @return  None
  * @note    None
  */
void DevKeyInit()
{
    _gKeyInfo[0].key_src = EN_KEY_SRC_LEFT;
    _gKeyInfo[0].key_status = EN_KEY_STATUS_IDLE;
    _gKeyInfo[0].key_press_volt = EN_KEY_PRESS_VOLTAGE_HIGH;
    _gKeyInfo[0].key_hw_mode = EN_KEY_HW_MODE_GPIO;
//    _pHandle[0] = &KeyLeftHandle;

    _gKeyInfo[1].key_src = EN_KEY_SRC_RIGHT;
    _gKeyInfo[1].key_status = EN_KEY_STATUS_IDLE;
    _gKeyInfo[1].key_press_volt = EN_KEY_PRESS_VOLTAGE_HIGH;
    _gKeyInfo[1].key_hw_mode = EN_KEY_HW_MODE_GPIO;
//    _pHandle[1] = &KeyRightHandle;

    _gKeyInfo[2].key_src = EN_KEY_SRC_CONFIRM;
    _gKeyInfo[2].key_status = EN_KEY_STATUS_IDLE;
    _gKeyInfo[2].key_press_volt = EN_KEY_PRESS_VOLTAGE_HIGH;
    _gKeyInfo[2].key_hw_mode = EN_KEY_HW_MODE_GPIO;
//    _pHandle[2] = &KeyConfirmHandle;

    _gKeyInfo[3].key_src = EN_KEY_SRC_ESC;
    _gKeyInfo[3].key_status = EN_KEY_STATUS_IDLE;
    _gKeyInfo[3].key_press_volt = EN_KEY_PRESS_VOLTAGE_HIGH;
    _gKeyInfo[3].key_hw_mode = EN_KEY_HW_MODE_GPIO;
//    _pHandle[3] = &KeyEscHandle;



    QueueInit(&gKeyMsgQueue, KEY_EVENT_MSG_QUEUE_NUM, sizeof(KeyNewMsg_t));
    KeyBspInit();
}


/**
  * @brief   按键采集轮询处理
  * @param   None
  * @return  None
  * @note    None
  */
void DevKeyDetectProcess()
{
    static uint32_t timeout = 0;
    KeyNewMsg_t     msg;
    static EN_KEY_SRC  key_index = EN_KEY_SRC_NUM;
    static EN_KEY_HW_STATUS key_status = EN_KEY_STATUS_IDLE;
    static uint32_t press_down_num[EN_KEY_SRC_NUM];


    for(key_index = EN_KEY_SRC_LEFT; key_index < EN_KEY_SRC_NUM; key_index++)
    {
        switch(_gKeyInfo[key_index].key_status)
        {
            case EN_KEY_STATUS_IDLE:
            {
                if(_KeyPress(key_index) == true)
                {
                    _gKeyInfo[key_index].key_status = EN_KEY_STATUS_FILTER;
                    _gKeyInfo[key_index].key_filter_time_cnt = GetSystick();
                }
                break;
            }
            case EN_KEY_STATUS_FILTER:
            {
                if((uint32_t)(GetSystick() - _gKeyInfo[key_index].key_filter_time_cnt) > KEY_FILTER_TIME)
                {
                    if(_KeyPress(key_index) == true)
                    {
                        _gKeyInfo[key_index].key_status = EN_KEY_STATUS_PRESS_DOWN;
                    }
                    else
                    {
                        _gKeyInfo[key_index].key_status = EN_KEY_STATUS_IDLE;
                    }
                    _gKeyInfo[key_index].key_press_time_cnt  = GetSystick();
                }
                break;
            }
            case EN_KEY_STATUS_PRESS_DOWN:
            {
                /*发送一次按下*/
                press_down_num[key_index] = 0;
                msg.key_src = key_index;
                msg.press_value = EN_KEY_SRC_PRESSED_DOWN;
                msg.press_down_time = 0;
                QueuePut(&gKeyMsgQueue, &msg);
                press_down_num[key_index]++;
                _gKeyInfo[key_index].key_press_time_cnt  = GetSystick();
                _gKeyInfo[key_index].key_status = EN_KEY_STATUS_SUSTAIN;
                break;
            }
            case EN_KEY_STATUS_SUSTAIN:
            {
                /*每隔100ms发送一次*/
                if((uint32_t)(GetSystick() - _gKeyInfo[key_index].key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
                {
                    msg.key_src = key_index;
                    msg.press_value = EN_KEY_SRC_PRESSED_DOWN;
                    msg.press_down_time = KEY_PRESS_CONTINUOUS_TIME * press_down_num[key_index];
                    QueuePut(&gKeyMsgQueue, &msg);
                    press_down_num[key_index]++;
                    _gKeyInfo[key_index].key_press_time_cnt  = GetSystick();
                }
                if(_KeyPress(key_index) == false)
                {
                    _gKeyInfo[key_index].key_status = EN_KEY_STATUS_PRESS_UP;
                }
                break;
            }
            case EN_KEY_STATUS_PRESS_UP:
            {
                /*发送一次抬起*/
                msg.key_src = key_index;
                msg.press_value = EN_KEY_SRC_PRESSED_UP;
                msg.press_down_time = 0;
                QueuePut(&gKeyMsgQueue, &msg);
                _gKeyInfo[key_index].key_status = EN_KEY_STATUS_IDLE;
                break;
            }
            default:
                break;
        }
    }

}



/**
  * @brief   按键是否按下检测
  * @param   None
  * @return  None
  * @note    None
  */
bool DevKeyPressed(EN_KEY_SRC key_src)
{
    if(_gKeyInfo[key_src].key_status == EN_KEY_STATUS_IDLE || _gKeyInfo[key_src].key_status == EN_KEY_STATUS_FILTER)
    {
        return false;
    }
    else
    {
        return true;
    }
}




