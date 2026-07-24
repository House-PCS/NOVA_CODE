/*
 * rcmu.c
 *
 *  Created on: 2025年10月30日
 *      Author: Nova006348
 */

#include "rcmu.h"
#include "gpio.h"
#include "debug_analysis.h"

#pragma CODE_SECTION(_LowPass1orderButterFilter,".TI.ramfunc");
#pragma CODE_SECTION(_RmsCalculate,".TI.ramfunc");
#pragma CODE_SECTION(_LeakCurrPreChk,".TI.ramfunc");
#pragma CODE_SECTION(_LeakCurrSelfChk,".TI.ramfunc");
#pragma CODE_SECTION(_LeakCurrNomalRun,".TI.ramfunc");
#pragma CODE_SECTION(_LeakCurr30maSaltateChk,".TI.ramfunc");
#pragma CODE_SECTION(_LeakCurr60maSaltateChk,".TI.ramfunc");
#pragma CODE_SECTION(_LeakCurr150maSaltateChk,".TI.ramfunc");

LeakDetect_t _gLeakDetector;
static RcmuLowPassfilter_1order_t _gLowfilter_1order;

/* 前向声明静态函数 */
static void _LeakCurrNomalRun(float current_rms);
static void _LeakCurr60maSaltateChk(float current_avg);
static void _LeakCurr30maSaltateChk(float current_avg);
static void _LeakCurrPreChk(float rms_current);
static void _LeakCurrSelfChk(float current_rms);
static void _LeakCurrInitDelay(uint16_t cont);
static void _LeakCurr150maSaltateChk(float current_avg);

/**
* @brief: _LowPass1orderButterFilter
* @param: void
* @return:Nothing
* @note:1阶巴特沃斯滤波器
*/

static float _LowPass1orderButterFilter(float32 W0 , float32 T , float32 xn)//yn=a0*x(n)+a1*x(n-1)-b1*y(n-1)
{

    _gLowfilter_1order.yn = xn*_gLowfilter_1order.a0 + _gLowfilter_1order.a1*_gLowfilter_1order.xn_last1 - _gLowfilter_1order.b1 * _gLowfilter_1order.yn_last1;

    _gLowfilter_1order.xn_last1 = xn;
    _gLowfilter_1order.yn_last1 = _gLowfilter_1order.yn;
    return _gLowfilter_1order.yn;
}
/**
* @brief: _LeakCurrInit
* @param: Nothing
* @return:Nothing
* @note:初始化漏电流参数
*/
static void _LeakCurrInit(void)
{
    memset(&_gLeakDetector, 0, sizeof(_gLeakDetector));

    _gLeakDetector.current_state = EN_LEAK_CURR_STATE_PRE_CHK;

    /* 初始化历史值 */
    for (int i = 0; i < 3; i++) {
        _gLeakDetector.history_150ma[i] = 0.0f;
        _gLeakDetector.history_60ma[i] = 0.0f;
        _gLeakDetector.history_30ma[i] = 0.0f;
    }

    /* 初始化输出结果 */
    _gLeakDetector.result.self_test_passed = 0;
    _gLeakDetector.result.fault_flag = false;
    _gLeakDetector.result.rms_current_ma = 0.0f;
    _gLeakDetector.result.avg_rms_current_ma = 0.0f;
    _gLeakDetector.result.instant_curr_ma = 0.0f;
    _gLeakDetector.self_test_requested = false;
    _gLeakDetector.fault_cleared = false;
}
/**
* @brief: _LeakCurrInit
* @param: leak_current_ma
*         cycle_sample_count
*         current_rms_ptr
* @return:cycle_completed
* @note:漏电流有效值计算
*/
static bool _RmsCalculate(float leak_current_ma, uint32_t cycle_sample_count, float* current_rms_ptr)
{
    /* 使用局部变量 - 移除了static，避免状态保持问题 */
    static float total_sum_squares = 0.0f;
    static uint32_t sample_count = 0;
    bool cycle_completed = false;

    /* 参数检查 */
    if (current_rms_ptr == NULL)
    {
        return false;
    }

    /* 将电流值转换为平方值 */
    float current_square = leak_current_ma * leak_current_ma;

    /* 累加平方值 */
    total_sum_squares += current_square;
    sample_count++;

    /* 检查是否完成一个周期的采样 */
    if (sample_count >= cycle_sample_count)
    {
        /* 计算当前周期的RMS值 */
        float current_rms = LEAK_CURRENT_CALCULATE_RMS(total_sum_squares, sample_count);
        *current_rms_ptr = current_rms;

        /* 重置当前周期数据 */
        total_sum_squares = 0.0f;
        sample_count = 0;
        cycle_completed = true;
    }

    return cycle_completed;
}
/**
* @brief: _LeakCurrPreChk
* @param: rms_current
* @return:nothing
* @note:漏电流预检查函数
*/
static void _LeakCurrPreChk(float rms_current)
{
    /* 更新自检平均值 */
    static float total_rms = 0.0f;
    static uint8_t cycle_count = 0;
    static uint8_t cycle_total_count = 0;
    float avg_rms = 0.0f;

    /* 累加RMS值和周期计数 */
    total_rms += rms_current;
    cycle_count++;
    if (cycle_count >= 3)
    {
        cycle_total_count ++;
        avg_rms = total_rms / cycle_count;
        /* 重置累加数据 */
        total_rms = 0.0f;
        cycle_count = 0;\
        /* 检查是否超过阈值 */
        if(avg_rms >= LEAK_CURRENT_SELF_TEST_THRESHOLD_MA)
        {
            _gLeakDetector.self_test_fault_count++;
        }
        else if(_gLeakDetector.self_test_fault_count != 0 && avg_rms < LEAK_CURRENT_SELF_TEST_THRESHOLD_MA)
        {
            _gLeakDetector.self_test_fault_count--;
        }
        if(cycle_total_count >= 4)
        {
            cycle_total_count = 0;
            if(_gLeakDetector.self_test_fault_count == 0)
            {
                /* 未超过阈值，正常进入下一阶段 */
                /***********该处需打开自检IO*************/
                SetRcmuChkState(1);
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_SELF_CHK;
                /* 重置故障计数 */
                _gLeakDetector.self_test_fault_count = 0;
            }
            else
            {
                _gLeakDetector.result.self_test_passed = -1;            /* 自检成功 */
                /* 触发漏电流故障 */
//                _gLeakDetector.result.fault_flag = true;
//                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                /* 重置故障计数 */
                _gLeakDetector.self_test_fault_count = 0;
            }
        }
    }
}
/**
* @brief: _LeakCurrSelfChk
* @param: rms_current
* @return:nothing
* @note:漏电流自检查函数
*/
static void _LeakCurrSelfChk(float current_rms)
{
    static uint8_t self_test_cont = 0;
    self_test_cont++;
    /* 检查是否在设定范围内 */
    if (current_rms >= LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA && current_rms <= LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA)
    {
        _gLeakDetector.self_test_success_count++;
    }
    else if(_gLeakDetector.self_test_success_count != 0 && (current_rms < LEAK_CURRENT_SELF_TEST_RANGE_MIN_MA || current_rms > LEAK_CURRENT_SELF_TEST_RANGE_MAX_MA))
    {
        _gLeakDetector.self_test_success_count--;
    }
    if (_gLeakDetector.self_test_success_count >= LEAK_CURRENT_SELF_TEST_FAULT_COUNT)
    {
        _gLeakDetector.result.self_test_passed = 1;            /* 自检成功 */
        _gLeakDetector.self_test_success_count = 0;
        self_test_cont = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_DELAY;
        /* 关闭自检IO */
        SetRcmuChkState(0);
        return;
    }
    if(self_test_cont >= 7)
    {
        /* 超时处理 */
        self_test_cont = 0;
        _gLeakDetector.self_test_success_count = 0;
//        _gLeakDetector.result.fault_flag = true;
        _gLeakDetector.result.self_test_passed = -1;
//        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
        /* 关闭自检IO */
        SetRcmuChkState(0);
    }
}
/**
* @brief: _LeakCurrInitDelay
* @param: 延时计数
* @return:nothing
* @note:初始化延迟阶段处理
*/
static void _LeakCurrInitDelay(uint16_t cont)
{
    static uint16_t delay_test_cont = 0;
    delay_test_cont++;
    if(delay_test_cont >= cont)
    {
        delay_test_cont = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_NOMAL;

        /* 初始化历史值 */
        for (int i = 0; i < 3; i++)
        {
            _gLeakDetector.history_60ma[i] = 0.0f;
            _gLeakDetector.history_30ma[i] = 0.0f;
        }
    }
}
/**
* @brief: _LeakCurrNomalRun
* @param: 有效值
* @return:nothing
* @note:漏电流正常运行
*/
static void _LeakCurrNomalRun(float current_rms)
{
    static uint8_t rms_avg_cont = 0;
    static float total_rms_current = 0;
    total_rms_current += current_rms;
    rms_avg_cont++;
    /* 立即故障判断 */
//    if (current_rms > LEAK_CURRENT_FAULT_THRESHOLD_THREE)
//    {
//        _gLeakDetector.result.fault_flag = true;
//        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
//        return;
//    }
    /* 300mA故障判断 */
    if (current_rms >= LEAK_CURRENT_FAULT_THRESHOLD_TWO)
    {
        _gLeakDetector.two_level_fault_count++;
    }
    else if(_gLeakDetector.two_level_fault_count != 0 && _gLeakDetector.two_level_fault_count > 0)
    {
        _gLeakDetector.two_level_fault_count--;
    }
    if (_gLeakDetector.two_level_fault_count >= LEAK_CURRENT_TWO_LEVEL_FAULT_THRESHOLD)
    {
        _gLeakDetector.result.fault_flag = true;
        _gLeakDetector.result.leak_type = 4;  // 设置漏电流种类为300mA
        _gLeakDetector.two_level_fault_count = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
        return;
    }
     _LeakCurr150maSaltateChk(current_rms);
    if(rms_avg_cont >= 2)
    {
        _gLeakDetector.result.avg_rms_current_ma = total_rms_current / rms_avg_cont;
        total_rms_current = 0;
        rms_avg_cont = 0;
       _LeakCurr60maSaltateChk(_gLeakDetector.result.avg_rms_current_ma);
       _LeakCurr30maSaltateChk(_gLeakDetector.result.avg_rms_current_ma);

    }
}
/**
* @brief: _LeakCurr150maSaltateChk
* @param: 有效值
* @return:nothing
* @note:漏电流150ma突变检测
*/
static void _LeakCurr150maSaltateChk(float current_avg)
{
    if (current_avg > _gLeakDetector.history_150ma[2])
    {  /* 当前值 > 上上次值 */
        //float square_diff = (current_avg * current_avg) - (_gLeakDetector.history_150ma[2] * _gLeakDetector.history_150ma[2]);

        //if (square_diff > (LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD))
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_150ma[2]+LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD)*(_gLeakDetector.history_150ma[2]+LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_150ma_count++;

            if (_gLeakDetector.square_diff_150ma_count >= LEAK_CURRENT_SQUARE_DIFF_150MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 3;  // 设置漏电流种类为150mA
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.square_diff_150ma_count = 0;
                return;
            }
        }
        else
        {
            /* 不满足突变条件，更新历史值并清空计数 */
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_150ma, current_avg);
            _gLeakDetector.square_diff_150ma_count = 0;
        }
    }
    else
    {
        /* 不满足突变条件，更新历史值并清空计数 */
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_150ma, current_avg);
        _gLeakDetector.square_diff_150ma_count = 0;
    }
}
/**
* @brief: _LeakCurr60maSaltateChk
* @param: 有效值
* @return:nothing
* @note:漏电流60ma突变检测
*/
static void _LeakCurr60maSaltateChk(float current_avg)
{
    if (current_avg > _gLeakDetector.history_60ma[2])
    {  /* 当前值 > 上上次值 */
        //float square_diff = (current_avg * current_avg) - (_gLeakDetector.history_60ma[2] * _gLeakDetector.history_60ma[2]);

        //if (square_diff > (LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD))
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_60ma[2]+LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD)*(_gLeakDetector.history_60ma[2]+LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_60ma_count++;

            if (_gLeakDetector.square_diff_60ma_count >= LEAK_CURRENT_SQUARE_DIFF_60MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 2;  // 设置漏电流种类为60mA
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.square_diff_60ma_count = 0;
                return;
            }
        }
        else
        {
            /* 不满足突变条件，更新历史值并清空计数 */
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_60ma, current_avg);
            _gLeakDetector.square_diff_60ma_count = 0;
        }
    }
    else
    {
        /* 不满足突变条件，更新历史值并清空计数 */
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_60ma, current_avg);
        _gLeakDetector.square_diff_60ma_count = 0;
    }
}
/**
* @brief: _LeakCurr30maSaltateChk
* @param: 有效值
* @return:nothing
* @note:漏电流30ma突变检测
*/
static void _LeakCurr30maSaltateChk(float current_avg)
{

    if (current_avg > _gLeakDetector.history_30ma[2])
    {
        /* 当前值 > 上上次值 */
        //float square_diff = (current_avg * current_avg) - (_gLeakDetector.history_30ma[2] * _gLeakDetector.history_30ma[2]);
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_30ma[2]+LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD)*(_gLeakDetector.history_30ma[2]+LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_30ma_count++;

            if (_gLeakDetector.square_diff_30ma_count >= LEAK_CURRENT_SQUARE_DIFF_30MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 1;  // 设置漏电流种类为30mA
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                return;
            }
        }
        else
        {
            /* 不满足突变条件，更新历史值并清空计数 */
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_30ma, current_avg);
            _gLeakDetector.square_diff_30ma_count = 0;
        }
    }
    else
    {  /* 当前值 <= 上上次值 */
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_30ma, current_avg);
        _gLeakDetector.square_diff_30ma_count = 0;
    }
}
/**
* @brief: LeakCurrRequestSelfTest
* @param: nothing
* @return:nothing
* @note:漏电流30ma突变检测
*/
void LeakCurrRequestSelfTest(void)
{
    _gLeakDetector.self_test_requested = true;
}
/**
* @brief: LeakCurrFaultClare
* @param: nothing
* @return:bool
* @note:漏电流故障清楚
*/
bool LeakCurrFaultClare(void)
{
    if (_gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        /* 只有在故障状态下才能清除故障 */
        _gLeakDetector.fault_cleared = true;
        _gLeakDetector.self_test_requested = true;
        return true;
    }
    return false;
}
/**
* @brief: GetLeakCurrSelfTestState
* @param: nothing
* @return:0
*         1 检测成功
*         -1检测失败
* @note:获取自检结果
*/
int GetLeakCurrSelfTestState(void)
{
    return _gLeakDetector.result.self_test_passed;
}
bool GetLeakCurrFaultFlag(void)
{
    return  _gLeakDetector.result.fault_flag;
}
float GetLeakCurrValue(void)
{
    return  _gLeakDetector.result.rms_current_ma;
}
float GetLeakCurrFilterValue(void)
{
    return  _gLeakDetector.result.instant_curr_ma;
}
/**
* @brief: LeakCurrProcess
* @param: leak_current_ma
*         samp_freq
*         inverter_freq
* @return:nothing
* @note: 主处理函数 - 在20KHz中断中调用
*/
void LeakCurrProcess(float leak_current_ma, uint32_t samp_freq, uint32_t inverter_freq)
{
    bool     rms_value_update_flag = false;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        _LeakCurrInit();
//        _gLowfilter_1order.a0 = LEAK_CURRENT_LowPass_W0*(1/(float)samp_freq)/(LEAK_CURRENT_LowPass_W0*(1/(float)samp_freq)+2);
//        _gLowfilter_1order.a1 = _gLowfilter_1order.a0;
//        _gLowfilter_1order.b1 = (LEAK_CURRENT_LowPass_W0*(1/(float)samp_freq)-2)/(LEAK_CURRENT_LowPass_W0*(1/(float)samp_freq)+2);
        _gLowfilter_1order.a0 = 0.072958f;
        _gLowfilter_1order.a1 = 0.072958f;
        _gLowfilter_1order.b1 = -0.854080f;
        first_run_flag = false;
    }
    /* 检查故障清除请求 */
    if (_gLeakDetector.fault_cleared && _gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        if(_gLeakDetector.result.self_test_passed == 1)
        {
            _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_DELAY;
            _gLeakDetector.result.fault_flag = false;          /*清除故障标志*/
        }
        else
        {
            /* 清除故障状态，重新开始自检 */
            _LeakCurrInit();
        }
        _gLeakDetector.fault_cleared = false;
        return;
    }
    /* 检查自检请求 */
    if (_gLeakDetector.self_test_requested && _gLeakDetector.current_state != EN_LEAK_CURR_STATE_PRE_CHK &&
        _gLeakDetector.current_state != EN_LEAK_CURR_STATE_SELF_CHK)
    {
        /* 重新开始自检流程 */
        _LeakCurrInit();
        _gLeakDetector.self_test_requested = false;
        return;
    }

    /*巴特沃斯滤波器，截止频率500hZ*/
    _gLeakDetector.result.instant_curr_ma = _LowPass1orderButterFilter(LEAK_CURRENT_LowPass_W0,1/(float)samp_freq,leak_current_ma);
    /* 计算每个周期的采样点数 */
    _gLeakDetector.cycle_sample_cont = samp_freq / ((inverter_freq > 0) ? ((inverter_freq < 30) ? 30 : ((inverter_freq > 70) ? 70 : inverter_freq)) :50);  // 默认为50Hz，将inverter_freq限制在30到70之间
    /* 计算漏电流有效值 */
    rms_value_update_flag = _RmsCalculate(_gLeakDetector.result.instant_curr_ma,_gLeakDetector.cycle_sample_cont,&_gLeakDetector.result.rms_current_ma);

    /* 如果当前处于故障状态且没有清除请求，则直接返回 */
    if (_gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        return;
    }

//    /*巴特沃斯滤波器，截止频率500hZ*/
//    _gLeakDetector.result.instant_curr_ma = _LowPass1orderButterFilter(LEAK_CURRENT_LowPass_W0,1/(float)samp_freq,leak_current_ma);
//    /* 计算每个周期的采样点数 */
//    _gLeakDetector.cycle_sample_cont = samp_freq / ((inverter_freq > 0) ? ((inverter_freq < 30) ? 30 : ((inverter_freq > 70) ? 70 : inverter_freq)) :50);  // 默认为50Hz，将inverter_freq限制在30到70之间
//    /* 计算漏电流有效值 */
//    rms_value_update_flag = _RmsCalculate(_gLeakDetector.result.instant_curr_ma,_gLeakDetector.cycle_sample_cont,&_gLeakDetector.result.rms_current_ma);
    if(rms_value_update_flag)
    {
        switch (_gLeakDetector.current_state)
        {
            case EN_LEAK_CURR_STATE_PRE_CHK:
                _LeakCurrPreChk(_gLeakDetector.result.rms_current_ma);
                break;

            case EN_LEAK_CURR_STATE_SELF_CHK:
                _LeakCurrSelfChk(_gLeakDetector.result.rms_current_ma);
                break;

            case EN_LEAK_CURR_STATE_RUN_DELAY:
                _LeakCurrInitDelay(LEAK_CURRENT_INIT_DELAY_TIME);
                break;

            case EN_LEAK_CURR_STATE_RUN_NOMAL:
                _LeakCurrNomalRun(_gLeakDetector.result.rms_current_ma);
                break;

            default:
                break;
        }
    }
}
