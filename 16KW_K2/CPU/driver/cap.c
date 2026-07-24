/*
 * cap.c
 *
 *  Created on: 2026年1月13日
 *      Author: Nova006348
 */

#include <cap.h>

volatile struct ECAP_REGS *ECap[] =
{
    0,&ECap1Regs, &ECap2Regs, &ECap3Regs, &ECap4Regs, &ECap5Regs, &ECap6Regs
};

/**
* @brief: InitEcapAsTimer
* @param: void
* @return:Nothing
* @note:该配置主要用于CPU中断时间测量
*/
static void _InitEcapAsTimer(uint16_t num)
{
    //使能eCAP1时钟
//    CpuSysRegs.PCLKCR3.bit.ECAP1 = 1;
    (*ECap[num]).ECCTL2.bit.TSCTRSTOP = 0;     // 确保计数器停止
    (*ECap[num]).ECCTL1.bit.CAPLDEN = 0;       // 禁用捕获寄存器加载（定时器模式不需要）
    (*ECap[num]).ECEINT.all = 0x0000;          // 禁用所有捕获中断
    (*ECap[num]).ECCLR.all = 0xFFFF;           // 清除所有中断标志
    //设置为连续运行的定时器（不再是捕获模式）
    (*ECap[num]).ECCTL2.bit.CONT_ONESHT = 0;   // 连续模式
    (*ECap[num]).ECCTL2.bit.STOP_WRAP = 0;     //计数器达到周期值(CAP1)后归零（Wrap）
    (*ECap[num]).ECCTL2.bit.SYNCI_EN = 0;      //禁用外部同步输入（定时器独立运行）
    (*ECap[num]).ECCTL2.bit.SYNCO_SEL = 2;     // 禁用同步输出
    (*ECap[num]).ECCTL2.bit.REARM = 1;         // 保持使能重载，在连续模式下也适用
    //关闭所有捕获边沿检测和差分复位
    (*ECap[num]).ECCTL1.bit.CAP1POL = 0;
    (*ECap[num]).ECCTL1.bit.CAP2POL = 0;
    (*ECap[num]).ECCTL1.bit.CAP3POL = 0;
    (*ECap[num]).ECCTL1.bit.CAP4POL = 0;
    (*ECap[num]).ECCTL1.bit.CTRRST1 = 0;       // 关闭事件1的计数器复位（非差分模式）
    (*ECap[num]).ECCTL1.bit.CTRRST2 = 0;       // 关闭事件2的计数器复位
    (*ECap[num]).ECCTL1.bit.CTRRST3 = 0;       // 关闭事件3的计数器复位
    (*ECap[num]).ECCTL1.bit.CTRRST4 = 0;       // 关闭事件4的计数器复位
    //设置计数器周期（归零值）为最大值，以获得最长的计数区间 ，CAP1寄存器在定时器模式下作为周期寄存器使用
    (*ECap[num]).CAP1 = 0xFFFFFFFF;            // 计数器从0递增到此值后归零
    //启动计数器
    (*ECap[num]).ECCTL2.bit.TSCTRSTOP = 1;
    //不需要启用捕获寄存器加载和捕获中断
    // (*ECap[num]).ECCTL1.bit.CAPLDEN = 1;
    // (*ECap[num]).ECEINT.bit.CEVT4 = 1;
}

/**
* @brief: Cap_Init
* @param: void
* @return:Nothing
* @note:CAP初始化
*/
void Cap_Init(void)
{
#ifdef EN_CLA_CONTROL_INT_TIME_MEASURE
    _InitEcapAsTimer(1);
#endif
}
