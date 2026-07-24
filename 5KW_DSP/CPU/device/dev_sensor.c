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

#define LN(a)         (_ApproLn(a))


#define TEMP_S0    25
#define TEMP_S1    26
#define TEMP_S2    27

Sensor_Handle_t gstSensorData;
Sensor_Drift_t gstSenorDrift;
static void _CalibrateOffset(void);
static void CD4051Init(void);
uint16_t CalibrationFlag = 0;
float AdcRef = 0;
uint16_t gIntCount = 0;
uint16_t gCD4051Chinnel = 0;
uint16_t garrTemperInitialData[8];
uint32_t gPvOvrCurRecoverCount = 0;

/**
* @brief: SensorInit
* @param: void
* @note:
*
*/
void SensorInit(void)
{
    uint16_t i;
    for(i = 0; i < 8; i++) // 20260429，优化代码
    {
        garrTemperInitialData[i] = 0;
    }
    AdcInit();
    _CalibrateOffset();
    CD4051Init();
    CD4051ChannelSet(gCD4051Chinnel);
    gstSenorDrift.inductor_cur_drift = 0.0;
    gstSenorDrift.grid_cur_drift = 0.0;
    gstSenorDrift.load_cur_drift = 0.0;
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

    EPwm2Regs.CMPB.bit.CMPB= EPwm2Regs.TBPRD - (ACQPS_SYS_CLKS <<1);

    EPwm2Regs.ETSEL.bit.SOCASEL = ET_CTR_PRD; //ET_CTRU_CMPA; // select CMPC when counting up to trigger the ADC
    EPwm2Regs.ETSEL.bit.SOCBSEL = ET_CTR_ZERO; //ET_CTRU_CMPA; // select CMPC when counting up to trigger the ADC

    EPwm2Regs.ETPS.bit.SOCAPRD = ET_1ST; /* Generate pulse on 1st even*/
    EPwm2Regs.ETSEL.bit.SOCAEN = 1; /* Enable SOC on A group*/
    EPwm2Regs.ETPS.bit.SOCBPRD = ET_1ST; /* Generate pulse on 1st even*/
    EPwm2Regs.ETSEL.bit.SOCBEN = 1; /* Enable SOC on A group*/

    //PWM1 INT is used to trigger the ISR
    EPwm2Regs.ETSEL.bit.INTSEL = ET_CTRD_CMPB;      //

    EPwm2Regs.ETSEL.bit.INTEN = 1;              // Enable INT
    EPwm2Regs.ETPS.bit.INTPRD = ET_1ST;         // Generate INT on every event

    EPwm2Regs.ETCLR.bit.INT=1;

// 配置PV的采样触发源
//
    EPwm11Regs.ETSEL.bit.SOCASEL = ET_CTR_PRD ;//ET_CTRU_CMPA; // select CMPC when counting up to trigger the ADC
//
    EPwm11Regs.ETPS.bit.SOCAPRD = ET_1ST; /* Generate pulse on 1st even*/
    EPwm11Regs.ETSEL.bit.SOCAEN = 1; /* Enable SOC on A group*/
//
    //PWM1 INT is used to trigger the ISR
    EPwm11Regs.ETSEL.bit.INTSEL = ET_CTRD_CMPB;      //
//
    EPwm11Regs.ETSEL.bit.INTEN = 1;              // Enable INT
    EPwm11Regs.ETPS.bit.INTPRD = ET_1ST;         // Generate INT on every event
//
    EPwm11Regs.ETCLR.bit.INT=1;
// 配置LLC的采样触发源
//
    EPwm6Regs.ETSEL.bit.SOCASEL = ET_CTR_PRD ;//ET_CTRU_CMPA; // select CMPC when counting up to trigger the ADC
//
    EPwm6Regs.ETPS.bit.SOCAPRD = ET_1ST; /* Generate pulse on 1st even*/
    EPwm6Regs.ETSEL.bit.SOCAEN = 1; /* Enable SOC on A group*/
//
    //PWM1 INT is used to trigger the ISR
    EPwm6Regs.ETSEL.bit.INTSEL = ET_CTRD_CMPB;      //
//
    EPwm6Regs.ETSEL.bit.INTEN = 1;              // Enable INT
    EPwm6Regs.ETPS.bit.INTPRD = ET_1ST;         // Generate INT on every event
//
    EPwm6Regs.ETCLR.bit.INT=1;

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

static double _ApproLn(double a)
{
    uint16_t n = 15;//我们取前15+1项来估算
    uint16_t k = 0;
    uint16_t nk = 0;
    double x = 0;
    double x2 = 0;
    double y = 0;

    x = (a - 1) / (a + 1);
    x2 = x * x;
    nk = 2*n + 1;
    y = 1.0 / nk;

    for(k = n; k > 0; k--)
    {
        nk -= 2;
        y = 1.0 / nk + x2 * y;
    }
    return 2.0 * x * y;
}

//单位K

/**
* @brief: _RtempCal
* @param: rtemp
* @return:
* @note:
*
*/
static float _RtempCal(float rtemp)
{
    float rtemp_1 = 0;;

    if(rtemp >= 3.3)
    {
        rtemp = 3.3;
    }
    rtemp = R_UP * rtemp / (3.3f - rtemp);
    return rtemp;
}

float GetTemperature(float Rntc)
{
    float tmp1 = 0;
    float res = 0;

    tmp1 = LN(Rntc / RTEMP25) / RTEMP_B + 1 / T25;
    res = 1 / tmp1 - 273.15f;

    return res;
}



/**
* @brief: INVTemperatureGet
* @param: void
* @note:
*
*/
void InvTemperatureGet(void)
{
    uint16_t adc_temp = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    adc_temp = garrTemperInitialData[4];

    if(adc_temp >= 3975)
    {
        adc_temp = 3975;
    }
    float32 v = AdcRef * adc_temp;
    rtotal = (R_UP  * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);
    //rntc = (VOL_UP * 5 / (VOL_UP - v)) - R_UP * 0.75;
    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstSensorData.temp[1] = rtemp * 10;

}

void PvTemperatureGet(void)
{
    uint16_t adc_temp = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    adc_temp = garrTemperInitialData[2];

    if(adc_temp >= 3975)
    {
        adc_temp = 3975;
    }

    float32 v = AdcRef * adc_temp;
    rtotal = (R_UP  * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);
    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstSensorData.temp[2] = rtemp * 10;

}

void DcdcTemperatureGet(void)
{
    uint16_t adc_temp = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    adc_temp = garrTemperInitialData[3];

    if(adc_temp >= 3975)
    {
        adc_temp = 3975;
    }

    float32 v = AdcRef * adc_temp;
    rtotal = (R_UP  * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);
    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstSensorData.temp[3] = rtemp * 10;

}

void ChassisInternalTemperatureGet(void)
{
    uint16_t adc_temp = 0;
    float32 rtotal = 0;
    float32 rntc = 0;
    float32 rtemp = 0;

    adc_temp = garrTemperInitialData[1];

    if(adc_temp >= 3975)
    {
        adc_temp = 3975;
    }

    float32 v = AdcRef * adc_temp;
    rtotal = (R_UP  * v) / (VOL_UP - v);
    rntc = (R_DOWN * rtotal) / (R_DOWN - rtotal);
    rtemp = LN(rntc / RTEMP25) / RTEMP_B + T25_1;
    rtemp = 1 / rtemp - 273.15f;

    gstSensorData.temp[5] = rtemp * 10;

}
void InvTemperMaxGet(void)
{
    uint16_t i = 0;
    InvTemperatureGet();
    PvTemperatureGet();
    DcdcTemperatureGet();
    ChassisInternalTemperatureGet();

    gstSensorData.temp[4] = -1000;
    for(i = 1; i < 4; i++)
    {
        if(gstSensorData.temp[i] > gstSensorData.temp[4])
        {
            gstSensorData.temp[4] = gstSensorData.temp[i];
        }
    }
}
