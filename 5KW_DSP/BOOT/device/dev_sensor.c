/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_sensor.c
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
#include "dev_sensor.h"

#define TEMP_S0    25
#define TEMP_S1    26
#define TEMP_S2    27

Sensor_Handle_t stSensorData;
static void _CalibrateOffset(void);
static void CD4051Init(void);
uint16_t CalibrationFlag = 0;
float AdcRef = 0;

/**
* @brief: SensorInit
* @param: void                        
* @note: 
*
*/
void SensorInit(void)
{
    AdcInit();
    _CalibrateOffset();
    CD4051Init();
    CD4051ChannelSet(6);
}

/**
* @brief: _CalibrateOffset
* @param: void                        
* @return: 
* @note: 
*
*/
static void _CalibrateOffset(void)
{
    EALLOW;

    EPwm2Regs.CMPB.bit.CMPB= EPwm1Regs.TBPRD - (ACQPS_SYS_CLKS <<1);

    EPwm2Regs.ETSEL.bit.SOCASEL = ET_CTRU_CMPB; //ET_CTRU_CMPA; // select CMPC when counting up to trigger the ADC

    EPwm2Regs.ETPS.bit.SOCAPRD = ET_1ST; /* Generate pulse on 1st even*/
    EPwm2Regs.ETSEL.bit.SOCAEN = 1; /* Enable SOC on A group*/

    //PWM1 INT is used to trigger the ISR
    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTRD_CMPB;      //

    EPwm2Regs.ETSEL.bit.INTEN = 1;              // Enable INT
    EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;         // Generate INT on every event

    EPwm2Regs.ETCLR.bit.INT=1;

    EDIS;

//  offsetCalCounter=0;
//  while(offsetCalCounter<25000)
//  {
//      if(EPwm1Regs.ETFLG.bit.INT==1)
//      {
//          if(offsetCalCounter>1000)
//          {
//              // offset of the inductor current sense
//              invIiOffset = k1*invIiOffset + k2*(IINV_FB1+IINV_FB2+IINV_FB3+IINV_FB4)*0.25*ADC_PU_SCALE_FACTOR;
//          }
//          EPwm1Regs.ETCLR.bit.INT=1;
//          offsetCalCounter++;
//      }
//  }

//  EALLOW;
//  AdcaRegs.ADCPPB1OFFREF=(0*4096.0);
//  AdcaRegs.ADCPPB2OFFREF=(0*4096.0);
//  AdcaRegs.ADCPPB3OFFREF=(0*4096.0);
//
//
//  AdcaRegs.ADCPPB1OFFREF =  0.5*4096.0;
//  AdcaRegs.ADCPPB1CONFIG.bit.TWOSCOMPEN=0;
//
//  AdcaRegs.ADCPPB2OFFREF =  0.5*4096.0;
//  AdcaRegs.ADCPPB2CONFIG.bit.TWOSCOMPEN=0;
//
//  EDIS;
}

/**
* @brief: CD4051Init
* @param: void                        
* @return: 
* @note: 
*
*/
static void CD4051Init(void)
{
    GPIO_SetupPinMux(TEMP_S0, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(TEMP_S0, GPIO_OUTPUT, GPIO_PUSHPULL);

    GPIO_SetupPinMux(TEMP_S1, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(TEMP_S1, GPIO_OUTPUT, GPIO_PUSHPULL);

    GPIO_SetupPinMux(TEMP_S2, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(TEMP_S2, GPIO_OUTPUT, GPIO_PUSHPULL);
}

/**
* @brief: CD4051ChannelSet
* @param: channel                     
* @note: 
*
*/
void CD4051ChannelSet(uint16_t channel)
{
    switch(channel)
    {
        case 0:
        {
            GPIO_WritePin(TEMP_S0, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S1, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S2, GPIO_CLEAR);
            break;
        }
        case 1:
        {
            GPIO_WritePin(TEMP_S0, GPIO_SET);
            GPIO_WritePin(TEMP_S1, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S2, GPIO_CLEAR);
            break;
        }
        case 2:
        {
            GPIO_WritePin(TEMP_S0, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S1, GPIO_SET);
            GPIO_WritePin(TEMP_S2, GPIO_CLEAR);
            break;
        }
        case 3:
        {
            GPIO_WritePin(TEMP_S0, GPIO_SET);
            GPIO_WritePin(TEMP_S1, GPIO_SET);
            GPIO_WritePin(TEMP_S2, GPIO_CLEAR);
            break;
        }
        case 4:
        {
            GPIO_WritePin(TEMP_S0, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S1, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S2, GPIO_SET);
            break;
        }
        case 5:
        {
            GPIO_WritePin(TEMP_S0, GPIO_SET);
            GPIO_WritePin(TEMP_S1, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S2, GPIO_SET);
            break;
        }
        case 6:
        {
            GPIO_WritePin(TEMP_S0, GPIO_CLEAR);
            GPIO_WritePin(TEMP_S1, GPIO_SET);
            GPIO_WritePin(TEMP_S2, GPIO_SET);
            break;
        }
        case 7:
        {
            GPIO_WritePin(TEMP_S0, GPIO_SET);
            GPIO_WritePin(TEMP_S1, GPIO_SET);
            GPIO_WritePin(TEMP_S2, GPIO_SET);
            break;
        }
        default:
            break;
    }
}
