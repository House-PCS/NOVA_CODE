/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        uart.c
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
  
#ifndef __FUNCTION_TEST_H_
#define __FUNCTION_TEST_H_

#include <stdint.h>
#include <stdbool.h>


typedef enum
{
    TEST_ITEM_BCU_IDLE = 0,
    
//    TEST_ITEM_BCU_ISOSPI,
    TEST_ITEM_BCU_EEPROM,
    TEST_ITEM_BCU_FLASH,
    
    

} EN_TEST_ITEM;

typedef struct 
{
    bool    test_mode;
//    uint8_t iso_spi;
    bool e2prom_test;
    bool flash_test_flag;
}FunctionTestDataType;

extern FunctionTestDataType FunctionTestDataInfo;



void BcuTextModeSet(void);
void BcuTextItemSet(EN_TEST_ITEM item);
void FunctionTestProcess(void);
uint8_t flash_test(void);

#endif
