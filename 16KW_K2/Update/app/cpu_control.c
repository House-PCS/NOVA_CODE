/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cpu_control.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-3-6
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2025-3-6    <td>V1.0.0    <td>            <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "cpu_control.h"
#include "debug_analysis.h"
#include "interrupt.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"
 
#include "energy_management.h"
#include "inv_state.h"
#include "fault.h"
#include "cla_cpu_share.h"
#include "ethernet_analysis.h"
#include "energy_ctrl.h"
 
#include "data_process.h"
#include "dev_ntc.h"
#include "SysSelfTest.h"
#include "sys_info.h"
#include "algo_interface.h" 
#include "cap.h"
#include "AlgoFw/package/cntl/pv/pv.h"
#include "AlgoFw/component/Comp_NoiseVar.h"
#include "AlgoFw/package/cntl/pv/pv_def.h"
#include "dev_dual_core_receive.h"
#include "dual_core.h"

// #pragma CODE_SECTION(SetCpuDebugData,".TI.ramfunc");
// #pragma CODE_SECTION(GetAdcResult,".TI.ramfunc");
// #pragma CODE_SECTION(Inv_State_Int1,".TI.ramfunc");



// float gVolPv[4];
// float gCurPv[4];
float gDutyPv[4];
//TAG:测试用
volatile uint16_t gEnPv = 0;
volatile uint16_t gSelfTestEnPvRun = 0;
extern EN_INV_STATE geInvWorkState;    //
CpuInterrupt_t gstCpuInterrupt;//CPU中断的状态
uint8_t SciDataTemp;    //缓存sci接收数据

static uint32_t _gInterruptRunCont;
uint32_t max1,max2,max3,max4;
uint32_t g_cpu2_heartbeat_cnt = 0;

/*guo test data：begin*/
extern  uint16_t _gEnMppt;
extern float _gBusVoltage;
extern float32 filtered_bus_voltage;
extern float32 bus_control_actpwr_out;
extern Comp_NoiseVar_t gstPvPwrNoiseVar[4]; //PV功率噪声方差计算组件
extern Comp_NoiseVar_t gstPvPwrTotalNoiseVar; //PV功率噪声方差计算组件

void CpuDebugMsgUpdate()
{
    SetCpuDebugData(0, gEnergyCtrlInput.inv_stat);
    SetCpuDebugData(1, gPvData.total_pwr_inst);
    SetCpuDebugData(2, gPvData.channel_cntl[1].observer.alpha_hat_smooth);//20
    SetCpuDebugData(3, gPvData.total_pwr_filter);
    SetCpuDebugData(4, gPvData.channel_cntl[1].o_vref);
    SetCpuDebugData(5, gPvData.channel_cntl[1].volt_inst);
    SetCpuDebugData(6, gPvData.channel_cntl[1].curr_inst);
    SetCpuDebugData(7, gPvData.channel_cntl[1].pwr_limit);
    SetCpuDebugData(8, gPvData.channel_cntl[1].power_inst);
    SetCpuDebugData(9, gEnergyCtrlInput.work_mode);//25
    SetCpuDebugData(10, gPvCfg.pwr_limit);//25
    SetCpuDebugData(11, gEnergyCtrlInput.peak_shift_stat);//25
    SetCpuDebugData(12,gPvData.channel_cntl[1].bus_voltage);
//    SetCpuDebugData(13, gPvData.channel_cntl[1].channel_protect.err.byte);
//    SetCpuDebugData(14, gstPvPwrNoiseVar[1].noise_std);
//    SetCpuDebugData(15, gPvData.channel_cntl[1].o_duty);
   //gPvData.channel_cntl[1].channel_protect.err.bit.pv_over_current

    // SetCpuDebugData(0, geInvWorkState);
    // SetCpuDebugData(1, gEnergyCtrlInput.work_mode);
    // SetCpuDebugData(2, gstFaultInfo.err_flag);//20
    // //SetCpuDebugData(3, gstPVfault.PV_ErrorCode.byte);
    // SetCpuDebugData(3, gstFaultInfo.inside_warn_gen.byte);
    // SetCpuDebugData(4, gEnergyCtrlInput.power_act_grid2pcs_get);
    // SetCpuDebugData(5, gEnergyCtrlInput.power_act_inv2pcs_get);                  
    // SetCpuDebugData(6, gstFaultInfo.inside_err_other.byte);
    // SetCpuDebugData(7, gEnergyCtrlInput.power_max_bat_chg_get);//25
    // SetCpuDebugData(8, gEnergyCtrlInput.power_max_bat_dsg_get);
    // SetCpuDebugData(9, gEnergyCtrlInput.power_max_gen_get);  
    // SetCpuDebugData(10, gstAlgoInterface.generator_active_power);   
    // SetCpuDebugData(11, gstFaultInfo.inside_warn_grid.byte);
    // SetCpuDebugData(12, gEnergyCtrlOutput.power_act_grid2pcs_set);
    // //SetCpuDebugData(12, gEnergyCtrlOutput.power_act_gen2pcs_set);
     SetCpuDebugData(13, gEnergyCtrlOutput.power_pv2bus_set);
     SetCpuDebugData(14, gEnergyCtrlOutput.power_bat2bus_set);
     SetCpuDebugData(15, g_cpu2_heartbeat_cnt);
    // SetCpuDebugData(15,gstUdpPcsPara.udp_pcs_hold.drm_state);

}

/*guo test data：end*/

/**
* @brief: AdcGpioStateSwitch
* @param: void
* @note:ADC的GPIO状态切换控制
*/
void AdcGpioStateSwitch(void)
{
    switch(gstCpuInterrupt.time)
    {
        case EN_CPU_INTERRUPT_TIME_1://TAG:运行时间9us
        {
            g_cpu2_heartbeat_cnt = DevDcr_GetCpu2Heartbeat();
            /*      pv AD采样值切换  当前处于 S0低电平,S1低电平,已完成AD拿值,切换至time2的采样通道*/
            GpioDataRegs.GPBSET.bit.GPIO34  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO35    = 1;
            /*      AD采样值切换  当前处于 S0低电平,S1低电平,已完成AD拿值,切换至time2的采样通道*/
            GpioDataRegs.GPBSET.bit.GPIO40  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO41    = 1;

            /***********************************/
                        //PV输入输出数据更新
            // gVolPv[0] = gstAdcResult.pv1_volt;
            // gVolPv[1] = gstAdcResult.pv2_volt;
            // gVolPv[2] = gstAdcResult.pv3_volt;
            // gVolPv[3] = gstAdcResult.pv4_volt;
            // gCurPv[0] = gstAdcResult.pv1_curr;
            // gCurPv[1] = gstAdcResult.pv2_curr;
            // gCurPv[2] = gstAdcResult.pv3_curr;
            // gCurPv[3] = gstAdcResult.pv4_curr;
            // PvDataUpdata(gVolPv,gCurPv, (gstAdcResult.bus_volt_all)); //TAG:执行时间250ns
            gPvData.bus_volt = gstAdcResult.bus_volt_all;//TAG:对母线电压进行一阶低通滤波，滤波系数0.001
            // gPvData.bus_volt = gstAdcResult.bus_volt_all*0.001+ gPvData.bus_volt*0.999;//TAG:对母线电压进行一阶低通滤波，滤波系数0.001
            gPvData.channel_cntl[0].volt_inst = gstAdcResult.pv1_volt;
            gPvData.channel_cntl[0].curr_inst = gstAdcResult.pv1_curr;
            gPvData.channel_cntl[0].power_inst = gstAdcResult.pv1_volt * gstAdcResult.pv1_curr;
            gPvData.channel_cntl[1].volt_inst = gstAdcResult.pv2_volt;
            gPvData.channel_cntl[1].curr_inst = gstAdcResult.pv2_curr;
            gPvData.channel_cntl[1].power_inst = gstAdcResult.pv2_volt * gstAdcResult.pv2_curr;
            gPvData.channel_cntl[2].volt_inst = gstAdcResult.pv3_volt;
            gPvData.channel_cntl[2].curr_inst = gstAdcResult.pv3_curr;
            gPvData.channel_cntl[2].power_inst = gstAdcResult.pv3_volt * gstAdcResult.pv3_curr;
            gPvData.channel_cntl[3].volt_inst = gstAdcResult.pv4_volt;
            gPvData.channel_cntl[3].curr_inst = gstAdcResult.pv4_curr;
            gPvData.channel_cntl[3].power_inst = gstAdcResult.pv4_volt * gstAdcResult.pv4_curr;
            //PV运行
            static uint16_t pv_calc_count = 0;
            if(++pv_calc_count >= 4) //每500us进行一次PV计算
            {
                pv_calc_count = 0;
                Pv_LoopData_Calc();
            }
            
            /***********************************/
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_2;
            break;
        }
        case EN_CPU_INTERRUPT_TIME_2://TAG:运行时间5.7us
        {
            /*      pv AD采样值切换  当前处于 S0低电平,S1高电平,已完成AD拿值,切换至time3的采样通道*/
            GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;
            GpioDataRegs.GPBSET.bit.GPIO35    = 1;
            /*      AD采样值切换  当前处于 S0低电平,S1高电平,已完成AD拿值,切换至time3的采样通道*/
            GpioDataRegs.GPBCLEAR.bit.GPIO40    = 1;
            GpioDataRegs.GPBSET.bit.GPIO41  = 1;

            /***********************************/
            //TAG:PV测试用
//            uint16_t bus_normal = (gstAdcResult.bus_volt_all > WARN_INV_BUS_OVER );
            /***********************************/
            /***********************************/
            //PV输入输出数据更新
            static uint16_t pv_run_count = 0;
            if(++pv_run_count >= 4) //每500ms进行一次MPPT计算
            {
                pv_run_count = 0;
            }
            Pv_Run(pv_run_count);
            gPvData.channel_cntl[0].pwr_noise_std = gstPvPwrNoiseVar[0].noise_std;
            gPvData.channel_cntl[1].pwr_noise_std = gstPvPwrNoiseVar[1].noise_std;
            gPvData.channel_cntl[2].pwr_noise_std = gstPvPwrNoiseVar[2].noise_std;
            gPvData.channel_cntl[3].pwr_noise_std = gstPvPwrNoiseVar[3].noise_std;
            gDutyPv[0] = gPvData.channel_cntl[0].o_duty;
            gDutyPv[1] = gPvData.channel_cntl[1].o_duty;
            gDutyPv[2] = gPvData.channel_cntl[2].o_duty;
            gDutyPv[3] = gPvData.channel_cntl[3].o_duty;
            PvEpwmUpdata(gDutyPv,gEnPv); //TODO:gEnPv与其他模块联系
            /***********************************/
            /***********************************/
            //TAG:测试用，后续gEnPv可由其他模块例如故障诊断模块，来关闭PV
            if(Get_cla_Inv_state() != INV_STATE_STANDBY && \
                        (gPvData.channel_cntl[0].is_err == 0 && \
                        gPvData.channel_cntl[1].is_err == 0 && \
                        gPvData.channel_cntl[2].is_err == 0 && \
                        gPvData.channel_cntl[3].is_err == 0))
            {
                gEnPv = 1;
            }
            else
            {
                gEnPv = 0;
            }
            if(GetError_Pv1_LoseControl()==1)
            {
                gEnPv = 0;
            }
            Pv_Set_EnRun(gEnPv);
            
            /***********************************/
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_3;
            break;

        }
        case EN_CPU_INTERRUPT_TIME_3:
        {
            /*      pv AD采样值切换  当前处于 S0高电平,S1高电平,已完成AD拿值,切换至time4的采样通道*/
            GpioDataRegs.GPBSET.bit.GPIO34  = 1;
            GpioDataRegs.GPBSET.bit.GPIO35    = 1;
            /*      AD采样值切换  当前处于 S0高电平,S1低电平,已完成AD拿值,切换至time4的采样通道*/
            GpioDataRegs.GPBSET.bit.GPIO40    = 1;
            GpioDataRegs.GPBSET.bit.GPIO41    = 1;
            /* 继电器中点电压 RMS、母线电压电流有效值与母线功率 — 与 CLA 原 gstMidRelay/gstBusData 一致，改在 CPU 侧计算 */
            PwrCpuRelayBusStep();
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_4;
            static uint32_t pv_fault_check_cont = 0;
            if(++pv_fault_check_cont >= 4)//每500us进行一次PV故障检测
            {
                pv_fault_check_cont = 0;
            }
            PV_FaultCheck_Run(pv_fault_check_cont); //PV故障检测运行函数
            Comp_NoiseVar_Run_F32(&gstPvPwrNoiseVar[0], gPvData.channel_cntl[0].power_inst);
            Comp_NoiseVar_Run_F32(&gstPvPwrNoiseVar[1], gPvData.channel_cntl[1].power_inst);
            Comp_NoiseVar_Run_F32(&gstPvPwrNoiseVar[2], gPvData.channel_cntl[2].power_inst);
            Comp_NoiseVar_Run_F32(&gstPvPwrNoiseVar[3], gPvData.channel_cntl[3].power_inst);
            Comp_NoiseVar_Run_F32(&gstPvPwrTotalNoiseVar, 
                gPvData.channel_cntl[0].power_inst
                + gPvData.channel_cntl[1].power_inst
                + gPvData.channel_cntl[2].power_inst
                + gPvData.channel_cntl[3].power_inst);
            break;
        }
        case EN_CPU_INTERRUPT_TIME_4:
        {
            /*      pv AD采样值切换  当前处于 S0低电平,S1低电平,已完成AD拿值,切换至time1的采样通道*/
            GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO35    = 1;
            /*      AD采样值切换  当前处于 S0高电平,S1高电平,已完成AD拿值,切换至time1的采样通道*/
            GpioDataRegs.GPBCLEAR.bit.GPIO40  = 1;
            GpioDataRegs.GPBCLEAR.bit.GPIO41  = 1;
//            Fault_Int1();s
             ErrWarningHandler();
             Inv_State_Int1();
            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_1;
            break;
        }
        default:
        {
            if(Get_Adc_Calibration_State() == 1)//已完成ADC的零点校准,开始正常切换adc的GPIO
            {
                if((EPwm10Regs.TBCTR > (EPwm10Regs.TBPRD >> 1)) && (EPwm10Regs.TBSTS.bit.CTRDIR == 1))
                {
                    gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_4;
                }
            }
            break;
        }
    }
}
/**
* @brief: CPU_Interrupt
* @param: void
* @return:Nothing
* @note:CPU中断服务函数
*/
void CpuIntServiceFun(void)
{

    static uint32_t  int_cont = 0;
     static uint32_t  int_cont_next = 0;
    int_cont = CpuTimer1Regs.TIM.all;

    TestCpuGpioSet();                       //测试使用
    /*    拿出AD值后,Gpio切换操作    */
    GetAdcResult(gstCpuInterrupt.time);    //获取当前时间段AD采样值
    AdcGpioStateSwitch();                  //ADC的GPIO切换控制

//todo:Pv相关信息打印
    EnergyCtrlInput_t* energy_ctrl_input = GetEnergyInput();
    EnergyCtrlOutput_t* energy_ctrl_output = GetEnergyOutput();

    /* 虚拟一个50us的时间周期 */
    if(gstCpuInterrupt.cpu_int_count % 4 == 0)
    {
        /* 接收ARC 485数据（scib） */
        ScibRxDataRecv();
    }
    else if(gstCpuInterrupt.cpu_int_count % 4 == 1)
    {
        // /* 接收MCU数据（scia）,是否需要接收MCU信号需看后续通信需求 */
        // SciaRxDataRecv();
        DualCore_TriggerCpu2Event(); //TAG:触发CPU2事件，用于CPU2进行数据处理
    }
    else if(gstCpuInterrupt.cpu_int_count % 4 == 2)
    {
        /* 接收Menter ISO485数据（scic） */
        ScicRxDataRecv();
    }
    else if(gstCpuInterrupt.cpu_int_count % 4 == 3)
    {
        /* 接收MCU数据（scia）,是否需要接收MCU信号需看后续通信需求 */
        SciaRxDataRecv();
    }

    gstCpuInterrupt.cpu_int_count++;

   


    int_cont_next = CpuTimer1Regs.TIM.all;
    if(int_cont > int_cont_next )
    {
        _gInterruptRunCont = int_cont - int_cont_next;
    }
    else
    {
        _gInterruptRunCont = int_cont + (0x7FFFFFFF - int_cont_next);
    }

        //计算每一个定时器中断时间，测试用
     if (gstCpuInterrupt.time == EN_CPU_INTERRUPT_TIME_1)
        max1 = _gInterruptRunCont;
     if (gstCpuInterrupt.time == EN_CPU_INTERRUPT_TIME_2)
        max2 = _gInterruptRunCont;
     if (gstCpuInterrupt.time == EN_CPU_INTERRUPT_TIME_3)
        max3 = _gInterruptRunCont;
     if (gstCpuInterrupt.time == EN_CPU_INTERRUPT_TIME_4)
        max4 = _gInterruptRunCont;  

    /*   新增加的CPU中断函数放在此上方添加   */
    TestCpuGpioReset();
}

/**
* @brief: CpuIntServiceFun
* @param: void
* @return:Nothing
* @note:CPU中断服务函数
*/
uint32_t GetCpuInterruptRunCont(void)
{
    return _gInterruptRunCont;
}

void CpuIntFunInit(void)
{
    PwrCpuRelayBusInit();
    RegisterCpuIntCallback(&CpuIntServiceFun);
    // Interrupt_Init();
}

