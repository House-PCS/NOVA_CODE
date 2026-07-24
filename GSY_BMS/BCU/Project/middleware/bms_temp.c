/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2022-10-31
  * @brief       
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2022-11-10    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#include "bms_temp.h"
//默认阻值表
const uint32_t B3435_10K_DefaultResTab[166]=
{
    213859, 201805, 190489, 179886, 169926, 160573, 151789, 143535, 135779, 128487,     //-40~-31
    121630, 115180, 109110, 103397,  98018,  92952,  88178,  83680,  79438,  75439,     //-30~-21
     71665,  68105,  64743,  61570,  58572,  55739,  53061,  50530,  48135,  45870,     //-20~-11
     43726,  41697,  39775,  37954,  36229,  34593,  33042,  31572,  30176,  28851,     //-10~-1
     27594,  26400,  25265,  24187,  23162,  22188,  21261,  20379,  19540,  18741,     //0~9
     17980,  17255,  16565,  15906,  15279,  14680,  14109,  13564,  13044,  12548,     //10~19
     12074,  11621,  11188,  10774,  10378,  10000,   9630,   9277,   8938,   8613,     //20~29
      8302,   8004,   7718,   7444,   7181,   6929,   6687,   6455,   6232,   6018,     //30~39
      5812,   5615,   5425,   5243,   5067,   4899,   4737,   4581,   4431,   4287,     //40~49
      4148,   4015,   3886,   3763,   3643,   3528,   3418,   3311,   3208,   3109,     //50~59
      3014,   2922,   2833,   2747,   2664,   2585,   2508,   2433,   2362,   2292,     //60~69
      2225,   2161,   2098,   2038,   1979,   1923,   1868,   1816,   1765,   1715,     //70~79
      1668,   1622,   1577,   1534,   1492,   1451,   1412,   1374,   1337,   1302,     //80~89
      1267,   1234,   1201,   1170,   1140,   1110,   1081,   1054,   1027,   1001,     //90~99
       975,    951,    927,    903,    881,    859,    837,    816,    796,    776,
       757,    738,    720,    702,    685,    668,    652,    636,    621,    605,
       591,    577,    563,    549,    536,    523,
};
/** 
* @brief  NTC温度计算
* @param  adc_vol：电压采样值，mV
* @param  rtc_tab：指向阻值表
* @param  vol_max：上拉电压值，mV
* @param  r_up：上拉电阻值，Ω 
* @return  int8_t：温度值
* @note:
*
*/
int8_t NTC_TempReckon(uint16_t adc_vol,const uint32_t* rtc_tab,uint16_t vol_max,uint32_t r_up)
{
    int16_t temp = 0, temp_start = -40,temp_end = 125,temp1 = 0, temp2 = temp_end - temp_start + 1;
    uint16_t temp_num = 0,volt = 0;
    uint32_t rtc_r = 0;
    
    volt = adc_vol;
    if(volt >= 3300)
    {
        volt = 3299;
    }
	rtc_r = r_up * volt / (vol_max - volt);   //上拉电阻
    do{
        temp_num =  temp2 - temp1;
        temp = temp_num / 2 + temp1;
        if(rtc_r >= rtc_tab[temp])      //负温度系数
        {
            temp2 = temp;
        }
        else
        {
            temp1 = temp;
        }
    }while(temp_num >1);
    if(rtc_r >= ((rtc_tab[temp1] + rtc_tab[temp2])/2))
    {
        temp = temp1 + temp_start;
    }
    else
    {
        temp = temp2 + temp_start;
    }
//    if(temp < temp_start)
//    {
//        temp = temp_start;
//    }
    if(temp > temp_end)
    {
        temp = temp_end;
    }
    return temp;
}
/** 
* @brief  NTC阻值计算
* @param  adc_vol：电压采样值，mV
* @param  r_up：上拉电阻值，Ω
* @param  vol：上拉电压值，mV
* @return  uint32_t：NTC阻值，Ω
* @note:
*
*/
uint32_t NTC_ResistorReckon(uint16_t adc_vol,uint32_t r_up,uint16_t vol)
{
    uint16_t volt = 0;
    
    volt = adc_vol;
    if(volt >= 3300)
    {
        volt = 3299;
    }
	return r_up * volt / (vol - volt);   //上拉电阻
}


