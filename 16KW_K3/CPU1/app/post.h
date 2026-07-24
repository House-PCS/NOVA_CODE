/*
 * post.h
 *
 *  Created on: 2025年11月10日
 *      Author: Nova006348
 */
//POWER ON SELF_TEST
#ifndef APP_POST_H_
#define APP_POST_H_

#include "F28x_Project.h"
#include "system.h"
#include "cpu_timers.h"
#ifdef __cplusplus
extern "C" {
#endif

#define SYS_SELF_TEST_TIMEOUT     25*1000      /*超时时间*/
#define SYS_TEST_TIME              15*1000      /*自检时间*/

typedef union
{
    uint16_t byte;
    struct
    {
        uint16_t fan_test_complete         : 1;
        uint16_t ntc_test_complete         : 1;
        uint16_t adc_test_complete         : 1;
        uint16_t Leak_test_complete        : 1;
        uint16_t relay_test_complete       : 1;
        uint16_t iso_test_complete         : 1;
        uint16_t reserve                   : 10;
    } bit;
} ModuleTestCompleteState_t;

typedef struct _Self_Test_stru_t
{
    ModuleTestCompleteState_t        module_state;         //DSP不上报的内部告警
    bool                             complete_flag;        //成功标志
    uint32_t                         max_test_time;        //超时时间
    bool                             timeout_flag;
    bool                             retest_request_flag;  //重新自检Flag
} SysSelfTest_t;

void SysSelfTestInit(void);
void SysSelfTest(void);
void SysRetestRequestSet(bool value);
bool GetSysTimeOutFlag(void);
bool GetSysSelfTestFlag(void);
#endif /* APP_POST_H_ */
