/*
 * interrupt.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */
#include "interrupt.h"
#include "F28x_Project.h"
#include "F2837xD_Mcbsp.h"
// 静态变量保存上层注册的回调函数指针
static CpuIntCallback pCallbackCpuInt = NULL;
//CLA搬运地址定义
extern uint32_t Cla1funcsRunStart, Cla1funcsLoadStart, Cla1funcsLoadSize;
extern uint32_t Cla1ConstRunStart, Cla1ConstLoadStart, Cla1ConstLoadSize;
extern uint32_t CLA1mathTablesRunStart, CLA1mathTablesLoadStart, CLA1mathTablesLoadSize;

__attribute__((interrupt)) void cla1Isr1 (){}
__attribute__((interrupt)) void cla1Isr2 (){}
__attribute__((interrupt)) void cla1Isr3 (){}
__attribute__((interrupt)) void cla1Isr4 (){}
__attribute__((interrupt)) void cla1Isr5 (){}
__attribute__((interrupt)) void cla1Isr6 (){}
__attribute__((interrupt)) void cla1Isr7 (){}
__attribute__((interrupt)) void cla1Isr8 (){}
/**
* @brief: CLA_configClaMemory
* @param: void
* @return:Nothing
* @note:CLA_内存分配函数，初始化调用一次
*/
static void _CLA_ConfigClaMemory(void)
{
    EALLOW;
    //从Flash拷贝CLA的代码到内部的RAM
    memcpy((uint32_t *)&Cla1funcsRunStart, (uint32_t *)&Cla1funcsLoadStart,
           (uint32_t)&Cla1funcsLoadSize);
    memcpy((uint32_t *)&Cla1ConstRunStart, (uint32_t *)&Cla1ConstLoadStart,//数据区，存放常量的地方
           (uint32_t)&Cla1ConstLoadSize);
    memcpy((uint32_t *)&CLA1mathTablesRunStart, (uint32_t *)&CLA1mathTablesLoadStart,//数据区，存放常量的地方
           (uint32_t)&CLA1mathTablesLoadSize);

    MemCfgRegs.MSGxINIT.bit.INIT_CLA1TOCPU = 1;//启动CLA1到CPU的RAM的初始化
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CLA1TOCPU != 1){};//RAM初始化完成了后继续运行

    MemCfgRegs.MSGxINIT.bit.INIT_CPUTOCLA1 = 1;//启动CPU到CLA1的RAM的初始化
    while(MemCfgRegs.MSGxINITDONE.bit.INITDONE_CPUTOCLA1 != 1){};//RAM初始化完成了后继续运行

    MemCfgRegs.LSxMSEL.bit.MSEL_LS0 = 1;    //配置MSEL_LS0这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS0 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS1 = 1;    //配置MSEL_LS1这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS1 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS2 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS2 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS3 = 1;    //配置MSEL_LS3这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS3 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    MemCfgRegs.LSxMSEL.bit.MSEL_LS4 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS4 = 1;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器

    MemCfgRegs.LSxMSEL.bit.MSEL_LS5 = 1;    //配置MSEL_LS2这个内存用于CPU和CLA1的共享内存，这里配置为0的话就是CPU自己独用的内存
    MemCfgRegs.LSxCLAPGM.bit.CLAPGM_LS5 = 0;//将这一块的内存配置为CLA的程序内存，这里配置为0的话是配置为CLA的数据存储器
    EDIS;
}
/**
* @brief: CLA_initCpu1Cla1
* @param: void
* @return:Nothing
* @note:CLA_配置中断函数，目前配置为只触发TASK函数（触发源37就是PWM2INT），不触发Task执行后的中断函数
*/
static void _CLA_InitCpu1Cla1(void)
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

//  Cla1Regs.MCTL.bit.IACKE = 1;    //启动IACK
//  Cla1Regs.MIER.all = (M_INT1 | M_INT8);//启用中断1和8
    Cla1Regs.MIER.all = (M_INT1);//启用中断1和8

    PieVectTable.CLA1_1_INT = &cla1Isr1;
    PieVectTable.CLA1_2_INT = &cla1Isr2;
    PieVectTable.CLA1_3_INT = &cla1Isr3;
    PieVectTable.CLA1_4_INT = &cla1Isr4;
    PieVectTable.CLA1_5_INT = &cla1Isr5;
    PieVectTable.CLA1_6_INT = &cla1Isr6;
    PieVectTable.CLA1_7_INT = &cla1Isr7;
    PieVectTable.CLA1_8_INT = &cla1Isr8;

    DmaClaSrcSelRegs.CLA1TASKSRCSEL1.bit.TASK1 = 45;//选择EPWM10_INT触发源给TASK1

    PieCtrlRegs.PIEIER11.all = 0xFFFF;
    IER |= (M_INT11);
    EDIS;
}
/**
* @brief: Cla_Interrupt_Init
* @param: void
* @return:Nothing
* @note:CLA中断配置总函数
*/
void Cla_Interrupt_Init(void)
{
    _CLA_ConfigClaMemory();  //地址内存配置
    _CLA_InitCpu1Cla1();     //CLA中断配置
}
/**
* @brief: Cla_Interrupt_Init
* @param: void
* @return:Nothing
* @note:注册回调函数的接口
*/
void RegisterCpuIntCallback(CpuIntCallback callback)
{
    pCallbackCpuInt = callback;
}

/**
* @brief: CPU_Interrupt
* @param: void
* @return:Nothing
* @note:CPU中断服务函数
*/
interrupt void CPU_Interrupt(void)
{
    if (pCallbackCpuInt != NULL)
    {
        pCallbackCpuInt(); // 调用上层注册的回调函数
    }
    ClearCpuInterrupt();
}
/**
* @brief: Cpu_Interrupt_Init
* @param: void
* @return:Nothing
* @note:CPU触发源配置函数
*/
void Cpu_Interrupt_Init(void)
{
    EALLOW;
    /*      PIE中断向量表配置        */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;     //向CPU请求中断，从这里就开始准备执行中断
    PieVectTable.EPWM7_INT = &CPU_Interrupt;    //EPWM7中断函数
    PieCtrlRegs.PIEIER3.bit.INTx7 = 1;          // EPWM7在中断向量表中3-5的位置
    IER |= M_INT3;
    EDIS;
}
/**
* @brief: ClearCpuInterrupt
* @param: void
* @return:Nothing
* @note:清除CPU中断标志位
*/
void ClearCpuInterrupt(void)
{
    /*清除中断标志,且对下次中断发出请求*/
    EPwm7Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}
/**
* @brief: Interrupt_Init
* @param: void
* @return:Nothing
* @note:中断配置总函数
*/
void Interrupt_Init(void)
{
    Cla_Interrupt_Init();   //CLA中断配置
    Cpu_Interrupt_Init();   //CPU中断配置
}
