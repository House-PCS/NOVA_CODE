/*
 * ct.c
 *
 *  Created on: 2025-11-17
 *      Author: Nova006348
 */

#include "ct.h"
#include "string.h"
#include <math.h>
#include <sample_update.h>
#include "ph3_calc_ifc_update.h"

 CtDataInfo_t  gCtDataInfo;
int16  _gCtPolarity[3];                        /* Default CT polarity aligned with current direction */
CtSlideWindow_t _gCtDataProcess[3];
bool ct_break_result[3] = {false,false,false};
/**
* @brief: CtParamInit()
* @param: nothing
* @return:nothing
* @note: CT data initialization
*/
static void _gCtParamInit(void)
{
    memset(&gCtDataInfo, 0, sizeof(gCtDataInfo));
    memset(&_gCtDataProcess, 0,  sizeof(_gCtDataProcess));
    _gCtPolarity[0] = 1;
    _gCtPolarity[1] = 1;
    _gCtPolarity[2] = 1;
}
/**
* @brief: CtAutoCorrentPower()
* @param: nothing
* @return:nothing
* @note: CT auto polarity detection; not suitable when remote load or parallel units are present
*/
void CtAutoCorrentPower(void)
{
    if(gstPh3CalcInfo.grid_power.a_phase_active * gstPh3CalcInfo.ct_power.a_phase_active < 0)
    {
        _gCtPolarity[0] = -1;
    }
    else
    {
        _gCtPolarity[0] = 1;
    }

    if(gstPh3CalcInfo.grid_power.b_phase_active * gstPh3CalcInfo.ct_power.b_phase_active < 0)
    {
        _gCtPolarity[1] = -1;
    }
    else
    {
        _gCtPolarity[1] = 1;
    }

    if(gstPh3CalcInfo.grid_power.c_phase_active * gstPh3CalcInfo.ct_power.c_phase_active < 0)
    {
        _gCtPolarity[2] = -1;
    }
    else
    {
        _gCtPolarity[2] = 1;
    }
}
/**
* @brief: CtReversePower()
* @param: nothing
* @return:nothing
* @note: CT polarity inversion
*/
void CtReversePower(int ct_inverse)
{
    static int en_inverse = 1;
    static bool first_run_flag = true;
    if(first_run_flag)
    {
        first_run_flag = false;
        _gCtParamInit();
    }
    if(ct_inverse)
    {
        en_inverse = -1;
    }
    else
    {
        en_inverse = 1;
    }
    gCtDataInfo.ct_power.a_phase_active = gstPh3CalcInfo.ct_power.a_phase_active * _gCtPolarity[0] * en_inverse;
    gCtDataInfo.ct_power.b_phase_active = gstPh3CalcInfo.ct_power.b_phase_active * _gCtPolarity[1] * en_inverse;
    gCtDataInfo.ct_power.c_phase_active = gstPh3CalcInfo.ct_power.c_phase_active * _gCtPolarity[2] * en_inverse;
    gCtDataInfo.ct_power.total_active = gCtDataInfo.ct_power.a_phase_active  + gCtDataInfo.ct_power.b_phase_active + gCtDataInfo.ct_power.c_phase_active;

    gCtDataInfo.ct_power.a_phase_reactive = gstPh3CalcInfo.ct_power.a_phase_reactive * _gCtPolarity[0] * en_inverse;
    gCtDataInfo.ct_power.b_phase_reactive = gstPh3CalcInfo.ct_power.b_phase_reactive * _gCtPolarity[1] * en_inverse;
    gCtDataInfo.ct_power.c_phase_reactive = gstPh3CalcInfo.ct_power.c_phase_reactive * _gCtPolarity[2] * en_inverse;
    gCtDataInfo.ct_power.total_reactive = gCtDataInfo.ct_power.a_phase_reactive  + gCtDataInfo.ct_power.b_phase_reactive + gCtDataInfo.ct_power.c_phase_reactive;

    gCtDataInfo.ct_power.a_phase_apperant = gstPh3CalcInfo.ct_power.a_phase_apperant ;
    gCtDataInfo.ct_power.b_phase_apperant = gstPh3CalcInfo.ct_power.b_phase_apperant ;
    gCtDataInfo.ct_power.c_phase_apperant = gstPh3CalcInfo.ct_power.c_phase_apperant ;

    gCtDataInfo.ct_power.total_apperant = gstPh3CalcInfo.ct_power.total_apperant;
}

/**
* @brief: _gAddWindowData()
* @param: nothing
* @return:nothing
* @note: Add sample to sliding window
*/
void _gAddWindowData(CtSlideWindow_t* window, float value)
{
    /* Remove oldest sample when window is full */
    if (window->count == WINDOW_SIZE)
    {
        float old_value = window->data[window->index];
        window->sum -= old_value;
        window->square_sum -= old_value * old_value;
    }
    else
    {
        window->count++;
    }
    /* Add new sample */
    window->data[window->index] = value;
    window->sum += value;
    window->square_sum += value * value;
    /* Advance write index */
    window->index = (window->index + 1) % WINDOW_SIZE;
}
/**
* @brief: _gCalculateVariance()
* @param: nothing
* @return:nothing
* @note: Calculate variance over current window
*/
float _gCalculateVariance(CtSlideWindow_t* window)
{
    if (window->count < 2)
    {
        return 0.0;  /* At least 2 samples required */
    }

    float mean = window->sum / window->count;
    float variance = (window->square_sum / window->count) - (mean * mean);

    /* Clamp small negative values from floating-point error */
    return variance > 0 ? variance : 0.0;
}
/**
* @brief: _gCalculateStdDev()
* @param: nothing
* @return:nothing
* @note: Calculate standard deviation over current window
*/
float _gCalculateStdDev(CtSlideWindow_t* window)     /* 0.5 used as threshold */
{
    return sqrt(_gCalculateVariance(window));
}
/**
* @brief: CtbreakJudge()
* @param: nothing
* @return:nothing
* @note: Detect CT wire-break fault
*/
bool* CtbreakJudge(void)
{
    static uint16_t cont = 0;
    float ct_std_dev[3] ;
    if(cont == 0)
    {
        _gAddWindowData(&_gCtDataProcess[0],gstAdcResult.ct_a_curr);
        ct_std_dev[0] = _gCalculateStdDev(&_gCtDataProcess[0]);

        if(ct_std_dev[0] < CT_BREAK_THRESHOLD)
        {
            ct_break_result[0] = true;
        }
        else
        {
            ct_break_result[0] = false;
        }
    }
    else if(cont == 1)
    {
        _gAddWindowData(&_gCtDataProcess[1],gstAdcResult.ct_b_curr);
        ct_std_dev[1] = _gCalculateStdDev(&_gCtDataProcess[1]);

        if(ct_std_dev[1] < CT_BREAK_THRESHOLD)
        {
            ct_break_result[1] = true;
        }
        else
        {
            ct_break_result[1] = false;
        }
    }
    else
    {
        _gAddWindowData(&_gCtDataProcess[2],gstAdcResult.ct_c_curr);
        ct_std_dev[2] = _gCalculateStdDev(&_gCtDataProcess[2]);

        if(ct_std_dev[2] < CT_BREAK_THRESHOLD)
        {
            ct_break_result[2] = true;
        }
        else
        {
            ct_break_result[2] = false;
        }
    }
    cont ++ ;
    if(cont >=3)
    {
        cont = 0;
    }

    return ct_break_result;
}
