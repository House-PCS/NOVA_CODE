/*
 * cla_cpu_share.c
 *
 *  Created on: 2025年3月26日
 *      Author: Nova006352
 */

#include "cla_cpu_share.h"
#include "cla_control.h"
#include "adc.h"
#include "product_specification.h"
#include "pwr_compute.h"

#define PWR_COMPUTE_FREQ_CPU    (1.0f / INV_CONTROL_TS)
#define PWR_COMPUTE_AC_PERIOD   (5.0f)

static PwrClaCompute_t s_midRelayCpu;
static PwrClaCompute_t s_busDataCpu;

volatile float32 garrDebugDataCla[16];
#pragma DATA_SECTION(garrDebugDataCla,"Cla1DataRam");
EN_INV_STATE geInvControlState;    //逆变运行状态 CPU控制CLA的状态接口
#pragma DATA_SECTION(geInvControlState,"CpuToCla1MsgRAM");
volatile float32 gInvTemp;
#pragma DATA_SECTION(gInvTemp,"CpuToCla1MsgRAM");
volatile AlgoInfo_t gstAlgoInterface;
#pragma DATA_SECTION(gstAlgoInterface,"Cla1DataRam");
//直流量结构体
ClaDirectType_t gstBusPos;      //BUS正
#pragma DATA_SECTION(gstBusPos,"Cla1DataRam");
ClaDirectType_t gstBusNeg;      //BUS负
#pragma DATA_SECTION(gstBusNeg,"Cla1DataRam");
//交流量结构体
// ClaAlternatingType_t gstGrid;   //市电
// // #pragma DATA_SECTION(gstGrid,"Cla1DataRam");
// ClaAlternatingType_t gstInv;    //逆变
// // #pragma DATA_SECTION(gstInv,"Cla1DataRam");
// ClaAlternatingType_t gstGenerator;   //油机电
// // #pragma DATA_SECTION(gstGenerator,"Cla1DataRam");
// ClaAlternatingType_t gstBackUp;   //油机电
// // #pragma DATA_SECTION(gstBackUp,"Cla1DataRam");
// ClaAlternatingType_t gstNormalLoad;   //油机电
// // #pragma DATA_SECTION(gstNormalLoad,"Cla1DataRam");


/**
* @brief: Get_Cla_Grid_A_Volt_Rms
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电A相有效值
*/
float32 Get_Cla_Grid_A_Volt_Rms(void)
{
    // return (gstGrid.volt.value.rms);
    return 0;
}
/**
* @brief: Get_Cla_Grid_B_Volt_Rms
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电B相有效值
*/
//float32 Get_Cla_Grid_B_Volt_Rms(void)
//{
//    return (gstGrid.volt.phase_b.rms);
//}
/**
* @brief: Get_Cla_Grid_C_Volt_Rms
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电C相有效值
*/
//float32 Get_Cla_Grid_C_Volt_Rms(void)
//{
//    return (gstGrid.volt.phase_c.rms);
//}
/**
* @brief: Get_Cla_Grid_A_Volt_Instant
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电A相瞬时值
*/
float32 Get_Cla_Grid_A_Volt_Instant(void)
{
    // return (gstGrid.volt.value.instant);
    return 0;
}
/**
* @brief: Get_Cla_Grid_B_Volt_Instant
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电B相瞬时值
*/
//float32 Get_Cla_Grid_B_Volt_Instant(void)
//{
//    return (gstGrid.volt.phase_b.instant);
//}
/**
* @brief: Get_Cla_Grid_C_Volt_Instant
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电C相瞬时值
*/
//float32 Get_Cla_Grid_C_Volt_Instant(void)
//{
//    return (gstGrid.volt.phase_c.instant);
//}
/**
* @brief: Get_Cla_Grid_PLL_Theta
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算市电的theta角度
*/
float32 Get_Cla_Grid_PLL_Theta(void)
{
    // return (gstGrid.volt.pll.theta);
    return 0;
}

/**
* @brief: Get_Cla_Inv_A_Volt_Rms
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变A相有效值
*/
float32 Get_Cla_Inv_A_Volt_Rms(void)
{
    // return (gstInv.volt.value.rms);
    return 0;
}
/**
* @brief: Get_Cla_Inv_B_Volt_Rms
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变B相有效值
*/
//float32 Get_Cla_Inv_B_Volt_Rms(void)
//{
//    return (gstInv.volt.phase_b.rms);
//}
/**
* @brief: Get_Cla_Inv_C_Volt_Rms
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变C相有效值
*/
//float32 Get_Cla_Inv_C_Volt_Rms(void)
//{
//    return (gstInv.volt.phase_c.rms);
//}

/**
* @brief: Get_Cla_Inv_A_Volt_Instant
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变A相瞬时值
*/
float32 Get_Cla_Inv_A_Volt_Instant(void)
{
    // return (gstInv.volt.value.instant);
    return 0;
}
/**
* @brief: Get_Cla_Inv_B_Volt_Instant
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变B相瞬时值
*/
//float32 Get_Cla_Inv_B_Volt_Instant(void)
//{
//    return (gstInv.volt.phase_b.instant);
//}
/**
* @brief: Get_Cla_Inv_C_Volt_Instant
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变C相瞬时值
*/
//float32 Get_Cla_Inv_C_Volt_Instant(void)
//{
//    return (gstInv.volt.phase_c.instant);
//}
/**
* @brief: Get_Cla_Inv_PLL_Theta
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变的theta角度
*/
float32 Get_Cla_Inv_PLL_Theta(void)
{
    // return (gstInv.volt.pll.theta);
    return 0;
}
/**
* @brief: Get_Cla_Inv_Power_P
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变的有功功率
*/
float32 Get_Cla_Inv_Power_P(void)
{
    // return (gstInv.power.p);
    return 0;
}
/**
* @brief: Get_Cla_Inv_Power_Q
* @param: static inline void
* @return:Nothing
* @note:获取cla的计算逆变的无功功率
*/
float32 Get_Cla_Inv_Power_Q(void)
{
    // return (gstInv.power.q);
    return 0;
}

/**
* @brief: Get_cla_Inv_state
* @param: void
* @return:EN_INV_STATE
* @note:获取逆变状态
*/

EN_INV_STATE Get_cla_Inv_state(void)
{
    return (gstAlgoInterface.inv_state);
}

void PwrCpuRelayBusInit(void)
{
    RmsPwrCpuInit(&s_midRelayCpu, PWR_COMPUTE_FREQ_CPU, INV_LOWEST_AC_FREQ, INV_HIGHEST_AC_FREQ,
                  PWR_COMPUTE_AC_PERIOD);
    RmsPwrCpuInit(&s_busDataCpu, PWR_COMPUTE_FREQ_CPU, 40.0f, 70.0f, PWR_COMPUTE_AC_PERIOD);
}

void PwrCpuRelayBusStep(void)
{
    RmsPwrCpuCalcStep(&s_midRelayCpu, gstAdcResult.relay_middle_volt_phase_a, 0.0f, 1.0f);
    RmsPwrCpuCalcStep(&s_busDataCpu, gstAdcResult.bus_volt_all, gstAdcResult.bus_curr, 1.0f);

    gstAlgoInterface.relay_middle_volt_phase_a_rms = s_midRelayCpu.vol_rms;
    gstAlgoInterface.bus_vol_rms = s_busDataCpu.vol_rms;
    gstAlgoInterface.bus_cur_rms = (gstAdcResult.bus_curr > 0.0f) ? s_busDataCpu.cur_rms
            : (-s_busDataCpu.cur_rms);
    gstAlgoInterface.bus2batpower = (gstAdcResult.bus_curr > 0.0f) ? s_busDataCpu.pwr_apperant_out
            : (-s_busDataCpu.pwr_apperant_out);
}

