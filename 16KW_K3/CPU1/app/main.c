/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        main.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-2-4
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
  */
#include "ph3_sts_ifc_update.h"
#include <adc.h>
#include <sample_update.h>
#include "dev_led.h"
#include "cpu_timers.h"
#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "sci.h"
#include "spi.h"
#include "system.h"
#include "CH390_Interface.h"
#include "cpu_control.h"
#include "dev_flash.h"
#include "fault.h"
#include "adc.h"
#include "cap.h"
#include "system.h"
#include "F28x_Project.h"
#include "debug_analysis.h"
#include "ethernet_analysis.h"
#include "string.h"
#include "dev_flash.h"
#include "algo_interface.h"
#include "fan.h"
#include "protocol_process.h"
#include "ntc.h"
#include "can.h"
#include "relay_ctrl.h"
#include "ct.h"
#include "cla_cpu_share.h"
#include "electric_calculate.h"
#include "AlgoFw/package/cntl/pv/pv.h"
#include "algo_interface.h"
#include "biz_fsm_interface.h"
#include "dual_core.h"
#include "dev_dual_core_receive.h"
#include "dual_data_share.h"
#include "ph3_calc_ifc_update.h"
#include "AlgoFw/package/cntl/inverter_3ph/inverter_3ph_interface.h"
#include "pwr_ctrl_interface.h"
#include "control_state.h"
#include "pwr_ctrl_interface.h"
#include "pwr_ctrl_func.h"
#include "phase_seq.h"
void main(void)
{
    InitSysCtrl();
    DELAY_MS(500);
    DINT;                               /* Disable global interrupts */

    InitFlash();
    IER = 0x0000;
    IFR = 0x0000;

    InitPieCtrl();
    InitPieVectTable();
    DualCore_InitEarly();               /* CPUSEL/GS ownership only */
    CpuIntFunInit();                    /* Register CPU ISR + interrupt init */
    FanInit();                          /* Fan module init */
    EPwm_Init();                        /* EPWM driver init */
    Cap_Init();
    Gpio_Init();                        /* GPIO configuration */
    Sci_Init();                         /* SCI configuration */
    Spi_Init();                         /* SPI configuration */
    AdcConfig();                        /* ADC configuration */
    CpuTimer_Init();                    /* CPU timer configuration */
    CanInit();
    DevLedInit();                       /* LED GPIO init */
    FlashInit();                        /* On-chip Flash init */
    ProtocolInit();                     /* Communication protocol init */
    ClearEpwmTZ();                      /* Clear EPWM trip-zone faults */

    Pv_Init();
    AlgoInterfaceInit();
    Inv3Ph_Init();
    #warning TODO: review test-only PV/print paths before release; keep behavior consistent
    NTCInit();
    RelayControlInit();
    ErrWarningCheckInit();
    BizFsm_Interface_Init();
    PwrCtrlInterface_Init();
    ControlStateInit();
    {
        DualCore_Status_t bootStatus =
            DualCore_BootCpu2WithTimeout(IPC_DUAL_BOOT_TIMEOUT_MS);

        if (bootStatus == DUAL_CORE_ERR_ALREADY_BOOTED)
        {
            bootStatus = DUAL_CORE_OK;
        }

        if (bootStatus == DUAL_CORE_OK)
        {
            bootStatus = DualCore_HandshakeCpu1(IPC_DUAL_HANDSHAKE_TIMEOUT_MS);
        }

        if (bootStatus == DUAL_CORE_OK)
        {
            Ph3CalcInfo_Init();
            DualDataShare_Init();
            if (DevDcr_ExchangeAddress(DEV_DCR_ADDR_EXCHANGE_TIMEOUT_MS) == false)
            {
                bootStatus = DUAL_CORE_ERR_CPU2_NOT_ALIVE;
            }
        }

        if (bootStatus != DUAL_CORE_OK)
        {
            DualCore_SetCpu2PowerOnOnline(false);
        }
    }
//    EALLOW;
//    ClearEpwmTz(1);
//    ClearEpwmTz(2);
//    ClearEpwmTz(8);
//    ClearEpwmTz(9);
//    EventClear(7);
//    EventClear(11);
//    CLATaskEnable();
//    CPUETINTSet(7);
//    CLAETINTSet(11);
//    EDIS;
    DualDataShare_UpdatePh3Cmd();
    Ph3StsInfo_Init();
    RelayControl(EN_RELAT_CTRL_TYPE_EXC, RELAY_ON);
    SetStsEnRunEn(true);
    SetStsEnForceGenMode(0);
    EINT;                               /* Enable global interrupts */
    ERTM;
    for(;;)
    {
        ProtocolProcess();
        CpuTimerFlag();
        DevLedProcess();
        BackUpSwitchCtrlFunc();
        if(Get_1ms_Flag() == 1)         /* 1 ms task */
        {
            CtReversePower(gstUdpPcsPara.udp_pcs_hold.ct_direction);      //CT方向调节
            EnergyManagementDataUpdate();                                 //更新功率模块信息
            EnergyScheduling();                                           //执行功率模块运算
            PcsStatePermit();
            if(!GetEthSysUpgradeState())
            {
                Debug_Send();
            }
            TempVolUpdate();

            SetCpuDebugData(0,gstFaultInfo.inside_err.byte[0]);//16
            SetCpuDebugData(1,gstFaultInfo.inside_err.byte[1]);//17
            SetCpuDebugData(2,gstFaultInfo.inside_add_fault.byte[0]);//18
            SetCpuDebugData(3,GetInvRelayFlag());//19
            SetCpuDebugData(4,Get3PhIsRunFlag());//20
            SetCpuDebugData(5,Get3PhIsConnectFlag());//21
            SetCpuDebugData(6,Get3PhIsErrFlag());//22
            SetCpuDebugData(7,Get3PhRunState());//23
            SetCpuDebugData(8,Get3PhErrType().word);//24

            SetCpuDebugData(9,  DualCore_IsCpu2RunOnline());//25
            SetCpuDebugData(10,  DualCore_IsCpu2PowerOnOnline());//26
            SetCpuDebugData(11,  DevDcr_GetCpu2IsrRunCnt());//27
            SetCpuDebugData(12,  GetCpuInterruptRunCont());//28
            SetCpuDebugData(13,  DevDcr_GetCpu2IsrIdleCnt());//29

            SetCpuDebugData(16,  gstPh3CalcInfo.ferq_devision);//32
            SetCpuDebugData(17,  gstPh3CalcInfo.inv_curr.a_phase_rms);//33
            SetCpuDebugData(18,  gstPh3CalcInfo.inv_curr.b_phase_rms);//34
            SetCpuDebugData(19,  gstPh3CalcInfo.inv_curr.c_phase_rms);//35
            SetCpuDebugData(20,  gstPh3CalcInfo.inv_curr.freq);//36
            SetCpuDebugData(21,  gstPh3CalcInfo.inv_curr.theta);//37
            SetCpuDebugData(22,  gstPh3CalcInfo.inv_vol.a_phase_rms);//38
            SetCpuDebugData(23,  gstPh3CalcInfo.inv_vol.b_phase_rms);//39
            SetCpuDebugData(24,  gstPh3CalcInfo.inv_vol.c_phase_rms);//40
            SetCpuDebugData(25,  gstPh3CalcInfo.inv_vol.freq);//41
            SetCpuDebugData(26,  gstPh3CalcInfo.inv_vol.theta);//42
            SetCpuDebugData(27,  gstPh3CalcInfo.bus.pos_vol_filter);//43
            SetCpuDebugData(28,  gstPh3CalcInfo.bus.neg_vol_filter);//44
            SetCpuDebugData(29,  gstPh3CalcInfo.bus.power_filter);//45
            SetCpuDebugData(30,  gstPh3CalcInfo.bus.curr_filter);//46

            SetCpuDebugData(31,  gstPh3CalcInfo.load_curr.a_phase_rms);//47
            SetCpuDebugData(32,  gstPh3CalcInfo.load_curr.b_phase_rms);//48
            SetCpuDebugData(33,  gstPh3CalcInfo.load_curr.c_phase_rms);//49
            SetCpuDebugData(34,  gstPh3CalcInfo.load_curr.freq);//50
            SetCpuDebugData(35,  gstPh3CalcInfo.load_curr.theta);//51
            SetCpuDebugData(36,  gstPh3CalcInfo.load_vol.a_phase_rms);//52
            SetCpuDebugData(37,  gstPh3CalcInfo.load_vol.b_phase_rms);//53
            SetCpuDebugData(38,  gstPh3CalcInfo.load_vol.c_phase_rms);//54
            SetCpuDebugData(39,  gstPh3CalcInfo.load_vol.freq);//55
            SetCpuDebugData(40,  gstPh3CalcInfo.load_vol.theta);//56
            SetCpuDebugData(41,  gstPh3CalcInfo.gen_curr.a_phase_rms);//57
            SetCpuDebugData(42,  gstPh3CalcInfo.gen_curr.b_phase_rms);//58
            SetCpuDebugData(43,  gstPh3CalcInfo.gen_curr.c_phase_rms);//59
            SetCpuDebugData(44,  gstPh3CalcInfo.gen_curr.freq);//60
            SetCpuDebugData(45,  gstPh3CalcInfo.gen_curr.theta);//61
            SetCpuDebugData(46,  gstPh3CalcInfo.gen_vol.a_phase_rms);//62
            SetCpuDebugData(47,  gstPh3CalcInfo.gen_vol.b_phase_rms);//63
            SetCpuDebugData(48,  gstPh3CalcInfo.gen_vol.c_phase_rms);//64
            SetCpuDebugData(49,  gstPh3CalcInfo.gen_vol.freq);//65
            SetCpuDebugData(50,  gstPh3CalcInfo.gen_vol.theta);//66
            SetCpuDebugData(51,  gstPh3CalcInfo.grid_curr.a_phase_rms);//67
            SetCpuDebugData(52,  gstPh3CalcInfo.grid_curr.b_phase_rms);//68
            SetCpuDebugData(53,  gstPh3CalcInfo.grid_curr.c_phase_rms);//69
            SetCpuDebugData(54,  gstPh3CalcInfo.grid_curr.freq);//70
            SetCpuDebugData(55,  gstPh3CalcInfo.grid_curr.theta);//71
            SetCpuDebugData(56,  gstPh3CalcInfo.grid_vol.a_phase_rms);//72
            SetCpuDebugData(57,  gstPh3CalcInfo.grid_vol.b_phase_rms);//73
            SetCpuDebugData(58,  gstPh3CalcInfo.grid_vol.c_phase_rms);//74
            SetCpuDebugData(59,  gstPh3CalcInfo.grid_vol.theta);//75
            SetCpuDebugData(60,  gstPh3CalcInfo.grid_vol.freq);//76
//            SetCpuDebugData(61,gstAdcResult.bus_volt_pos);//77
//            SetCpuDebugData(62,gstAdcResult.bus_volt_neg);   //78
//            SetCpuDebugData(63,gstAdcResult.inv_volt_phase_a);//79
//            SetCpuDebugData(64,gstAdcResult.inv_volt_phase_b);//80
//            SetCpuDebugData(65,gstAdcResult.inv_volt_phase_c);//81
//            SetCpuDebugData(66,gstAdcResult.inv_curr_phase_a);//82
//            SetCpuDebugData(67,gstAdcResult.inv_curr_phase_b);//83
//            SetCpuDebugData(68,gstAdcResult.load_volt_phase_a);//84
//            SetCpuDebugData(69,gstAdcResult.load_volt_phase_b);//85
//            SetCpuDebugData(70,gstAdcResult.load_volt_phase_c);//86
//            SetCpuDebugData(71,gstAdcResult.load_curr_phase_a);//87
//            SetCpuDebugData(72,gstAdcResult.load_curr_phase_b);//88
//            SetCpuDebugData(73,gstAdcResult.load_curr_phase_c);//89
            SetCpuDebugData(74,gstAdcResult.grid_volt_phase_a);//90
           SetCpuDebugData(75,gstAdcResult.grid_volt_phase_b);//91
            SetCpuDebugData(76,gstAdcResult.grid_volt_phase_c);//92
//            SetCpuDebugData(77,gstAdcResult.grid_curr_phase_a);//93
//            SetCpuDebugData(78,gstAdcResult.grid_curr_phase_b);//94
//            SetCpuDebugData(79,gstAdcResult.grid_curr_phase_c);//95


        //    SetCpuDebugData(88,gstAdcZero.common_calibration_zero);//104
        //    SetCpuDebugData(89,gstAdcZero.inv_curr_phase_a_zero);//105
        //    SetCpuDebugData(90,gstAdcZero.inv_curr_phase_b_zero);//106
        //    SetCpuDebugData(91,gstAdcZero.inv_curr_phase_c_zero);//107
        //    SetCpuDebugData(92,gstAdcZero.grid_curr_phase_a_zero);//108
        //    SetCpuDebugData(93,gstAdcZero.grid_curr_phase_b_zero);//109
        //    SetCpuDebugData(94,gstAdcZero.grid_curr_phase_c_zero);//110
        //    SetCpuDebugData(95,gstAdcZero.pv1_curr_zero);//111
        //    SetCpuDebugData(96,gstAdcZero.pv2_curr_zero);//112
        //    SetCpuDebugData(97,gstAdcZero.pv3_curr_zero);//113
        //    SetCpuDebugData(98,gstAdcZero.gen_curr_phase_a_zero);//114
        //    SetCpuDebugData(99,gstAdcZero.gen_curr_phase_b_zero);//115
        //    SetCpuDebugData(100,gstAdcZero.gen_curr_phase_c_zero);//116
        //    SetCpuDebugData(101,gstAdcZero.load_curr_phase_a_zero);//117
        //    SetCpuDebugData(102,gstAdcZero.load_curr_phase_b_zero);//118
        //    SetCpuDebugData(103,gstAdcZero.load_curr_phase_c_zero);//119
        //    SetCpuDebugData(104,gstAdcResult.V1_5);//120
        //    SetCpuDebugData(105,  gstPh3CalcInfo.load_curr.a_phase_rms);//121
        //    SetCpuDebugData(106,  gstPh3CalcInfo.load_curr.b_phase_rms);//122
        //    SetCpuDebugData(107,  gstPh3CalcInfo.load_curr.c_phase_rms);//123

            Timers_1ms();
        }
        if(Get_5ms_Flag() == 1)         /* 5 ms task */
        {
            Timers_5ms();
        }
        if(Get_10ms_Flag() == 1)        /* 10 ms task */
        {
            BizFsm_Interface_Update(NULL);
            Timers_10ms();
        }
        if(Get_20ms_Flag() == 1)        /* 20 ms task */
        {
            SlowErrWarningHandler();    /* Slow fault / warning handler */
            ElectricParamClear(&gstUdpPcsPara.udp_pcs_hold.sys_elec_clear);    //电量清除
            ElectricPowerAccumulate();  /* Energy metering accumulation */
            Timers_20ms();
        }
        if(Get_100ms_Flag() == 1)       /* 100 ms task */
        {
            FanProcessAll();            /* Fan control */

            static uint16_t pv_count = 0;
            if(++pv_count >= 2)
            {
                pv_count = 0;
                Pv_Mppt_Run();          /* MPPT algorithm (every 200 ms) */
            }
            Timers_100ms();
        }
        if(Get_500ms_Flag() == 1)       /* 500 ms task */
        {
            Timers_500ms();
        }

        if(Get_1s_Flag() == 1)          /* 1 s task */
        {
            //printf("%d,%d,%d\r\n", (int)(gstAdcResult.pv_temp * 100),(int)(gstAdcResult.inv_temp * 100),(int)(gstAdcResult.ctr_temp * 100));
            Pv_Set_Pwr_Limit(24000*1.1f);
            Timers_1s();
        }
    }
}
//
// End of file
//
