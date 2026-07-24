/*
 * relay_ctrl.h
 *
 *  Created on: 2025年11月7日
 *      Author: Nova006348
 */

#ifndef APP_RELAY_CTRL_H_
#define APP_RELAY_CTRL_H_

#include "F28x_Project.h"
#include "system.h"
#include "cpu_timers.h"
#ifdef __cplusplus
extern "C" {
#endif


#define RELAY_MAX_RETRY_CONT             2
#define VOLTAGE_IN_RANGE(voltage, min, max) (((voltage) >= (min)) && ((voltage) <= (max)) ? 1 : 0)


// 状态机状态定义
typedef enum {
    STATE_IDLE = 0,           // 空闲状态
    STATE_CHECK_GRID,         // 检测电网电压
    STATE_RELAY_CLOSE_TEST,   // 继电器闭合
    RELAY_TEST_FAIL                // 自检失败
} RelaySelfTestState;
// 继电器状态结构
typedef struct
{
    RelaySelfTestState  selfTestState;
    int16    test_complete;
    bool   retest_requested;
} RelaySelfTest_t;

typedef struct
{
    float  mid_a_vol_rms;
    float  mid_b_vol_rms;
    float  mid_c_vol_rms;
    float  mid_a_vol_instant;
    float  mid_b_vol_instant;
    float  mid_c_vol_instant;
    float  gird_a_vol_rms;
    float  gird_b_vol_rms;
    float  gird_c_vol_rms;
    float  gird_a_vol_instant;
    float  gird_b_vol_instant;
    float  gird_c_vol_instant;
} RelaySelfTestInValue_t;

#ifdef __cplusplus
}
#endif

void RelaySelfTestInit(void);
/*继电器初始化函数*/
RelaySelfTestInValue_t* GetRelaySelfInputValue(void);
void RelaySelfTest(void);
void RelayRetestRequestSet(bool value);
int16 GetRelaySelfTestResult(void);


#endif /* APP_RELAY_CTRL_H_ */
