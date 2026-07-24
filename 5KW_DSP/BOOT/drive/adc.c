/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        adc.c
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


#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "adc.h"

void ConfigureADC(void);
void SetupADCConversion(void);

//variables to store conversion results
//Uint16 AdcaResult0;
//Uint16 AdcaResult1;
//Uint16 AdcbResult0;
//Uint16 AdcbResult1;

/**
* @brief: AdcInit
* @param: void                        
* @note: 
*
*/
void AdcInit(void)
{
//Configure the ADCs and power them up
    ConfigureADC();

//Setup the ADCs for software conversions
    SetupADCConversion();

////take conversions indefinitely in loop
//    do
//    {
//    //convert, wait for completion, and store results
//      //start conversions immediately via software, ADCA
//      AdcaRegs.ADCSOCFRC1.all = 0x0003; //SOC0 and SOC1
//      //start conversions immediately via software, ADCB
//      AdcbRegs.ADCSOCFRC1.all = 0x0003; //SOC0 and SOC1
//      //wait for ADCA to complete, then acknowledge flag
//      while(AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0);
//      AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
//      //wait for ADCB to complete, then acknowledge flag
//      while(AdcbRegs.ADCINTFLG.bit.ADCINT1 == 0);
//      AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;
//      //store results
//      AdcaResult0 = AdcaResultRegs.ADCRESULT0;
//      AdcaResult1 = AdcaResultRegs.ADCRESULT1;
//      AdcbResult0 = AdcbResultRegs.ADCRESULT0;
//      AdcbResult1 = AdcbResultRegs.ADCRESULT1;
//
//      //at this point, conversion results are stored in
//      //AdcaResult0, AdcaResult1, AdcbResult0, and AdcbResult1
//
//      //software breakpoint, hit run again to get updated conversions
//      asm("   ESTOP0");
//
//    }while(1);
}


/**
* @brief: ConfigureADC
* @param: void                        
* @note: 
*
*/
void ConfigureADC(void)
{
    EALLOW;

    //write configurations
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdccRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcdRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
//    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
//    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
//    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
//    AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcaRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdcaRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdcbRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdcbRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdccRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdccRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    AdcdRegs.ADCCTL2.bit.RESOLUTION = ADC_RESOLUTION_12BIT;
    AdcdRegs.ADCCTL2.bit.SIGNALMODE = ADC_SIGNALMODE_SINGLE;
    //Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //power up the ADCs
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    //delay for 1ms to allow ADC time to power up
    DELAY_US(2000);

    EDIS;
}

/**
* @brief: SetupADCConversion
* @param: void                        
* @note: 
*
*/
void SetupADCConversion(void)
{
    EALLOW;

    // ADC_Load_Current
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert Inv-1
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger on ePWM1 SOCA/C

    //ADC_MicroGrid_Current
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 1;  //SOC1 will convert Iinv-2
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_PowerGrid_Current
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 2;  //SOC2 will convert Inv-3
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Leak_Current
    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 3;  //SOC3 will convert Inv-4
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Inductor_Current
    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 4;  //SOC4 will convert Vout
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Transformer_Current
    AdcaRegs.ADCSOC5CTL.bit.CHSEL = 5;  //SOC5 will convert Vin
    AdcaRegs.ADCSOC5CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC5CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Inductor_Current
    AdcaRegs.ADCSOC6CTL.bit.CHSEL = 14;  //SOC4 will convert Vout
    AdcaRegs.ADCSOC6CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC6CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Transformer_Current
    AdcaRegs.ADCSOC7CTL.bit.CHSEL = 15;  //SOC5 will convert Vin
    AdcaRegs.ADCSOC7CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcaRegs.ADCSOC7CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    // ADC_PV2_Current
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert Inv-1
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger on ePWM1 SOCA/C

    //ADC_PV1_Voltage
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 1;  //SOC1 will convert Iinv-2
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_PV2_Voltage
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 2;  //SOC2 will convert Inv-3
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Invert_Voltage
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 3;  //SOC3 will convert Inv-4
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    // ADC_DC_Component
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 2;  //SOC0 will convert Inv-1
    AdccRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger on ePWM1 SOCA/C

    //ADC_CT_Current
    AdccRegs.ADCSOC1CTL.bit.CHSEL = 3;  //SOC1 will convert Iinv-2
    AdccRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdccRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Temp
    AdccRegs.ADCSOC2CTL.bit.CHSEL = 4;  //SOC2 will convert Inv-3
    AdccRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdccRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    // ADC_PowerGrid_Voltage
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert Inv-1
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger on ePWM1 SOCA/C

    //ADC_MicroGrid_Voltage
    AdcdRegs.ADCSOC1CTL.bit.CHSEL = 1;  //SOC1 will convert Iinv-2
    AdcdRegs.ADCSOC1CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Load_Voltage
    AdcdRegs.ADCSOC2CTL.bit.CHSEL = 2;  //SOC2 will convert Inv-3
    AdcdRegs.ADCSOC2CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC2CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Bat_Voltage
    AdcdRegs.ADCSOC3CTL.bit.CHSEL = 3;  //SOC3 will convert Inv-4
    AdcdRegs.ADCSOC3CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC3CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger

    //ADC_Bus_Voltage
    AdcdRegs.ADCSOC4CTL.bit.CHSEL = 4;  //SOC4 will convert Vout
    AdcdRegs.ADCSOC4CTL.bit.ACQPS = ACQPS_SYS_CLKS; //sample window in SYSCLK cycles
    AdcdRegs.ADCSOC4CTL.bit.TRIGSEL = ADC_TRIG_SOURCE; //trigger
    EDIS;
}
