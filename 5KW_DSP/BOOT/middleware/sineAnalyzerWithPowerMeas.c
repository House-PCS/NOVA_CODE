/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        sineAnalyzerWithPowerMeas.c
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

#include "sineAnalyzerWithPowerMeas.h"

/**
* @brief: SineAnalyzerWithPowerMeasInit
* @param: v                           
* @note: 
*
*/
void SineAnalyzerWithPowerMeasInit(sineAnalyzerWithPowerMeas *v)
{
    v->v=0;
    v->i=0;
    v->sampleFreq=0;
    v->threshold=0;
    v->vRms=0;
    v->vAvg=0;
    v->vEma=0;
    v->acFreq=0;
    v->iRms=0;
    v->pRms=0;
    v->vaRms=0;
    v->powerFactor=0;
    v->zcd=0;
    v->vSum=0;
    v->vSqrSum=0;
    v->iSqrSum=0;
    v->pSum=0;
    v->vaSumMul=0;
    v->vNorm=0;
    v->iNorm=0;
    v->prevSign=1;
    v->currSign=1;
    v->nSamples=0;
    v->nSamplesMin = 0;
    v->nSamplesMax = 0;
    v->inverse_nSamples=0;
    v->sqrt_inverse_nSamples=0;
    v->slewPowerUpdate=0;
    v->pRmsSumMul=0;
    v->acFreqSum=0;
    v->acFreqAvg=0;
    v->jitterCount=0;
    v->emaFilterMultiplier=0;
}

/**
* @brief: SineAnalyzerWithPowerMeasCalc
* @param: v                           
* @note: 
*
*/
void SineAnalyzerWithPowerMeasCalc(sineAnalyzerWithPowerMeas *v)
{
    v->vNorm = fabs(v->v);
    v->iNorm = fabs(v->i);
    v->currSign = ( v->v > v->threshold) ? 1 : 0;
    v->nSamples++;
    v->vSum = v->vSum+v->vNorm;
    v->vSqrSum = v->vSqrSum+(v->vNorm*v->vNorm);
    v->vEma = v->vEma+(v->emaFilterMultiplier*(v->vNorm - v->vEma));
    v->iSqrSum = v->iSqrSum+(v->iNorm*v->iNorm);
    v->pSum = v->pSum+(v->i*v->v);
    v->zcd=0;

    if((v->prevSign != v->currSign)
            && (v->currSign == 1))
    {
        // check if the nSamples are in the ball park of a real frequency
        // that can be on the grid, this is done by comparing the nSamples
        // with the max value and min value it can be for the AC Grid Connection
        // these Max and Min are initialized by the user in the code
        if(v->nSamplesMin < v->nSamples )
        {
            v->zcd=1;
            v->inverse_nSamples = (1.0)*__einvf32(v->nSamples);
            v->sqrt_inverse_nSamples = (1.0)*__eisqrtf32(v->nSamples);
            v->vAvg = (v->vSum*v->inverse_nSamples);
            v->vRms = sqrt(v->vSqrSum)*v->sqrt_inverse_nSamples;
            v->iRms = sqrt(v->iSqrSum)*v->sqrt_inverse_nSamples;
            v->pRmsSumMul = v->pRmsSumMul + (v->pSum*v->inverse_nSamples);
            v->vaSumMul = v->vaSumMul + v->vRms*v->iRms;
            v->acFreq = (v->sampleFreq*v->inverse_nSamples);
            v->acFreqSum = v->acFreqSum + v->acFreq;

            v->slewPowerUpdate++;

            if(v->slewPowerUpdate >= 100)
            {
                v->slewPowerUpdate=0;
                v->pRms = (v->pRmsSumMul*(0.01));
                v->pRmsSumMul = 0;
                v->vaRms = v->vaSumMul * (0.01);
                v->vaSumMul = 0;
                v->powerFactor=v->pRms/v->vaRms;
                v->acFreqAvg=v->acFreqSum*0.01;
                v->acFreqSum=0;
            }

            v->jitterCount=0;

            v->nSamples=0;
            v->vSum=0;
            v->vSqrSum=0;
            v->iSqrSum=0;
            v->pSum =0;
        }
        else
        {
            // otherwise it may be jitter ignore this reading
            // but count the number of jitters you are getting
            // but do not count to infinity as then when the grid comes back
            //it will take too much time to wind down the jitter count
            if(v->jitterCount<30)
                v->jitterCount++;
            v->nSamples=0;
        }

    }

    if(v->nSamples>v->nSamplesMax || v->jitterCount>20)
    {
        // most certainly the AC voltage is not present
        v->vRms = 0;
        v->vAvg = 0;
        v->vEma = 0;
        v->acFreq=0;
        v->iRms = 0;
        v->pRms = 0;
        v->vaRms =0;
        v->powerFactor=0;

        v->zcd=0;
        v->vSum=0;
        v->vSqrSum=0;
        v->iSqrSum=0;
        v->pSum=0;
        v->vaSumMul=0;
        v->pRmsSumMul = 0;
        v->acFreqAvg = 0;
        v->acFreqSum =0 ;
        v->nSamples=0;
        v->jitterCount=0;
    }

    v->prevSign = v->currSign;
}
