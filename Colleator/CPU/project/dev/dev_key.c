
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

Queue_t	gKeyMsgQueue;


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


//static bool _CombinationPressDetect(EN_KEY_SRC index)
//{
//    int j = 0;
//    for(j = 0; j < EN_KEY_SRC_NUM; j++)
//    {
//        if(j != index)
//        {
//            if(_KeyPress(j) == true)/*有组合按键*/
//            {
//                return true;
//            }
//        }
//    }
//    return false;
//}

//static void _CombinationLongPressFunc(EN_KEY_SRC index)
//{
//    KeyMsg_t    msg;
//    switch(index)
//    {
//        default:break;
//    }
//}




void DevKeyInit()
{
    _gKeyInfo[0].key_src = EN_KEY_SRC_RST;
    _gKeyInfo[0].key_status = EN_KEY_STATUS_IDLE;
//    _gKeyInfo[0].key_press_func = EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS;
    _gKeyInfo[0].key_press_volt = EN_KEY_PRESS_VOLTAGE_LOW;
    _gKeyInfo[0].key_hw_mode = EN_KEY_HW_MODE_GPIO;
    _pHandle[0] = &KeyRstHandle;
    
    QueueInit(&gKeyMsgQueue, KEY_EVENT_MSG_QUEUE_NUM, sizeof(KeyNewMsg_t));
    KeyBspInit();
}



void DevKeyDetectProcess()
{
    static uint32_t timeout = 0;
    KeyNewMsg_t     msg;
    static EN_KEY_SRC  key_index = EN_KEY_SRC_NUM; 
    static EN_KEY_HW_STATUS key_status = EN_KEY_STATUS_IDLE;
    static uint32_t press_down_num[EN_KEY_SRC_NUM];
    
        
    for(key_index = 0; key_index < EN_KEY_SRC_NUM; key_index++)
    {
        switch(_gKeyInfo[key_index].key_status)
        {
            case EN_KEY_STATUS_IDLE:
            {
                if(_KeyPress(key_index) == true)
                {
                    _gKeyInfo[key_index].key_status = EN_KEY_STATUS_FILTER;
                    _gKeyInfo[key_index].key_filter_time_cnt = tim_get_systime();
                }
                break;
            }
            case EN_KEY_STATUS_FILTER:
            {
                if((uint32_t)(tim_get_systime() - _gKeyInfo[key_index].key_filter_time_cnt) > KEY_FILTER_TIME)
                {
                    if(_KeyPress(key_index) == true)
                    {
                        _gKeyInfo[key_index].key_status = EN_KEY_STATUS_PRESS_DOWN;
                    }
                    else
                    {
                        _gKeyInfo[key_index].key_status = EN_KEY_STATUS_IDLE;
                    }
                    _gKeyInfo[key_index].key_press_time_cnt  = tim_get_systime();
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
                _gKeyInfo[key_index].key_press_time_cnt  = tim_get_systime();
                _gKeyInfo[key_index].key_status = EN_KEY_STATUS_SUSTAIN;
                break;
            }
            case EN_KEY_STATUS_SUSTAIN:
            {
                /*每隔100ms发送一次*/ 
                if((uint32_t)(tim_get_systime() - _gKeyInfo[key_index].key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
                {
                    msg.key_src = key_index;
                    msg.press_value = EN_KEY_SRC_PRESSED_DOWN;
                    msg.press_down_time = KEY_PRESS_CONTINUOUS_TIME * press_down_num[key_index];
                    QueuePut(&gKeyMsgQueue, &msg);
                    press_down_num[key_index]++;
                    _gKeyInfo[key_index].key_press_time_cnt  = tim_get_systime();
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
            default:break;
        }
    }
    
}








//void DevKeyDetectProcess()
//{
//    static uint32_t timeout = 0;
//    static uint32_t key_filter_time_cnt = 0;
//    static uint32_t key_press_time_cnt = 0;
//    KeyMsg_t    msg;
//    
//    static EN_KEY_SRC  key_index = EN_KEY_SRC_NUM; 
//    static EN_KEY_HW_STATUS key_status = EN_KEY_STATUS_IDLE;
//    
//    switch(key_status)
//    {
//        case EN_KEY_STATUS_IDLE:
//        {
//            uint8_t i;
//            /*每次只处理一个按键*/
//            for(i = 0; i < EN_KEY_SRC_NUM; i++)
//            {
//                if(_KeyPress(i) == true)
//                {
//                    key_index = i;
//                    key_status = EN_KEY_STATUS_FILTER;
//                    key_filter_time_cnt = tim_get_systime();
//                    return;
//                }
//            }
//            if(i == EN_KEY_SRC_NUM)
//            {
//                key_index = EN_KEY_SRC_NUM; 
//            }
//            break;
//        }
//        case EN_KEY_STATUS_FILTER:
//        {
//            if((uint32_t)(tim_get_systime() - key_filter_time_cnt) > KEY_FILTER_TIME)
//            {
//                if(_KeyPress(key_index) == true)
//                {
//                    key_status = EN_KEY_STATUS_PRESS_DOWN;
//                }
//                else
//                {
//                    key_status = EN_KEY_STATUS_IDLE;
//                }
//            }
//            break;
//        }
//        case EN_KEY_STATUS_PRESS_DOWN:
//        {
//            if(_CombinationPressDetect(key_index) == true)/*有组合按键*/
//            {
//                key_status = EN_KEY_STATUS_SUSTAIN;
//            }
//            else/*无组合按键*/
//            {
//                if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_ONLY_SHORT_PRESS)/*仅支持短按，按下即触发*/
//                {
//                    msg.press_value = EN_KEY_SRC_SHORT_PRESSED + key_index;
//                    QueuePut(&gKeyMsgQueue, &msg);
//                    key_status = EN_KEY_STATUS_SUSTAIN;
//                }
//                else if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)/*长按单次*/
//                {
//                    key_status = EN_KEY_STATUS_SUSTAIN;
//                    key_press_time_cnt  = tim_get_systime();
//                }
//                else/*长按连续*/  //目前这种模式不支持组合按键
//                {
//                    /*先触发一次短按*/
//                    msg.press_value = EN_KEY_SRC_SHORT_PRESSED + key_index;
//                    QueuePut(&gKeyMsgQueue, &msg);
//                    key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE;
//                    key_press_time_cnt  = tim_get_systime();
//                }
//            }
//            break;
//        }
//        case EN_KEY_STATUS_SUSTAIN:
//        {
//            if(_gKeyInfo[key_index].key_press_func == EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS)/*长按单次*/
//            {
//                if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_5S_TIME)
//                {
//                    if(_CombinationPressDetect(key_index) == false)
//                    {
//                        msg.press_value = EN_KEY_SRC_5S_PRESSED + key_index;
//                        QueuePut(&gKeyMsgQueue, &msg);
//                        key_status = EN_KEY_STATUS_WAIT;
//                        key_press_time_cnt  = tim_get_systime();
//                    }
//                }
//                else if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_10S_TIME)
//                {
//                    if(_CombinationPressDetect(key_index) == false)
//                    {
//                        msg.press_value = EN_KEY_SRC_10S_PRESSED + key_index;
//                        QueuePut(&gKeyMsgQueue, &msg);
//                        key_status = EN_KEY_STATUS_WAIT;
//                        key_press_time_cnt  = tim_get_systime();
//                    }
//                }
//                else
//                {
//                    if(_KeyPress(key_index) == false)/*准备执行按键抬起触发短按*/
//                    {
//                        key_status = EN_KEY_STATUS_PRESS_UP;
//                    }
//                }
//            }
//            else/*非抬起执行*/
//            {
//                if(_KeyPress(key_index) == false)
//                {
//                    key_status = EN_KEY_STATUS_IDLE;
//                }
//            }
//            
//            if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_COMBINATION_PRESS_LONG_TIME)
//            {
//                if(_CombinationPressDetect(key_index) == true)
//                {
//                    /*执行组合按键功能*/
//                    _CombinationLongPressFunc(key_index);
//                    key_status = EN_KEY_STATUS_WAIT;
//                }
//            }
//            
//            break;
//        }
//        case EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE:/*只在长按连续模式触发*/
//        {
//            if(_KeyPress(key_index) == false)
//            {
//                key_status = EN_KEY_STATUS_IDLE;
//            }
//            else
//            {
//                if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_CONTINUOUS_PRE_TIME)
//                {
//                    key_status = EN_KEY_STATUS_SUSTAIN_CONTINUOUS;                        
//                }
//            }
//            break;
//        }
//        case EN_KEY_STATUS_SUSTAIN_CONTINUOUS:/*只在长按连续模式触发*/
//        {
//            if(_KeyPress(key_index) == false)
//            {
//                key_status = EN_KEY_STATUS_IDLE;
//            }
//            else
//            {
//                if((uint32_t)(tim_get_systime() - key_press_time_cnt) > KEY_PRESS_CONTINUOUS_TIME)
//                {
//                    msg.press_value = EN_KEY_SWITCH_SHORT_PRESSED + key_index;
//                    QueuePut(&gKeyMsgQueue, &msg);
//                    key_press_time_cnt  = tim_get_systime();
//                }
//            }
//            break;
//        }
//        case EN_KEY_STATUS_WAIT:
//        {
//            if(_KeyPress(key_index) == false)
//            {
//                key_status = EN_KEY_STATUS_IDLE;
//            }
//            break;
//        }
//        case EN_KEY_STATUS_PRESS_UP:/*同时支持短按和长按单次功能的按键，短按是在按键抬起时触发(按下时长未达到长按阈值)*/
//        {
//            msg.press_value = EN_KEY_SRC_SHORT_PRESSED + key_index;
//            QueuePut(&gKeyMsgQueue, &msg);
//            key_status = EN_KEY_STATUS_IDLE;
//            break;
//        }
//        default:break;
//    }
//}

