
#ifndef __EXTERNAL_TRIGGER_PROCESS_H_
#define __EXTERNAL_TRIGGER_PROCESS_H_

#include "dev_key.h"
#include "dev_led.h"
#include "dev_system_power.h"
#include "modbus_analysis.h"
#include "sox.h"
#include "dev_mos_control.h"
#include "dev_communication.h"
#include "bms_protocol.h"

typedef enum
{
    EN_BMS_TURNOFF_START = 0,
    EN_BMS_TURNOFF_WAIT,
    EN_BMS_TRUNOFF_END,
    EN_BMS_TURN_NORMAL,
} EN_BMS_POWER_STATE;
void ShutDownProcess(void);
void KeyValueProcess(void);
#endif













