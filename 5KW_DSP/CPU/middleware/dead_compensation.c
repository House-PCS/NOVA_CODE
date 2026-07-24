#include "dead_compensation.h"
#include "inverter_loop_cntl.h"

#define COFF_RAMP 0.0265957f 

static float _verro_p_p_compute(float ip_p,float ith,float verror_th)
{
    float ramp = verror_th / ith;
    float error = 0.0f;
    if(ip_p >= 0)
    {
        error = -verror_th;
    }
    else if(ip_p < -ith)
    {
        error = 0.0f;
    }
    else
    {
        error = -(ip_p + ith) * ramp;
    }
    return error;
}
static float _verro_p_n_compute(float ip_n,float ith,float verror_th)
{
    float ramp = verror_th / ith;
    float error = 0.0f;
    if(ip_n >= 0)
    {
        error = 0.0f;
    }
    else if(ip_n < -ith)
    {
        error = verror_th;
    }
    else
    {
        error = - ip_n * ramp;
    }
    return error;
}
static float _verro_n_p_compute(float in_p,float ith,float verror_th)
{
    float ramp = verror_th / ith;
    float error = 0.0f;
    if(in_p < 0)
    {
        error = verror_th;
    }
    else if(in_p >= ith)
    {
        error = 0.0f;
    }
    else
    {
        error = - (in_p - ith) * ramp;
    }
    return error;
}
static float _verro_n_n_compute(float in_n,float ith,float verror_th)
{
    float ramp = verror_th / ith;
    float error = 0.0f;
    if(in_n < 0)
    {
        error = 0.0f;
    }
    else if(in_n >= ith)
    {
        error = -verror_th;
    }
    else
    {
        error = -in_n * ramp;
    }
    return error;
}

/**
* @brief: dead_compensation
* @param: ilf 电感电流采样值
* @param: duty_in 控制环路输出占空比
* @param: vbus 逆变器母线电压
* @param: vo 逆变器输出电压
* @return: dead_compensation
* @note: 
*
*/
float dead_compensation(float ilf, float duty_in, float vbus, float vo)
{
    float error1 = 0;
    float error2 = 0;
    float error_amp_n = 0.0f;
    float error_amp_p = 0.0f;
    float ip_lf = 0.0f;
    float in_lf = 0.0f;
    float i_th = 1.0f;
    float verror_th = 0.01f;

    float duty_compensation = 0;
    if(duty_in >= 0)
    {
        ip_lf = ilf + COFF_RAMP * duty_in * (vbus - vo);
        in_lf = ip_lf - 2 * COFF_RAMP * (1 - duty_in) * vo;
        error_amp_p = _verro_p_p_compute(ip_lf,i_th,verror_th);
        error_amp_n = _verro_p_n_compute(in_lf,i_th,verror_th);
    }
    else
    {
        ip_lf = ilf + COFF_RAMP * duty_in * (vbus + vo);
        in_lf = ip_lf - 2 * COFF_RAMP * (1 + duty_in) * vo;
        error_amp_p = _verro_n_p_compute(ip_lf,i_th,verror_th);
        error_amp_n = _verro_n_n_compute(in_lf,i_th,verror_th);
    }
    error2 = error_amp_p + error_amp_n;
    duty_compensation = -error2;
    return duty_compensation; 
}


