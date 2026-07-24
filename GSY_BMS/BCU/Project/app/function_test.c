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
  
  
#include "function_test.h"
#include "systick.h"

//#include "prot_can_test.h"
#include "mcu_bsp.h"
#include "enet_prot_asp.h"
#include "dev_spiflash.h"
#include "dev_24c32.h"

FunctionTestDataType FunctionTestDataInfo;
static EN_TEST_ITEM BcuTextState = TEST_ITEM_BCU_IDLE;

uint8_t arr_rdata[4096] = {0};
uint8_t arr_wdata[4096] = {0};

uint8_t flash_test(void)
{
    
    for(unsigned int i = 0; i < 4096; i ++)
    {
        arr_wdata[i] = i % 256;
    }
    SpiFlashWriteBytesWithoutErasenoAligned(0, arr_wdata, 4096);  
    SpiFlashReadData(0, arr_rdata, 4096);
    for(unsigned int i = 0; i < 4096; i ++)
    {
        if(arr_wdata[i] != arr_rdata[i])
        {
            return 1;
        }
    } 
    return 0;
    
}

//void SpiFlashTest()
//{
//    if(flash_test() == 0)
//    {
//        FunctionTestDataInfo.flash_test_flag = true;
//    }
//}
void BcuTextItemSet(EN_TEST_ITEM item)
{
    BcuTextState = item;
}
void BcuTextModeSet()
{
    FunctionTestDataInfo.test_mode = true;
}
uint8_t TestDataWrite[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t TestDataRead[10];
#define EEPROM_TEST_ADDR    100
void FunctionTestProcess()
{
    uint8_t i = 0;
    
    if(!FunctionTestDataInfo.test_mode)
    {
        return;
    }
    switch((uint8_t)BcuTextState)
    {
        case TEST_ITEM_BCU_IDLE:
        {

            break;
        }
//        case TEST_ITEM_BCU_ISOSPI:
//        {
//            for(i = 0; i < 2; i++)
//            {
//                AfeSpiWriteReadBytes(i, ComTestData, ComTestRxData, 4);
//                uint8_t j = 0;
//                for(j = 0; j < 4; j++)
//                {
//                    if(ComTestRxData[j] != (ComTestData[j] + 1))    
//                    {
//                        break;
//                    }
//                }
//                if(j == 4)
//                {
//                    FunctionTestDataInfo.iso_spi |= (1<<i);
//                }
//            }
//            BcuTextState = TEST_ITEM_BCU_IDLE;
//        }
        case TEST_ITEM_BCU_EEPROM:
        {
            EepromWrite(TestDataWrite,EEPROM_TEST_ADDR,10);
            EepromRead(TestDataRead,EEPROM_TEST_ADDR,10);
            for(i = 0; i < 10; i++)
            {
                if(TestDataRead[i] != TestDataWrite[i])
                {
                    break;
                }
            }
            if(i == 10)
            {
                FunctionTestDataInfo.e2prom_test = true;
            }
            BcuTextState = TEST_ITEM_BCU_IDLE;
        }
        case TEST_ITEM_BCU_FLASH:
        {
            if(flash_test() == 0)
            {
                FunctionTestDataInfo.flash_test_flag = true;
            }
            BcuTextState = TEST_ITEM_BCU_IDLE;
        }
        default :   break;
    }
}

