/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sineAnalyzerWithPowerMeas.h
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

#ifndef SINEANALYZERWITHPOWERMEAS_H
#define SINEANALYZERWITHPOWERMEAS_H

#ifdef __cplusplus

extern "C" {
#endif

//*****************************************************************************
// the includes
//*****************************************************************************
#include <stdint.h>
#include <math.h>

//*****************************************************************************
//defines
//*****************************************************************************
typedef struct
{
    float v;                    // Input: Voltage Sine Signal
    float i;                    // Input Current Signal
    float sampleFreq;           // Input: Signal Sampling Freq
    float threshold;            // Input: Voltage level corresponding to zero i/p
    float vRms;                 // Output: RMS Value
    float vAvg;                 // Output: Average Value
    float vEma;                 // Output: Exponential Moving Average Value
    float acFreq;               // Output: Signal Freq
    float acFreqAvg;            // Output: Signal Freq
    float iRms;                 // Output: RMS Value of current
    float pRms;                 // Output: RMS Value of input power
    float vaRms;                // Output: RMS VA
    float powerFactor;          // Output: powerFactor
    int16_t  zcd;               // Output: Zero Cross detected
    float vSum;             // Internal : running sum for vacc_avg calculation over one sine cycle
    float vSqrSum;              // Internal : running sum for vacc_rms calculation over one sine cycle
    float iSqrSum;              // Internal : running sum for Iacc_rms calculation over one sine cycle
    float acFreqSum;            // Output: Signal Freq
    float pSum;             // Internal : running sum for Pacc_rms calculation over one sine cycle
    float vaSumMul;             // Internal : running sum for Pacc_rms calculation over one sine cycle
    float vNorm;            // Internal: Normalized value of the input voltage
    float iNorm;            // Internal: Normalized value of the input current
    int16_t  prevSign;          // Internal: Flag to detect ZCD
    int16_t  currSign;          // Internal: Flag to detect ZCD
    int32_t  nSamples;          // Internal: No of samples in one cycle of the sine wave
    int32_t  nSamplesMin;       // Internal: Lowerbound for no of samples in one sine wave cycle
    int32_t  nSamplesMax;       // Internal: Upperbound for no of samples in one sine wave cycle
    float inverse_nSamples;     // Internal: 1/( No of samples in one cycle of the sine wave)
    float sqrt_inverse_nSamples;// Internal: sqrt(1/( No of samples in one cycle of the sine wave))
    int16_t  slewPowerUpdate;   // Internal: used to slew update of the power value
    float pRmsSumMul;               // Internal: used to sum Pac value over multiple sine cycles (100)
    int16_t jitterCount;
    float emaFilterMultiplier;
} sineAnalyzerWithPowerMeas;

typedef sineAnalyzerWithPowerMeas *sineAnalyzerWithPowerMeas_Handle;

//*****************************************************************************
// the function prototypes
//*****************************************************************************

void SineAnalyzerWithPowerMeasInit(sineAnalyzerWithPowerMeas *v);
void SineAnalyzerWithPowerMeasCalc(sineAnalyzerWithPowerMeas *v);

#ifdef __cplusplus
}
#endif                                  /* extern "C" */


#endif /*SineAlanyzer_diff_wPower_F_C_H_*/
