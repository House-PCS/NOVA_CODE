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
        // EPwm11Regs.TZFRC.bit.OST = 1;
        EDIS;
        return 0;
    }
    else
    {
        EALLOW;
        EPwm10Regs.TZCLR.bit.OST = 1;
        // EPwm11Regs.TZCLR.bit.OST = 1;
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
    // EPwm11Regs.CMPA.bit.CMPA = (uint32_t)(EPwm11Regs.TBPRD * (1 - duty[2]));
    // EPwm11Regs.CMPB.bit.CMPB = (uint32_t)(EPwm11Regs.TBPRD * (1 - duty[3]));
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
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器
    (*EPwm[num]).TBPRD = (Uint16)(INV_PWM_PERIOD >> 1);    //设定PWM频率
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //禁止加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //CTR=0,时基计数器TBCTR=0
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //占空比置0
    (*EPwm[num]).CMPB.bit.CMPB = 0;                        //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQSFRC.bit.RLDCSF = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_SET;                  //当时间计数器值等于CMPA时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA作为上升沿及下降沿时的信号源
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;      //使能上升沿及下降沿的延时信号
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HIC;           //主高互补 EPWMxB翻转极性
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)INV_DEADBAND;   //上升沿死区时间0.5us
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)INV_DEADBAND;   //下降沿死区时间0.5us
    /*      TZ保护模块        */
    (*EPwm[num]).TZSEL.bit.OSHT1 = 1;                      //TODO:TZ上电一直触发需排查，先屏蔽TZ，后续查问题使能TZ1(逆变过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZSEL.bit.OSHT2 = 0;                      //TODO:TZ上电一直触发需排查，先屏蔽TZ，后续查问题使能TZ2(母线过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ保护时强制EPWMxA为低电平
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ保护时强制EPWMxB为低电平
    (*EPwm[num]).TZOSTCLR.bit.OST1 = 1;                        //清楚TZ故障
    (*EPwm[num]).TZOSTCLR.bit.OST2 = 1;                        //清楚TZ故障
    (*EPwm[num]).TZCLR.bit.OST = 1;                        //清楚TZ故障
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
    (*EPwm[num]).TBPRD = (CPU_INTERRUPT_PWM_PERIOD >> 1);    //设定PWM频率
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //使能加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //CTR=0,时基计数器TBCTR=0
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (*EPwm[num]).TBPRD;       //占空比置0
    (*EPwm[num]).CMPB.bit.CMPB = 0;                        //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQSFRC.bit.RLDCSF = CC_CTR_ZERO;                   //软件强制showdle模式
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_SET;                  //当时间计数器值等于CMPA时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA作为上升沿及下降沿时的信号源
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_FULL_ENABLE;      //使能上升沿及下降沿的延时信号
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HIC;           //主高互补 EPWMxB翻转极性
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)CPU_INTERRUPT_DEADBAND;//上升沿死区时间0.5us
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)CPU_INTERRUPT_DEADBAND;//下降沿死区时间0.5us
    /*      配置CPU中断的触发事件        */
    (*EPwm[num]).CMPA.bit.CMPA = 70;            //预留采样时间
    (*EPwm[num]).CMPB.bit.CMPB = EPwm7Regs.TBPRD - 70;
    (*EPwm[num]).ETSEL.bit.SOCASEL = ET_CTRD_CMPA;          //TBCTR = CMPA时,且计数器递减时产生触发事件
    (*EPwm[num]).ETSEL.bit.SOCBSEL = ET_CTRU_CMPB;          //TBCTR = CMPB时,且计数器递增时产生触发事件
    (*EPwm[num]).ETPS.bit.SOCAPRD = ET_1ST;                 //每发生一次事件,产生ADC启动信号
    (*EPwm[num]).ETSEL.bit.SOCAEN = 1;                      //使能SOCA的ADC启动脉冲
    (*EPwm[num]).ETPS.bit.SOCBPRD = ET_1ST;                 //每发生一次事件,产生ADC启动信号
    (*EPwm[num]).ETSEL.bit.SOCBEN = 1;                      //使能SOCB的ADC启动脉冲
    /*      选择CPU中断对应的触发事件        */
    (*EPwm[num]).ETSEL.bit.INTSEL = 0;            //TBCTR = PERIOD 时产生
    (*EPwm[num]).ETSEL.bit.INTEN = 0;                       //关闭中断信号
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //每发生一次产生中断信号EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //清除中断信号EPWMx_INT标志位
}
/**
* @brief: ClaInterruptEpwmxInit
* @param: void
* @return:Nothing
* @note:CLA中断的EPWM配置函数
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
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //禁止加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //CTR=0,时基计数器TBCTR=0
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (Uint16)(CLA_INTERRUPT_PWM_PERIOD >> 2); // 占空比设置为0.5
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
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HIC;           //主高互补 EPWMxB翻转极性
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)CLA_INTERRUPT_DEADBAND;//上升沿死区时间0.5us
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)CLA_INTERRUPT_DEADBAND;//下降沿死区时间0.5us
    /*      选择CLA中断对应的触发事件        */
    (*EPwm[num]).ETSEL.bit.INTSEL = 0;            //TBCTR = CMPA 时产生
    (*EPwm[num]).ETSEL.bit.INTEN = 0;                       //关闭能中断信号
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //每发生一次产生中断信号EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //清除中断信号EPWMx_INT标志位
}
/**
  * @brief  FanEpwmxInit
  * @param  None
  * @return None
  * @note   None
*/
void FanEpwmxInit(uint16_t num)
{
    /*      时基模块        */
    (*EPwm[num]).TBCTL.bit.PRDLD = TB_SHADOW;              //使用TB模块影子寄存器
    (*EPwm[num]).TBPRD = (Uint16)(FAN_PWM_PERIOD >> 1);//设定PWM频率
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_ENABLE;             //禁止加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL = TB_SYNC_IN;         //CTR=0,时基计数器TBCTR=0
    /*      动作限定模块        */
    (*EPwm[num]).CMPA.bit.CMPA = (Uint16)(FAN_PWM_PERIOD >> 1);// 占空比设置为1
    (*EPwm[num]).CMPB.bit.CMPB = (Uint16)(FAN_PWM_PERIOD >> 1);// 占空比设置为1                        //占空比置0
    (*EPwm[num]).CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //映射模式
    (*EPwm[num]).CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;       //计数器为0时加载占空比
    (*EPwm[num]).AQCTLA.all = 0;
    (*EPwm[num]).AQCTLB.all = 0;
    (*EPwm[num]).AQCTLA.bit.CAU = AQ_SET;                  //当时间计数器值等于CMPA时且正在递增,使EPWMA输出高电平
    (*EPwm[num]).AQCTLA.bit.CAD = AQ_CLEAR;                //当时间计数器值等于CMPA时且正在递减,使EPWMA输出低电平
    (*EPwm[num]).AQCTLB.bit.CBU = AQ_SET;                //当时间计数器值等于CMPA时且正在递增,使EPWMB输出低电平
    (*EPwm[num]).AQCTLB.bit.CBD = AQ_CLEAR;                  //当时间计数器值等于CMPA时且正在递减,使EPWMB输出高电平
    /*      死区模块        */
    (*EPwm[num]).DBCTL.bit.IN_MODE = DBA_ALL;              //EPWMxA作为上升沿及下降沿时的信号源
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE;      //禁止死区模块
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HIC;           //主高互补 EPWMxB翻转极性
    (*EPwm[num]).DBRED.bit.DBRED = 0;//上升沿死区时间0
    (*EPwm[num]).DBFED.bit.DBFED = 0;//下降沿死区时间0
    /*      选择CLA中断对应的触发事件        */
    (*EPwm[num]).ETSEL.bit.INTSEL = 0;                      //不产生中断
    (*EPwm[num]).ETSEL.bit.INTEN = 0;                       //关闭能中断信号
    (*EPwm[num]).ETPS.bit.INTPRD = ET_1ST;                  //每发生一次产生中断信号EPWMx_INT
    (*EPwm[num]).ETCLR.bit.INT = 1;                         //清除中断信号EPWMx_INT标志位
    
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
    ClaInterruptEpwmxInit(11);                  //占用EPWM10作为触发源,触发20K的CLA中断
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
/**
  * @brief  FanEpwm_Init
  * @param  None
  * @return None
  * @note   None
*/
static void FanEpwm_Init(void)
{
    FanEpwmxInit(10);
    
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
    (*EPwm[num]).TBPRD = (Uint16)(PV_PWM_PERIOD >> 1);    //设定PWM频率
    (*EPwm[num]).TBPHS.bit.TBPHS = 0;                      //忽略相位寄存器值
    (*EPwm[num]).TBCTR = 0;                                //计数器写0
    (*EPwm[num]).TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;      //计数器为增减计数
    (*EPwm[num]).TBCTL.bit.HSPCLKDIV = TB_DIV1;            //高速时钟分频(分频系数为1)
    (*EPwm[num]).TBCTL.bit.CLKDIV = TB_DIV1;               //基准时钟分频位(分频系数为1)
    (*EPwm[num]).TBCTL.bit.PHSEN = TB_DISABLE;             //禁止加载相位寄存器TBPHS中的值
    (*EPwm[num]).TBCTL.bit.SYNCOSEL =TB_SYNC_IN;         //CTR=0,时基计数器TBCTR=0
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
    (*EPwm[num]).DBCTL.bit.OUT_MODE = DB_DISABLE;      //使能上升沿及下降沿的延时信号
    (*EPwm[num]).DBCTL.bit.POLSEL = DB_ACTV_HI;            //正常模式 EPWMxA和EPWMxB均不翻转极性
    (*EPwm[num]).DBRED.bit.DBRED = (Uint16)PV_DEADBAND;   //上升沿死区时间0.5us
    (*EPwm[num]).DBFED.bit.DBFED = (Uint16)PV_DEADBAND;   //下降沿死区时间0.5us
    /*      TZ保护模块        */
    (*EPwm[num]).TZSEL.bit.OSHT1 = 1;                      //使能TZ1(逆变过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZSEL.bit.OSHT2 = 1;                      //使能TZ2(母线过流)作为EPWM的单次故障保护触发输入
    (*EPwm[num]).TZCTL.bit.TZA = TZ_FORCE_LO;              //TZ保护时强制EPWMxA为低电平
    (*EPwm[num]).TZCTL.bit.TZB = TZ_FORCE_LO;              //TZ保护时强制EPWMxB为低电平
    (*EPwm[num]).TZOSTCLR.bit.OST1 = 1;                        //清楚TZ故障
    (*EPwm[num]).TZOSTCLR.bit.OST2 = 1;                        //清楚TZ故障
    (*EPwm[num]).TZCLR.bit.OST = 1;                        //清楚TZ故障
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
* @brief: EPwm_Init
* @param: void
* @return:Nothing
* @note:PWM配置总函数
*/
void EPwm_Init(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   //禁止所有EPWM时基时钟

    InterruptEpwm_Init();                   //中断EPWM初始化
    InvEpwm_Init();                         //逆变EPWM初始化
    PvEpwm_Init();                          //Pv的EPWM初始化
    FanEpwm_Init();                         //风扇EPWM初始化

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   //使能所有EPWM时基时钟
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
    EDIS;
}
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
    EPwm8Regs.CMPB.bit.CMPB = (uint32_t)(EPwm8Regs.TBPRD * (dutypv[1]));
    EPwm9Regs.CMPA.bit.CMPA = (uint32_t)(EPwm9Regs.TBPRD * (1 - dutypv[2]));
    EPwm9Regs.CMPB.bit.CMPB = (uint32_t)(EPwm9Regs.TBPRD * (dutypv[3]));
    EALLOW;
    EPwm8Regs.TZCLR.bit.OST = 1;
    EPwm9Regs.TZCLR.bit.OST = 1;
    EDIS;
    return 1;
}

/**
  * @brief  ClearEpwmTz
  * @param  None
  * @retval None
  * @note   None
*/
void ClearEpwmTz(Uint16 num)
{
    (*EPwm[num]).TZOSTCLR.bit.OST1 = 1;                        //清楚TZ故障
    (*EPwm[num]).TZOSTCLR.bit.OST2 = 1;                        //清楚TZ故障
    (*EPwm[num]).TZCLR.bit.OST = 1;                        //清楚TZ故障
}
/**
  * @brief  EventClear
  * @param  None
  * @return None
  * @note   清除中断标志位
*/
void EventClear(Uint16 num)
{
    (*EPwm[num]).ETCLR.all = 0xFF;
}
/**
  * @brief  CPUETINTSet
  * @param  None
  * @return None
  * @note   设置CPU的EPWM的中断源以及中断使能
*/
void CPUETINTSet(Uint16 num)
{
    (*EPwm[num]).ETSEL.bit.INTEN = 1;                       //使能中断信号
    (*EPwm[num]).ETSEL.bit.INTSEL = ET_CTR_PRD;                       //使能中断信号
}
/**
  * @brief  CLAETINTSet
  * @param  None
  * @return None
  * @note   设置CLA的EPWM的中断源以及中断使能
*/
void CLAETINTSet(Uint16 num)
{
    (*EPwm[num]).ETSEL.bit.INTEN = 1;                       //使能中断信号
    (*EPwm[num]).ETSEL.bit.INTSEL = ET_CTRD_CMPA;                       //使能中断信号
}
