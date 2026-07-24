/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        modbus_analysis.c
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
  * <tr><td>2024-2-4    <td>V1.0.1    <td>xingxiao    <td>modbus_reg LSB Save
  * </table>
  ******************************************************************************
  */

#include "ut_test_mode.h"
uint8_t load_flg[4];
uint8_t save_flg[16];
uint8_t ut_buf1[16];
uint8_t ut_mode = 0;
uint16_t timediff = 0;
void ut_test_process()
{
    static  uint32_t test_time = 0;
    if(ut_mode)
    {
        if(TimDifftimeFromNow(test_time) > 2000)
        {
            test_time =  TimGetSystime();
            for(uint16_t i = 0; i < 16; i++)
            {
                save_flg[i] = i;
            }
            //Ð´DDR
            WriteDDRSend(0x20000000, (uint8_t *)save_flg, 16);
            //¶ÁDDR
            memcpy(((uint8_t *) & (ut_buf1)), (uint8_t *)0x20000000, 16);
//            if(memcmp(save_flg, ut_buf1, 12) != 0)
//            {
//                LedBlinkTime();
//            }                
        }
    }
}
