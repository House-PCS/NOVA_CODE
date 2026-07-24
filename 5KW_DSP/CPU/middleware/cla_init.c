/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        cla_init.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-8-30
  * @brief
  ******************************************************************************
  * @attention
  * @relation

  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author
  * <tr><td>2024-8-30     <td>V1.0.0    <td>jindongze
  * </table>
  ******************************************************************************
  */
#include "F28x_Project.h"
#include "dev_sensor.h"
#include "F2837xD_Mcbsp.h"
#include "cla_algo.h"

#pragma DATA_SECTION(gGridVolPll,"Cla1DataRam");
#pragma DATA_SECTION(gInvVolPll,"Cla1DataRam");
#pragma DATA_SECTION(gGridCurPll,"Cla1DataRam");
#pragma DATA_SECTION(gInvCurPll,"Cla1DataRam");
#pragma DATA_SECTION(gCtCurPll,"Cla1DataRam");
#pragma DATA_SECTION(gLoadCurPll,"Cla1DataRam");
SPLL_1ph_F gGridVolPll;
SPLL_1ph_F gInvVolPll;
SPLL_1ph_F gGridCurPll;
SPLL_1ph_F gInvCurPll;
SPLL_1ph_F gCtCurPll;
SPLL_1ph_F gLoadCurPll;

// #pragma DATA_SECTION(gGridVolAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gGridVolPllAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gInvVolAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gLoadVolAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gGridCurAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gInvCurAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gLoadCurAnlz,"Cla1DataRam");
// #pragma DATA_SECTION(gCtCurAnlz,"Cla1DataRam");
// RmsCla_t gGridVolAnlz;
// RmsCla_t gGridVolPllAnlz;
// RmsCla_t gInvVolAnlz;
// RmsCla_t gLoadVolAnlz;
// RmsCla_t gGridCurAnlz;
// RmsCla_t gInvCurAnlz;
// RmsCla_t gLoadCurAnlz;
// RmsCla_t gCtCurAnlz;
#pragma DATA_SECTION(gstGridAnlz,"Cla1DataRam");
#pragma DATA_SECTION(gstInvAnlz,"Cla1DataRam");
#pragma DATA_SECTION(gstLoadAnlz,"Cla1DataRam");
#pragma DATA_SECTION(gstCtAnlz,"Cla1DataRam");
RmsCla_t gGridVolPllAnlz;

PwrClaCompute_t gstGridAnlz;
PwrClaCompute_t gstInvAnlz;
PwrClaCompute_t gstLoadAnlz;
PwrClaCompute_t gstCtAnlz;
//搬运地址定义
extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;
extern uint32_t Cla1ConstRunStart, Cla1ConstLoadStart, Cla1ConstLoadSize;

/**
* @brief: CLA_configClaMemory
* @param: void
* @return:Nothing
* @note:CLA_内存分配函数，初始化调用一次
*/
static void _ClaConfigClaMemory(void)
{
    EALLOW;
    //从Flash拷贝CLA的代码到内部的RAM，
    memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart,
           (uint32_t)&Cla1funcsLoadSize);
    memcpy((uint32_t *)&Cla1ConstRunStart, (uint32_t *)&Cla1ConstLoadStart,//数据区，存放常量的地方
           (uint32_t)&Cla1ConstLoadSize);

    //
    // Initialize and wait for CLA1ToCPUMsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;//启动CLA1到CPU的RAM的初始化
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1){};//RAM初始化完成了后继续运行

    //
    // Initialize and wait for CPUToCLA1MsgRAM
    //
    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;//启动CPU到CLA1的RAM的初始化
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1){};//RAM初始化完成了后继续运行

    //选择 LS4RAM 和 LS5RAM 作为 CLA 的编程空间
    MemCfgRegs.LSxMSEL.bit.MSEL_LS2 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS2 = 0;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS3 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS3 = 0;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器

    EDIS;
}

/**
* @brief: CLA_initCpu1Cla1
* @param: void
* @return:Nothing
* @note:CLA_配置中断函数，目前配置为只触发TASK函数（触发源37就是PWM2INT），不触发Task执行后的中断函数
*/
static void _ClaInitCpu1Cla1(void)
{
    //CLA 支持八项任务。任务 1 的优先级最高，任务 8 的优先级最低
    EALLOW;
    Cla1Regs.MVECT1 = (uint16_t)(&Cla1Task1);
    Cla1Regs.MVECT2 = (uint16_t)(&Cla1Task2);
    Cla1Regs.MVECT3 = (uint16_t)(&Cla1Task3);
    Cla1Regs.MVECT4 = (uint16_t)(&Cla1Task4);
    Cla1Regs.MVECT5 = (uint16_t)(&Cla1Task5);
    Cla1Regs.MVECT6 = (uint16_t)(&Cla1Task6);
    Cla1Regs.MVECT7 = (uint16_t)(&Cla1Task7);
    Cla1Regs.MVECT8 = (uint16_t)(&Cla1Task8);

    Cla1Regs.MCTL.bit.IACKE = 1;    //启动IACK
//  Cla1Regs.MIER.all = (M_INT1 | M_INT8);//启用中断1和8
    Cla1Regs.MIER.all = (M_INT1| M_INT2);//启用中断1和8

    PieVectTable.CLA1_1_INT = &cla1Isr1;
    PieVectTable.CLA1_2_INT = &cla1Isr2;
    PieVectTable.CLA1_3_INT = &cla1Isr3;
    PieVectTable.CLA1_4_INT = &cla1Isr4;
    PieVectTable.CLA1_5_INT = &cla1Isr5;
    PieVectTable.CLA1_6_INT = &cla1Isr6;
    PieVectTable.CLA1_7_INT = &cla1Isr7;
    PieVectTable.CLA1_8_INT = &cla1Isr8;

    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1 = 37;//选择EPWM2_INT触发源给TASK1

    PieCtrlRegs.PIEIER11.all = 0xFFFF;
    IER |= (M_INT11 );
    EDIS;
}

void ClaInit(void)
{
    _ClaConfigClaMemory();//地址内存配置
    _ClaInitCpu1Cla1();   //CLA中断配置
}

__attribute__((interrupt)) void cla1Isr1 (){}
__attribute__((interrupt)) void cla1Isr2 (){}
__attribute__((interrupt)) void cla1Isr3 (){}
__attribute__((interrupt)) void cla1Isr4 (){}
__attribute__((interrupt)) void cla1Isr5 (){}
__attribute__((interrupt)) void cla1Isr6 (){}
__attribute__((interrupt)) void cla1Isr7 (){}
__attribute__((interrupt)) void cla1Isr8 (){}
