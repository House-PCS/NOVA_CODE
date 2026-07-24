#ifndef SYSTEM_CTRL_HHH
#define SYSTEM_CTRL_HHH
#include "slave_collect.h"


#define     CAP_STORAGE_BASE_ADDR           0x258000

typedef  enum
{
    SYS_INIT_STATUS,
    SYS_WAKEUP_STATUS,
    SYS_IDLE_STATUS,
    SYS_WORKING_STATUS,
    SYS_ERROR_STATUS,
}teSysStatus;




typedef enum
{
    CHG_INIT,
    CHG_SET_MAX_VOLT_STATUS,
    CHG_SET_MAX_CURR_STATUS,
    CHG_SET_POWER_STATUS,
    CHG_WORKING_STATUS,
    CHG_OVP_STATUS,
}teChgAdjustStatus;






#pragma pack()

void SystemCtrlInit(void);
void SystemCtrlProc(void);

#endif