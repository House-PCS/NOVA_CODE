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

#include "dev_led.h"
#include "cpu_timers.h"
#include "gpio.h"
#include "interrupt.h"
#include "pwm.h"
#include "sci.h"
#include "spi.h"
#include "system.h"
#include "CH390_Interface.h"
#include "dev_flash.h"

#include "F28x_Project.h"
#include "debug_analysis.h"
#include "ethernet_analysis.h"
#include "system.h"
#include "string.h"
#include "adc.h"
 
#include "energy_management.h"
#include <algo_interface.h>
#include "dev_ntc.h"

#include "cpu_control.h"//TODO:测试用后续删除
#include "dev_flash.h"
#include "protocol_process.h"
#include "sys_info.h"
#include "energy_management.h"
#include "fan.h"
#include "relay_ctrl.h"
#include "SysSelfTest.h"
#include "ElectricInfoGet.h"
#include "flash_default.h"
#include "cap.h"
#include "cla_control_cpu.h"
#include "AlgoFw/package/cntl/pv/pv.h"
#include "AlgoFw/component/Comp_NoiseVar.h"
#include "dual_core.h"
#include "dev_dual_core_receive.h"
#include "dual_data_share.h"

extern Comp_NoiseVar_t gstPvPwrNoiseVar[4]; //PV功率噪声方差计算组件
//TODO:CMD文件目前没适配boot程序,在线升级稳定后需要修改CMD中APP与BOOT的空间分配,以及注意编译出来的bin文件大小问题

    bool en_inv ;

void CpuDebugMsgUpdate();
void main(void)
{
    InitSysCtrl();

    DELAY_MS(500);
    DINT;//DINT关闭中断

    InitFlash();
    IER = 0x0000;
    IFR = 0x0000;

    InitPieCtrl();
    InitPieVectTable();
    DualCore_InitEarly();               /* CPUSEL/GS ownership only */
    CpuIntFunInit();
    Interrupt_Init();   //中断外设配置
    EPwm_Init();        //EPWM外设配置

    Cap_Init();

    Gpio_Init();        //GPIO外设配置
    Sci_Init();         //SCI外设配置
    Spi_Init();         //Spi外设配置
    Adc_Init();         //ADC配置初始化
    CpuTimer_Init();    //CPU定时器外设配置

    DevLedInit();       //Led外设初始化
    FlashInit();        //片外Flash初始化
    ProtocolInit();     //所有协议初始化
    FanInit();          //风扇初始化
    SysSelfTestInit();
    RelaySelfTestInit();
    // printf("Test OK!");
    Pv_Init();           //PV控制初始化

    //TAG:测试用
//    MpptSetPvPwr(2000);
    /*guo:pv boost test begin:*/
    //PvRunStateSet(1);
    /*guo:pv boost test end:*/
    AlgoInterfaceInit();
    ClaControlCpuInitOnce();
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
            DualDataShare_Init();
        }

        if (bootStatus != DUAL_CORE_OK)
        {
            DualCore_SetCpu2Online(false);
        }
    }

    EALLOW;
    ClearEpwmTz(1);
    ClearEpwmTz(2);
    ClearEpwmTz(8);
    ClearEpwmTz(9);
    //清除
    EventClear(7);
    EventClear(11);
    CLATaskEnable();
    CPUETINTSet(7);
    CLAETINTSet(11);
    EDIS;
    EINT;//EINT打开中断
    ERTM;
    for(;;)
    {
        ProtocolProcess();
        CpuTimerFlag();
        DevLedProcess();
        BackUpSwitchCtrlFunc();
        InterfaceCmd(gDebugValue[0], 0,  gDebugValue[2], 0,gstAdcZero.calibration_finish); //TODO:
        if(Get_1ms_Flag() == 1) //1ms函数
        {
#if !PRINTF_ENABLE
            CpuDebugMsgUpdate();
            if(GetEthSysUpgradeState() == false)
            {
                Debug_Send();
            }
#endif
            EnergyScheduling();
            TempValueUpdate();
            Timers_1ms();
        }
        if(Get_5ms_Flag() == 1) //5ms函数
        {
//            EnergyScheduling();
            Timers_5ms();
        }
        if(Get_10ms_Flag() == 1) //10ms函数
        {
            if (Get_Adc_Calibration_State())
            {
                SysSelfTest();
            }
            // ShutDown_SaveFlash();
            UdpDspHoldPara_AutoSaveIfNeeded();
            Restore_Factory_Settings();

            // Comp_NoiseVar_Run_F32(&gstPvPwrNoiseVar[2], gPvData.channel_cntl[2].power_inst);
            Timers_10ms();
        }
        if(Get_20ms_Flag() == 1) //20ms函数
        {
//            Fault20msHandle();
            static uint16_t mppt_count = 0;
            if(++mppt_count >= 4) //每500ms进行一次MPPT计算
            {
                mppt_count = 0;
            }
            Pv_Mppt_Run(mppt_count);
            Timers_20ms();
        }
        if(Get_100ms_Flag() == 1) //100ms函数
        {
            FanProcessAll();         //风扇控制函数
            //gInvTemp = gstNtcTemperature.Inv_temperature;
            gInvTemp = GetNTCMaxTemp();
            ErrWarningHandler_100ms();

            Timers_100ms();
        }
        if(Get_1s_Flag() == 1) //1s函数
        {
            updateEnergyState();
            // Pv_Set_Pwr_Limit(2000);
            Timers_1s();
        }
    }
}
//
// End of file
//
