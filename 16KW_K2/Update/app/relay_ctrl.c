/*
 * relay_ctrl.c
 *
 *  Created on: 2025年11月7日
 *      Author: Nova006348
 */

#include "relay_ctrl.h"
#include "gpio.h"
#include <string.h>
#include "cla_cpu_share.h"
#include <algo_interface.h>
#include "debug_analysis.h"

// 模块全局变量
static RelaySelfTestInValue_t _gRelayTestInValue;
static RelaySelfTest_t _gRelaySelfTest;

 void RelaySelfTestInit(void)
{
    _gRelaySelfTest.retest_requested = false;
    _gRelaySelfTest.test_complete = 0;
    _gRelaySelfTest.selfTestState = STATE_IDLE;
}

RelaySelfTestInValue_t* GetRelaySelfInputValue(void)
{
    return &_gRelayTestInValue;
}

/**
* @brief: RelaySelfTest
* @param: Nothing
* @return:Nothing
* @note: 继电器自检
*/
void RelaySelfTest(void)
{
    static uint32_t timeout;
    static uint16_t relayopen_delay_cnt;
    // 检查是否需要重新自检
    if (_gRelaySelfTest.retest_requested && _gRelaySelfTest.selfTestState == RELAY_TEST_FAIL)
    {
        RelaySelfTestInit();

    }
    if(_gRelaySelfTest.test_complete !=0)
    {
        return;
    }

    // 状态机处理
    switch (_gRelaySelfTest.selfTestState)
    {
        case STATE_IDLE:
        {
            // 初始化测试条件
            RELAY_GRID_CLOSE;  // 确保继电器断开
            timeout = TimGetSystime();
            _gRelaySelfTest.selfTestState = STATE_CHECK_GRID;
            break;
        }
        case STATE_CHECK_GRID:
        {
            if(TimDifftimeFromNow(timeout)  < 140)    /*延时40ms*/
            {
                break;
            }
            // 判断电网电压是否在有效范围内,在设定范围内才会进行检测
            if(VOLTAGE_IN_RANGE(_gRelayTestInValue.gird_a_vol_rms,170,280))
            {
                if(_gRelayTestInValue.mid_a_vol_rms <= 50 )
                {
                    RELAY_GRID_OPEN;  // 闭合继电器
                    _gRelaySelfTest.selfTestState = STATE_RELAY_CLOSE_TEST;
                    timeout = TimGetSystime();
                }
                else     /*继电器未开但存在电压*/
                {
                    _gRelaySelfTest.test_complete = -1;
                    _gRelaySelfTest.selfTestState = RELAY_TEST_FAIL;
                }
            }
            else
            {
                _gRelaySelfTest.test_complete = 1;
                _gRelaySelfTest.selfTestState = STATE_IDLE;
            }
            break;
        }
        case STATE_RELAY_CLOSE_TEST:
        {
            if(TimDifftimeFromNow(timeout)  < 2160)     
            {
                break;
            }
            if( (fabsf(_gRelayTestInValue.mid_a_vol_instant - _gRelayTestInValue.gird_a_vol_instant) < 10.0f) )
            {
                relayopen_delay_cnt++;

                if (relayopen_delay_cnt <= 200)      // 2S之内持续判断继电器短路逻辑,同时避免电网继电器开关过快造成硬件过流
                {
                    _gRelaySelfTest.test_complete = 0;
                    _gRelaySelfTest.selfTestState = STATE_RELAY_CLOSE_TEST;
                }
                else
                {
                    _gRelaySelfTest.test_complete = 1;
                    _gRelaySelfTest.selfTestState = STATE_IDLE;
                    relayopen_delay_cnt = 0;
                }
            }
            else
            {
                _gRelaySelfTest.test_complete = -1;
                _gRelaySelfTest.selfTestState = RELAY_TEST_FAIL;
            }
            break;
        }
        default:
            // 未知状态，回到初始状态
            _gRelaySelfTest.selfTestState = STATE_IDLE;
            break;
    }
}

/**
* @brief: GetRelaySelfTestResult
* @param: Nothing
* @return:Nothing
* @note: 获取继电器自检结果
*/
int16 GetRelaySelfTestResult(void)
{
   return _gRelaySelfTest.test_complete;
}

void RelayRetestRequestSet(bool value)
{
    _gRelaySelfTest.retest_requested = value;
}
