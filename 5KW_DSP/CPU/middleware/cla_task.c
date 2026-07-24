/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cla_task.cla
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-30
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author
  * <tr><td>2024-8-30     <td>V1.0.0    <td>jindongze
  * </table>
  ******************************************************************************
  */
#include "cla_algo.h"
#include "CLAmath.h"
void PllCla(SPLL_1ph_F* v, float32 input)
{
    v->AC_input = input;
    if(v->AC_input > 0.999)
        v->AC_input = 0.999;
    if(v->AC_input < -0.999)
        v->AC_input = -0.999;
        /* Phase Detect*/
    v->Upd[0] = (v->AC_input * v->cos[1]);
    /* Notch Filter*/
    v->ynotch[0] = -(v->notch_coeff.A1_notch * v->ynotch[1]) - (v->notch_coeff.A2_notch * v->ynotch[2]) + (v->notch_coeff.B0_notch * v->Upd[0]) + (v->notch_coeff.B1_notch * v->Upd[1]) + (v->notch_coeff.B2_notch * v->Upd[2]);
    /* Update Upd Array for future use*/
    v->Upd[2] = v->Upd[1];
    v->Upd[1] = v->Upd[0];
    /* LPF*/
    v->ylf[0] = -(v->lpf_coeff.A1_lf * v->ylf[1]) + (v->lpf_coeff.B0_lf * v->ynotch[0]) + (v->lpf_coeff.B1_lf * v->ynotch[1]);
    if(v->ylf[0] > 150)
    {
        v->ylf[0] = 150;
    }
    if(v->ylf[0] < -150)
    {
        v->ylf[0] = -150;
    }
    /* Update Array for future use*/
    v->ynotch[2] = v->ynotch[1];
    v->ynotch[1] = v->ynotch[0];
    v->ylf[1] = v->ylf[0];
    /*VCO*/
    v->wo = v->wn + v->ylf[0];
    /* integration process */
    v->sin[0] = v->sin[1] + (((v->delta_t * v->wo)) * v->cos[1]);
    v->cos[0] = v->cos[1] - (((v->delta_t * v->wo)) * v->sin[1]);
    if(v->sin[0] > (float32)(0.99))
    {
        v->sin[0] = (float32)(0.99);
    }
    else if(v->sin[0] < (float32)(-0.99))
    {
        v->sin[0] = (float32)(-0.99);
    }
    if(v->cos[0] > (float32)(0.99))
    {
        v->cos[0] = (float32)(0.99);
    }
    else if(v->cos[0] < (float32)(-0.99))
    {
        v->cos[0] = (float32)(-0.99);
    }
    v->theta[0] = v->theta[1] + ((v->wo) * v->delta_t);
    if(v->sin[0] > (float32)(0.0) && v->sin[1] <= (float32)(0.0))
    {
        v->theta[0] = (float32)(0.0);
    }
    v->theta[1] = v->theta[0];
    v->sin[1] = v->sin[0];
    v->cos[1] = v->cos[0];


}

void RmsClaRun(RmsCla_t* v, float32 input)
{
    v->vin = input;
    if(v->vin > v->threshold)
    {
        v->curr_sample_norm = v->vin;
        v->curr_sign = 1;
    }
    else
    {
        v->curr_sample_norm = fabsf(v->vin);
        v->curr_sign = 0;
    }

    if((v->prev_sign != v->curr_sign) && (v->curr_sign == 1))
    {
        v->zcd_count++;
        if(v->nsamples_min < v->nsamples && v->nsamples < v->nsamples_max)
        {
            v->inv_sqrt_nsamples = (1.0) / CLAsqrt(v->nsamples);
            v->vrms = (CLAsqrt(v->vacc_rms) * v->inv_sqrt_nsamples);
            v->sig_freq = (v->sample_freq / v->nsamples) * v->zcd_count;
            v->prev_sign = v->curr_sign;
            v->vacc_rms = 0;
            v->nsamples = 0;
            v->zcd_count = 0;
        }
    }
    if(v->nsamples > v->nsamples_max)
    {
        v->vrms = 0;
        v->sig_freq = 0;
        v->vacc_avg = 0;
        v->vacc_rms = 0;
        v->nsamples = 0;
    }
    v->nsamples = v->nsamples + 1;
    v->vacc_rms = v->vacc_rms + (v->curr_sample_norm * v->curr_sample_norm);
    v->prev_sign = v->curr_sign;
}
void RmsPwrClaCalcStep(PwrClaCompute_t* data, float32 vol_input, float cur_input, float power_refactor)
{
    int32_t sign = 1;
    float32 isnsamples = 0.0f;
    data->vol_in = vol_input;
    data->cur_in = cur_input;
    if(power_refactor < 0)
    {
        sign = -1;
    }
    
    data->sign = sign;

    data->vol_sample_norm = data->vol_in * data->vol_in;
    data->cur_sample_norm = data->cur_in * data->cur_in;
    data->pwr_sample_norm = data->vol_in * data->cur_in;
    if(data->vol_in > data->threshold)
    {
        data->curr_sign = 1;
    }
    else
    {
        data->curr_sign = 0;
    }
    
    if((data->nsamples > data->nsamples_max) || (data->compute_complete == 1))
    {
        data->pwr_acc_reactive = CLAsqrt(data->pwr_acc_reactive_square);
        // data->sig_freq = 0;
        data->vol_acc_avg = 0;
        data->vol_acc_rms = 0;
        data->cur_acc_avg = 0;
        data->cur_acc_rms = 0;
        data->pwr_acc_avg = 0;
        data->pwr_acc_rms = 0;
        data->nsamples = 0;
        data->zcd_count= 0;
        data->compute_complete = 0;
        data->pwr_reactive_out = power_refactor * data->pwr_apperant_out;
    }
    if((data->prev_sign != data->curr_sign) && (data->curr_sign == 1) || (data->nsamples >= data->nsamples_max))
    {
        data->zcd_count++;
        if(data->nsamples_min < data->nsamples && data->nsamples <= data->nsamples_max)
        {
//            data->nsamples = 400 * 5; // 20260316

            isnsamples = 1.0f / data->nsamples; 
            data->inv_sqrt_nsamples = CLAisqrt(data->nsamples);
            data->vol_acc_rms_sqrt = CLAsqrt(data->vol_acc_rms); 
            data->cur_acc_rms_sqrt = CLAsqrt(data->cur_acc_rms);
            data->sig_freq = (data->sample_freq * isnsamples) * data->zcd_count;
            data->prev_sign = data->curr_sign;
            data->vol_rms = data->inv_sqrt_nsamples * data->vol_acc_rms_sqrt;
            data->cur_rms = data->inv_sqrt_nsamples * data->cur_acc_rms_sqrt;
            data->pwr_active_out = data->pwr_acc_rms * isnsamples;
            data->pwr_apperant_out = data->vol_rms * data->cur_rms;
            data->pwr_acc_reactive_square = data->pwr_apperant_out * data->pwr_apperant_out - data->pwr_active_out * data->pwr_active_out;
            data->vol_avg = data->vol_acc_avg * isnsamples;
            data->cur_avg = data->cur_acc_avg * isnsamples;
            data->compute_complete = 1;
        }
    }
    data->nsamples = data->nsamples + 1;
    data->vol_acc_rms = data->vol_acc_rms + data->vol_sample_norm;
    data->cur_acc_rms = data->cur_acc_rms + data->cur_sample_norm;
    data->pwr_acc_rms = data->pwr_acc_rms + data->pwr_sample_norm;
    data->vol_acc_avg = data->vol_acc_avg + data->vol_in;
    data->cur_acc_avg = data->cur_acc_avg + data->cur_in;
    data->prev_sign = data->curr_sign;
}

void RmsPwrClaInit50Hz(PwrClaCompute_t* data)
{
    data->sample_freq = 20000;
    data->threshold = 0;
    data->nsamples_max = data->sample_freq / (40) * 5;
    data->nsamples_min = data->sample_freq / (60) * 5;
}
#include "algo_info.h"

void ClaTask(void)
{
    PllCla(&gGridVolPll, gAlgoMsg.grid_vol / gstGridAnlz.vol_rms / 1.414f);
    PllCla(&gGridCurPll, gAlgoMsg.grid_cur / gstGridAnlz.cur_rms / 1.414f);
    PllCla(&gInvVolPll, gAlgoMsg.inv_vol / gstInvAnlz.vol_rms / 1.414f);
    PllCla(&gInvCurPll, gAlgoMsg.inv_cur / gstInvAnlz.cur_rms / 1.414f);
    PllCla(&gCtCurPll, gAlgoMsg.ct_cur / gstCtAnlz.cur_rms / 1.414f);
    PllCla(&gLoadCurPll, gAlgoMsg.load_cur / gstLoadAnlz.cur_rms / 1.414f);

    float32 CT_power_factor = gGridVolPll.cos[0] * gCtCurPll.cos[0] + gGridVolPll.sin[0] * gCtCurPll.sin[0];
    float32 CT_power_refactor = gGridVolPll.sin[0] * gCtCurPll.cos[0] - gGridVolPll.cos[0] * gCtCurPll.sin[0];
    float32 grid_power_factor = gGridVolPll.cos[0] * gGridCurPll.cos[0] + gGridVolPll.sin[0] * gGridCurPll.sin[0];
    float32 grid_power_refactor = gGridVolPll.sin[0] * gGridCurPll.cos[0] - gGridVolPll.cos[0] * gGridCurPll.sin[0];
    float32 inv_power_factor = gInvVolPll.cos[0] * gInvCurPll.cos[0] + gInvVolPll.sin[0] * gInvCurPll.sin[0];
    float32 inv_power_refactor = gInvVolPll.sin[0] * gInvCurPll.cos[0] - gInvVolPll.cos[0] * gInvCurPll.sin[0];
    float32 load_power_factor = -(gInvVolPll.cos[0] * gLoadCurPll.cos[0] + gInvVolPll.sin[0] * gLoadCurPll.sin[0]);  //Load_Cur与电压反相
    float32 load_power_refactor = -(gInvVolPll.sin[0] * gLoadCurPll.cos[0] - gInvVolPll.cos[0] * gLoadCurPll.sin[0]); //Load_Cur与电压反相

    RmsPwrClaCalcStep(&gstGridAnlz, gAlgoMsg.grid_vol, gAlgoMsg.grid_cur, grid_power_refactor);
    RmsPwrClaCalcStep(&gstInvAnlz, gAlgoMsg.inv_vol, gAlgoMsg.inv_cur, inv_power_refactor);
    RmsPwrClaCalcStep(&gstLoadAnlz, gAlgoMsg.load_vol, gAlgoMsg.load_cur, load_power_refactor);
    RmsPwrClaCalcStep(&gstCtAnlz, gAlgoMsg.grid_vol, gAlgoMsg.ct_cur, CT_power_refactor);
    // RmsClaRun(&gGridVolAnlz, gAlgoMsg.grid_vol);
    // RmsClaRun(&gGridCurAnlz, gAlgoMsg.grid_cur);
    // RmsClaRun(&gInvVolAnlz, gAlgoMsg.inv_vol);
    // RmsClaRun(&gInvCurAnlz, gAlgoMsg.inv_cur);
    // RmsClaRun(&gLoadVolAnlz, gAlgoMsg.load_vol);
    // RmsClaRun(&gLoadCurAnlz, gAlgoMsg.load_cur);
    // RmsClaRun(&gCtCurAnlz, gAlgoMsg.ct_cur);

//    gGridVolPll.sin[0]=gGridVolRms.vrms;
}

__attribute__((interrupt)) void Cla1Task1(void)
{
    ClaTask();
}

void RmsClaInit(RmsCla_t* v)
{
    v->sample_freq = 20000;
    v->threshold = 0;
    v->nsamples_max = v->sample_freq / 40 * 5;
    v->nsamples_min = v->sample_freq / 50 * 5;
}

void PllClaInit(int Grid_freq, float32 DELTA_T, SPLL_1ph_F* spll_obj)
{
    spll_obj->Upd[0] = (float32)(0.0);
    spll_obj->Upd[1] = (float32)(0.0);
    spll_obj->Upd[2] = (float32)(0.0);

    spll_obj->ynotch[0] = (float32)(0.0);
    spll_obj->ynotch[1] = (float32)(0.0);
    spll_obj->ynotch[2] = (float32)(0.0);

    spll_obj->ylf[0] = (float32)(0.0);
    spll_obj->ylf[1] = (float32)(0.0);

    spll_obj->sin[0] = (float32)(0.0);
    spll_obj->sin[1] = (float32)(0.0);

    spll_obj->cos[0] = (float32)(0.999);
    spll_obj->cos[1] = (float32)(0.999);

    spll_obj->theta[0] = (float32)(0.0);
    spll_obj->theta[1] = (float32)(0.0);

    spll_obj->wn = (float32)(2 * 3.14 * Grid_freq);

    // loop filter coefficients for 20kHz
    spll_obj->lpf_coeff.B0_lf = (float32)(166.9743);
    spll_obj->lpf_coeff.B1_lf = (float32)(-166.266);
    spll_obj->lpf_coeff.A1_lf = (float32)(-1.0);

    spll_obj->delta_t = DELTA_T;
}

void PllClaCoeffUpdate(float32 delta_T, float32 wn, float32 c2, float32 c1, SPLL_1ph_F* spll_obj)
{
    // Note c2<<c1 for the notch to work
    float32 x, y, z;
    x = (float32)(2.0 * c2 * wn * delta_T);
    y = (float32)(2.0 * c1 * wn * delta_T);
    z = (float32)(wn * delta_T * wn * delta_T);

    spll_obj->notch_coeff.A1_notch = (float32)(y - 2);
    spll_obj->notch_coeff.A2_notch = (float32)(z - y + 1);
    spll_obj->notch_coeff.B0_notch = (float32)(1.0);
    spll_obj->notch_coeff.B1_notch = (float32)(x - 2);
    spll_obj->notch_coeff.B2_notch = (float32)(z - x + 1);
}

__attribute__((interrupt)) void Cla1Task2(void)
{
    // RmsClaInit(&gGridVolAnlz);
    RmsClaInit(&gGridVolPllAnlz);
    // RmsClaInit(&gInvVolAnlz);
    // RmsClaInit(&gLoadVolAnlz);
    // RmsClaInit(&gGridCurAnlz);
    // RmsClaInit(&gInvCurAnlz);
    // RmsClaInit(&gLoadCurAnlz);
    // RmsClaInit(&gCtCurAnlz);
    RmsPwrClaInit50Hz(&gstGridAnlz);
    RmsPwrClaInit50Hz(&gstInvAnlz);
    RmsPwrClaInit50Hz(&gstLoadAnlz);
    RmsPwrClaInit50Hz(&gstCtAnlz);

    PllClaInit(50, 0.00005, &gGridVolPll);
    PllClaInit(50, 0.00005, &gInvVolPll);
    PllClaInit(50, 0.00005, &gGridCurPll);
    PllClaInit(50, 0.00005, &gInvCurPll);
    PllClaInit(50, 0.00005, &gCtCurPll);
    PllClaInit(50, 0.00005, &gLoadCurPll);
    PllClaCoeffUpdate(0.00005, 200 * 3.1415, 0.00001, 1.0, &gGridVolPll);
    PllClaCoeffUpdate(0.00005, 200 * 3.1415, 0.00001, 1.0, &gInvVolPll);
    PllClaCoeffUpdate(0.00005, 200 * 3.1415, 0.00001, 1.0, &gGridCurPll);
    PllClaCoeffUpdate(0.00005, 200 * 3.1415, 0.00001, 1.0, &gInvCurPll);
    PllClaCoeffUpdate(0.00005, 200 * 3.1415, 0.00001, 1.0, &gCtCurPll);
    PllClaCoeffUpdate(0.00005, 200 * 3.1415, 0.00001, 1.0, &gLoadCurPll);
}

__attribute__((interrupt)) void Cla1Task3(void)
{

}

__attribute__((interrupt)) void Cla1Task4(void)
{

}

__attribute__((interrupt)) void Cla1Task5(void)
{

}

__attribute__((interrupt)) void Cla1Task6(void)
{

}

__attribute__((interrupt)) void Cla1Task7(void)
{

}

__attribute__((interrupt)) void Cla1Task8(void)
{

}
