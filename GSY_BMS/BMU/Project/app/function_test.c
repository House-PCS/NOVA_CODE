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
#include "mcu_bsp.h"
#include "dev_spiflash.h"
#include "dev_ioctl.h"



FunctionTestDataType FunctionTestDataInfo = {0};
static EN_TEST_ITEM BcuTextState = TEST_ITEM_BMU_IDLE;
bool _gIODirectInputState[2] = {false};

static uint8_t ComTestData[4] = {0x11, 0x22, 0x33, 0x44};
uint8_t ComTestRxData[4];

uint8_t arr_rdata[4096] = {0};
uint8_t arr_wdata[4096] = {0};

static void ReadTwoWayIO()
{
	_gIODirectInputState[0] = DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_1);
	_gIODirectInputState[1] = DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_2);
}

uint8_t flash_test(void)
{
    
    for(unsigned int i = 0; i < 4096; i ++)
    {
        arr_wdata[i] = i % 256;
    }
    SpiFlashWriteBytesWithErase(0, arr_wdata, 4096);  
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

void BmuTextItemSet(EN_TEST_ITEM item)
{
    BcuTextState = item;
}
void BmuTextModeSet()
{
    FunctionTestDataInfo.test_mode = true;
}
void BmuTextModeDown()
{
    FunctionTestDataInfo.test_mode = false;
}
uint8_t TestDataWrite[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
uint8_t TestDataRead[10];
#define EEPROM_TEST_ADDR    100
void FunctionTestProcess()
{
	ReadTwoWayIO();
    uint8_t i = 0;
    
    if(!FunctionTestDataInfo.test_mode)
    {
        return;
    }
    switch((uint8_t)BcuTextState)
    {
        case TEST_ITEM_BMU_IDLE:
        {

            break;
        }
        case TEST_ITEM_BMU_FLASH:
        {
            if(flash_test() == 0)
            {
                FunctionTestDataInfo.flash_test_flag = true;
            }
            BcuTextState = TEST_ITEM_BMU_IDLE;
			break;
        }
//		case TEST_ITEM_BMU_TWOWAYIO_GET:
//        {
//			_gIODirectInputState[0] = DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_1);
//			_gIODirectInputState[1] = DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_2);
//            BcuTextState = TEST_ITEM_BMU_IDLE;
//			break;
//        }
		case TEST_ITEM_BMU_TWOWAYIO_SET:
        {
			DevIOCtlOutSet(EN_DEV_IOCTL_IDX_JACK_OUT_1);
			DevIOCtlOutSet(EN_DEV_IOCTL_IDX_JACK_OUT_2);
            BcuTextState = TEST_ITEM_BMU_IDLE;
			break;
        }
		case TEST_ITEM_BMU_TWOWAYIO_RESET:
        {
			DevIOCtlOutReset(EN_DEV_IOCTL_IDX_JACK_OUT_1);
			DevIOCtlOutReset(EN_DEV_IOCTL_IDX_JACK_OUT_2);
            BcuTextState = TEST_ITEM_BMU_IDLE;
			break;
        }
        default : break;
    }
}

