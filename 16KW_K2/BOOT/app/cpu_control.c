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
#include "interrupt.h"
#include "gpio.h"
#include "adc.h"
#include "pwm.h"

float gVolPv[4];
float gCurPv[4];
float gDutyPv[4];
//TAG:测试用
volatile uint16_t gEnPv = 0;

CpuInterrupt_t gstCpuInterrupt;//CPU中断的状态
uint8_t SciDataTemp;    //缓存sci接收数据

/**
* @brief: AdcGpioStateSwitch
* @param: void
* @note:ADC的GPIO状态切换控制
*/
//void AdcGpioStateSwitch(void)
//{
//    switch(gstCpuInterrupt.time)
//    {
//        case EN_CPU_INTERRUPT_TIME_1://TAG:运行时间9us
//        {
//            /*      pv AD采样值切换  当前处于 S0低电平,S1低电平,已完成AD拿值,切换至time2的采样通道*/
//            GpioDataRegs.GPBSET.bit.GPIO34  = 1;
//            GpioDataRegs.GPBCLEAR.bit.GPIO35    = 1;
//            /*      AD采样值切换  当前处于 S0低电平,S1低电平,已完成AD拿值,切换至time2的采样通道*/
//            GpioDataRegs.GPBSET.bit.GPIO40  = 1;
//            GpioDataRegs.GPBCLEAR.bit.GPIO41    = 1;
//            /*      温度采样切换至001或101  */
//            GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
//            GpioDataRegs.GPASET.bit.GPIO27    = 1;
//            /***********************************/
//            //PV运行
//            MpptInverter();//TAG:执行时间7us
//            /***********************************/
//            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_2;
//            break;
//        }
//        case EN_CPU_INTERRUPT_TIME_2://TAG:运行时间5.7us
//        {
//            /*      pv AD采样值切换  当前处于 S0低电平,S1高电平,已完成AD拿值,切换至time3的采样通道*/
//            GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;
//            GpioDataRegs.GPBSET.bit.GPIO35    = 1;
//            /*      AD采样值切换  当前处于 S0低电平,S1高电平,已完成AD拿值,切换至time3的采样通道*/
//            GpioDataRegs.GPBCLEAR.bit.GPIO40    = 1;
//            GpioDataRegs.GPBSET.bit.GPIO41  = 1;
//            /*      温度采样切换至010或110  */
//            GpioDataRegs.GPASET.bit.GPIO26    = 1;
//            GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
//            /***********************************/
//            //TAG:PV测试用
//            uint16_t bus_normal = (gstAdcResult.bus_volt_all > 400);
//            /***********************************/
//            /***********************************/
//            //PV输入输出数据更新
//            gVolPv[0] = gstAdcResult.pv1_volt;
//            gVolPv[1] = gstAdcResult.pv2_volt;
//            gVolPv[2] = gstAdcResult.pv3_volt;
//            gVolPv[3] = gstAdcResult.pv4_volt;
//            gCurPv[0] = gstAdcResult.pv1_curr;
//            gCurPv[1] = gstAdcResult.pv2_curr;
//            gCurPv[2] = gstAdcResult.pv3_curr;
//            gCurPv[3] = gstAdcResult.pv4_curr;
//            PvDataUpdata(gVolPv,gCurPv, (gstAdcResult.bus_volt_all)); //TAG:执行时间250ns
//            gDutyPv[0] = PvDutyRead(0);
//            gDutyPv[1] = PvDutyRead(1);
//            gDutyPv[2] = PvDutyRead(2);
//            gDutyPv[3] = PvDutyRead(3);
//            PvEpwmUpdata(gDutyPv,gEnPv); //TODO:gEnPv与其他模块联系
//            /***********************************/
//            /***********************************/
//            //TAG:测试用，后续gEnPv可由其他模块例如故障诊断模块，来关闭PV
//
//            if(gEnPv == 1 && bus_normal == 0)
//            {
//                EnMpptRun();
//            }
//            else
//            {
//                CloseMpptRun();
//            }
//
//            /***********************************/
//            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_3;
//            break;
//
//        }
//        case EN_CPU_INTERRUPT_TIME_3:
//        {
//            /*      pv AD采样值切换  当前处于 S0高电平,S1高电平,已完成AD拿值,切换至time4的采样通道*/
//            GpioDataRegs.GPBSET.bit.GPIO34  = 1;
//            GpioDataRegs.GPBSET.bit.GPIO35    = 1;
//            /*      AD采样值切换  当前处于 S0高电平,S1低电平,已完成AD拿值,切换至time4的采样通道*/
//            GpioDataRegs.GPBSET.bit.GPIO40    = 1;
//            GpioDataRegs.GPBSET.bit.GPIO41    = 1;
//            /*      温度采样切换至011或111  */
//            GpioDataRegs.GPASET.bit.GPIO26    = 1;
//            GpioDataRegs.GPASET.bit.GPIO27    = 1;
//            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_4;
//            break;
//        }
//        case EN_CPU_INTERRUPT_TIME_4:
//        {
//            /*      pv AD采样值切换  当前处于 S0低电平,S1低电平,已完成AD拿值,切换至time1的采样通道*/
//            GpioDataRegs.GPBCLEAR.bit.GPIO34  = 1;
//            GpioDataRegs.GPBCLEAR.bit.GPIO35    = 1;
//            /*      AD采样值切换  当前处于 S0高电平,S1高电平,已完成AD拿值,切换至time1的采样通道*/
//            GpioDataRegs.GPBCLEAR.bit.GPIO40  = 1;
//            GpioDataRegs.GPBCLEAR.bit.GPIO41  = 1;
//            /*      温度采样切换至001或101  */
//            if(GpioDataRegs.GPADAT.bit.GPIO25 == 0)
//            {
//                GpioDataRegs.GPASET.bit.GPIO25    = 1;
//                GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
//                GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
//            }
//            else
//            {
//                GpioDataRegs.GPACLEAR.bit.GPIO25  = 1;
//                GpioDataRegs.GPACLEAR.bit.GPIO26  = 1;
//                GpioDataRegs.GPACLEAR.bit.GPIO27  = 1;
//            }
//            gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_1;
//            break;
//        }
//        default:
//        {
//            if(Get_Adc_Calibration_State() == 1)//已完成ADC的零点校准,开始正常切换adc的GPIO
//            {
//                gstCpuInterrupt.time = EN_CPU_INTERRUPT_TIME_1;
//            }
//            break;
//        }
//    }
//}
/**
* @brief: CPU_Interrupt
* @param: void
* @return:Nothing
* @note:CPU中断服务函数
*/
void CpuIntServiceFun(void)
{
    /* 虚拟一个50us的时间周期 */
    if(gstCpuInterrupt.cpu_int_count % 4 == 0)
    {
        /* 接收ARC 485数据（scib） */
        ScibRxDataRecv();
    }
    else if(gstCpuInterrupt.cpu_int_count % 4 == 1)
    {

    }
    else if(gstCpuInterrupt.cpu_int_count % 4 == 2)
    {
        /* 接收Menter ISO485数据（scic） */
        ScicRxDataRecv();
    }
    else if(gstCpuInterrupt.cpu_int_count % 4 == 3)
    {

    }

    gstCpuInterrupt.cpu_int_count++;
}

void CpuIntFunInit(void)
{
    RegisterCpuIntCallback(&CpuIntServiceFun);
    Interrupt_Init();
}

