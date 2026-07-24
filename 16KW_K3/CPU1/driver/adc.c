/*
 * ADCadc.c
 *
 *  Created on: 2025年10月13日
 *      Author: Nova006348
 */
#include <adc.h>

/**
 * @brief AdcConfig
 * @param
 */
void AdcConfig(void)
{
    EALLOW;
        //write configurations
        AdcaRegs.ADCCTL2.bit.PRESCALE = 14;
        AdcbRegs.ADCCTL2.bit.PRESCALE = 14;
        AdccRegs.ADCCTL2.bit.PRESCALE = 14;
        AdcdRegs.ADCCTL2.bit.PRESCALE = 14;
        AdcaRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdcaRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdcbRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdcbRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdccRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdccRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdcdRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
        AdcdRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
        AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;
        AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;
        AdcaRegs.ADCSOCPRICTL.bit.SOCPRIORITY = 1; //SOC0具有最高优先级

        DELAY_US(2000);

        /*     Adc A0    */
        AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //逆变A相电压
        AdcaRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc A1    */
        AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //逆变A相电流
        AdcaRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc A2    */
        AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //PV1电流
        AdcaRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //PV2电流
        AdcaRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //PV3电流
        AdcaRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //PV4电流
        AdcaRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc A3    */
        AdcaRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //市电B相电压
        AdcaRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC7CTL.bit.CHSEL = 3;                  //发电机B相电压
        AdcaRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC8CTL.bit.CHSEL = 3;                  //负载B相电压
        AdcaRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC9CTL.bit.CHSEL = 3;                  //总继电器与单继电器中间的B相电压(ADC_RelayMid_VOL_B)
        AdcaRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc A4    */
        AdcaRegs.ADCSOC10CTL.bit.CHSEL = 4;                  //市电A相电压
        AdcaRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC11CTL.bit.CHSEL = 4;                  //发电机A相电压
        AdcaRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC12CTL.bit.CHSEL = 4;                  //负载A相电压
        AdcaRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcaRegs.ADCSOC13CTL.bit.CHSEL = 4;                  //总继电器与单继电器中间的A相电压(ADC_RelayMid_VOL_A)
        AdcaRegs.ADCSOC13CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC13CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc A5    */
        AdcaRegs.ADCSOC14CTL.bit.CHSEL = 5;                  //A相CT电流
        AdcaRegs.ADCSOC14CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcaRegs.ADCSOC14CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc B0    */
        AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //逆变B相电压
        AdcbRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc B1    */
        AdcbRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //逆变B相电流
        AdcbRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc B2    */
        AdcbRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //B相CT电流
        AdcbRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc B3    */
        AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3;                  //市电C相电压
        AdcbRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcbRegs.ADCSOC4CTL.bit.CHSEL = 3;                  //发电机C相电压
        AdcbRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcbRegs.ADCSOC5CTL.bit.CHSEL = 3;                  //负载C相电压
        AdcbRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcbRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //总继电器与单继电器中间的A相电压(ADC_RelayMid_VOL_C)
        AdcbRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcbRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc C2    */
        AdccRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //PV1电压
        AdccRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //PV2电压
        AdccRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //PV3电压
        AdccRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //PV4电压
        AdccRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc C3    */
        AdccRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //市电B相电流
        AdccRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC7CTL.bit.CHSEL = 3;                  //发电机B相电流
        AdccRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC8CTL.bit.CHSEL = 3;                  //负载B相电流
        AdccRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC9CTL.bit.CHSEL = 3;                  //预留
        AdccRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc C4    */
        AdccRegs.ADCSOC10CTL.bit.CHSEL = 4;                  //市电A相电流
        AdccRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC11CTL.bit.CHSEL = 4;                  //发电机A相电流
        AdccRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC12CTL.bit.CHSEL = 4;                  //负载A相电流
        AdccRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdccRegs.ADCSOC13CTL.bit.CHSEL = 4;                  //预留
        AdccRegs.ADCSOC13CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdccRegs.ADCSOC13CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D0    */
        AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;                  //逆变C相电压
        AdcdRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D1    */
        AdcdRegs.ADCSOC1CTL.bit.CHSEL = 1;                  //逆变C相电流
        AdcdRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D2    */
        AdcdRegs.ADCSOC2CTL.bit.CHSEL = 2;                  //市电C相电流
        AdcdRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcdRegs.ADCSOC3CTL.bit.CHSEL = 2;                  //发电机C相电流
        AdcdRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcdRegs.ADCSOC4CTL.bit.CHSEL = 2;                  //负载C相电流
        AdcdRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcdRegs.ADCSOC5CTL.bit.CHSEL = 2;                  //预留
        AdcdRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D3    */
        AdcdRegs.ADCSOC6CTL.bit.CHSEL = 3;                  //C相CT电流
        AdcdRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D4    */
        AdcdRegs.ADCSOC7CTL.bit.CHSEL = 4;                  //RCMU电压(漏电流电压)
        AdcdRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D14    */
        AdcdRegs.ADCSOC8CTL.bit.CHSEL = 14;                 //BUS正电压
        AdcdRegs.ADCSOC8CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC8CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcdRegs.ADCSOC9CTL.bit.CHSEL = 14;                 //BUS负电压
        AdcdRegs.ADCSOC9CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC9CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcdRegs.ADCSOC10CTL.bit.CHSEL = 14;                 //BUS的平均电流
        AdcdRegs.ADCSOC10CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC10CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源

        AdcdRegs.ADCSOC11CTL.bit.CHSEL = 14;                 //市电N对PE电压
        AdcdRegs.ADCSOC11CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC11CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        /*     Adc D15    */
        AdcdRegs.ADCSOC12CTL.bit.CHSEL = 15;                 //温度获取
        AdcdRegs.ADCSOC12CTL.bit.ACQPS = ACQPS_SYS_CLKS;     //使用采样窗口时间
        AdcdRegs.ADCSOC12CTL.bit.TRIGSEL = ADC_TRIG_SOURCE;  //触发源
        EDIS;
}
