/*
 * rcmu.c
 *
 *  Created on: Oct 30, 2025
 *      Author: Nova006348
 */

#include "rcmu.h"
#include "gpio.h"
#include "debug_analysis.h"
#include "mid_filter.h"

LeakDetect_t _gLeakDetector;
static Mid_Filter1OrderButter_t _gLowfilter_1order;

/* Forward declarations */
static void _LeakCurrNomalRun(float current_rms);
static void _LeakCurr60maSaltateChk(float current_avg);
static void _LeakCurr30maSaltateChk(float current_avg);
static void _LeakCurrPreChk(float rms_current);
static void _LeakCurrSelfChk(float current_rms);
static void _LeakCurrInitDelay(uint16_t cont);
static void _LeakCurr150maSaltateChk(float current_avg);

/**
 * @brief Initialize leak current detector
 */
static void _LeakCurrInit(void)
{
    memset(&_gLeakDetector, 0, sizeof(_gLeakDetector));

    _gLeakDetector.current_state = EN_LEAK_CURR_STATE_PRE_CHK;

    /* Clear history buffers */
    for (int i = 0; i < 3; i++) {
        _gLeakDetector.history_60ma[i] = 0.0f;
        _gLeakDetector.history_30ma[i] = 0.0f;
    }

    /* Reset output result */
    _gLeakDetector.result.self_test_passed = 0;
    _gLeakDetector.result.fault_flag = false;
    _gLeakDetector.result.rms_current_ma = 0.0f;
    _gLeakDetector.result.avg_rms_current_ma = 0.0f;
    _gLeakDetector.result.instant_curr_ma = 0.0f;
    _gLeakDetector.self_test_requested = false;
    _gLeakDetector.fault_cleared = false;
}

/**
 * @brief Calculate leak current RMS over one AC cycle
 * @param leak_current_ma Filtered instantaneous current (mA)
 * @param cycle_sample_count Samples per cycle
 * @param current_rms_ptr Output RMS pointer
 * @return true when one cycle RMS is updated
 */
static bool _RmsCalculate(float leak_current_ma, uint32_t cycle_sample_count, float* current_rms_ptr)
{
    static float total_sum_squares = 0.0f;
    static uint32_t sample_count = 0;
    bool cycle_completed = false;

    if (current_rms_ptr == NULL)
    {
        return false;
    }

    /* Accumulate squared samples */
    float current_square = leak_current_ma * leak_current_ma;

    total_sum_squares += current_square;
    sample_count++;

    /* Update RMS when one cycle completes */
    if (sample_count >= cycle_sample_count)
    {
        float current_rms = LEAK_CURRENT_CALCULATE_RMS(total_sum_squares, sample_count);
        *current_rms_ptr = current_rms;

        total_sum_squares = 0.0f;
        sample_count = 0;
        cycle_completed = true;
    }

    return cycle_completed;
}

/**
 * @brief Pre-check before self-test (test IO off)
 */
static void _LeakCurrPreChk(float rms_current)
{
    static float total_rms = 0.0f;
    static uint8_t cycle_count = 0;
    static uint8_t cycle_total_count = 0;
    float avg_rms = 0.0f;

    total_rms += rms_current;
    cycle_count++;
    if (cycle_count >= 3)
    {
        cycle_total_count ++;
        avg_rms = total_rms / cycle_count;
        total_rms = 0.0f;
        cycle_count = 0;
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
                /* Pre-check passed, enable self-test IO */
                SetRcmuChkState(1);
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_SELF_CHK;
                _gLeakDetector.self_test_fault_count = 0;
            }
            else
            {
                _gLeakDetector.result.self_test_passed = -1;            /* Self-test failed */
//                _gLeakDetector.result.fault_flag = true;
//                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.self_test_fault_count = 0;
            }
        }
    }
}

/**
 * @brief Self-check with test IO on
 */
static void _LeakCurrSelfChk(float current_rms)
{
    static uint8_t self_test_cont = 0;
    self_test_cont++;
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
        _gLeakDetector.result.self_test_passed = 1;            /* Self-test passed */
        _gLeakDetector.self_test_success_count = 0;
        self_test_cont = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_DELAY;
        /* Disable self-test IO */
        SetRcmuChkState(0);
        return;
    }
    if(self_test_cont >= 7)
    {
        /* Self-test timeout */
        self_test_cont = 0;
        _gLeakDetector.self_test_success_count = 0;
//        _gLeakDetector.result.fault_flag = true;
        _gLeakDetector.result.self_test_passed = -1;
//        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
        SetRcmuChkState(0);
    }
}

/**
 * @brief Initialization delay before normal run
 * @param cont Delay count threshold
 */
static void _LeakCurrInitDelay(uint16_t cont)
{
    static uint16_t delay_test_cont = 0;
    delay_test_cont++;
    if(delay_test_cont >= cont)
    {
        delay_test_cont = 0;
        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_NOMAL;

        /* Clear step-detection history */
        for (int i = 0; i < 3; i++)
        {
            _gLeakDetector.history_60ma[i] = 0.0f;
            _gLeakDetector.history_30ma[i] = 0.0f;
        }
    }
}

/**
 * @brief Normal leak current monitoring
 * @param current_rms Cycle RMS current (mA)
 */
static void _LeakCurrNomalRun(float current_rms)
{
    static uint8_t rms_avg_cont = 0;
    static float total_rms_current = 0;
    total_rms_current += current_rms;
    rms_avg_cont++;
//    if (current_rms > LEAK_CURRENT_FAULT_THRESHOLD_THREE)
//    {
//        _gLeakDetector.result.fault_flag = true;
//        _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
//        return;
//    }
    /* ~300mA level-2 fault detection */
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
        _gLeakDetector.result.leak_type = 4;  /* Sudden leak type: ~300mA */
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
 * @brief 150mA sudden-change detection
 */
static void _LeakCurr150maSaltateChk(float current_avg)
{
    if (current_avg > _gLeakDetector.history_150ma[2])
    {  /* Current value > value before previous */
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_150ma[2]+LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD)*(_gLeakDetector.history_150ma[2]+LEAK_CURRENT_SQUARE_DIFF_150MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_150ma_count++;

            if (_gLeakDetector.square_diff_150ma_count >= LEAK_CURRENT_SQUARE_DIFF_150MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 3;  /* Sudden leak type: 150mA */
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.square_diff_150ma_count = 0;
                return;
            }
        }
        else
        {
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_150ma, current_avg);
            _gLeakDetector.square_diff_150ma_count = 0;
        }
    }
    else
    {
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_150ma, current_avg);
        _gLeakDetector.square_diff_150ma_count = 0;
    }
}

/**
 * @brief 60mA sudden-change detection
 */
static void _LeakCurr60maSaltateChk(float current_avg)
{
    if (current_avg > _gLeakDetector.history_60ma[2])
    {  /* Current value > value before previous */
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_60ma[2]+LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD)*(_gLeakDetector.history_60ma[2]+LEAK_CURRENT_SQUARE_DIFF_60MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_60ma_count++;

            if (_gLeakDetector.square_diff_60ma_count >= LEAK_CURRENT_SQUARE_DIFF_60MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 2;  /* Sudden leak type: 60mA */
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                _gLeakDetector.square_diff_60ma_count = 0;
                return;
            }
        }
        else
        {
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_60ma, current_avg);
            _gLeakDetector.square_diff_60ma_count = 0;
        }
    }
    else
    {
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_60ma, current_avg);
        _gLeakDetector.square_diff_60ma_count = 0;
    }
}

/**
 * @brief 30mA sudden-change detection
 */
static void _LeakCurr30maSaltateChk(float current_avg)
{

    if (current_avg > _gLeakDetector.history_30ma[2])
    {
        /* Current value > value before previous */
        float square = current_avg * current_avg;
        if (square > ((_gLeakDetector.history_30ma[2]+LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD)*(_gLeakDetector.history_30ma[2]+LEAK_CURRENT_SQUARE_DIFF_30MA_THRESHOLD)))
        {
            _gLeakDetector.square_diff_30ma_count++;

            if (_gLeakDetector.square_diff_30ma_count >= LEAK_CURRENT_SQUARE_DIFF_30MA_COUNT)
            {
                _gLeakDetector.result.fault_flag = true;
                _gLeakDetector.result.leak_type = 1;  /* Sudden leak type: 30mA */
                _gLeakDetector.current_state = EN_LEAK_CURR_STATE_FAULT;
                return;
            }
        }
        else
        {
            LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_30ma, current_avg);
            _gLeakDetector.square_diff_30ma_count = 0;
        }
    }
    else
    {  /* Current value <= value before previous */
        LEAK_CURRENT_UPDATE_HISTORY_VALUES(_gLeakDetector.history_30ma, current_avg);
        _gLeakDetector.square_diff_30ma_count = 0;
    }
}

/**
 * @brief Request leak current self-test
 */
void LeakCurrRequestSelfTest(void)
{
    _gLeakDetector.self_test_requested = true;
}

/**
 * @brief Clear fault and trigger re-test
 */
bool LeakCurrFaultClare(void)
{
    if (_gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        _gLeakDetector.fault_cleared = true;
        _gLeakDetector.self_test_requested = true;
        return true;
    }
    return false;
}

/**
 * @brief Get self-test state: 0 idle, 1 pass, -1 fail
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
 * @brief Leak current process, called from 20kHz ISR
 * @param leak_current_ma Raw leak current (mA)
 * @param samp_freq Sample frequency (Hz)
 * @param inverter_freq Inverter frequency (Hz)
 */
void LeakCurrProcess(float leak_current_ma, uint32_t samp_freq, uint32_t inverter_freq)
{
    bool     rms_value_update_flag = false;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        _LeakCurrInit();
        Mid_Filter1OrderButter_Init(&_gLowfilter_1order, LEAK_CURRENT_LPF_CUTOFF_HZ, LEAK_CURRENT_SAMPLE_FREQ_HZ);
        first_run_flag = false;
    }
    /* Handle fault clear request */
    if (_gLeakDetector.fault_cleared && _gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        if(_gLeakDetector.result.self_test_passed == 1)
        {
            _gLeakDetector.current_state = EN_LEAK_CURR_STATE_RUN_DELAY;
            _gLeakDetector.result.fault_flag = false;
        }
        else
        {
            _LeakCurrInit();
        }
        _gLeakDetector.fault_cleared = false;
        return;
    }
    /* Handle self-test request */
    if (_gLeakDetector.self_test_requested && _gLeakDetector.current_state != EN_LEAK_CURR_STATE_PRE_CHK &&
        _gLeakDetector.current_state != EN_LEAK_CURR_STATE_SELF_CHK)
    {
        _LeakCurrInit();
        _gLeakDetector.self_test_requested = false;
        return;
    }

    /* 500Hz first-order Butterworth LPF */
    _gLeakDetector.result.instant_curr_ma = Mid_Filter1OrderButter_Update(&_gLowfilter_1order, leak_current_ma);
    /* Samples per AC cycle, clamp inverter freq to 30~70Hz, default 50Hz */
    _gLeakDetector.cycle_sample_cont = samp_freq / ((inverter_freq > 0) ? ((inverter_freq < 30) ? 30 : ((inverter_freq > 70) ? 70 : inverter_freq)) :50);
    /* Calculate leak current RMS */
    rms_value_update_flag = _RmsCalculate(_gLeakDetector.result.instant_curr_ma,_gLeakDetector.cycle_sample_cont,&_gLeakDetector.result.rms_current_ma);

    if (_gLeakDetector.current_state == EN_LEAK_CURR_STATE_FAULT)
    {
        return;
    }

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
