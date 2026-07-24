/*
 * pwm.c
 *
 *  Created on: 2025年1月6日
 *      Author: Nova007456
 */
#include "F28x_Project.h"
#include "adc.h"
#include "pwm.h"

volatile struct EPWM_REGS *EPwm[] =
{
    0,&EPwm1Regs, &EPwm2Regs, &EPwm3Regs, &EPwm4Regs, &EPwm5Regs, &EPwm6Regs,
    &EPwm7Regs, &EPwm8Regs, &EPwm9Regs, &EPwm10Regs, &EPwm11Regs, &EPwm12Regs
};
/**
* @brief: ClearEpwmTz
* @param: void
* @return:Nothing
* @note:清除EPWM的TZ保护
*/
void ClearEpwmTz(uint16_t num)
{
    (*EPwm[num]).TZCLR.bit.OST = 1;                        //清除TZ故障
}
/**
  * @brief  FanEpwmUpdata
  * @param  pvdutyaddress:存储pv占空比的数组地址;en_pv：PV模块使能
  * @retval None
  * @note   None
*/
uint16_t FanEpwmUpdata(float * dutyaddress, uint16_t en_pv)
{
    float duty[4];
    if(en_pv == 0)
    {
        EALLOW;
        EPwm10Regs.TZFRC.bit.OST = 1;
        EPwm11Regs.TZFRC.bit.OST = 1;
        EDIS;
        return 0;
    }
    else
    {
        EALLOW;
        EPwm10Regs.TZCLR.bit.OST = 1;
        EPwm11Regs.TZCLR.bit.OST = 1;
        EDIS;
    }

    if(dutyaddress[0] > 0 && dutyaddress[0] < 0.98)
    {
        duty[0] = dutyaddress[0];
    }
    else
    {
        duty[0] = 0;
    }
    if(dutyaddress[1] > 0 && dutyaddress[1] < 0.98)
    {
        duty[1] = dutyaddress[1];
    }
    else
    {
        duty[1] = 0;
    }
    if(dutyaddress[2] > 0 && dutyaddress[2] < 0.98)
    {
        duty[2] = dutyaddress[2];
    }
    else
    {
        duty[2] = 0;
    }
    if(dutyaddress[3] > 0 && dutyaddress[3] < 0.98)
    {
        duty[3] = dutyaddress[3];
    }
    else
    {
        duty[3] = 0;
    }
    EPwm10Regs.CMPA.bit.CMPA = (uint32_t)(EPwm10Regs.TBPRD * (1 - duty[0]));
    EPwm10Regs.CMPB.bit.CMPB = (uint32_t)(EPwm10Regs.TBPRD * (1 - duty[1]));
    EPwm11Regs.CMPA.bit.CMPA = (uint32_t)(EPwm11Regs.TBPRD * (1 - duty[2]));
    EPwm11Regs.CMPB.bit.CMPB = (uint32_t)(EPwm11Regs.TBPRD * (1 - duty[3]));
    return 1;
}
/**
  * @brief  PvEpwmUpdata
  * @param  pvdutyaddress:存储pv占空比的数组地址;en_pv：PV模块使能
  * @retval None
  * @note   None
*/
//uint16_t PvEpwmUpdata(float pvduty,uint16_t channel, uint16_t en_pv)
//{
//    float dutypv;
//
//    if(en_pv == 0)
//    {
//        EALLOW;
//        EPwm8Regs.TZFRC.bit.OST = 1;
//        EPwm9Regs.TZFRC.bit.OST = 1;
//        EDIS;
//        return 0;
//    }
//    else
//    {
//        EALLOW;
//        EPwm8Regs.TZCLR.bit.OST = 1;
//        EPwm9Regs.TZCLR.bit.OST = 1;
//        EDIS;
//    }
//
//    if((pvduty> 0 )&&(pvduty < 0.98f))
//    {
//        dutypv = pvduty ;
//    }
//    else
//    {
//        dutypv  = 0;
//    }
//
//    switch(channel)
//    {
//         case 0:
//
//             EPwm8Regs.CMPA.bit.CMPA = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv));
//         break;
//
//         case 1:
//
//             EPwm8Regs.CMPB.bit.CMPB = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv));
//
//         break;
//
//         case 2:
//
//            EPwm9Regs.CMPA.bit.CMPA = (uint32_t)(EPwm9Regs.TBPRD * (1 - dutypv));
//
//         break;
//
//         default:
//
//         break;
//
//    }
//
//    return 1;
//}
/**
  * @brief  PvEpwmUpdata
  * @param  pvdutyaddress:存储pv占空比的数组地址;en_pv：PV模块使能
  * @retval None
  * @note   None
*/
uint16_t PvEpwmUpdata(float * pvdutyaddress, uint16_t en_pv)
{
    float dutypv[4];

    if(en_pv == 0)
    {
        EALLOW;
        EPwm8Regs.TZFRC.bit.OST = 1;
        EPwm9Regs.TZFRC.bit.OST = 1;
        EDIS;
        return 0;
    }
    if(pvdutyaddress[0] > 0 && pvdutyaddress[0] < 0.98)
    {
        dutypv[0] = pvdutyaddress[0];
    }
    else
    {
        dutypv[0] = 0;
    }
    if(pvdutyaddress[1] > 0 && pvdutyaddress[1] < 0.98)
    {
        dutypv[1] = pvdutyaddress[1];
    }
    else
    {
        dutypv[1] = 0;
    }
    if(pvdutyaddress[2] > 0 && pvdutyaddress[2] < 0.98)
    {
        dutypv[2] = pvdutyaddress[2];
    }
    else
    {
        dutypv[2] = 0;
    }
    if(pvdutyaddress[3] > 0 && pvdutyaddress[3] < 0.98)
    {
        dutypv[3] = pvdutyaddress[3];
    }
    else
    {
        dutypv[3] = 0;
    }

    EPwm8Regs.CMPA.bit.CMPA = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv[0]));
    EPwm8Regs.CMPB.bit.CMPB = (uint32_t)(EPwm8Regs.TBPRD * (1 - dutypv[1]));
    EPwm9Regs.CMPA.bit.CMPA = (uint32_t)(EPwm9Regs.TBPRD * (1 - dutypv[2]));
    EPwm9Regs.CMPB.bit.CMPB = (uint32_t)(EPwm9Regs.TBPRD * (dutypv[3]));
    EALLOW;
    EPwm8Regs.TZCLR.bit.OST = 1;
    EPwm9Regs.TZCLR.bit.OST = 1;
    EDIS;
    return 1;
}
/**
* @brief: _InvEpwmxInit
* @param: void
* @return:Nothing
* @note:逆变12个IGBT的EPWM配置函数
*/
void InvEpwmxInit(uint16_t num)
{
    /*      时基模块        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器（特定时刻下影子寄存器的值才会加载至工作寄存器）
    (*EPwm[num]).TBPRD = (Uint16)(INV_PWM_PERIOD >> 1);    //设定PWM频率             /*PWM频率为20K,TBCLK/(2*TBPRD)*/
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值         /*该PWM的相位模块从0开始计数，没有相位偏移*/
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)     /*默认不分频*/
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)   /*默认不分频*/
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //使能加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //同步输入信号触发计数器相位同步
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //占空比置0                    /*当计数器等于2.5时触发对应动作*/
    (*EPwm[num]).CMPB.bit.CMPB = 0;                        //占空比置0                    /*设置CMPB为0*/
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比          /*使能影子寄存器，当计数器为0时触发加载事件*/
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;                           /*清除CMPA与CMPB所有事件操作*/
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_CLEAR;                //当时间计数器值等于CMPB时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_SET;                  //当时间计数器值等于CMPB时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA作为上升沿及下降沿时的信号源    /*设置死区模块输入信号源为EPWMAX*/
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;      //使能上升沿及下降沿的延时信号          /*完全使能死区，即上升沿与下降沿同时使能*/
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HIC;           //主高互补 EPWMxB翻转极性             /*高电平有效，EPWMA作为原始信号且经过上升下降死区，EPWMB直接以死区后的A信号进行翻转，未使用CMPB信号*/
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)INV_DEADBAND;   //上升沿死区时间1us                 /高电平延时时间/     T=DBRED/TBCLK
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)INV_DEADBAND;   //下降沿死区时间1us                 /*低电平延时时间*/
    /*      TZ保护模块        */
    (*EPwm[num]).TZSEL.bit.OSHT1 = 1;                      //使能TZ1(逆变过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZSEL.bit.OSHT2 = 1;                      //使能TZ2(母线过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ保护时强制EPWMxA为低电平
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ保护时强制EPWMxB为低电平
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //设置TZ保护                   /*设置软件保护*/
}
/**
* @brief: CpuInterruptEpwmxInit
* @param: void
* @return:Nothing
* @note:CPU中断的EPWM配置函数
*/
void CpuInterruptEpwmxInit(uint16_t num)
{
    /*      时基模块        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器
    (*EPwm[num]).TBPRD = (CPU_INTERRUPT_PWM_PERIOD >> 1);    //设定PWM频率    设置PWM频率为80K
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //允许加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //同步输入信号触发同步事件
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //占空比置0
    (*EPwm[num]).CMPB.bit.CMPB = 0;                        //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_SET;                  //当时间计数器值等于CMPA时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA作为上升沿及下降沿时的信号源
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;      //使能上升沿及下降沿的延时信号
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HI;            //设置死区输出为高电平有效
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)CPU_INTERRUPT_DEADBAND;//上升沿死区时间1us
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)CPU_INTERRUPT_DEADBAND;//下降沿死区时间1us
    /*      配置CPU中断的触发事件        */
    (*EPwm[num]).CMPA.bit.CMPA = ACQPS_SYS_CLKS;            //预留采样时间    29
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD - ACQPS_SYS_CLKS;       //625
    (*EPwm[num]).ETSEL.bit.SOCASEL = ET_CTRD_CMPA;          //TBCTR = CMPA时,且计数器递减时产生触发事件    /*事件在计数器向下计数且等于CMPA时触发SOCA（ADC启动转换）*/
    (*EPwm[num]).ETSEL.bit.SOCBSEL = ET_CTRU_CMPB;          //TBCTR = CMPB时,且计数器递增时产生触发事件     /*事件在计数器向上计数且等于CMPB时触发SOCB（ADC启动转换）*/
    (*EPwm[num]).ETPS.bit.SOCAPRD = ET_1ST;                 //每发生一次事件,产生ADC启动信号
    (*EPwm[num]).ETSEL.bit.SOCAEN = 1;                      //使能SOCA的ADC启动脉冲
    (*EPwm[num]).ETPS.bit.SOCBPRD = ET_1ST;                 //每发生一次事件,产生ADC启动信号
    (*EPwm[num]).ETSEL.bit.SOCBEN = 1;                      //使能SOCB的ADC启动脉冲
    /*      选择CPU中断对应的触发事件        */
    (*EPwm[num]).ETSEL.bit.INTSEL = ET_CTR_ZERO;            //TBCTR = 0 时产生                         /*计数器为0时触发中断事件*/
    (*EPwm[num]).ETSEL.bit.INTEN = 1;                       //使能中断信号
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //每发生一次产生中断信号EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //清除中断信号EPWMx_INT标志位
}
/**
* @brief: ClaInterruptEpwmxInit
* @param: void
* @return:Nothing
* @note:CLA中断  FAN的EPWM配置函数
*/
void ClaInterruptEpwmxInit(uint16_t num)
{
    /*      时基模块        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器
    (*EPwm[num]).TBPRD = (Uint16)(CLA_INTERRUPT_PWM_PERIOD >> 1);//设定PWM频率
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //允许加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //同步输入信号触发同步事件
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD ;       //占空比置0
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD ;       //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMB输出高电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMB输出低电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE; // 禁用死区，直通模式
    /*      TZ保护模块        */
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ保护时强制EPWMxA为低电平
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ保护时强制EPWMxB为低电平
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //设置TZ保护
    /*      选择CLA中断对应的触发事件        */
    (*EPwm[num]).ETSEL.bit.INTSEL = ET_CTR_ZERO;            //TBCTR = 0 时产生
    (*EPwm[num]).ETSEL.bit.INTEN = 1;                       //使能中断信号
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //每发生一次产生中断信号EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //清除中断信号EPWMx_INT标志位
}
/**
* @brief: _FANEpwmxInit
* @param: void
* @return:Nothing
* @note:风扇的EPWM配置函数
*/
void FanEpwmxInit(uint16_t num)
{
    /*      时基模块        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器
    (*EPwm[num]).TBPRD = (Uint16)(FAN_PWM_PERIOD >> 1);    //设定PWM频率             /*风扇的PWM频率为20K,TBCLK/(2*TBPRD)*/
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)     /*默认不分频*/
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)   /*默认不分频*/
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //使能加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //同步输入信号触发计数器相位同步
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //占空比置0
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD;       //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                  //当时间计数器值等于CMPB时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                //当时间计数器值等于CMPB时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE; // 禁用死区，直通模式
    /*      TZ保护模块        */
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ保护时强制EPWMxA为低电平
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ保护时强制EPWMxB为低电平
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //设置TZ保护                   /*设置软件保护*/
}
/**
* @brief: InterruptEpwm_Init
* @param: void
* @return:Nothing
* @note:CLA中断配置总函数
*/
void InterruptEpwm_Init(void)
{
    /*      EPWM初始化        */
    CpuInterruptEpwmxInit(7);                   //占用EPWM7作为触发源,触发80K的CPU中断
    /*CLA && FAN的EPWM初始化*/
    ClaInterruptEpwmxInit(10);                  //占用EPWM10作为触发源,触发20K的CLA中断,该端口EPWM做为风扇触发
    FanEpwmxInit(11);                           /*EPWM11的EPWM配置*/
}
/**
* @brief: InvEpwm_Init
* @param: void
* @return:Nothing
* @note:逆变器PWM配置
*/
void InvEpwm_Init(void)
{
    InvEpwmxInit(1);                        //EPWM1 A → Q1  B→ Q3
    InvEpwmxInit(2);                        //EPWM2 A → Q2  B→ Q4
    InvEpwmxInit(3);                        //EPWM3 A → Q5  B→ Q7
    InvEpwmxInit(4);                        //EPWM4 A → Q6  B→ Q8
    InvEpwmxInit(5);                        //EPWM5 A → Q9  B→ Q11
    InvEpwmxInit(6);                        //EPWM6 A → Q10 B→ Q12
}
/*
* @brief: PVEpwmxInit
* @param: void
* @return:Nothing
* @note:光伏的EPWM配置函数
*/
void PvEpwmxInit(Uint16 num)
{
    /*      时基模块        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器
    (*EPwm[num]).TBPRD = (Uint16)(PV_PWM_PERIOD >> 1);    //设定PWM频率     /*20K*/
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_DISABLE;             //禁止加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;         //CTR=0,时基计数器TBCTR=0
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD ;       //占空比置0
    (*EPwm[num]).CMPB.bit.CMPB = (*EPwm[num]).TBPRD ;       //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                  //当时间计数器值等于CMPB时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                //当时间计数器值等于CMPB时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE;            // 禁用死区，直通模式
    /*      TZ保护模块        */
    (*EPwm[num]).TZSEL.bit.OSHT1 = 1;                      //使能TZ1(逆变过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZSEL.bit.OSHT2 = 1;                      //使能TZ2(母线过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ保护时强制EPWMxA为低电平
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ保护时强制EPWMxB为低电平
    (*EPwm[num]).TZFRC.bit.OST = 1;                        //设置TZ保护
}
/**
* @brief: PvEpwm_Init
* @param: void
* @return:Nothing
* @note:逆变器PWM配置
*/
void PvEpwm_Init(void)
{
    PvEpwmxInit(8);                        //EPWM8A → PV1 EPWM8B → PV2
    PvEpwmxInit(9);                        //EPWM9A → PV3 EPWM9B → PV4
}
/**
* @brief: ClearEpwmTZ
* @param: void
* @return:Nothing
* @note:清除EPWM的TZ保护标志
*/
void ClearEpwmTZ(void)
{
    EALLOW;
    /*      逆变器TZ保护清除        */
    ClearEpwmTz(1);
    ClearEpwmTz(2);
    ClearEpwmTz(3);
    ClearEpwmTz(4);
    ClearEpwmTz(5);
    ClearEpwmTz(6);
    /*      PV部分TZ保护清除        */
    ClearEpwmTz(8);
    ClearEpwmTz(9);
    /*风扇部分TZ保护清除*/
    ClearEpwmTz(10);
    ClearEpwmTz(11);
    EDIS;
}
/**
* @brief: EPwm_Init
* @param: void
* @return:Nothing
* @note:PWM配置总函数
*/
void EPwm_Init(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   //禁止所有EPWM时基时钟
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   //使能所有EPWM时基时钟

    InterruptEpwm_Init();                   //中断EPWM初始化
    InvEpwm_Init();                         //逆变EPWM初始化
    PvEpwm_Init();                          //Pv的EPWM初始化
    /*触发一次计数器同步，由EPWM1触发同步事件，相位同步后关闭所有EPWM同步配置，避免后续打破相位事件*/
    EPwm1Regs.TBCTL.bit.SWFSYNC = 1; // 触发一次相位同步
    EPwm1Regs.TBCTL.bit.SWFSYNC = 0; // 触发一次相位同步
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm3Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm5Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm6Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm7Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm8Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm9Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm10Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm11Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm12Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EDIS;
}
