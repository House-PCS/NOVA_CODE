/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dc_injection.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-11-27
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-2-4    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  **/
#include"dc_injection.h"
#define DELTA_AVG 0.0001f
#define DC_DUTY_MAX 0.01f
#define DC_DUTY_MIN -0.01f

static float32 _gDutyAvg = 0.0f;
float32 DcRejection(float32 vol_avg, float32 cur_avg)
{
    static float32 cur_avg_ref = 0.0f; //直流分量目标值为0A
    static float32 delta_avg = DELTA_AVG;
    static float32 cur_avg_old = 0.0f;
    float32 duty_avg_out = 0.0f;
    if(cur_avg_old != cur_avg)
    {
        _gDutyAvg = _gDutyAvg + delta_avg * (cur_avg_ref - cur_avg);
    }
    cur_avg_old = cur_avg;
    //限幅
    if(_gDutyAvg > DC_DUTY_MAX)
    {
        _gDutyAvg = DC_DUTY_MAX;
    }
    else if(_gDutyAvg < DC_DUTY_MIN)
    {
        _gDutyAvg = DC_DUTY_MIN;
    }
    duty_avg_out = _gDutyAvg;
    return duty_avg_out;
}
void ResetDutyAvg(void)
{
    _gDutyAvg = 0.0f;
}
