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

//
// Included Files
//
#include "F28x_Project.h"
#include "cpu_timers.h"
#include "adc.h"
#include "algorithm_interface.h"
#include "algo_debug_msg.h"
#include "algo_err_check.h"
#include "algo_info.h"
#include "dev_flash.h"
#include "inverter_loop_cntl.h"
#include "llc_dcdc_cntl.h"
#include "sci.h"
#include "mid_comservice.h"
#include "voltagesourceinvlcfltr.h"
#include "modbus_analysis.h"
#include "energy_management.h"
#include "inverter_loop_cntl.h"
#include "pwm_cntl.h"
#include "fault_diagnosis.h"
#include "sys_info.h"
#include "fan.h"
#include "F2837xD_Ipc_drivers.h"
#include "activation.h"
#include "authentication_func.h"
#include "cla_algo.h"
#include "can.h"
#include "continus_operation.h"
#include "can_comm_process.h"
// Defines
//
#define BLINKY_LED_GPIO    75
#define DSP_CRASH       86
#define RELAY0          87
#define RELAY1          88
#define RELAY2          89
#define RELAY3          90
#define RELAY4          91
#define RELAY5          92
#define GPIO2           29
#define GPIO1           28
#define LOW_SYNC_IN     63
#define HIGH_SYNC_IN    62
#define LOW_SYNC_OUT    1
#define HIGH_SYNC_OUT   0

//static InverterState_t* _gInvSta;
uint16_t gLowSyncInInitStatus = 0;
uint16_t gHighSyncInInitStatus = 0;
uint16_t rxMsgData[8];
CAN_MsgFrameType eFrameType;
extern ModbusRegCSIP_t      gstModbusRegCSIP;


void main(void)
{
//
// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the F2837xD_SysCtrl.c file.
//

    InitSysCtrl();
    DINT;
    //  FALSH烧写配置
#ifdef _FLASH
    InitFlash();
#endif
//
// Step 2. Initialize GPIO:
// This example function is found in the F2837xD_Gpio.c file and
// illustrates how to set the GPIO to it's default state.

    IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
    EALLOW;
    DevCfgRegs.CPUSEL5.bit.SCI_D=1;
    EDIS;

//
    InitGpio();
    GPIO_SetupPinMux(BLINKY_LED_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(BLINKY_LED_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);

    //  DSP_CRASH
    GPIO_SetupPinMux(DSP_CRASH, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(DSP_CRASH, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(DSP_CRASH, 0);
    // ARC_CRASH
    GPIO_SetupPinMux(GPIO1, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(GPIO1, GPIO_INPUT, GPIO_PULLUP);
//    GPIO_WritePin(DSP_CRASH, 0);

    GPIO_SetupPinMux(RELAY0, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RELAY0, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RELAY0, 0);

    GPIO_SetupPinMux(RELAY2, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RELAY2, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RELAY2, 0);

    GPIO_SetupPinMux(RELAY3, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RELAY3, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RELAY3, 0);

    GPIO_SetupPinMux(RELAY1, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RELAY1, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RELAY1, 0);

    GPIO_SetupPinMux(RELAY4, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RELAY4, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RELAY4, 0);

    GPIO_SetupPinMux(RELAY5, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(RELAY5, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(RELAY5, 0);

    GPIO_SetupPinMux(IODSPCRASH, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(IODSPCRASH, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(IODSPCRASH, 0);

    GPIO_SetupPinMux(GPIO2, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(GPIO2, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(GPIO2, 1);

    GPIO_SetupPinMux(KEY_GPIO, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(KEY_GPIO, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(KEY_GPIO, 1);

    //同步信号
    GPIO_SetupPinMux(LOW_SYNC_IN, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(LOW_SYNC_IN, GPIO_INPUT, GPIO_PUSHPULL);
    gLowSyncInInitStatus = GPIO_ReadPin(LOW_SYNC_IN);

    GPIO_SetupPinMux(HIGH_SYNC_IN, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(HIGH_SYNC_IN, GPIO_INPUT, GPIO_PUSHPULL);
    gHighSyncInInitStatus = GPIO_ReadPin(HIGH_SYNC_IN);

    GPIO_SetupPinMux(LOW_SYNC_OUT, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(LOW_SYNC_OUT, GPIO_OUTPUT, GPIO_PULLUP);
    GPIO_WritePin(LOW_SYNC_OUT, 0);

    GPIO_SetupPinMux(HIGH_SYNC_OUT, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(HIGH_SYNC_OUT, GPIO_OUTPUT, GPIO_PULLUP);
    GPIO_WritePin(HIGH_SYNC_OUT, 0);


// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
        // GPIO_SetupPinMux(28, GPIO_MUX_CPU1, 0);
        // GPIO_SetupPinOptions(28, GPIO_OUTPUT, GPIO_PUSHPULL);
        // GPIO_WritePin(28, 0);
//
    DINT;

//
// Initialize the PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the F2837xD_PieCtrl.c file.
//
    InitPieCtrl();

//
// Disable CPU interrupts and clear all CPU interrupt flags:
//
    IER = 0x0000;
    IFR = 0x0000;

//
// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in F2837xD_DefaultIsr.c.
// This function is found in F2837xD_PieVect.c.
//
    InitPieVectTable();

    CpuTimersInit();
    DELAY_MS(2000); // 20260112增加2s延时，等CAN的5V电源先启动，后初始化CAN
    ClaInit();
    SciaFifoInit();
    ScidFifoInit();
    SpiFlashInit();
    AlgoStateInit();
    AlgoErrCheckInit();
    PwmCntlInit();
    PVCntlInit();
    LlcCntlInit();
    InverterLoopInit();
    AlgoInit();
    Cmpss_Init(); //电感电流硬件过流初始化
    EpwmInit();
    SensorInit();
    FanInit();
    //CanInit();
    CanProtoInit();
    Cla1ForceTask2andWait();
    ContinusOperationInit();
    //  PWM  BAT 输出245OE使能
    //GPIO_WritePin(RELAY0, 1);
    //GPIO_WritePin(RELAY1, 1);
    //  PWM  BAT 输出245OE使能
    GPIO_WritePin(RELAY2, 0);
    GPIO_WritePin(RELAY3, 0);
    GPIO_WritePin(RELAY4, 1);
    GPIO_WritePin(RELAY5, 1);

//    FlashTest();
//    uint8_t data[256];
//  uint16_t j = 0;
//  for(j = 0; j < 256; j ++)
//  {
//  data[j] = j;
//  }
//  scia_send(data, 256);

    MidComserviceInit();
//
// Enable global Interrupts and higher priority real-time debug events:
//
    setupInterrupt();//增加一次中断设置，否则在此处有概率出现中断无法进入的情况。
    Clear_cmpss_ost();
    CsipInit();

    static uint16_t calibrate_count = 0;
    while(Calibrate_cmpss_dac(2047) == 0) //TODO:如果长时间校正不通过，则需返回自检故障。
    {
        if(calibrate_count++ > 100)
        {
            break;
        }
    }

    Clear_cmpss_ost();
    EPwm2Regs.TZCLR.bit.OST = 1; 
    EPwm3Regs.TZCLR.bit.OST = 1;

    /*发送100k的方波重置MCU使能*/
    for(int i = 0; i < 10000; ++i)
    {
        GPIO_WritePin(IODSPCRASH, 1);
        DELAY_US(5);
        GPIO_WritePin(IODSPCRASH, 0);
        DELAY_US(5);
    }

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM
//    AdcInit();
//
// Step 6. IDLE loop. Just sit and loop forever (optional):
    static uint32_t poll_time = 0;
    ModbusRegInit();
    DebugWageInit();
    for(;;)
    {

        CanProtoProcess();
        ModbusCmdProcess();
        SetSourceMode(gSysSetInfo.dc_source); //设置
        UserBackupSwitchSet();//backup负载开关
        PvInvAllowMaxPower();//功率等级
        ApprovalStdInfoGet();//认证标准参数赋值
        if((TimGetSystime() - poll_time) > 100)//100ms轮询一次
        {
//            CSIPOnGridCertification();
            AlgoPwrInfoCalc();
            InvTemperMaxGet();
//            MainPollingProcess();
            ActivationFsmRun();//激活
            poll_time = TimGetSystime();
            FanCntl();
//            CANTransmitMsg();
            DebugReadMsg();
        }
        FaultDiagnosisProcess();
        SysInfoGet();
        // PvSendStateWave();
//        ScidRxDataRecv();
//        ScidSendMsgErr();
//        TimingStorage();//2h存储一次电量
//        ResetFactory();
        UpgradeProcesshandle();
        PvSendStateWave();
        InverterSendStateWave();
        LlcSendStateWave();
        AlgoSendStateWave();
    }
}

//
// End of file
//

