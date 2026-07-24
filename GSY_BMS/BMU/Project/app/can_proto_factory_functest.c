/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_factory_test.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-6-20      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "can_tx_rx_process.h"
#include "mcu_bsp.h"
#include "function_test.h"
#include "afe_ctl.h"
#include "dev_pwm_detect_cfg.h"
#include "flash.h"
#include "bmu_data_manage.h"

//uint8_t TxData_FuncTest[8];
//uint8_t RxData_FuncTest[8];

BmuTestInfo_t RxDataBuffInfo;

typedef enum
{
    EN_FTY_BMU_TEST_MODE = 0,
    EN_FTY_BMU_VOLTAGE,
    EN_FTY_BMU_TEMP,
    EN_FTY_BMU_BALANCE_TEMP,
    EN_FTY_BMU_TWO_WAY_IO,
//    EN_FTY_BMU_DO,
    EN_FTY_BMU_PWM,
//    EN_FTY_BMU_DI,
    EN_FTY_BMU_FLASH,
	EN_FTY_BMU_VERSION,
    EN_FTY_BMU_TEST_END,
    
    EN_FTY_CAN_TOTAL,
} EN_FTY_CAN_BMU;

static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoFactoryFuncTestTx[] = 
{
    {0x1400CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestEnterModeTxPrep,     NULL}, 
    {0x1430CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200, false,  BmuFactoryTestVoltageTxPrep,       tx_polling}, 
    {0x1431CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200, false,  BmuFactoryTestTempTxPrep,          tx_polling}, 
    {0x1432CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 200, false,  BmuFactoryTestBalanceTempTxPrep,   tx_polling}, 
    {0x1433CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestTwoWayIOTxPrep,      NULL}, 
//    {0x1434CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestDoTxPrep,            NULL}, 
    {0x1435CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestPwmTxPrep,        	NULL}, 
//    {0x1436CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  FuncTestTx_DiPrep,            	  NULL}, 
    {0x1438CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestFlashTxPrep,         NULL}, 
	{0x1440CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestVersionTxPrep,       NULL}, 
    {0x1480CC9F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   false,  BmuFactoryTestEndModeTxPrep,  		NULL}, 
};

CanProtoListTxInfo_t gCanProtoListInfoFactoryFuncTestTx = 
{
    .p_proto_info = _gCanProtoInfoFactoryFuncTestTx,
    .num = EN_FTY_CAN_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoFactoryFuncTestRx[] = 
{
    {0x14009FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestEnterModeRxPrep,        NULL}, 
    {0x14309FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestVoltageRxPrep,          NULL}, 
    {0x14319FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestTempRxPrep,             NULL}, 
    {0x14329FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestBalanceTempRxPrep,      NULL}, 
    {0x14339FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestTwoWayIORxPrep,         NULL}, 
//    {0x14349FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestDoRxPrep,               NULL}, 
    {0x14359FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestPwmRxPrep,           	  NULL}, 
//    {0x14369FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, FuncTestRx_DiProcess,                 NULL}, 
    {0x14389FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestFlashRxPrep,            NULL}, 
	{0x14409FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestVersionRxPrep,          NULL}, 
    {0x14809FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmuFactoryTestEndModeRxPrep,      	  NULL}, 

};

CanProtoListRxInfo_t gCanProtoListInfoFactoryFuncTestRx = 
{
    .p_proto_info = _gCanProtoInfoFactoryFuncTestRx,
    .num = EN_FTY_CAN_TOTAL
};


////////////////RX///////////////
/** 
* @brief    BMU 测试模式 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestEnterModeRxPrep(uint8_t *p_buf)
{
	_AddrDistriSucessFlag = true;
	BmuTextItemSet(TEST_ITEM_BMU_FLASH); //提前读写flash，为falsh测试做好准备
	if(p_buf[0] == 0x01)
	{
		BmuTextModeSet();
		DevPwmOutDutySet(50);  //开启PWM
		for(uint8_t i = 0; i < 8; i++)
		{
			RxDataBuffInfo.test_mode_data_buff[i] = p_buf[i];
		}  
		BmuFactoryTestEnterModeTxPrep(EN_FTY_BMU_TEST_MODE);
	}
}

/** 
* @brief    BMU 单体电压 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestVoltageRxPrep(uint8_t *p_buf)
{
	if(p_buf[0] == 0x01)
	{
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE].regular_default_enable = true;
	}
}
/** 
* @brief    BMU 单体温度 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestTempRxPrep(uint8_t *p_buf)
{
	if(p_buf[0] == 0x01)
	{
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].regular_default_enable = true;
	}
}
/** 
* @brief    BMU 均衡温度 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestBalanceTempRxPrep(uint8_t *p_buf)
{
	if(p_buf[0] == 0x01)
	{
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_BALANCE_TEMP].regular_default_enable = true;
	}
}
/** 
* @brief    BMU 双向IO 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestTwoWayIORxPrep(uint8_t *p_buf)
{
	
	if(p_buf[0] == 0x01)
	{
		//先配置输出模式再拉高管脚并回包
		BmuTextItemSet(TEST_ITEM_BMU_TWOWAYIO_SET);
	}
	else if(p_buf[0] == 0x02)
	{
		//拉低管脚并回包
		BmuTextItemSet(TEST_ITEM_BMU_TWOWAYIO_RESET);
	}
	else if(p_buf[0] == 0x03)
	{
		//再将管脚配置为输入模式并回包
//		BmuTextItemSet(TEST_ITEM_BMU_TWOWAYIO_GET);
	}
	else if(p_buf[0] == 0x04)
	{
		//回读输入模式下的管脚状态并回包
	}
	RxDataBuffInfo.two_way_io_data_buff[0] = p_buf[0];
	BmuFactoryTestTwoWayIOTxPrep(EN_FTY_BMU_TWO_WAY_IO);
}
///** 
//* @brief    BMU IO输出 接收数据处理
//* @param    p_buf:接收到的协议数据内容(8byte)
//* @return   NULL
//* @note:
//*/
//void BmuFactoryTestDoRxPrep(uint8_t *p_buf)
//{
//	if(p_buf[0] == 0x01 || p_buf[0] == 0x02)
//	{
//		for(uint8_t i = 0; i < 8; i++)
//		{
//			RxDataBuffInfo.io_output_data_buff[i] = p_buf[i];
//		} 
//	}
//	BmuFactoryTestDoTxPrep(EN_FTY_BMU_DO);
//}

/** 
* @brief    BMU PWM输出 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestPwmRxPrep(uint8_t *p_buf)
{
	uint32_t freq;
	uint8_t duty;
	bool pwm_test_flag = false;
	if(p_buf[0] == 0x01)
	{
		pwm_test_flag = true;
	}
	else if(p_buf[0] == 0x02)	
	{
		DevPwmInGet(&freq, &duty);
		if(freq >= 24000 && freq <= 26000 && duty >= 48 && duty <= 52)
		{
			pwm_test_flag = true;
		}
	}
	if(pwm_test_flag == true)
	{
		for(uint8_t i = 0; i < 8; i++)
		{
			RxDataBuffInfo.pwm_out_data_buff[i] = p_buf[i];
		} 
	}
	BmuFactoryTestPwmTxPrep(EN_FTY_BMU_PWM);
}
///** 
//* @brief    BMU IO输入 接收数据处理
//* @param    p_buf:接收到的协议数据内容(8byte)
//* @return   NULL
//* @note:
//*/
//void FuncTestRx_DiProcess(uint8_t *p_buf)
//{
//    for(uint8_t i = 0; i < 8; i++)
//    {
//        RxDataBuffInfo.io_input_data_buff[i] = p_buf[i];
//    } 
//}
/** 
* @brief    BMU flash 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestFlashRxPrep(uint8_t *p_buf)
{
	if(p_buf[0] == 0x01)
	{
		BmuFactoryTestFlashTxPrep(EN_FTY_BMU_FLASH);
	}	
}

/** 
* @brief    BMU Version 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestVersionRxPrep(uint8_t *p_buf)
{
	if(p_buf[0] == 0x01)
    {
        BmuFactoryTestVersionTxPrep(EN_FTY_BMU_VERSION);
    }
	
}
/** 
* @brief    BMU test_end 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmuFactoryTestEndModeRxPrep(uint8_t *p_buf)
{
	if(p_buf[0] == 0x01)
	{
		BmuTextModeDown();
		for(uint8_t i = 0; i < 8; i++)
		{
			RxDataBuffInfo.test_end_data_buff[i] = p_buf[i];
		}
	}
	BmuFactoryTestEndModeTxPrep(EN_FTY_BMU_TEST_END);
}
////////////////TX///////////////

/** 
* @brief    BMU 开始测试 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestEnterModeTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = 1;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
	
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEST_MODE], buf);
}
/** 
* @brief    BMU 单体电压 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestVoltageTxPrep(uint8_t idx)
{    
    static uint8_t cnt = 0;
    uint8_t buf[8];
    
    _gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE].tx_regular_period = 6;
    buf[0] = (CELL_VOLT_SAMPLE_MAX + 2) / 3;
    buf[1] = cnt / 3;
//	//测试假数据
//    uint16_t virtual_voltage = 0xBB8;  
    for(uint8_t i = 0; i < 3; i++)
    {
        if((cnt + i) < CELL_VOLT_SAMPLE_MAX)
        {
            uint8_t temp1 = (cnt + i) / 16;
            uint8_t temp2 = (cnt + i) % 16;
            buf[i * 2 + 2] = gAFESampleInfo.cell_voltage[temp1][temp2] >> 8;
            buf[i * 2 + 3] = gAFESampleInfo.cell_voltage[temp1][temp2];
			
//			buf[i * 2 + 2] = virtual_voltage;
//            buf[i * 2 + 3] = virtual_voltage >> 8;
        }
    }  
	if(cnt < CELL_VOLT_SAMPLE_MAX)
	{
		CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE], buf);
		cnt += 3;
	} 
	else
	{
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE].regular_default_enable = false;
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE].tx_regular_period = 200;
		cnt = 0;
	}	
}

/** 
* @brief    BMU 单体温度 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestTempTxPrep(uint8_t idx)
{
    static uint8_t cnt = 0;
    uint8_t buf[8];
    
    _gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].tx_regular_period = 3;
    buf[0] = (PACK_TEMP_SAMPLE_MAX + 5) / 6;
    buf[1] = cnt / 6;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if((cnt + i) < PACK_TEMP_SAMPLE_MAX)
        {
            uint8_t temp1 = (cnt + i) / 16;
            uint8_t temp2 = (cnt + i) % 16;
            
            buf[i + 2] = gAFESampleInfo.cell_voltage[temp1][temp2];
//			//测试假数据
//			uint8_t virtual_temp = 0x19;
//			buf[i + 2] = virtual_temp;
        }
    } 
	if(cnt < PACK_TEMP_SAMPLE_MAX)
	{
		CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP], buf);
		cnt += 6;
	} 
	else
	{
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].regular_default_enable = false;
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].tx_regular_period = 200;
		cnt = 0;
	}
}

/** 
* @brief    BMU BalanceTemp 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestBalanceTempTxPrep(uint8_t idx)
{
    static uint8_t cnt = 0;
    uint8_t buf[8];
    
    _gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_BALANCE_TEMP].tx_regular_period = 3;
    buf[0] = (BALANCE_TEMP_SAMPLE_MAX + 5) / 6;
    buf[1] = cnt / 6;
    
    for(uint8_t i = 0; i < 6; i++)
    {
        if((cnt + i) < BALANCE_TEMP_SAMPLE_MAX)
        {
            uint8_t temp1 = (cnt + i) / 16;
            uint8_t temp2 = (cnt + i) % 16;
//			uint8_t virtual_temp = 0x19;
            buf[i + 2] = gAFESampleInfo.cell_voltage[temp1][temp2];
//			buf[i + 2] = virtual_temp;
        }
    } 
	if(cnt < BALANCE_TEMP_SAMPLE_MAX)
	{
		CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_BALANCE_TEMP], buf);
		cnt += 6;
	} 
	else
	{
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].regular_default_enable = false;
		_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_BALANCE_TEMP].tx_regular_period = 200;
		cnt = 0;
	}
}

/** 
* @brief    BMU 双向IO 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestTwoWayIOTxPrep(uint8_t idx)
{
    uint8_t temp_u8_input = 0;
    uint8_t temp_u8_output = 0;
    uint8_t buf[8];
    
    if(RxDataBuffInfo.two_way_io_data_buff[0] <= 3)
    {
        buf[0] = 1;
        buf[1] = 0;
        buf[2] = RxDataBuffInfo.two_way_io_data_buff[0];
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    }
    else
    {
        temp_u8_input = (_gIODirectInputState[0]) | (_gIODirectInputState[1] << 1);
        buf[0] = 1;
        buf[1] = 0;
        buf[2] = RxDataBuffInfo.two_way_io_data_buff[0];
        buf[3] = temp_u8_input;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    }      
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TWO_WAY_IO], buf);
}

/** 
* @brief    BMU IO输出 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
//void BmuFactoryTestDoTxPrep(uint8_t idx)
//{
//    uint8_t buf[8];
//    
//    buf[0] = RxDataBuffInfo.io_output_data_buff[0];
//    buf[1] = 0;
//    buf[2] = 0;
//    buf[3] = 0;
//    buf[4] = 0;
//    buf[5] = 0;
//    buf[6] = 0;
//    buf[7] = 0;
//    
//    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_DO], buf);
//}

/** 
* @brief    BMU PWM输出 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestPwmTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = RxDataBuffInfo.pwm_out_data_buff[0];
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_PWM], buf);
}

///** 
//* @brief    BMU IO输入 发送数据准备
//* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
//* @return   NULL
//* @note:
//*/
//void FuncTestTx_DiPrep(uint8_t idx)
//{
//    uint8_t buf[8];
//    uint8_t temp_u8 = 0;
//    
//    temp_u8 = GpioBitGet((&DI_Handle[FAN_RD1]));

//    buf[0] = 1;
//    buf[1] = 0;
//    buf[2] = temp_u8;
//    buf[3] = 0;
//    buf[4] = 0;
//    buf[5] = 0;
//    buf[6] = 0;
//    buf[7] = 0;
//    
//    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_TX_BMU_DI], buf);
//}
/** 
* @brief    BMU flash 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestFlashTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    if(FunctionTestDataInfo.flash_test_flag)
    {
        buf[0] = 1;
    }
    else
    {
        buf[0] = 2;
    }
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;
	buf[4] = 0;
	buf[5] = 0;
	buf[6] = 0;
	buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_FLASH], buf);
}
/** 
* @brief    BMU Version 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestVersionTxPrep(uint8_t idx)
{
    uint8_t buf[8];
	
	InternalFlashRead(0x10000 - 256, buf, 3);
	buf[3] = 0; 
	buf[4] = 0x1;	//该字节的0x00~0x1A分别代表26个字母
    buf[5] = 0;	
	buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VERSION], buf);
}
/** 
* @brief    BMU 测试模式关闭 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmuFactoryTestEndModeTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = 1;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEST_END], buf);
}



static void tx_polling(uint8_t idx)
{
    switch(idx)
    {
        case EN_FTY_BMU_VOLTAGE:
        {
            static uint32_t time_count = 0;
            if(_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_VOLTAGE].tx_regular_period)
                {
                    BmuFactoryTestVoltageTxPrep(EN_FTY_BMU_VOLTAGE);
                    time_count = tim_get_systime();
                }            
            }
            break;
        }
        case EN_FTY_BMU_TEMP:
        {
            static uint32_t time_count = 0;
            if(_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_TEMP].tx_regular_period)
                {
                    BmuFactoryTestTempTxPrep(EN_FTY_BMU_TEMP);
                    time_count = tim_get_systime();
                }            
            }
            break;
        }
        case EN_FTY_BMU_BALANCE_TEMP:
        {
            static uint32_t time_count = 0;
            if(_gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_BALANCE_TEMP].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoFactoryFuncTestTx[EN_FTY_BMU_BALANCE_TEMP].tx_regular_period)
                {
                    BmuFactoryTestBalanceTempTxPrep(EN_FTY_BMU_BALANCE_TEMP);
                    time_count = tim_get_systime();
                }            
            }
            break;
        }
        default:break;
    }
}
