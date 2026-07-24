/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        mppt_process.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-3-19
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


#include "mppt_process.h"
#include "algorithm_interface.h"
#include "algo_info.h"
#include "pwm_cntl.h"
#include "algo_err_check.h"
#include "mid_comservice.h"
#include "energy_management.h"
#include "sys_info.h"
#include "FSM.h"
#include "activation.h"
#include "energy_management.h"
#define MPPT_AVE_COUNT      ((uint32_t)((uint32_t)100 * (uint32_t)20))    // 1ms = 20，100对应100ms
#define LIMIT_NO_LIMIT 2 //中间态
#define LIMIT 1
#define NO_LIMIT 0
#define DUTY_MAX 0.9
#define DUTY_MIN 0.02
#define PV_CUR_MAX 16 //PV的最大工作电流
#define PV_CUR_PROTECT 30 //PV触发关管电流
#define OPEN_CLOSE_PERIOD 1 //间歇开管周期
// #define TEST_MODE_DUTY_INIT 0.38f //对应280V下的占空比
#define TEST_MODE_DUTY_INIT 0.28f //对应320V下的占空比
#define TEST_MODE_DC_VOLTAGE 300.0f //直流源模式下PV启动阈值300V，对应320V的电源
static EN_INV_PWR_LEVEL _gePwrLevel;
#define BAT_CHG_CUR_OVER_LEAVEL1 (ALGOERR_THRESHOLD_BAT_CUR_OVER_L1-5.0f)
#define BAT_CHG_CUR_OVER_LEAVEL2 (ALGOERR_THRESHOLD_BAT_CUR_OVER_L2-5.0f)
#define BAT_CHG_CUR_OVER_LEAVEL3 (ALGOERR_THRESHOLD_BAT_CUR_OVER_L3-5.0f)

#define PWR_STATE_THRESHOD 50 //PV满功率运行和限功率状态转换阈值
static uint16_t _Mppt_Coninc_Vstep(PvData_t* pv_data, float bus_vol, EN_SOURCE_MODE_DEF source_mode);
static void _PvBoostRun(float vol_pv1, float vol_pv2, float Vbus_target, float Vbus);
static void _AverageData();
static void _PvFsmRun();
static void _MpptRun();
static float32 _garrSciBuf[16];                  //Sci发送缓冲
float gPvPwrRef = 0;
static const AlgoInfo_t* _gAlgoInfo;
static InverterState_t* _gInvSta;
static uint16_t _gTestMode = 0;
static float32 _gBatCurFilter = 0.0f;
float gPv1_Voltage_Ave = 0;
float gPv1_Current_Ave = 0;
float gPv2_Voltage_Ave = 0;
float gPv2_Current_Ave = 0;
float gBat_Voltage_Ave = 0;
float gBat_Current_Ave = 0;
float gBus_Voltage_Ave = 0;
float gGrid_Power_Ave = 0;
PvData_t gstPv1Data = {0,   0,    0,   0,
                      0,   0,  0.1, 0.1,
                      0.1, 0.1,  0.02, 0.02,
                      0,   0,    0,   0,
                      0,   0,    0,   0,    
                      0,   0,    0,   0,   0,  0};
PvData_t gstPv2Data = {0,   0,    0,   0,
                      0,   0,  0.1, 0.1,
                      0.1, 0.1,  0.02, 0.02,
                      0,   0,    0,   0,
                      0,   0,    0,   0,    
                      0,   0,    0,   0,   0,  0};

static uint32_t _gPvPowerCntHours = 0;
uint16_t gPvPowerHoursComputedFlag = 0;
uint32_t gPv1PowerTotalHours = 0;
uint32_t gPv2PowerTotalHours = 0;
uint32_t gPurchasePowerTotalHours = 0;
uint32_t gSalePowerTotalHours = 0;
uint32_t gLoadPowerTotalHours = 0;
uint32_t gNorLoadPowerTotalHours = 0;
uint32_t gBatChgPowerTotalHours = 0;
uint32_t gBatDsgPowerTotalHours = 0;


EN_PARALLEL_OFF_NETWORK_STA gePvInvSta = EN_OFF_GRID_MODE;
static EN_PV_STATE_DEF gePVFsm = EN_PV_STATE_DEF_READY;
static uint16_t _gPvRunStateFlag = 0;

static float _gPv1DutyEff = 0.0;
static float _gPv2DutyEff = 0.0;
static float _gDutyRamp = 0.0;
extern ParallelComm_t gParallelComm;

uint16_t flag_PV_over_current = 0; // 20260511,新增PV过流标志位

/**
* @brief: MpptInverter
* @param: void
* @note: 200Hz in 20kHz Inv
*
*/
static uint32_t _gMpptInvCount = 5;
static uint32_t _gMpptOpenCloseCnt = 0;
void MpptInverter(void)
{
    _AverageData();
    _PvFsmRun();
    static float pv1_power_add_hours = 0;
    static float pv2_power_add_hours = 0;
    static float purchase_power_add_hours = 0;
    static float sale_power_add_hours = 0;
    static float load_power_add_hours = 0;
    static float Norload_power_add_hours = 0;
    static float bat_chg_power_add_hours = 0;
    static float bat_dsg_power_add_hours = 0;
    static float power_cnt = 0;
//    extern StaData_t _gSta;
    if(_gMpptInvCount == 3) //交错计算，优化中断时间
    {
        _gPvPowerCntHours++;

        if(_gPvPowerCntHours == 1)//100ms统计一次
        {
            power_cnt ++;
            _gPvPowerCntHours = 0;

            if(gParallelComm.mode == 1)     //主机模式电量统计
            {
                pv1_power_add_hours += (gstPv1Data.pv_pwr + gParallelComm.slave_info.slave_pv_power); //单位为W  主机pv1 + 从机pv
                pv2_power_add_hours += gstPv2Data.pv_pwr; //单位为W   主机pv2
                load_power_add_hours += (-1 * (_gState_algo.load_active_power + gParallelComm.slave_info.slave_backup_power));  //主机 back + 从机 back
                Norload_power_add_hours += (_gState_algo.norload_active_power - gParallelComm.slave_info.slave_grid_port_power);   // 并机计算时要再减去从机并网口功率
                if(gParallelComm.slave_info.slave_bat_power >= 0)
                {
                    bat_dsg_power_add_hours += gParallelComm.slave_info.slave_bat_power;    //电池放电需加上从机
                }
                else
                {
                    bat_chg_power_add_hours += (-1 * gParallelComm.slave_info.slave_bat_power);  //电池充电需加上从机                                      
                }
            }
            else    //正常模式 or 从机模式
            {
                pv1_power_add_hours += gstPv1Data.pv_pwr; //单位为W       只算主机
                pv2_power_add_hours += gstPv2Data.pv_pwr; //单位为W       只算主机
                load_power_add_hours += (-1 * _gState_algo.load_active_power);  // 只算主机
                Norload_power_add_hours += _gState_algo.norload_active_power;   // 单机不用管从机
            }
            
//            Norload_power_add_hours += _gState_algo.norload_active_power;   // 远端负载不分主从
            if(gBatpower >= 0)
            {
                bat_dsg_power_add_hours +=  gBatpower;      // 不管是否并机模式，最后都再加上主机的
            }
            else
            {
                bat_chg_power_add_hours += (-1 * gBatpower);    // 不管是否并机模式，最后都再加上主机的
            }
            if(_gState_algo.CT_active_power >= 0)
            {
                purchase_power_add_hours += _gState_algo.CT_active_power;   // CT功率不分主从
            }
            else if(_gState_algo.CT_active_power < 0)
            {
                sale_power_add_hours += (-1 * _gState_algo.CT_active_power);    // CT功率不分主从
            }

            if(power_cnt == 600)//1min上报一次
            {
                power_cnt = 0;
                gPv1PowerTotalHours = (uint32_t)pv1_power_add_hours;
                gPv2PowerTotalHours = (uint32_t)pv2_power_add_hours;
                gPurchasePowerTotalHours = (uint32_t)purchase_power_add_hours;
                gSalePowerTotalHours = (uint32_t)sale_power_add_hours;
                gLoadPowerTotalHours = (uint32_t)load_power_add_hours;
                gBatChgPowerTotalHours = (uint32_t)bat_chg_power_add_hours;
                gBatDsgPowerTotalHours = (uint32_t)bat_dsg_power_add_hours;
                gNorLoadPowerTotalHours = (uint32_t)Norload_power_add_hours;

                pv1_power_add_hours = 0;
                pv2_power_add_hours = 0;
                purchase_power_add_hours = 0;
                sale_power_add_hours = 0;
                load_power_add_hours = 0;
                bat_chg_power_add_hours = 0;
                bat_dsg_power_add_hours = 0;
                Norload_power_add_hours = 0;
                gPvPowerHoursComputedFlag = 1; //外部读取后清0，等待下一次置位累加
            }
        }
        /* code */
    }

    if(gPvOvrCurRecoverCount < PV_RECOVER_COUNT)
    {
        flag_PV_over_current = 1;
        _gPv1DutyEff = 0;
        _gPv2DutyEff = 0;
    }
    else
    {
        flag_PV_over_current = 0;
    }

    PwmSetpv1Duty(_gPv1DutyEff);
    PwmSetpv2Duty(_gPv2DutyEff);
}

void* PvGetStateWave()
{
    // _garrSciBuf[0] = ADC_PV1_CURRENT ;
    _garrSciBuf[0] = gPv1_Voltage_Ave * 10;
    _garrSciBuf[1] = gPv1_Current_Ave * 100;
    _garrSciBuf[2] = gPv1_Voltage_Ave * gPv1_Current_Ave;
    // _garrSciBuf[2] = gstSensorData.temp[0];
    _garrSciBuf[3] = _gPv1DutyEff * 100;
    _garrSciBuf[4] = gstPv1Data.pv_pwr_out;
    _garrSciBuf[5] = gstSensorData.pv1_current * 100;
    _garrSciBuf[6] = gstPv1Data.mppt_state * 100;

    // _garrSciBuf[7] = ADC_PV2_CURRENT ;
    _garrSciBuf[7] = gPv2_Voltage_Ave * 10;
    _garrSciBuf[8] = gPv2_Current_Ave * 100;
    _garrSciBuf[9] = gPv2_Voltage_Ave * gPv2_Current_Ave;
    _garrSciBuf[10] = _gPv2DutyEff * 100;
    _garrSciBuf[11] = gstPv2Data.pv_pwr_out;
    _garrSciBuf[12] = gstSensorData.pv2_current * 100;
    _garrSciBuf[13] = gstPv2Data.mppt_state * 100;
    _garrSciBuf[14] = gPvPwrRef;

    // _garrSciBuf[6] = gstPv1Data.dP*100;
    // _garrSciBuf[7] = gstPv1Data.dV*100;
    // _garrSciBuf[8] = gstPv1Data.mppt_duty_add *10000; 
    // _garrSciBuf[9] = gstSensorData.pv1_voltage * 10;
    // _garrSciBuf[10] = gstSensorData.pv1_current * 100;
    // _garrSciBuf[11] = gstPv1Data.duty_eff*100;
    // _garrSciBuf[12] = (gstPv1Data.dI_divid_dV + gstPv1Data.pvI_divid_pvV) * 100;
    return _garrSciBuf;
}
void PvSendStateWave()
{
    PvGetStateWave();
}

void MpptSetPv1Pwr(float pwr)
{
    float pvpwr_set = 0;
    if(pwr >= 0 && pwr <= DUAL_PV_MAX_POWER)
    {
        pvpwr_set = pwr;
    }
    else
    {
        if(pwr > DUAL_PV_MAX_POWER)
        {
            pvpwr_set = DUAL_PV_MAX_POWER;
        }
        if(pwr < 0)
        {
            pvpwr_set = 0;
        }
        
    }
    gPvPwrRef = pvpwr_set;
}

/**
* @brief: Read_Mppt_state
* @return: 读取PV存在状态，如有一路存在则返回值不为0
* @note: 返回值如不为0则两路PV至少有一路PV存在
*
*/

uint16_t Read_Mppt_state()
{
    return (gstPv1Data.mppt_state || gstPv2Data.mppt_state);
}
/**
* @brief: _Mppt_Coninc_Vstep
* @param: pv_data PV变量结构体
* @param: bus_vol 母线电压
* @param: source_mode 直流源类型 0为PV源，1为直流源
* @return:
* @note: 梯度导纳增量法
*
*/
static uint16_t _Mppt_Coninc_Vstep(PvData_t* pv_data, float bus_vol,EN_SOURCE_MODE_DEF source_mode)
{
    float mppt_duty_max = DUTY_MAX;
    float mppt_duty_min = DUTY_MIN;
    float duty_add_init = 0.05;
    float duty_subtract_init = 0.05;
    float d_duty_max = 0.02;
    static uint16_t pv_over_flag = 0;

    pv_data->pv_pwr = pv_data->pv_vol * pv_data->pv_cur;
    if(pv_data->pv_pwr < 0)
    {
        pv_data->pv_pwr = 0;
    }

    pv_data->dP = pv_data->pv_pwr - pv_data->pv_pwr_old;
    pv_data->dV = pv_data->pv_vol - pv_data->pv_vol_old;
    pv_data->dI = pv_data->pv_cur - pv_data->pv_cur_old;
    float d_duty_coef = (1 - pv_data->mppt_duty) * (1 - pv_data->mppt_duty);//线性化系数
    //PV综合爬坡方向
    if(gPvPwrRef > (gstPv1Data.pv_pwr + gstPv2Data.pv_pwr))
    {
        pv_data->dir = 1;
    }
    else if(gPvPwrRef < (gstPv1Data.pv_pwr + gstPv2Data.pv_pwr))
    {
        pv_data->dir = -1;
    }
    else
    {
        pv_data->dir = 0;
    }
    //单PV爬坡功率限制
    if((pv_data->pv_pwr > pv_data->pv_vol * PV_CUR_MAX) && (pv_data->mppt_state != 0))
    {
        pv_data->dir = -1;
    }
    if(pv_data->pv_pwr > SINGLE_PV_MAX_POWER) //单路PV功率限制5kW
    {
        pv_data->dir = -1;
    }
    //20240810:PV状态暂时不用，屏蔽掉
//     if (pv_data->pv_pwr_set > (pv_data->pv_pwr + PWR_STATE_THRESHOD))
//     {
//         pv_data->pv_pwr_nlimit_count++;
//         pv_data->pv_pwr_limit_count = 0;
//         if (pv_data->pv_pwr_nlimit_count >= 5)
//         {
//             pv_data->pv_pwr_state = NO_LIMIT;          //状态转换
//             pv_data->pv_pwr_nlimit_count = 0;
//         }
//     }
//     else if (pv_data->pv_pwr_set < (pv_data->pv_pwr - PWR_STATE_THRESHOD))
//     {
//         pv_data->pv_pwr_limit_count++;
//         pv_data->pv_pwr_nlimit_count = 0;
//         if (pv_data->pv_pwr_limit_count >= 5)
//         {
//             pv_data->pv_pwr_state = LIMIT;
//             pv_data->pv_pwr_limit_count = 0;
//         }
//     }
//     else
//     {
//         pv_data->pv_pwr_state = LIMIT_NO_LIMIT;
//     }
    float pv_pwr_diff = fabs(gPvPwrRef - gstPv1Data.pv_pwr - gstPv2Data.pv_pwr); //功率差值计算
    if(pv_pwr_diff > 2500)
    {
        pv_pwr_diff = 2500;
    }

    //变步长计算
    pv_data->mppt_duty_add = duty_add_init * pv_pwr_diff * d_duty_coef * 0.0002;
    pv_data->mppt_duty_subtract = duty_subtract_init * pv_pwr_diff * d_duty_coef * 0.0002;
    pv_data->mppt_duty_high_threshold = 1 - (pv_data->pv_vol / bus_vol);
    pv_data->mppt_duty_low_threshold = pv_data->mppt_duty_high_threshold - pv_data->mppt_duty_add_old * 1.3;
    float mppt_duty_add_low = 0;
    mppt_duty_add_low = 500 / (fabs(pv_data->pv_pwr) + 1.0) / (pv_data->pv_vol + 1.0);
    
    if(pv_data->mppt_duty < pv_data->mppt_duty_low_threshold) //优化小功率下扰动较慢问题
    {
        pv_data->mppt_duty_add = 4 * mppt_duty_add_low;
        pv_data->mppt_duty_subtract = 4 * mppt_duty_add_low;
        d_duty_max = pv_data->mppt_duty_high_threshold - pv_data->mppt_duty - 0.0025;
        if(d_duty_max > 0.06)
        {
            d_duty_max = 0.06;
        }
        if(d_duty_max > pv_data->mppt_duty_add)
        {
            pv_data->mppt_duty_add_old = pv_data->mppt_duty_add;
            pv_data->mppt_duty_subtract_old = pv_data->mppt_duty_subtract;
        }
        else
        {
            pv_data->mppt_duty_add_old = d_duty_max;
            pv_data->mppt_duty_subtract_old = d_duty_max;
        }
    }
    else if((pv_data->mppt_duty < pv_data->mppt_duty_high_threshold) && (pv_data->mppt_duty_add_old > 0.016))
    {
        pv_data->mppt_duty_add = mppt_duty_add_low;
        pv_data->mppt_duty_subtract = mppt_duty_add_low;
    }

    if(pv_data->mppt_duty_add > d_duty_max)
    {
        pv_data->mppt_duty_add = d_duty_max;
    }
    if(pv_data->mppt_duty_subtract > d_duty_max)
    {
        pv_data->mppt_duty_subtract = d_duty_max;
    }
    if(pv_data->mppt_duty_add < 0.0005) //最小步长限制
    {
        pv_data->mppt_duty_add = 0.0005;
    }
    if(pv_data->mppt_duty_subtract < 0.0005)
    {
        pv_data->mppt_duty_subtract = 0.0005;
    }
    float pv_power_diff = gPvPwrRef - gstPv1Data.pv_pwr - gstPv2Data.pv_pwr; 
    if((pv_power_diff) < -200) //如果给定小于实际PV功率500W则增大减小步长，且不受步长变化最大值限制
    {
        if(pv_power_diff < -500)
        {
            pv_data->mppt_duty_subtract = 0.2;
            pv_data->mppt_duty_add = 0.2;
        }
        else if(pv_power_diff < -400)
        {
            pv_data->mppt_duty_subtract = 0.1;
            pv_data->mppt_duty_add = 0.1;
        }
        else if(pv_power_diff < -300)
        {
            pv_data->mppt_duty_subtract = 0.05;
            pv_data->mppt_duty_add = 0.05;
        }
        else
        {
            pv_data->mppt_duty_subtract = 0.005;
            pv_data->mppt_duty_add = 0.005;
        }
        pv_over_flag = 1;
    }
    else if((pv_data->pv_pwr - SINGLE_PV_MAX_POWER) > 200) //如果超出单个功率200W，则减小步长为0.2
    {
        pv_data->mppt_duty_subtract = 0.2;
        pv_data->mppt_duty_add = 0.2;
        pv_over_flag = 1;
    }
    else
    {
        pv_over_flag = 0;

    }
    if(source_mode == EN_DC_SOURCE)// 直流源模式下根据DI判断功率走向
    {
        pv_data->mppt_state = 8;
    }
    else
    {
        if(pv_data->dV == 0) 
        {
            if(pv_data->dI == 0)
            {
                pv_data->mppt_state = 4;
            }
            if(pv_data->dI > 0)
            {
                pv_data->mppt_state = 5;
            }
            if(pv_data->dI < 0)
            {
                pv_data->mppt_state = 6;
            }
        }
        else
        {
            pv_data->dI_divid_dV = pv_data->dI / pv_data->dV;
            pv_data->pvI_divid_pvV = -pv_data->pv_cur / pv_data->pv_vol;
            if(pv_data->dI_divid_dV == pv_data->pvI_divid_pvV)
            {
                pv_data->mppt_state = 1;
            }
            if(pv_data->dI_divid_dV > pv_data->pvI_divid_pvV)
            {
                pv_data->mppt_state = 2;
            }
            if(pv_data->dI_divid_dV < pv_data->pvI_divid_pvV)
            {
                pv_data->mppt_state = 3;
            }
        }
    }
    if((pv_data->pv_vol == 0) && (pv_data->pv_cur == 0) && (bus_vol == 0))
    {
        pv_data->mppt_state = 0;
    }
    else if(gPvPwrRef == 0) 
    {
        pv_data->mppt_state = 7;
    }
    

    switch (pv_data->mppt_state)
    {
        case 0:
        {
            pv_data->mppt_duty = 0;
            break;
        }
        case 1:
        {
            break;
        }
        case 2:
        {
            if((pv_over_flag == 1) || ((pv_data->dP * pv_data->dV >= 0) && (pv_data->dir == -1)) ) //判断在左半区，则先往右追踪，避免短路工况
            {
                pv_data->mppt_duty = pv_data->mppt_duty + pv_data->dir * pv_data->mppt_duty_subtract;
            }
            else
            {
                pv_data->mppt_duty = pv_data->mppt_duty - pv_data->dir * pv_data->mppt_duty_subtract;
            }
            break;
        }
        case 3:
        {
            if((pv_data->dP * pv_data->dV < 0) || (pv_data->dir == 1) || (pv_over_flag == 1))
            {
                pv_data->mppt_duty = pv_data->mppt_duty + pv_data->dir * pv_data->mppt_duty_add;
            }
            else
            {
                pv_data->mppt_duty = pv_data->mppt_duty - pv_data->dir * pv_data->mppt_duty_add;
            }
            break;
        }
        case 4:
        {
            break;
        }
        case 5:
        {
            if ((pv_data->dP * pv_data->dV < 0) || (pv_data->dir == 1)) //判断在左半区，则先往右追踪，避免短路工况
            {
                pv_data->mppt_duty = pv_data->mppt_duty - pv_data->dir * pv_data->mppt_duty_subtract;
            }
            else
            {
                pv_data->mppt_duty = pv_data->mppt_duty + pv_data->dir * pv_data->mppt_duty_subtract;
            }
            break;
        }
        case 6:
        {
            if ((pv_data->dP * pv_data->dV < 0) || (pv_data->dir == 1))
            {
                pv_data->mppt_duty = pv_data->mppt_duty + pv_data->dir * pv_data->mppt_duty_add;
            }
            else
            {
                pv_data->mppt_duty = pv_data->mppt_duty - pv_data->dir * pv_data->mppt_duty_add;
            }
            break;
        }
        case 7: //就绪状态
        {
            pv_data->mppt_duty = 0;
            break;
        }
        case 8: 
        {
            pv_data->mppt_duty = pv_data->mppt_duty + pv_data->dir * pv_data->mppt_duty_add;
            break;
        }
        default:
        {
            pv_data->mppt_duty = 0;
            break;
        }
    }
    if (pv_data->mppt_duty >= mppt_duty_max)
    {
        pv_data->mppt_duty = mppt_duty_max;
    }
    else if (pv_data->mppt_duty <= mppt_duty_min)
    {
        pv_data->mppt_duty = mppt_duty_min;
    }

    pv_data->pv_vol_old = pv_data->pv_vol;
    pv_data->pv_cur_old = pv_data->pv_cur;
    pv_data->pv_pwr_old = pv_data->pv_pwr;
    if(pv_data->pv_pwr < 0)
    {
        pv_data->pv_pwr_out = 0.0f;
    }
    else
    {
        pv_data->pv_pwr_out = pv_data->pv_pwr;
    }
    

    return 1;
}



void PVCntlInit()
{
    _gAlgoInfo = AlgoStateGet();
}

/**
* @brief: _PvFsmRun
* @param:
* @param:
* @return:
* @note: PV状态机运行
*
*/
void _PvFsmRun()
{
    /*************************/
    //状态切换
    switch(gePVFsm)
    {
    case EN_PV_STATE_DEF_READY:
    {
        /***************************/
        //condition
        /***************************/
        if (_gPvRunStateFlag == PV_BOOST) //切换至PVBOOST模式
        {
            /***************************/
            //nextstate
            gePVFsm = EN_PV_STATE_DEF_ACTIVATIONBOOST;
            /***************************/
            /***************************/
            //action
            _gPvRunStateFlag = 0;  //状态设置命令复位 
            /***************************/
        }
        /***************************/
        //condition
        /***************************/
        else if(_gPvRunStateFlag == PV_NORMAL) //切换至正常运行模式
        {
            /***************************/
            //nextstate
            gePVFsm = EN_PV_STATE_DEF_NORMAL;
            /***************************/
            /***************************/
            //action
            _gPvRunStateFlag = 0;  //状态设置命令复位 
            /***************************/
        }

        break;
    }
    case EN_PV_STATE_DEF_ACTIVATIONBOOST:
    {
        /***************************/
        //condition
        /***************************/
        if(_gPvRunStateFlag == PV_READY)
        {
            /***************************/
            //nextstate
            gePVFsm = EN_PV_STATE_DEF_READY;
            /***************************/
            /***************************/
            //action
            _gPvRunStateFlag = 0;  //状态设置命令复位 
            /***************************/
        }
        /***************************/
        //condition
        /***************************/
        else if(_gPvRunStateFlag == PV_NORMAL) //切换至正常运行模式
        {
            /***************************/
            //nextstate
            gePVFsm = EN_PV_STATE_DEF_NORMAL;
            /***************************/
            /***************************/
            //action
            _gPvRunStateFlag = 0;  //状态设置命令复位 
            /***************************/
        }
        break;
    }
    case EN_PV_STATE_DEF_NORMAL:
    {
        if(_gPvRunStateFlag == PV_READY)
        {
            /***************************/
            //nextstate
            gePVFsm = EN_PV_STATE_DEF_READY;
            /***************************/
            /***************************/
            //action
            _gPvRunStateFlag = 0;  //状态设置命令复位 
            /***************************/
        }
        break;
    }
    default:
    {
        gePVFsm = EN_PV_STATE_DEF_READY;
        break;
    }
    }
    /************************/
    /************************/
    //状态运行
    switch(gePVFsm)
    {
    case EN_PV_STATE_DEF_READY:
    {
        gstPv1Data.duty_eff = 0.0;
        gstPv1Data.mppt_duty = 0.0;
        gstPv2Data.duty_eff = 0.0;
        gstPv2Data.mppt_duty = 0.0;
        _gPv1DutyEff = 0;
        _gPv2DutyEff = 0;
        _gDutyRamp = 0.0;
        break;
    }
    case EN_PV_STATE_DEF_ACTIVATIONBOOST:
    {
        _PvBoostRun(gPv1_Voltage_Ave, gPv2_Voltage_Ave, PVACTIVAT_BUS_THRESHOLD + 10, gstSensorData.bus_voltage); //NOTE:Boost模式,仅支持PV1激活
        break;
    }
    case EN_PV_STATE_DEF_NORMAL:
    {
        _MpptRun();  //正常运行
        break;
    }

    default:
    {
        _gPv1DutyEff = 0;
        _gPv2DutyEff = 0;
        _gDutyRamp = 0.0;
        break;
    }
    }
    /************************/
}
/**
* @brief: PvRunStateSet
* @param: pvrunstate
* @return:
* @note: PV运行状态设置
*
*/
void PvRunStateSet(uint16_t pvrunstate)
{
    if(pvrunstate == PV_BOOST)
    {
        _gPvRunStateFlag = PV_BOOST;  // PVBoost模式
    }
    else if(pvrunstate == PV_NORMAL)  // PV正常运行模式
    {
        _gPvRunStateFlag = PV_NORMAL;
    }
    else if(pvrunstate == PV_READY)  // PV就绪模式
    {
        _gPvRunStateFlag = PV_READY;
    }
}
/**
* @brief: PvRunStateRead
* @param:
* @return: gePVFsm
* @note: PV运行状态读取
*
*/
EN_PV_STATE_DEF PvRunStateRead()
{
    return gePVFsm;
}



/**
* @brief: _PvBoostRun
* @param: vol_pv PV电压
* @return:
* @note: PVboost模式运行
*
*/
static void _PvBoostRun(float vol_pv1, float vol_pv2, float Vbus_target, float Vbus)
{
    // 公共的占空比斜坡调整逻辑
    if(Vbus > Vbus_target)
    {
        _gDutyRamp -= 0.0005;
    }
    else if(Vbus <= Vbus_target)
    {
        _gDutyRamp += 0.0005;
    }

    // 限制占空比斜坡范围
    if(_gDutyRamp > 1)
    {
        _gDutyRamp = 1;
    }
    else if(_gDutyRamp < 0)
    {
        _gDutyRamp = 0;
    }

    // 优先级控制：PV1优先，PV1没有才用PV2
    if(vol_pv1 > 70)  // PV1存在且电压正常
    {
        // 使用PV1
        _gPv1DutyEff = 30 / vol_pv1 * _gDutyRamp;
        if(_gPv1DutyEff < DUTY_MIN)
        {
            _gPv1DutyEff = 0;
        }
        else if(_gPv1DutyEff > 0.2)
        {
            _gPv1DutyEff = 0.2;
        }

        // PV2关闭
        _gPv2DutyEff = 0;
    }
    else if(vol_pv2 > 70)  // PV1不存在，但PV2存在且电压正常
    {
        // PV1关闭
        _gPv1DutyEff = 0;

        // 使用PV2
        _gPv2DutyEff = 30 / vol_pv2 * _gDutyRamp;
        if(_gPv2DutyEff < DUTY_MIN)
        {
            _gPv2DutyEff = 0;
        }
        else if(_gPv2DutyEff > 0.2)
        {
            _gPv2DutyEff = 0.2;
        }
    }
    else  // 两个PV都不存在
    {
        _gPv1DutyEff = 0;
        _gPv2DutyEff = 0;
    }

    // 故障处理（覆盖正常逻辑）
    if(gAlgoWarningDef.err.pv2_vol_low == 1)
    {
        _gPv2DutyEff = 0;  // PV2故障时强制关闭
    }
}


/**
* @brief: _AverageData
* @param:
* @return:
* @note: pv|bus|bat|grid_power求平均
*
*/
static void _AverageData()
{
    static float pv1_voltage_total = 0;
    static float pv1_current_total = 0;
    static float pv2_voltage_total = 0;
    static float pv2_current_total = 0;
    static float bus_voltage_total = 0;
    static float bat_voltage_total = 0;
    static float bat_current_total = 0;
    static float grid_power_total = 0;
    static float32 bat_cur_filter_add = 0;
    static uint16_t count = 0;
    bat_cur_filter_add += (gstSensorData.bat_current);
    count++;
    if(count >= 8)
    {
        _gBatCurFilter = bat_cur_filter_add / 8.0f;
        bat_cur_filter_add = 0;
        count = 0;
    }

    _gMpptInvCount++;
    pv1_voltage_total += gstSensorData.pv1_voltage;
    pv1_current_total += gstSensorData.pv1_current;
    pv2_voltage_total += gstSensorData.pv2_voltage;
    pv2_current_total += gstSensorData.pv2_current;
    bus_voltage_total += gstSensorData.bus_voltage;
    bat_voltage_total += gstSensorData.bat_voltage;
    bat_current_total += gstSensorData.bat_current;
    grid_power_total += grid_power;
    _gInvSta = InverterGetState();
    gePvInvSta = _gInvSta->sta_grid_connect;

    if(_gMpptInvCount >= MPPT_AVE_COUNT)
    {
        gPv1_Voltage_Ave = pv1_voltage_total / MPPT_AVE_COUNT;
        gPv1_Current_Ave = pv1_current_total / MPPT_AVE_COUNT;
        gPv2_Voltage_Ave = pv2_voltage_total / MPPT_AVE_COUNT;
        gPv2_Current_Ave = pv2_current_total / MPPT_AVE_COUNT;
        gBus_Voltage_Ave = bus_voltage_total / MPPT_AVE_COUNT;
        gBat_Voltage_Ave = bat_voltage_total / MPPT_AVE_COUNT;
        gBat_Current_Ave = bat_current_total / MPPT_AVE_COUNT;
        gGrid_Power_Ave = grid_power_total / MPPT_AVE_COUNT;

        gstPv1Data.pv_vol = gPv1_Voltage_Ave;
        gstPv1Data.pv_cur = gPv1_Current_Ave;
        gstPv2Data.pv_vol = gPv2_Voltage_Ave;
        gstPv2Data.pv_cur = gPv2_Current_Ave;

        _gMpptInvCount = 0;
        pv1_voltage_total = 0;
        pv1_current_total = 0;
        pv2_voltage_total = 0;
        pv2_current_total = 0;
        bus_voltage_total = 0;
        bat_voltage_total = 0;
        bat_current_total = 0;
        grid_power_total = 0;
        _gMpptOpenCloseCnt++;
        if(_gMpptOpenCloseCnt >= OPEN_CLOSE_PERIOD)
        {
            _gMpptOpenCloseCnt = 0;
        }
    }

}

/**
* @brief: _MpptRun
* @param:
* @return:
* @note: Mppt正常运行函数
*
*/
static void _MpptRun()
{
    static EN_SOURCE_MODE_DEF source_mode_pv1 = EN_PV_SOURCE;
    static EN_SOURCE_MODE_DEF source_mode_pv2 = EN_PV_SOURCE;
    float bat_cur_charge_threshold = -70.0f;
    float bat_cur_charge_threshold2 = -70.0f;
    _gePwrLevel = geInvPwrLevel;
    switch(_gePwrLevel )
    {
        case EN_INV_PWR_LEVEL1:
        {
            bat_cur_charge_threshold = -BAT_CHG_CUR_OVER_LEAVEL1;
            bat_cur_charge_threshold2 = -BAT_CHG_CUR_OVER_LEAVEL1 - 40;
            break;
        }
        case EN_INV_PWR_LEVEL2:
        {
            bat_cur_charge_threshold = -BAT_CHG_CUR_OVER_LEAVEL2;
            bat_cur_charge_threshold2 = -BAT_CHG_CUR_OVER_LEAVEL2 - 40;
            break;
        }
        case EN_INV_PWR_LEVEL3:
        {
            bat_cur_charge_threshold = -BAT_CHG_CUR_OVER_LEAVEL3;
            bat_cur_charge_threshold2 = -BAT_CHG_CUR_OVER_LEAVEL3 - 40;
            break;
        }
        default:
        {
            break;
        }
    }
    if(gstPv1Data.duty_eff < gstPv1Data.mppt_duty) //降低过充电流，平滑变换占空比
    {
        gstPv1Data.duty_eff += 0.0005;
    }
    if(gstPv1Data.duty_eff > gstPv1Data.mppt_duty)
    {
        gstPv1Data.duty_eff -= 0.0005;
    }
    if(gstPv1Data.duty_eff > DUTY_MAX) //限幅处理
    {
        gstPv1Data.duty_eff = DUTY_MAX;
    }
    if(gstPv1Data.duty_eff < DUTY_MIN)
    {
        gstPv1Data.duty_eff = DUTY_MIN;
    }
    _gPv1DutyEff = gstPv1Data.duty_eff;

    if(gstPv2Data.duty_eff < gstPv2Data.mppt_duty) //降低过充电流，平滑变换占空比
    {
        gstPv2Data.duty_eff += 0.0005;
    }
    if(gstPv2Data.duty_eff > gstPv2Data.mppt_duty)
    {
        gstPv2Data.duty_eff -= 0.0005;
    }
    if(gstPv2Data.duty_eff > DUTY_MAX) //限幅处理
    {
        gstPv2Data.duty_eff = DUTY_MAX;
    }
    if(gstPv2Data.duty_eff < DUTY_MIN)
    {
        gstPv2Data.duty_eff = DUTY_MIN;
    }
    _gPv2DutyEff = gstPv2Data.duty_eff;

    if(_gTestMode != 1) //非测试模式
    {
        source_mode_pv1 = EN_PV_SOURCE;
        source_mode_pv2 = EN_PV_SOURCE;
        if(((gstPv1Data.pv_vol < 65) && (gstPv1Data.pv_cur < 0.5)) || \
           (gAlgoWarningDef.err.pv1_vol_low == 1) || \
           (_gAlgoInfo->llc_start_ok == 0) || \
           (gAlgoWarningDef.err.pv_loss_control == 1) || (flag_PV_over_current == 1) || \
           (gPvPwrRef == 0) || \
           ((gstPv1Data.pv_vol > (gBus_Voltage_Ave * 0.95)) && ((gPvPwrRef - gstPv2Data.pv_pwr) < 1200) && \
               ((gChgPower + gLoadPower + gFeedPower * (gePvInvSta == EN_GRID_CONNECT_MODE) - gFeedPower * gSysSetInfo.antireverseflow_enable * (gePvInvSta == EN_GRID_CONNECT_MODE) - gstPv2Data.pv_pwr) < 1200)) || \
           (gstSensorData.pv1_current > PV_CUR_PROTECT) || \
           (gstSensorData.bus_voltage > 525) || \
           (gBat_Current_Ave < bat_cur_charge_threshold) || \
           (_gBatCurFilter < bat_cur_charge_threshold2)
                ) //PV不存在,或直流母线电压未建立不执行
        {
            if((gAlgoWarningDef.err.pv1_vol_low == 1) && (_gMpptOpenCloseCnt == 0)) //欠压下间歇开管防止PV电压被二极管漏电流充电
            {
                _gPv1DutyEff = DUTY_MIN;
            }
            else
            {
                _gPv1DutyEff = 0;
            }
            gstPv1Data.duty_eff = gstPv1Data.mppt_duty;
            gstPv1Data.pv_vol = 0;
            gstPv1Data.pv_cur = 0;
            if((gAlgoWarningDef.err.pv_loss_control == 1) || (gPv1_Voltage_Ave < 65) || (flag_PV_over_current == 1))
            {
                _Mppt_Coninc_Vstep(&gstPv1Data, 0, source_mode_pv1);
            }
            else
            {
                _Mppt_Coninc_Vstep(&gstPv1Data, gBus_Voltage_Ave, source_mode_pv1);
            }


        }
        else
        {
            if(_gMpptInvCount == 1)
            {
                gstPv1Data.duty_eff = gstPv1Data.mppt_duty;
                _Mppt_Coninc_Vstep(&gstPv1Data, gBus_Voltage_Ave, source_mode_pv1);
                if(gPvPwrRef == 0)
                {
                    gstPv1Data.mppt_duty = DUTY_MIN;
                    _gPv1DutyEff = 0;
                }

            }
        }
        if(((gstPv2Data.pv_vol < 65) && (gstPv2Data.pv_cur < 0.5)) || \
           (gAlgoWarningDef.err.pv2_vol_low == 1) || \
           (_gAlgoInfo->llc_start_ok == 0) || \
           (gAlgoWarningDef.err.pv_loss_control == 1) || (flag_PV_over_current == 1) || \
           (gPvPwrRef == 0) || \
           ((gstPv2Data.pv_vol > (gBus_Voltage_Ave * 0.95)) && ((gPvPwrRef - gstPv1Data.pv_pwr) < 1200) && \
               ((gChgPower + gLoadPower + gFeedPower * (gePvInvSta == EN_GRID_CONNECT_MODE) - gFeedPower * gSysSetInfo.antireverseflow_enable * (gePvInvSta == EN_GRID_CONNECT_MODE) - gstPv1Data.pv_pwr) < 1200)) || \
           (gstSensorData.pv2_current > PV_CUR_PROTECT) || \
           (gstSensorData.bus_voltage > 525) ||  \
           (gBat_Current_Ave < bat_cur_charge_threshold) || \
           (_gBatCurFilter < bat_cur_charge_threshold2)
                ) //PV不存在,或直流母线电压未建立不执行
        {
            if((gAlgoWarningDef.err.pv2_vol_low == 1) && (_gMpptOpenCloseCnt == 0)) //欠压下间歇开管防止PV电压被二极管漏电流充电
            {
                _gPv2DutyEff = DUTY_MIN;
            }
            else
            {
                _gPv2DutyEff = 0;
            }
            gstPv2Data.duty_eff = gstPv2Data.mppt_duty;
            gstPv2Data.pv_vol = 0;
            gstPv2Data.pv_cur = 0;
            if((gAlgoWarningDef.err.pv_loss_control == 1) || (gPv2_Voltage_Ave < 65) || (flag_PV_over_current == 1))
            {
                _Mppt_Coninc_Vstep(&gstPv2Data, 0, source_mode_pv2);
            }
            else
            {
                _Mppt_Coninc_Vstep(&gstPv2Data, gBus_Voltage_Ave, source_mode_pv2);
            }
        }
        else
        {
            if(_gMpptInvCount == 2) //PV1和PV2交错计算
            {
                gstPv2Data.duty_eff = gstPv2Data.mppt_duty;
                _Mppt_Coninc_Vstep(&gstPv2Data, gBus_Voltage_Ave, source_mode_pv2);
                if(gPvPwrRef == 0)
                {
                    gstPv2Data.mppt_duty = DUTY_MIN;
                    _gPv2DutyEff = 0;
                }
            }
        }
    }
    else //测试模式
    {
        source_mode_pv1 = EN_DC_SOURCE;
        source_mode_pv2 = EN_DC_SOURCE;
        if(((gstPv1Data.pv_vol <  TEST_MODE_DC_VOLTAGE) && (gstPv1Data.pv_cur < 0.5)) || \
           (gAlgoWarningDef.err.pv1_vol_low == 1) || \
           (_gAlgoInfo->llc_start_ok == 0) || \
           (gAlgoWarningDef.err.pv_loss_control == 1) || (flag_PV_over_current == 1) || \
           (gPvPwrRef == 0)\
        )
        {
            gstPv1Data.pv_vol = 0;
            gstPv1Data.pv_cur = 0;
            _Mppt_Coninc_Vstep(&gstPv1Data, gBus_Voltage_Ave, source_mode_pv1);
            _gPv1DutyEff = DUTY_MIN;
            gstPv1Data.duty_eff = DUTY_MIN;
            gstPv1Data.mppt_duty = TEST_MODE_DUTY_INIT;
        }
        else
        {
            if(_gMpptInvCount == 1)
            {
                gstPv1Data.duty_eff = gstPv1Data.mppt_duty;
                _Mppt_Coninc_Vstep(&gstPv1Data, gBus_Voltage_Ave, source_mode_pv1);
                if(gPvPwrRef == 0)
                {
                    gstPv1Data.mppt_duty = TEST_MODE_DUTY_INIT;
                    _gPv1DutyEff = 0;
                }
            }
        }
        if(((gstPv2Data.pv_vol < TEST_MODE_DC_VOLTAGE) && (gstPv2Data.pv_cur < 0.5)) || \
           (gAlgoWarningDef.err.pv2_vol_low == 1) || \
           (_gAlgoInfo->llc_start_ok == 0) || \
           (gAlgoWarningDef.err.pv_loss_control == 1) || (flag_PV_over_current == 1) || \
           (gPvPwrRef == 0)\
        )
        {
            gstPv2Data.pv_vol = 0;
            gstPv2Data.pv_cur = 0;
            _Mppt_Coninc_Vstep(&gstPv2Data, gBus_Voltage_Ave, source_mode_pv2);
            _gPv2DutyEff = DUTY_MIN;
            gstPv2Data.duty_eff = DUTY_MIN;
            gstPv2Data.mppt_duty = TEST_MODE_DUTY_INIT;
        }
        else
        {
            if(_gMpptInvCount == 2)
            {
                gstPv2Data.duty_eff = gstPv2Data.mppt_duty;
                _Mppt_Coninc_Vstep(&gstPv2Data, gBus_Voltage_Ave, source_mode_pv2);
                if(gPvPwrRef == 0)
                {
                    gstPv2Data.mppt_duty = TEST_MODE_DUTY_INIT;
                    _gPv2DutyEff = 0;
                }
            }
        }
    }

    if(gstSensorData.bus_voltage > 525 || \
       gInvErr == 1 || \
       (gstSensorData.pv2_current > PV_CUR_PROTECT) || \
       (gstSensorData.pv1_current > PV_CUR_PROTECT) || \
       (gBat_Current_Ave < bat_cur_charge_threshold)
       )
    {
        EALLOW;
        EPwm11Regs.TZFRC.bit.OST = 1;
        EDIS;
    }
    float32 pv1_pwr = gPv1_Voltage_Ave * gPv1_Current_Ave;
    float32 pv2_pwr = gPv2_Voltage_Ave * gPv2_Current_Ave;
    if(gstPv1Data.pv_pwr_out < pv1_pwr)
    {
        gstPv1Data.pv_pwr_out = pv1_pwr;
    }
    if(gstPv2Data.pv_pwr_out < pv2_pwr)
    {
        gstPv2Data.pv_pwr_out = pv2_pwr;
    }
}
void SetSourceMode(uint16_t mode)//要设置外部接口（DC源、PV源）
{
    _gTestMode = mode;
}
