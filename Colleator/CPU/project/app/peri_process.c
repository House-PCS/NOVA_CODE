
#include "peri_process.h"
#include "sys_info.h"
#include "fc41d.h"
#include "collector.h"

extern void CollectorSetDefault();

int32_t _gDeviceInitDebug = 0;
int32_t _gDeviceInit = 1;

#define KEY_PRESS_DOWN_TIMEOUT  500

static KeyFuncInfo_t _gRstKeyFuncInfo;


void PeriStatusInit()
{
    _gRstKeyFuncInfo.key_status = EN_KEY_STATUS_NO_PRESS;
    _gRstKeyFuncInfo.key_func = EN_RST_KEY_FUNC_IDLE;
    _gRstKeyFuncInfo.key_press_time = 0;
}

void LedSetInitStatus(int32_t init)
{
	_gDeviceInit = init;
}

static void _LedProcess()
{
	if (_gDeviceInitDebug)
	{
		switch (_gDeviceInitDebug)
		{
		case 1:
			_LedON(LED_SRC_SYS);
			_LedON(LED_SRC_PCS);
			break;
		case 2:
			LedCtlBlink(LED_SRC_SYS, 200);
			LedCtlBlink(LED_SRC_PCS, 200);
			break;
		case 3:
			LedCtlBlink(LED_SRC_SYS, 50);
			LedCtlBlink(LED_SRC_PCS, 50);
			break;
		}

		return;
	}



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
		/*????*/
		LedCtlBlink(LED_SRC_SYS, 2000);
		/*PCS*/

		if (_gDeviceInit)
		{
			LedCtlBlink(LED_SRC_PCS, 200);
		}
		else
		{
			switch(gCollectorDevInfo.inv_comm_flag)
			{
			case EN_INV_COMM_FAIL:
				{
					LedCtlOff(LED_SRC_PCS);
					break;
				}
			case EN_INV_COMM_OK:
				{
					LedCtlOn(LED_SRC_PCS);
					break;
				}
			case EN_INV_COMM_ERROR_CODE:
				{
					LedCtlBlink(LED_SRC_PCS, 800);
					break;
				}
			default:break;
			}
		}
		/*CONN*/
		switch(gCollectorDevInfo.wifi_module_flag)
		{
		case EN_WIFI_MODULE_UNDETECTED:
			{
				LedCtlOff(LED_SRC_CONN);
				break;
			}
		case EN_WIFI_MODULE_OFF_GRID:
			{
				LedCtlBlink(LED_SRC_CONN, 2000);
				break;
			}
		case EN_WIFI_MODULE_OFF_CLOUD:
			{
				LedCtlBlink(LED_SRC_CONN, 500);
				break;
			}
		case EN_WIFI_MODULE_OK:
			{
				LedCtlOn(LED_SRC_CONN);
				break;
			}
		default: break;
		}

//        LedCtlBlink(LED_SRC_SYS, 1000);
//        LedCtlBlink(LED_SRC_PCS, 2000);
//        LedCtlBlink(LED_SRC_CONN, 3000);
    }
}



/*???1S¸??
 *????5S¸??
 *????10S¸??:
 */
static void _RstKeyProcess()
{
    if(_gRstKeyFuncInfo.key_status == EN_KEY_STATUS_PRESS)
    {
        if((uint32_t)(tim_get_systime() - _gRstKeyFuncInfo.key_press_timeout_count) > KEY_PRESS_DOWN_TIMEOUT)//h?????û??????????µ??????????d??????
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
			Fc41dReset();
        }
        else if(_gRstKeyFuncInfo.key_func == EN_RST_KEY_FUNC_2)
        {
            //????
            __set_FAULTMASK(1);
            NVIC_SystemReset();
        }
        else if(_gRstKeyFuncInfo.key_func == EN_RST_KEY_FUNC_3)
        {
            //????+???????

			CollectorSetDefault();
			CollectorSpiFlashWriteCfg();

            __set_FAULTMASK(1);
            NVIC_SystemReset();
        }
        /*?????????d?????*/
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

    _RstKeyProcess();

}



void PeriProcess()
{
    _LedProcess();
    _KeyProcess();
}











