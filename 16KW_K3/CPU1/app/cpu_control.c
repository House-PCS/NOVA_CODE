/*
 * cpu_control.c
 *
 *  Created on: Nov 22, 2024
 *      Author: Nova007456
 */
#include <math.h>
#include <sample_update.h>
#include "cpu_control.h"
#include "interrupt.h"
#include "control_state.h"
#include "fault.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
#include "algo_interface.h"
#include "debug_analysis.h"
#include "cla_cpu_share.h"
#include "cap.h"
#include "AlgoFw/package/cntl/pv/pv.h"
#include "AlgoFw/component/Comp_NoiseVar.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_interface.h"
#include "inv_3ph_pwm.h"
#include "biz_fsm_interface.h"
#include "dual_core.h"
#include "dev_dual_core_receive.h"
#include "ph3_calc_ifc_update.h"

#pragma CODE_SECTION(AdcGpioStateSwitch,".TI.ramfunc");
#pragma CODE_SECTION(DualCore_TriggerCpu2Event,".TI.ramfunc");
#pragma CODE_SECTION(Ph3CalcInfo_Update,".TI.ramfunc");


extern Comp_NoiseVar_t gstPvPwrNoiseStd[4];

float gDutyPv[4];

CpuInterrupt_t gstCpuInterrupt; /* CPU interrupt time-slot state */
uint8_t SciDataTemp;          /* Temporary buffer for SCI received byte */
static uint32_t _gInterruptRunCont;
volatile EN_CPU_INTERRUPT_TIME gCpuAdcSampleTimeSlot = EN_CPU_INTERRUPT_TIME_0;
volatile uint32_t gCpuTimeSlotHitCnt[5];

/**
* @brief: AdcGpioStateSwitch
* @param: void
* @note: ADC sampling and GPIO mux state switch task
*/

void AdcGpioStateSwitch(void)
{
    switch(gstCpuInterrupt.time)
    {
        case EN_CPU_INTERRUPT_TIME_1: /* Time slot 1: switch mux to time slot 2 */
            GpioDataRegs.GPBSET.bit.GPIO34  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO35    = 1;
            GpioDataRegs.GPBSET.bit.GPIO40  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO41    = 1;
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_2;
            break;
        case EN_CPU_INTERRUPT_TIME_2: /* Time slot 2: switch mux to time slot 3 */
            GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;
            GpioDataRegs.GPBSET.bit.GPIO35    = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO40  = 1;
            GpioDataRegs.GPBSET.bit.GPIO41    = 1;
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_3;
            break;
        case EN_CPU_INTERRUPT_TIME_3: /* Time slot 3: switch mux to time slot 4 */
            GpioDataRegs.GPBSET.bit.GPIO34  = 1;
            GpioDataRegs.GPBSET.bit.GPIO35  = 1;
            GpioDataRegs.GPBSET.bit.GPIO40  = 1;
            GpioDataRegs.GPBSET.bit.GPIO41  = 1;
            FastErrWarningHandler();               /* Fast fault and warning handler */
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_4;
            break;
        case EN_CPU_INTERRUPT_TIME_4: /* Time slot 4: switch mux to time slot 1 */
            GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO35    = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO40  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO41  = 1;
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_1;
            break;
        default:
            if(Get_Adc_Time0_Calibration_State() == 1) /* ADC zero calibration done; start ADC/GPIO mux switching */
            {
               gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_1;
            }
            break;
    }
}
/**
* @brief: Cpu_Interrupt_Process
* @param: void
* @note: CPU interrupt task body, runs at 20 kHz; keep execution time short
*/
void Cpu_Interrupt_Process(void)
{
    static uint8_t pv_cal_cont = 0;

    switch(gstCpuInterrupt.time)
    {
        case EN_CPU_INTERRUPT_TIME_1:
            if(pv_cal_cont == 0)
            {
//                gPvData.bus_volt = gstAdcResult.bus_volt_pos + gstAdcResult.bus_volt_neg;
//                gPvData.channel_cntl[0].volt_inst = gstAdcResult.pv1_volt;
//                gPvData.channel_cntl[0].curr_inst = gstAdcResult.pv1_curr;
//                gPvData.channel_cntl[0].power_inst = gstAdcResult.pv1_volt * gstAdcResult.pv1_curr;
//                gPvData.channel_cntl[1].volt_inst = gstAdcResult.pv2_volt;
//                gPvData.channel_cntl[1].curr_inst = gstAdcResult.pv2_curr;
//                gPvData.channel_cntl[1].power_inst = gstAdcResult.pv2_volt * gstAdcResult.pv2_curr;
//                gPvData.channel_cntl[2].volt_inst = gstAdcResult.pv3_volt;
//                gPvData.channel_cntl[2].curr_inst = gstAdcResult.pv3_curr;
//                gPvData.channel_cntl[2].power_inst = gstAdcResult.pv3_volt * gstAdcResult.pv3_curr;
//                Pv_LoopData_Calc();
            }
            Ph3CalcInfo_Update();
            DualCore_TriggerCpu2Event();
            DualCore_CheckCpu2RunOnline(DevDcr_GetCpu2Heartbeat());
            break;
        case EN_CPU_INTERRUPT_TIME_2:
            if(pv_cal_cont == 0)
            {
//                Pv_Run();
//                gPvData.channel_cntl[0].pwr_noise_std = gstPvPwrNoiseStd[0].noise_std;
//                gPvData.channel_cntl[1].pwr_noise_std = gstPvPwrNoiseStd[1].noise_std;
//                gPvData.channel_cntl[2].pwr_noise_std = gstPvPwrNoiseStd[2].noise_std;
//                gDutyPv[0] = gPvData.channel_cntl[0].o_duty;
//                gDutyPv[1] = gPvData.channel_cntl[1].o_duty;
//                gDutyPv[2] = gPvData.channel_cntl[2].o_duty;
//                gDutyPv[3] = 0;
//                PvEpwmUpdata(gDutyPv,1);
            }
            Inv_State_Int1();                      /* Inverter state machine (INT1 rate) */
            SciaRxDataRecv();
            break;
        case EN_CPU_INTERRUPT_TIME_3:
            if(pv_cal_cont == 0)
            {
//                PV_FaultCheck_Run();
//                Comp_NoiseVar_Run_F32(gstPvPwrNoiseStd + 0,gPvData.channel_cntl[0].power_inst);
//                Comp_NoiseVar_Run_F32(gstPvPwrNoiseStd + 1,gPvData.channel_cntl[1].power_inst);
//                Comp_NoiseVar_Run_F32(gstPvPwrNoiseStd + 2,gPvData.channel_cntl[2].power_inst);
            }
            pv_cal_cont = (uint8_t)((pv_cal_cont + 1U) % 5U);
            Inv3Ph_FaultFastCheck_Run();
            ScicRxDataRecv();
            break;
        case EN_CPU_INTERRUPT_TIME_4:
            ScibRxDataRecv();
            break;
        default:
            break;
    }
}

/**
* @brief: CpuIntServiceFun
* @param: void
* @return:Nothing
* @note: CPU interrupt service routine
*/
void CpuIntServiceFun(void)
{
    static uint32_t  int_cont = 0;
    static uint32_t  int_cont_next = 0;

    int_cont = CpuTimer1Regs.TIM.all;
    gCpuAdcSampleTimeSlot = gstCpuInterrupt.time;
    if(gstCpuInterrupt.time <= EN_CPU_INTERRUPT_TIME_4)
    {
        gCpuTimeSlotHitCnt[gstCpuInterrupt.time]++;
    }
    GetAdcResult(gstCpuInterrupt.time);    /* Fetch ADC samples for current time slot */
    AdcGpioStateSwitch();                  /* Switch MUX before next EPWM ADC trigger */
    Cpu_Interrupt_Process();               /* Time-sliced interrupt tasks */
    int_cont_next = CpuTimer1Regs.TIM.all;
    if(int_cont > int_cont_next )
    {
        _gInterruptRunCont = int_cont - int_cont_next;
    }
    else if(int_cont < int_cont_next)
    {
        _gInterruptRunCont = int_cont + (CpuTimer1Regs.PRD.all + 1U - int_cont_next);
    }
    else
    {
        _gInterruptRunCont = 0U;
    }
}
/**
* @brief: GetCpuInterruptRunCont
* @param: void
* @return: Interrupt execution time in timer counts
* @note: Returns CPU interrupt service routine duration
*/
uint32_t GetCpuInterruptRunCont(void)
{
    return _gInterruptRunCont;
}
/**
* @brief: CpuIntFunInit
* @param: void
* @return:Nothing
* @note: Register CPU interrupt callback and initialize interrupts
*/
void CpuIntFunInit(void)
{
    RegisterCpuIntCallback(&CpuIntServiceFun); /* Register CPU ISR callback */
    Interrupt_Init();
}
