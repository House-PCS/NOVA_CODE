/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.h
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
  * <tr><td>2022-10-31    <td>V1.0.0    <td>           <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
  
#ifndef _BMS_TEMP__
#define _BMS_TEMP__

#include <stdint.h>


extern const uint32_t B3435_10K_DefaultResTab[166];
    
uint32_t NTC_ResistorReckon(uint16_t adc_vol,uint32_t r_up,uint16_t vol);
int8_t NTC_TempReckon(uint16_t adc_vol,const uint32_t* rtc_tab,uint16_t vol_max,uint32_t r_up);


#endif
