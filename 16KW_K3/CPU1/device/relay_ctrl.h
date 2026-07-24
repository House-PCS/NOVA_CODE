/**
 * @file relay_ctrl.h
 * @brief Relay control and self-test
 */

#ifndef APP_RELAY_CTRL_H_
#define APP_RELAY_CTRL_H_

#include "F28x_Project.h"
#include "system.h"
#include "cpu_timers.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Relay channel index */
typedef enum
{
    EN_RELAT_CTRL_TYPE_LOAD = 0,
    EN_RELAT_CTRL_TYPE_GRID,
    EN_RELAT_CTRL_TYPE_INV,
    EN_RELAT_CTRL_TYPE_GEN,
    EN_RELAT_CTRL_TYPE_N_PE,
    EN_RELAT_CTRL_TYPE_COUNT,
    EN_RELAT_CTRL_TYPE_GEN_START,
    EN_RELAT_CTRL_TYPE_EXC
} EN_RELAT_CTRL_TYPE;

#define RELAY_OFF 0
#define RELAY_ON  1

#define RELAY_MAX_RETRY_CONT 2
#define VOLTAGE_IN_RANGE(voltage, min, max) (((voltage) >= (min)) && ((voltage) <= (max)) ? 1 : 0)

typedef struct
{
    bool open_flag;
} RelayInfo_t;

/** Relay self-test state machine */
typedef enum
{
    STATE_IDLE = 0,
    STATE_CHECK_GRID,
    STATE_RELAY_CLOSE_TEST,
    RELAY_TEST_FAIL
} RelaySelfTestState;

typedef struct
{
    RelaySelfTestState selfTestState;
    int16 test_complete;
    bool retest_requested;
} RelaySelfTest_t;

/** Sampled voltages used during relay self-test */
typedef struct
{
    float mid_a_vol_rms;
    float mid_b_vol_rms;
    float mid_c_vol_rms;
    float mid_a_vol_instant;
    float mid_b_vol_instant;
    float mid_c_vol_instant;
    float grid_a_vol_rms;
    float grid_b_vol_rms;
    float grid_c_vol_rms;
    float grid_a_vol_instant;
    float grid_b_vol_instant;
    float grid_c_vol_instant;
    float Load_a_vol_rms;
    float Load_b_vol_rms;
    float Load_c_vol_rms;
    float Gen_a_vol_rms;
    float Gen_b_vol_rms;
    float Gen_c_vol_rms;
    float Inv_a_vol_rms;
    float Inv_b_vol_rms;
    float Inv_c_vol_rms;
} RelaySelfTestInValue_t;

void RelayControlInit(void);
void RelayControl(EN_RELAT_CTRL_TYPE relay_type, uint8_t state);
RelaySelfTestInValue_t* GetRelaySelfInputValue(void);
void RelaySelfTest(void);
void RelayRetestRequestSet(bool value);
int16 GetRelaySelfTestResult(void);
int16 GetRelayState(EN_RELAT_CTRL_TYPE relay_type);

#ifdef __cplusplus
}
#endif

#endif /* APP_RELAY_CTRL_H_ */
