
#ifndef __PERI_PROCESS_H_
#define __PERI_PROCESS_H_

#include "dev_led.h"
#include "dev_key.h"



//typedef enum
//{
//    EN_KEY_PRESS_FUNC_MODE_ONLY_SINGLE,
//}EN_KEY_PRESS_FUNC_MODE;

typedef enum
{
    EN_RST_KEY_FUNC_IDLE,
    EN_RST_KEY_FUNC_1,
    EN_RST_KEY_FUNC_2,
    EN_RST_KEY_FUNC_3,
}EN_RST_KEY_FUNC;

typedef enum
{
    EN_KEY_STATUS_NO_PRESS,
    EN_KEY_STATUS_PRESS,
}EN_KEY_STATUS;



typedef struct _key_func_info_stru_t
{
    uint8_t         key_func;
    EN_KEY_STATUS   key_status;
    uint32_t        key_press_time;
    uint32_t        key_press_timeout_count;
}KeyFuncInfo_t;




void LedSetInitStatus(int32_t init);
void PeriStatusInit();
void PeriProcess();


#endif








