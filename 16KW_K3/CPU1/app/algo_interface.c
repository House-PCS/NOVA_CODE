/*
 * cla_pv_control.cla
 *
 *  Created on: 2025年3月10日1
 *      Author: Nova007456
 */
#include <algo_interface.h>
#include <string.h>
#include "debug_analysis.h"
#include "pwm.h"
#include "AlgoFw/package/cntl/pv/pv_def.h"
#include "AlgoFw/component/Comp_NoiseVar.h"
#include "post.h"
#include "algo_interface.h"
#include "ph3_calc_ifc_update.h"
#include <sample_update.h>
#include "cla_cpu_share.h"

/**********************************/
Comp_NoiseVar_t gstPvPwrNoiseStd[4];

uint16_t  busflag = 0;

static PvLimitInterface_t _gPvLimitInterfaceInfo;
 PVController_t  _gPvData;


#define GET_MIN(value1, value2)     ((value1 < value2) ? value1 : value2)
#define GET_MAX(value1, value2)     ((value1 > value2) ? value1 : value2)
/*******************************INV部分接口**********************************************/
static  Inv3Ph_Interface_t  gstInv3phDataUpdateValue;

static uint16_t _gInvRelagEnFlag = 0;
void Inv3PhSetEnRun(uint8_t flag)
{
    Inv3Ph_Set_EnRun(flag);
}

void Inv3PhSetEnConnect(uint8_t flag)
{
    Inv3Ph_Set_EnGrid(flag);
}

void Inv3PhSetEnConnectRun(uint8_t flag)
{
    Inv3Ph_Set_EnGridRun(flag);
}
void Inv3PhDataUpdate(void)
{

    static uint16_t data_div = 0;
    float32 grid_vol_inst = 0;
    grid_vol_inst = (gstPh3CalcInfo.grid_vol.a_phase_rms + \
                gstPh3CalcInfo.grid_vol.b_phase_rms + gstPh3CalcInfo.grid_vol.c_phase_rms) / 3.0f * 1.414f;
//    Inv3Ph_Set_GridVoltInst(grid_vol_inst);
//    Inv3Ph_Set_GridTheta(gstPh3CalcInfo.grid_vol.theta);
    gstInv3phDataUpdateValue.i_bus_vol_pos = gstAdcResult.bus_volt_pos;
    gstInv3phDataUpdateValue.i_bus_vol_neg = gstAdcResult.bus_volt_neg;
    gstInv3phDataUpdateValue.i_bus_vol = gstAdcResult.bus_volt_pos + gstAdcResult.bus_volt_neg;

    gstInv3phDataUpdateValue.i_bus_vol_filter = gstPh3CalcInfo.bus.vol_filter;
    gstInv3phDataUpdateValue.i_bus_vol_pos_filter = gstPh3CalcInfo.bus.pos_vol_filter;
    gstInv3phDataUpdateValue.i_bus_vol_neg_filter = gstPh3CalcInfo.bus.neg_vol_filter;

    gstInv3phDataUpdateValue.i_A_vol = gstAdcResult.inv_volt_phase_a;
    gstInv3phDataUpdateValue.i_B_vol = gstAdcResult.inv_volt_phase_b;
    gstInv3phDataUpdateValue.i_C_vol = gstAdcResult.inv_volt_phase_c;
    gstInv3phDataUpdateValue.i_A_currL1 = gstAdcResult.inv_curr_phase_a;
    gstInv3phDataUpdateValue.i_B_currL1 = gstAdcResult.inv_curr_phase_b;
    gstInv3phDataUpdateValue.i_C_currL1 = gstAdcResult.inv_curr_phase_c;
   if(data_div == 0)
   {
       gstInv3phDataUpdateValue.i_A_vol_Rms = gstPh3CalcInfo.inv_vol.a_phase_rms;
       gstInv3phDataUpdateValue.i_B_vol_Rms = gstPh3CalcInfo.inv_vol.b_phase_rms;
       gstInv3phDataUpdateValue.i_C_vol_Rms = gstPh3CalcInfo.inv_vol.c_phase_rms;
   }
   else if(data_div == 1)
   {
       gstInv3phDataUpdateValue.i_A_currL1_Rms = gstPh3CalcInfo.inv_curr.a_phase_rms;
       gstInv3phDataUpdateValue.i_B_currL1_Rms = gstPh3CalcInfo.inv_curr.b_phase_rms;
       gstInv3phDataUpdateValue.i_C_currL1_Rms = gstPh3CalcInfo.inv_curr.c_phase_rms;
   }
   else if(data_div == 2)
   {
       gstInv3phDataUpdateValue.i_A_pwr_app = gstPh3CalcInfo.inv_power.a_phase_apperant;
       gstInv3phDataUpdateValue.i_B_pwr_app = gstPh3CalcInfo.inv_power.b_phase_apperant;
       gstInv3phDataUpdateValue.i_C_pwr_app = gstPh3CalcInfo.inv_power.c_phase_apperant;
   }
   else if(data_div == 3)
   {
       gstInv3phDataUpdateValue.i_A_pwr_act = gstPh3CalcInfo.inv_power.a_phase_active;
       gstInv3phDataUpdateValue.i_B_pwr_act = gstPh3CalcInfo.inv_power.b_phase_active;
       gstInv3phDataUpdateValue.i_C_pwr_act = gstPh3CalcInfo.inv_power.c_phase_active;
   }
   else if(data_div == 4)
   {
       gstInv3phDataUpdateValue.i_A_pwr_rea = gstPh3CalcInfo.inv_power.a_phase_reactive;
       gstInv3phDataUpdateValue.i_B_pwr_rea = gstPh3CalcInfo.inv_power.b_phase_reactive;
       gstInv3phDataUpdateValue.i_C_pwr_rea = gstPh3CalcInfo.inv_power.c_phase_reactive;
   }
   else if(data_div == 5)
   {
        Inv3Ph_Set_OutputData(&gstInv3phDataUpdateValue);
       //INV继电器控制
       _gInvRelagEnFlag = ((gstInv3phDataUpdateValue.o_rly_ctrl > 0) ? 1 : 0);
   }
   gstInv3phDataUpdateValue.i_hw_cur_over = ((EPwm1Regs.TZFLG.bit.OST == 1) || (EPwm2Regs.TZFLG.bit.OST == 1) || (EPwm3Regs.TZFLG.bit.OST == 1) || \
                       (EPwm4Regs.TZFLG.bit.OST == 1) || (EPwm5Regs.TZFLG.bit.OST == 1) || (EPwm6Regs.TZFLG.bit.OST == 1));
   Inv3Ph_Set_InputData(&gstInv3phDataUpdateValue);
//    Inv3Ph_Set_OutputData(&inv3ph_data_value);

   data_div = ((data_div >= 6) ? 0 : (data_div ++));
}

void Inv3PhCpuTransforClaData(void)
{
    gstCpuTransforClaData.bus_vol = gInv3Ph_Data.bus_vol.inst;
    gstCpuTransforClaData.bus_vol_pos = gInv3Ph_Data.bus_vol_pos.inst;
    gstCpuTransforClaData.bus_vol_neg = gInv3Ph_Data.bus_vol_neg.inst;

    gstCpuTransforClaData.inv_vol_a = gInv3Ph_Data.phase_A.vol.inst;
    gstCpuTransforClaData.inv_vol_b = gInv3Ph_Data.phase_B.vol.inst;
    gstCpuTransforClaData.inv_vol_c = gInv3Ph_Data.phase_C.vol.inst;

    gstCpuTransforClaData.inv_curr_L1_a = gInv3Ph_Data.phase_A.cur.inst;
    gstCpuTransforClaData.inv_curr_L1_b = gInv3Ph_Data.phase_B.cur.inst;
    gstCpuTransforClaData.inv_curr_L1_c = gInv3Ph_Data.phase_C.cur.inst;

    gstCpuTransforClaData.inv_curr_L2_a = gInv3Ph_Data.phase_A.curL2.inst;
    gstCpuTransforClaData.inv_curr_L2_b = gInv3Ph_Data.phase_B.curL2.inst;
    gstCpuTransforClaData.inv_curr_L2_c = gInv3Ph_Data.phase_C.curL2.inst;


    gstCpuTransforClaData.ClaInv3PhState = Get3PhRunState();
    gstCpuTransforClaData.theta = gInv3Ph_Data.theta;
    gstCpuTransforClaData.theta_offset = gInv3Ph_Data.theta_offset;

    gstCpuTransforClaData.w0 = gInv3Ph_Data.w0;
    gstCpuTransforClaData.w_offset = gInv3Ph_Data.w_offset;

    gstCpuTransforClaData.vol_amp_ref = gInv3Ph_Data.inv_amp_vol;
//    gstCpuTransforClaData.vol_amp_ref_offset = gInv3Ph_Data.inv_amp_vol_offset;

}
void Inv3PhWorkRun(void)
{
    Inv3Ph_Run();
}

uint8_t GetInvRelayFlag(void)
{
    return _gInvRelagEnFlag;
}

uint8_t Get3PhIsRunFlag(void)
{
    return Inv3Ph_Get_Status_Obj()->is_running;
}

uint8_t Get3PhIsConnectFlag(void)
{
    return Inv3Ph_Get_Status_Obj()->is_grid_connected;
}

uint8_t Get3PhIsErrFlag(void)
{
    return Inv3Ph_Get_Status_Obj()->is_err;
}

EN_INV3PH_STATE Get3PhRunState(void)
{
    return Inv3Ph_Get_Status_Obj()->state;
}

Inv3Ph_Err_Un_t Get3PhErrType(void)
{
    return *Inv3Ph_Get_Err_Obj();
}

Inv3Ph_Warn_Un_t Get3PhWarnType(void)
{
    return *Inv3Ph_Get_Warn_Obj();
}
/***********************************************************************************************/

/**
* @brief: PvDataUpdate()
* @param: nothing
* @return:PVInterfaceParam_t
* @note: PV限制数据传递
*/
PvLimitInterface_t*  PvLimitDataSet(void)
{
    return &_gPvLimitInterfaceInfo;
}

/**
* @brief: _gPvDataUpdate
* @param:
* @return:
* @note: MPPT数据更新
*/
void PvFliterDataUpdate(void)
{

}


/**
  * @brief  AlgoInterfaceInit
  * @param  None
  * @return None
  * @note   接口初始化
*/
void AlgoInterfaceInit(void)
{
    Comp_NoiseVar_F32_Init(gstPvPwrNoiseStd + 0,5000,0.01,2);
    Comp_NoiseVar_F32_Init(gstPvPwrNoiseStd + 1,5000,0.01,2);
    Comp_NoiseVar_F32_Init(gstPvPwrNoiseStd + 2,5000,0.01,2);
    //Comp_NoiseVar_F32_Init(gstPvPwrNoiseStd + 3,2000,0.01,2);

    //_gPvData.pv_value.dutytest = 0;
}


//void PvRunStateSet(EN_PV_STATE state)
//{
//    _gPvData.en_pv_state = state;
//}
/**
  * @brief  PVRunSet
  * @param  run_set
  * @retval None
  * @note   PV运行使能
*/
void PVRunSet(bool run_set)
{
    _gPvData.en_pv = run_set;
}

/**
  * @brief  PvVolAveRead
  * @param  channal_number
  * @retval None
  * @note   返回对应通道PV电压值
*/
float PvVolAveRead(uint16_t channal_number)
{
    return _gPvData.channel[channal_number].value.pv_v_filter;
}


/**
  * @brief  PvPwrAveRead
  * @param  channal_number
  * @retval None
  * @note   返回对应通道PV功率值
*/
float PvPwrAveRead(uint16_t channal_number)
{
    return _gPvData.channel[channal_number].value.pv_p_filter;
}
/**
  * @brief  PvCurAveRead
  * @param  channal_number
  * @retval None
  * @note   返回对应通道PV电流
*/
float PvCurAveRead(uint16_t channal_number)
{
    return _gPvData.channel[channal_number].value.pv_i_filter;
}
/**
  * @brief  GetPvTotalPower
  * @param  channal_number
  * @retval None
  * @note   返回PV总功率
*/
float PvPwrRead(void)
{
    return gPvData.total_pwr_ave ;
}
/**
  * @brief  GetPvTotalPower
  * @param  channal_number
  * @retval None
  * @note   返回PV总功率
*/
void MpptSetPvPwr(float pwr)
{
    float pvpwr_set  = (pwr < 0) ? 0 : ((pwr >= _gPvData.pv_value_limit.total_power_max) \
                                 ? _gPvData.pv_value_limit.total_power_max : pwr);

    _gPvData.pv_value.total_p_set = pvpwr_set;
}
/**
  * @brief  GetPvTotalPower
  * @param  channal_number
  * @retval None
  * @note   PV最大总功率设置
*/
void PvMaxPowerSet(float pwr)
{
    float pvpwr_set  = ((pwr < 0) ? 0 :  pwr);
    _gPvData.pv_value_limit.total_power_max = pvpwr_set;
}

/**
* @brief: Read_Mppt_state
* @return: 读取PV存在状态，如有一路存在则返回值不为0
* @note: 返回值如不为0则至少有一路PV存在
*
*/
uint16_t Read_Mppt_state(uint16_t num)
{
    return (_gPvData.channel[num].value.mppt_state == 0);
}
uint16_t PvVolLowWarningCodeRead(uint16_t channal_number)
{
    return  gPvData.channel_cntl[channal_number].channel_protect.warn.bit.pv_vol_low;
}

uint16_t PvLossCtrlWarningCodeRead(uint16_t channal_number)
{
    return  gPvData.channel_cntl[channal_number].channel_protect.err.bit.pv_loss_control;
}

uint16_t PvReverseErrCodeRead(uint16_t channal_number)
{
    return  gPvData.channel_cntl[channal_number].channel_protect.err.bit.pv_reverse_err;
}

uint16_t PvWarningCodeRead(uint16_t channal_number)
{
    //**后续需补充
    return 0;
}
uint16_t PvErroCodeRead(uint16_t channal_number)
{
    //**后续需补充
    return 0;
}


