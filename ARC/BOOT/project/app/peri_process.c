
#include "peri_process.h"
#include "sys_info.h"
#include "boot.h"

#define KEY_PRESS_DOWN_TIMEOUT  500

static KeyFuncInfo_t _gRstKeyFuncInfo;


void PeriStatusInit()
{
    _gRstKeyFuncInfo.key_status = EN_KEY_STATUS_NO_PRESS;
    _gRstKeyFuncInfo.key_func = EN_RST_KEY_FUNC_IDLE;
    _gRstKeyFuncInfo.key_press_time = 0;
}



static void _LedProcess()
{
    if(_gRstKeyFuncInfo.key_status == EN_KEY_STATUS_PRESS)
    {
        if(_gRstKeyFuncInfo.key_press_time > 1000)
        {
            LedCtlBlink(LED_SRC_SYS, 500);
        }
        if(_gRstKeyFuncInfo.key_press_time > 5000)
        {
            LedCtlBlink(LED_SRC_SYS, 200);
        }
        if(_gRstKeyFuncInfo.key_press_time > 10000)
        {
            LedCtlOff(LED_SRC_SYS);
        }
    }
    else
    {
//        /*系统灯*/
//        LedCtlBlink(LED_SRC_SYS, 2000);
//        /*PCS*/
//        switch(gCollectorDevInfo.inv_comm_flag)
//        {
//            case EN_INV_COMM_FAIL:
//            {
//                LedCtlOff(LED_SRC_PCS);
//                break;
//            }
//            case EN_INV_COMM_OK:
//            {
//                LedCtlOn(LED_SRC_PCS);
//                break;
//            }
//            case EN_INV_COMM_ERROR_CODE:
//            {
//                LedCtlBlink(LED_SRC_PCS, 400);
//                break;
//            }
//            default:break;
//        }
//        /*CONN*/
//        switch(gCollectorDevInfo.wifi_module_flag)
//        {
//            case EN_WIFI_MODULE_UNDETECTED:
//            {
//                LedCtlOff(LED_SRC_CONN);
//            }
//            case EN_WIFI_MODULE_OFF_GRID:
//            {
//                LedCtlBlink(LED_SRC_CONN, 2000);
//            }
//            case EN_WIFI_MODULE_OFF_CLOUD:
//            {
//                LedCtlBlink(LED_SRC_CONN, 500);
//            }
//            case EN_WIFI_MODULE_OK:
//            {
//                LedCtlOn(LED_SRC_CONN);
//            }
//            default: break;
//        }
    }
}



/*短按1S抬起：
 *长按5S抬起：
 *长按10S抬起:
 */
static void _RstKeyProcess()
{
    if(_gRstKeyFuncInfo.key_status == EN_KEY_STATUS_PRESS)
    {
        if((uint32_t)(tim_get_systime() - _gRstKeyFuncInfo.key_press_timeout_count) > KEY_PRESS_DOWN_TIMEOUT)//一定时间没检测到按键按下的消息，则恢复未按下状态
        {
            _gRstKeyFuncInfo.key_press_timeout_count = tim_get_systime();
            _gRstKeyFuncInfo.key_status = EN_KEY_STATUS_NO_PRESS;
        }
        if(_gRstKeyFuncInfo.key_press_time > 1000)
        {
            _gRstKeyFuncInfo.key_func = EN_RST_KEY_FUNC_1;
        }
        if(_gRstKeyFuncInfo.key_press_time > 5000)
        {
            _gRstKeyFuncInfo.key_func = EN_RST_KEY_FUNC_2;
        }
        if(_gRstKeyFuncInfo.key_press_time > 10000)
        {
            _gRstKeyFuncInfo.key_func = EN_RST_KEY_FUNC_3;
        }
    }
    else
    {
        if(_gRstKeyFuncInfo.key_func == EN_RST_KEY_FUNC_1)
        {

        }
        else if(_gRstKeyFuncInfo.key_func == EN_RST_KEY_FUNC_2)
        {
            //重启
            
        }
        else if(_gRstKeyFuncInfo.key_func == EN_RST_KEY_FUNC_3)
        {
            //重启+参数清除
            
        }
        /*恢复至按键未执行状态*/
        _gRstKeyFuncInfo.key_func = EN_RST_KEY_FUNC_IDLE;
        _gRstKeyFuncInfo.key_press_time = 0;
    }
}


static void _KeyProcess()
{
    KeyNewMsg_t    *p_msg;
    if(QueueGet(&gKeyMsgQueue, (void *)&p_msg) == true)
    {
        switch(p_msg->key_src)
        {
            case EN_KEY_SRC_RST:
            {
                if(p_msg->press_value == EN_KEY_SRC_PRESSED_DOWN)
                {
                    if(_gRstKeyFuncInfo.key_status == EN_KEY_STATUS_NO_PRESS)
                    {
                        _gRstKeyFuncInfo.key_status = EN_KEY_STATUS_PRESS;
                        _gRstKeyFuncInfo.key_press_time = 0;
                    }
                    else
                    {
                        _gRstKeyFuncInfo.key_press_time = p_msg->press_down_time;
                    }
                    _gRstKeyFuncInfo.key_press_timeout_count = tim_get_systime();
                    
                }
                else
                {
                    _gRstKeyFuncInfo.key_status = EN_KEY_STATUS_NO_PRESS;
                }
                
                break;
            }
            default:break;
        }
    }
    
//    _RstKeyProcess();
    
}



void PeriProcess()
{
    _KeyProcess();
    if(_gRstKeyFuncInfo.key_status == EN_KEY_STATUS_PRESS)
    {
        LedCtlBlink(LED_SRC_SYS, 300);
        LedCtlBlink(LED_SRC_PCS, 300);
        LedCtlBlink(LED_SRC_CONN, 300);
    }
    else
    {
        LedCtlBlink(LED_SRC_SYS, 1000);
        LedCtlBlink(LED_SRC_PCS, 1000);
        LedCtlBlink(LED_SRC_CONN, 1000);
    }
    
    
    if(_gRstKeyFuncInfo.key_press_time > 5000)
    {
        ExternalFWForceTrans();
    }
    
}











