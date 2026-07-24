
#ifndef __DEV_KEY_H_
#define __DEV_KEY_H_


#include "bsp.h"
#include "systick.h"
#include "queue.h"

#define KEY_EVENT_MSG_QUEUE_NUM	50


typedef enum 
{
    EN_KEY_STATUS_IDLE,
    EN_KEY_STATUS_FILTER,
    EN_KEY_STATUS_PRESS_DOWN,
    EN_KEY_STATUS_SUSTAIN,//按下持续过程
    EN_KEY_STATUS_SUSTAIN_CONTINUOUS_PRE,//连续按键第一次触发等待时间，类似于键盘效果
    EN_KEY_STATUS_SUSTAIN_CONTINUOUS,//按下持续一段时间后，进入连续输入模式(仅支持EN_KEY_PRESS_FUNC_SHORT_LONG_CONTINUS_PRESS)
    EN_KEY_STATUS_WAIT,
    EN_KEY_STATUS_PRESS_UP
}EN_KEY_HW_STATUS;


typedef enum
{
    EN_KEY_SRC_SWITCH = 0,
//    EN_KEY_SRC_EX5V,
//    EN_KEY_SRC_DSG_OC,
//    EN_KEY_SRC_CHG_OC,
    EN_KEY_SRC_NUM
}EN_KEY_SRC;


typedef enum
{
    EN_KEY_SWITCH_SHORT_PRESSED,
    EN_KEY_EX5V_SHORT_PRESSED,
    EN_KEY_DSG_OC_SHORT_PRESSED,
    EN_KEY_CHG_OC_SHORT_PRESSED,
    
    EN_KEY_SWITCH_LONG_SINGLE_PRESS,
    EN_KEY_EX5V_LONG_PRESSED,
    EN_KEY_DSG_OC_LONG_PRESSED,
    EN_KEY_CHG_OC_LONG_PRESSED,
    
    EN_KEY_SWITCH_VERY_LONG_PRESSED,
    EN_KEY_EX5V_VERY_LONG_PRESSED,
    EN_KEY_DSG_OC_VERY_LONG_PRESSED,
    EN_KEY_CHG_OC_VERY_LONG_PRESSED,
    
}EN_KEY_PRESSED_MSG;


typedef enum
{
    EN_KEY_PRESS_FUNC_ONLY_SHORT_PRESS,
    EN_KEY_PRESS_FUNC_SHORT_LONG_SINGLE_PRESS,
    EN_KEY_PRESS_FUNC_SHORT_LONG_CONTINUS_PRESS,/*类似于键盘，长按连续触发*/
}EN_KEY_PRESS_FUNC;

typedef enum
{
    EN_KEY_PRESS_VOLTAGE_LOW,
    EN_KEY_PRESS_VOLTAGE_HIGH,
}EN_KEY_PRESS_VOLTAGE;

typedef enum
{
    EN_KEY_HW_MODE_GPIO,
    EN_KEY_HW_MODE_ADC,
}EN_KEY_HW_MODE;


typedef struct _key_sub_msg_stru_t
{
    EN_KEY_PRESSED_MSG  press_value;
//    uint8_t             coefficient;
}KeyMsg_t;









typedef struct _key_info_t
{
    EN_KEY_SRC              key_src;
//    EN_KEY_HW_STATUS        key_status;
    EN_KEY_PRESS_FUNC       key_press_func;
    EN_KEY_PRESS_VOLTAGE    key_press_volt;
    EN_KEY_HW_MODE          key_hw_mode;
//    uint32_t                key_filter_time_cnt;
//    uint32_t                key_press_time_cnt;
//    bool                    (*key_press)();
}KeyInfo_t;


extern Queue_t	gKeyMsgQueue;

void KeyInit(void);
void KeyDetectProcess(void);
void GetKeyValueProcess(void);
uint8_t  GetPowerInputStatus(void);
uint8_t  GetKeyStatus(void);
#endif
















