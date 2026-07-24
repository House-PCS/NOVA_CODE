/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        tm1681.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-1-4
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-1-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "dev_key.h"
#include "gpio.h"
#define LOG_TAG "dev_key"

#define KEY_POLLING_PERIOD              20

#define KEY_FILTER_TIME                 20
#define KEY_PRESS_MIDDLE_TIME           300
#define KEY_PRESS_CONTINUOUS_PRE_TIME   1000
#define KEY_PRESS_CONTINUOUS_TIME       100
#define KEY_PRESS_LONG_TIME             500
#define KEY_PRESS_VERY_LONG_TIME        3000
#define KEY_COMBINATION_PRESS_LONG_TIME 2000     
  
  
//static KeyMsg_t	_gKeyMsg[KEY_EVENT_MSG_QUEUE_NUM];

Queue_t	gKeyMsgQueue;

static KeyInfo_t _gKeyInfo[EN_KEY_SRC_NUM];
static Gpio_HandleType *pHandle[EN_KEY_SRC_NUM];
static uint8_t power_input_status=0;
static bool _KeyPress(EN_KEY_SRC src)
{
    if(_gKeyInfo[src].key_hw_mode == EN_KEY_HW_MODE_GPIO)
    {
        uint8_t value = GpioBitGet(pHandle[src]);
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





void KeyInit()
{
    _gKeyInfo[0].key_src = EN_KEY_SRC_SWITCH;
    _gKeyInfo[0].key_press_func = EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS;
    _gKeyInfo[0].key_press_volt = EN_KEY_PRESS_VOLTAGE_LOW;
    _gKeyInfo[0].key_hw_mode = EN_KEY_HW_MODE_GPIO;
    pHandle[0] = &AllPackPowerKeyHandle;    
        
    QueueInit(&gKeyMsgQueue, KEY_EVENT_MSG_QUEUE_NUM, sizeof(KeyMsg_t));
    
    PowerKeyInit();
}


static bool _CombinationPressDetect(EN_KEY_SRC index)
{
    int j = 0;
    for(j = 0; j < EN_KEY_SRC_NUM; j++)
    {
        if(j != index)
        {
            if(_KeyPress((EN_KEY_SRC)j) == true)/*有组合按键*/
            {
                return true;
            }
        }
    }
    return false;
}

//static void _CombinationLongPressFunc(EN_KEY_SRC index)
//{
//    KeyMsg_t    msg;
//    switch(index)
//    {
//        case EN_KEY_SRC_SWITCH:
//        {
//            /*开关和CHA*/
//            if(_KeyPress(EN_KEY_SRC_CHA_MODE) == true)
//            {
//                msg.press_value = EN_KEY_SWITCH_CHA_LONG_PRESSED;
//                QueuePut(&gKeyMsgQueue, &msg);
//            }
//            break;
//        }
//        default:break;
//    }
//}



void KeyDetectProcess()
{
    static uint32_t timeout = 0;
    
    static uint32_t key_filter_time_cnt = 0;
    static uint32_t key_press_time_cnt = 0;
    static uint32_t key_wait_time = 0;          //等待松开信号消抖时间
    static uint32_t key_first_wait_flag = 0;    //首次进入等待状态标志
    KeyMsg_t    msg;
    
    static EN_KEY_SRC  key_index = EN_KEY_SRC_NUM; 
    static EN_KEY_HW_STATUS key_status = EN_KEY_STATUS_IDLE;
       
    switch(key_status)
    {
        case EN_KEY_STATUS_IDLE:
        {
            uint8_t i;
            /*每次只处理一个按键*/
            for(i = 0; i < EN_KEY_SRC_NUM; i++)
            {
                if(_KeyPress((EN_KEY_SRC)i) == true)
                {
                    key_index = (EN_KEY_SRC)i;
                    key_status = EN_KEY_STATUS_FILTER;
                    key_filter_time_cnt = tim_get_systime();
                    return;
                }
            }
            if(i == EN_KEY_SRC_NUM)
            {
                key_index = EN_KEY_SRC_NUM; 
            }
            break;
        }
        case EN_KEY_STATUS_FILTER:
        {
            if((uint32_t)(tim_get_systime() - key_filter_time_cnt) > KEY_FILTER_TIME)
            {
                if(_KeyPress(key_index) == true)
                {
                    key_status = EN_KEY_STATUS_PRESS_DOWN;
                }
                else
                {
                    key_status = EN_KEY_STATUS_IDLE;
                }
            }
            break;
        }
        case EN_KEY_STATUS_PRESS_DOWN:
        {
            if(_CombinationPressDetect(key_index) == true)/*有组合按键*/
            {
                key_status = EN_KEY_STATUS_SUSTAIN;
            }
            else/*无组合按键*/
            {
                if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_ONLY_SHORT_PRESS)/*仅支持短按，按下即触发*/
                {
                    msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
                    QueuePut(&gKeyMsgQueue, &msg);
                    key_status = EN_KEY_STATUS_SUSTAIN;
                }
                else if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)/*长按单次*/
                {
                    key_status = EN_KEY_STATUS_SUSTAIN;
                    key_press_time_cnt  = tim_get_systime();
                }
                else/*长按连续*/  //目前这种模式不支持组合按键
                {
                    /*先触发一次短按*/
                    msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
                    QueuePut(&gKeyMsgQueue, &msg);
                    key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE;
                    key_press_time_cnt  = tim_get_systime();
                    
                }
            }
            break;
        }
        case EN_KEY_STATUS_SUSTAIN:
        {
            if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)/*长按单次*/
            {
                if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_LONG_TIME)
                {

                    if(_CombinationPressDetect(key_index) == false)
                    {
                        msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_LONG_SINGLE_PRESS + key_index);    //为什么？？？？？？？？？？？？？
                        QueuePut(&gKeyMsgQueue, &msg);
                        key_status = EN_KEY_STATUS_WAIT;
                        key_press_time_cnt  = tim_get_systime();
                    }
                }
                else
                {
                    if(_KeyPress(key_index) == false)/*准备执行按键抬起触发短按*/
                    {
                        key_status = EN_KEY_STATUS_PRESS_UP;
                    }
                }
            }
            else/*非抬起执行*/
            {
                if(_KeyPress(key_index) == false)
                {
                    key_status = EN_KEY_STATUS_IDLE;
                }
            }
            
            if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_COMBINATION_PRESS_LONG_TIME)
            {
                if(_CombinationPressDetect(key_index) == true)
                {
                    /*执行组合按键功能*/
//                    _CombinationLongPressFunc(key_index);
                    key_status = EN_KEY_STATUS_WAIT;
                }
            }
            
            break;
        }
        case EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE:/*只在长按连续模式触发*/
        {
            if(_KeyPress(key_index) == false)
            {
                key_status = EN_KEY_STATUS_IDLE;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_CONTINUOUS_PRE_TIME)
                {
                    key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS;                        
                }
            }
            break;
        }
        case EN_KEY_STATUS_SUSTAIN_CONTINUOUS:/*只在长按连续模式触发*/
        {
            if(_KeyPress(key_index) == false)
            {
                key_status = EN_KEY_STATUS_IDLE;
            }
            else
            {
                if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
                {
                    msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
                    QueuePut(&gKeyMsgQueue, &msg);
                    key_press_time_cnt  = tim_get_systime();
                }
            }
            break;
        }
        case EN_KEY_STATUS_WAIT:
        {
            if(_KeyPress(key_index) == false)
            {
                if(key_first_wait_flag == 0)
                {
                    key_first_wait_flag = 1;
                    key_wait_time = tim_get_systime();
                }
                if(tim_difftime_from_now(key_wait_time) > 5)
                {
                    key_status = EN_KEY_STATUS_IDLE;
                    key_first_wait_flag = 0;
                }
            }
            break;
        }
        case EN_KEY_STATUS_PRESS_UP:/*同时支持短按和长按单次功能的按键，短按是在按键抬起时触发(按下时长未达到长按阈值)*/
        {
            msg.press_value = (EN_KEY_PRESSED_MSG)(EN_KEY_SWITCH_SHORT_PRESSED + key_index);
            QueuePut(&gKeyMsgQueue, &msg);
            key_status = EN_KEY_STATUS_IDLE;
            break;
        }
        default:break;
    }
    return;
#if 0
    for(i = 0; i < EN_KEY_SRC_NUM; i++)
    {
        switch(_gKeyInfo[i].key_status)
        {
            case EN_KEY_STATUS_IDLE:
            {
                if(_gKeyInfo[i].key_press() == true)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_FILTER;
                    _gKeyInfo[i].key_filter_time_cnt = tim_get_systime();
                }
                break;
            }
            case EN_KEY_STATUS_FILTER:
            {
                if((uint32_t)(tim_get_systime() - _gKeyInfo[i].key_filter_time_cnt) > KEY_FILTER_TIME)
                {
                    if(_gKeyInfo[i].key_press() == true)
                    {
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_PRESS_DOWN;
                    }
                    else
                    {
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                    }
                    _gKeyInfo[i].key_press_time_cnt  = tim_get_systime();
                }
                break;
            }
            case EN_KEY_STATUS_PRESS_DOWN:
            {
                if(_gKeyInfo[i].key_press_func == EN_KEY_PRESS_FUNC_ONLY_SHORT_PRESS)
                {
                    msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                    QueuePut(&gKeyMsgQueue, &msg);
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_WAIT;
                }
                else if(_gKeyInfo[i].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)/*长按单次*/
                {
                    if((uint32_t)(tim_get_systime() - _gKeyInfo[i].key_press_time_cnt) > KEY_PRESS_LONG_TIME)
                    {
                        msg.press_value = EN_KEY_SWITCH_LONG_PRESSED + i;
                        QueuePut(&gKeyMsgQueue, &msg);
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_WAIT;
                        _gKeyInfo[i].key_press_time_cnt  = tim_get_systime();
                    }
                    else
                    {
                        if(_gKeyInfo[i].key_press() == false)
                        {
                            _gKeyInfo[i].key_status = EN_KEY_STATUS_PRESS_UP;
                        }
                    }
                }
                else/*长按连续*/
                {
                    /*先触发一次短按*/
                    msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                    QueuePut(&gKeyMsgQueue, &msg);
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE;
                    _gKeyInfo[i].key_press_time_cnt  = tim_get_systime();
                    
                }
                break;
            }
            case EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE:/*只在长按连续模式触发*/
            {
                if(_gKeyInfo[i].key_press() == false)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                }
                else
                {
                    if((uint32_t)(tim_get_systime() - _gKeyInfo[i].key_press_time_cnt) > KEY_PRESS_CONTINUOUS_PRE_TIME)
                    {
                        _gKeyInfo[i].key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS;                        
                    }
                }
                break;
            }
            case EN_KEY_STATUS_SUSTAIN_CONTINUOUS:/*只在长按连续模式触发*/
            {
                if(_gKeyInfo[i].key_press() == false)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                }
                else
                {
                    if((uint32_t)(tim_get_systime() - _gKeyInfo[i].key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
                    {
                        msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                        QueuePut(&gKeyMsgQueue, &msg);
                        _gKeyInfo[i].key_press_time_cnt  = tim_get_systime();
                    }
                }
                break;
            }
            case EN_KEY_STATUS_WAIT:
            {
                if(_gKeyInfo[i].key_press() == false)
                {
                    _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                }
                break;
            }
            case EN_KEY_STATUS_PRESS_UP:/*同时支持短按和长按单次功能的按键，短按是在按键抬起时触发(按下时长未达到长按阈值)*/
            {
                msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + i;
                QueuePut(&gKeyMsgQueue, &msg);
                _gKeyInfo[i].key_status = EN_KEY_STATUS_IDLE;
                break;
            }
            default:break;
        }
    }  
#endif
}

void GetKeyValueProcess()
{
    KeyMsg_t    *p_msg;
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        if(p_msg->press_value == EN_KEY_SWITCH_SHORT_PRESSED)
        {
//            LOG_DEBUG(LOG_TAG, "short press \r\n");
//            LOG_DEBUG(LOG_TAG,"power short press \r\n");
        }
//        else if(p_msg->press_value == EN_KEY_SWITCH_LONG_PRESSED)
//        {
//            LOG_DEBUG(LOG_TAG,"power long press \r\n");
//        }
//        else if(p_msg->press_value == EN_KEY_AC_SHORT_PRESSED)
//        {
//            LOG_DEBUG(LOG_TAG,"ac short press \r\n");
//        }
//        else if(p_msg->press_value == EN_KEY_CHA_SHORT_PRESSED)
//        {
//            LOG_DEBUG(LOG_TAG,"mode short press \r\n");
//        }
//        else if(p_msg->press_value == EN_KEY_DC_SHORT_PRESSED)
//        {
//            LOG_DEBUG(LOG_TAG,"dc short press \r\n");
//        }                
    }
}


uint8_t GetKeyStatus()
{
    return GpioBitGet((&AllPackPowerKeyHandle));
}

