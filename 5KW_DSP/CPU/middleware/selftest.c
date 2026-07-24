

/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        selftest.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-11-04
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-3-19    <td>V1.0.0    <td>xingxiao    <td>InitialVersion
  * </table>
  ******************************************************************************
  */
 #include "selftest.h"
 #include "dev_sensor.h"
uint16_t gZeroDriftState = 0;
static uint16_t _gStartZeroDriftFlag = 0;
#define CURRENT_PRESISION 1.0f
#define PV_CURRENT_PRESISION 0.8f
static uint16_t _read_zero_drift_correct()
{
    return gZeroDriftState;
}
static void _start_zero_drift_correct()
{
    if(_read_zero_drift_correct() == ZERODRIFTING)
    {
        _gStartZeroDriftFlag = 1;
    }
    
}
void ZeroDriftCorrect()
{
    static float inductor_cur_add = 0;
    static float grid_cur_add = 0;
    static float load_cur_add = 0;
    static float pv1_cur_add = 0;
    static float pv2_cur_add = 0;
    float inductor_cur_ave = 0;
    float grid_cur_ave = 0;
    float load_cur_ave = 0;
    float pv1_cur_ave = 0;
    float pv2_cur_ave = 0;
    static uint16_t count = 0;
    if(_gStartZeroDriftFlag)
    {
        count++;
        inductor_cur_add += gstSensorData.inductor_current; 
        grid_cur_add += gstSensorData.power_grid_current; 
        load_cur_add += gstSensorData.load_current; 
        // pv1_cur_add += gstSensorData.pv1_current;
        // pv2_cur_add += gstSensorData.pv2_current;
        if(count > 20000)
        {
            _gStartZeroDriftFlag = 0;
            inductor_cur_ave = inductor_cur_add / 20000.0f;
            grid_cur_ave = grid_cur_add / 20000.0f;
            load_cur_ave = load_cur_add / 20000.0f;
            // pv1_cur_ave = pv1_cur_add / 20000.0f;
            // pv2_cur_ave = pv2_cur_add / 20000.0f;
            if((fabs(inductor_cur_ave) < CURRENT_PRESISION) && (fabs(grid_cur_ave) < CURRENT_PRESISION) && fabs(load_cur_ave < CURRENT_PRESISION))
            {
                gstSenorDrift.inductor_cur_drift = inductor_cur_ave;
                gstSenorDrift.grid_cur_drift = grid_cur_ave;
                gstSenorDrift.load_cur_drift = load_cur_ave;
                // gstSenorDrift.pv1_cur_drift = pv1_cur_ave;  //接口
                // gstSenorDrift.pv2_cur_drift = pv2_cur_ave;  //接口
                gstSenorDrift.pv1_cur_drift = 0.0f;  //接口
                gstSenorDrift.pv2_cur_drift = 0.0f;  //接口
                if(gstSenorDrift.pv1_cur_drift < - PV_CURRENT_PRESISION || gstSenorDrift.pv1_cur_drift > PV_CURRENT_PRESISION)
                {
                    gstSenorDrift.pv1_cur_drift = 0;  //接口较正失败
                }
                if(gstSenorDrift.pv2_cur_drift < - PV_CURRENT_PRESISION || gstSenorDrift.pv2_cur_drift > PV_CURRENT_PRESISION)
                {
                    gstSenorDrift.pv2_cur_drift = 0;  //接口较正失败
                }
                gZeroDriftState = ZERODRIFTOK;
            }
            else
            {
                gZeroDriftState = ZERODRIFTERR;
            }
        }
        else
        {
            gZeroDriftState = ZERODRIFTING;
        }
    }
    else
    {
        count = 0;
        inductor_cur_add = 0.0;
        grid_cur_add = 0.0;
        load_cur_add = 0.0;
    }
}
void SelfTest()
{
    _start_zero_drift_correct(); //TODO:需要修改逻辑
}
uint16_t ReadSelfTestState()
{
    uint16_t state = SELFTESTING;
    if(_read_zero_drift_correct() == ZERODRIFTING)
    {
        state = SELFTESTING;
    }
    else if(_read_zero_drift_correct() == ZERODRIFTOK)
    {
        state = ZERODRIFTOK;
    }
    else
    {
        state = ZERODRIFTERR;
    }
    return state;
}
