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


extern bool _gIODirectInputState[2];

typedef enum
{
    TEST_ITEM_BMU_IDLE = 0,
    
    TEST_ITEM_BMU_FLASH,
//	TEST_ITEM_BMU_TWOWAYIO_GET,
	TEST_ITEM_BMU_TWOWAYIO_SET,
	TEST_ITEM_BMU_TWOWAYIO_RESET,
    
}EN_TEST_ITEM;

typedef struct 
{
    bool    test_mode;
    bool    flash_test_flag;
}FunctionTestDataType;

extern FunctionTestDataType FunctionTestDataInfo;


uint8_t flash_test(void);
void BmuTextModeSet(void);
void BmuTextModeDown(void);
void BmuTextItemSet(EN_TEST_ITEM item);
void FunctionTestProcess(void);
static void ReadTwoWayIO(void);

#endif
