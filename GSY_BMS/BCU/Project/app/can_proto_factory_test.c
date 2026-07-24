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
#include "function_test.h"
#include "dev_ioctl.h"
#include "dev_volt_detect.h"
#include "ins_res_detect.h"
#include "current_detect.h"
#include "temp_detect.h"
#include "data_storage_manage.h"

#define INTERNAL_FLASH_FW_INFO_ADDR_1      (0x10000 - 256)

static uint8_t temp_buff[8];

typedef enum
{
    EN_FTY_BCU_ENTER,
    EN_FTY_BCU_EXIT,
    EN_FTY_BCU_HVOLT,
    EN_FTY_BCU_INS,
//    EN_FTY_BCU_CURDIV,
    EN_FTY_BCU_HALL,
    EN_FTY_BCU_TEMP,
    EN_FTY_BCU_IOINPUT,
    EN_FTY_BCU_IOOUTPUT,
    EN_FTY_BCU_EEPROM,
    EN_FTY_BCU_FLASH,
    EN_FTY_BCU_RTC,
	EN_FTY_BCU_VERSION,
	EN_FTY_BCU_CAN,
	
    EN_FTY_NUM,
}EN_CAN_PROTO_FTY_TEST_IDX;

//TX
void BcuFactoryTestEnterTestModeTxPrep(uint8_t idx);
void BcuFactoryTestExitTestModeTxPrep(uint8_t idx);
void BcuFactoryTestHighVoltageTxPrep(uint8_t idx);
void BcuFactoryTestInsTestTxPrep(uint8_t idx);
void BcuFactoryTestHallTxPrep(uint8_t idx);
void BcuFactoryTestTempTxPrep(uint8_t idx);
void BcuFactoryTestIOInputTxPrep(uint8_t idx);
void BcuFactoryTestIOOutputTxPrep(uint8_t idx);
void BcuFactoryTestEepromTxPrep(uint8_t idx);
void BcuFactoryTestFlashTxPrep(uint8_t idx);
void BcuFactoryTestRtcTxPrep(uint8_t idx);
void BcuFactoryTestCanTestTxPrep(uint8_t idx);
void BcuFactoryVersionReportTxPrep(uint8_t idx);
//RX
void BcuFactoryTestEnterTestModeRxProcess(uint8_t *p_buf);
void BcuFactoryTestExitTestModeRxProcess(uint8_t *p_buf);
void BcuFactoryTestHighVoltageRxProcess(uint8_t *p_buf);
void BcuFactoryTestInsTestRxProcess(uint8_t *p_buf);
void BcuFactoryTestHallRxProcess(uint8_t *p_buf);
void BcuFactoryTestTempRxProcess(uint8_t *p_buf);
void BcuFactoryTestIOInputRxProcess(uint8_t *p_buf);
void BcuFactoryTestIOOutputRxProcess(uint8_t *p_buf);
void BcuFactoryTestEepromRxProcess(uint8_t *p_buf);
void BcuFactoryTestFlashRxProcess(uint8_t *p_buf);
void BcuFactoryTestRtcRxProcess(uint8_t *p_buf);
void BcuFactoryTestCanTestRxProcess(uint8_t *p_buf);
void BcuFactoryVersionReportRxProcess(uint8_t *p_buf);

static CanProtoTxInfo_t _gCanProtoInfoFactoryTestTx[] = 
{
    {0x1400CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestEnterTestModeTxPrep,   NULL}, //BCU EnterTestMode
    {0x1480CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestExitTestModeTxPrep,    NULL}, //BCU ExitTestMode
    {0x1405CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestHighVoltageTxPrep,     NULL}, //BCU HighVoltageTest
    {0x1406CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestInsTestTxPrep,         NULL}, //BCU InsTest
    {0x1408CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestHallTxPrep,            NULL}, //BCU HallTest
    {0x1409CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestTempTxPrep,            NULL}, //BCU TempTest
    {0x140ACC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestIOInputTxPrep,         NULL}, //BCU IOInputTest
    {0x140BCC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestIOOutputTxPrep,        NULL}, //BCU IOOutputTest
    {0x140ECC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestEepromTxPrep,          NULL}, //BCU EepromTest
    {0x140FCC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestFlashTxPrep,           NULL}, //BCU FlashTest
    {0x1410CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestRtcTxPrep,             NULL}, //BCU RtcTest
	{0x1415CC7F, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, 	 true,  BcuFactoryVersionReportTxPrep,  	 NULL}, //BCU VersionReport
	{0x190155AA, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,   true,  BcuFactoryTestCanTestTxPrep,   	  	 NULL}, //BCU CAN_Test	
};


CanProtoListTxInfo_t gCanProtoListInfoFactoryTestTx = 
{
    .p_proto_info = _gCanProtoInfoFactoryTestTx,
    .num = EN_FTY_NUM
};

  
static CanProtoRxInfo_t _gCanProtoInfoFactoryTestRx[] = 
{
    {0x14007FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestEnterTestModeRxProcess,   NULL}, //BCU EnterTestMode
    {0x14807FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestExitTestModeRxProcess,    NULL}, //BCU ExitTestMode
    {0x14057FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestHighVoltageRxProcess,     NULL}, //BCU HighVoltageTest
    {0x14067FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestInsTestRxProcess,         NULL}, //BCU InsTest
    {0x14087FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestHallRxProcess,            NULL}, //BCU HallTest
    {0x14097FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestTempRxProcess,            NULL}, //BCU TempTest
    {0x140A7FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestIOInputRxProcess,         NULL}, //BCU IOInputTest
    {0x140B7FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestIOOutputRxProcess,        NULL}, //BCU IOOutputTest
    {0x140E7FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestEepromRxProcess,          NULL}, //BCU EepromTest
    {0x140F7FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestFlashRxProcess,           NULL}, //BCU FlashTest
    {0x14107FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestRtcRxProcess,             NULL}, //BCU RtcTest
	{0x14157FCC, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryVersionReportRxProcess,       NULL}, //BCU Version
	{0x1901AA55, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuFactoryTestCanTestRxProcess,         NULL}, //BCU CAN_Test
};

CanProtoListRxInfo_t gCanProtoListInfoFactoryTestRx = 
{
    .p_proto_info = _gCanProtoInfoFactoryTestRx,
    .num = EN_FTY_NUM
};



/** 
* @brief    BCU EnterTestMode发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestEnterTestModeTxPrep(uint8_t idx)
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
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_ENTER], buf);
}


/** 
* @brief    BCU EnterTestMode接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestEnterTestModeRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01)
    {
        BcuTextModeSet();
        test_tool_flag = true;
        BcuFactoryTestEnterTestModeTxPrep(EN_FTY_BCU_ENTER);
    }
}



/** 
* @brief    BCU ExitTestMode发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestExitTestModeTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = 0x01;
    memset(&buf[1], 0x00, 7);
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_EXIT], buf);
}


/** 
* @brief    BCU ExitTestMode接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestExitTestModeRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01)
    {
        //退出测试(待添加)
//        test_tool_flag = false;
        
        BcuFactoryTestExitTestModeTxPrep(EN_FTY_BCU_EXIT);
    }
}



/** 
* @brief    BCU 高压检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestHighVoltageTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = 1;
    buf[1] = 0;
//    buf[2] = gHighVoltInfo.v_bp;
//    buf[3] = gHighVoltInfo.v_bp >> 8;
//    buf[4] = gHighVoltInfo.v_pp;
//    buf[5] = gHighVoltInfo.v_pp >> 8;
	buf[2] = 0xD6; 
	buf[3] = 0x01; 
	buf[4] = 0xD6; 
	buf[5] = 0x01; 
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_HVOLT], buf);
}


/** 
* @brief    BCU 高压检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestHighVoltageRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01 && p_buf[1] == 0x00)//总帧数为1，序号为0
    {
        BcuFactoryTestHighVoltageTxPrep(EN_FTY_BCU_HVOLT);
    }
}



/** 
* @brief    BCU 绝缘阻抗检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestInsTestTxPrep(uint8_t idx)
{
    /*先只做到第一步检测*/
    uint8_t buf[8];
    buf[0] = 1;
    buf[1] = 0;
//    buf[2] = gInsResInfo.bn_res;
//    buf[3] = gInsResInfo.bn_res >> 8;
//    buf[4] = gInsResInfo.bp_res;
//    buf[5] = gInsResInfo.bp_res >> 8;
	buf[2] = 0x64;
    buf[3] = 0x00;
    buf[4] = 0x64;
    buf[5] = 0x00;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_INS], buf);
}


/** 
* @brief    BCU 绝缘阻抗检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestInsTestRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01 && p_buf[1] == 0x00)//总帧数为1，序号为0
    {
        BcuFactoryTestInsTestTxPrep(EN_FTY_BCU_INS);
    }
}

/** 
* @brief    BCU 霍尔采样检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestHallTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t temp_valtage1 = 0, temp_valtage2 = 0;
    
    buf[0] = 1;
    buf[1] = 0;
//    temp_valtage1 = gCurrentInfo.filter_big_range_current / 1000;
//    buf[2] = temp_valtage1;
//    buf[3] = temp_valtage1 >> 8;
//    temp_valtage2 = gCurrentInfo.filter_small_range_current / 1000;
//    buf[4] = temp_valtage2;
//    buf[5] = temp_valtage2 >> 8;
	buf[2] = 0xC4;
    buf[3] = 0x09;
    buf[4] = 0xC4;
    buf[5] = 0x09;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_HALL], buf);
}


/** 
* @brief    BCU 霍尔检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestHallRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01 && p_buf[1] == 0x00)//总帧数为1，序号为0
    {
        BcuFactoryTestHallTxPrep(EN_FTY_BCU_HALL);
    }
}


/** 
* @brief    BCU 温度采样检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestTempTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = 1;
    buf[1] = 0;
//    buf[2] = gTempInfo.temp_relay_p;
//    buf[3] = gTempInfo.temp_relay_n;
//    buf[4] = gTempInfo.temp_relay_pre;
//    buf[5] = gTempInfo.temp_hvbox;
	buf[2] = 0x19;
    buf[3] = 0x19;
    buf[4] = 0x19;
    buf[5] = 0x19;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_TEMP], buf);
}


/** 
* @brief    BCU 霍尔检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestTempRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01 && p_buf[1] == 0x00)//总帧数为1，序号为0
    {
        BcuFactoryTestTempTxPrep(EN_FTY_BCU_TEMP);
    }
}


/** 
* @brief    BCU IO输入检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestIOInputTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t temp_u8 = 0;
    for(uint8_t i = EN_DEV_IOCTL_IDX_ACT_LOW_IN_1; i <= EN_DEV_IOCTL_IDX_ISO_IN_1; i++)
    {
        if(DevIOCtlInGet(i))
        {
            temp_u8 |= (1 << (i - EN_DEV_IOCTL_IDX_ACT_LOW_IN_1));
        }
        else
        {
            temp_u8 |= (0 << (i - EN_DEV_IOCTL_IDX_ACT_LOW_IN_1));
        }
    }
    
    buf[0] = 1;
    buf[1] = 0;
    buf[2] = temp_u8;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_IOINPUT], buf);
}


/** 
* @brief    BCU IO输入检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestIOInputRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01 && p_buf[1] == 0x00)//总帧数为1，序号为0
    {
        BcuFactoryTestIOInputTxPrep(EN_FTY_BCU_IOINPUT);
    }
}

/** 
* @brief    BCU IO输出检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestIOOutputRxProcess(uint8_t *p_buf)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        temp_buff[i] = p_buf[i];
    }
    if(temp_buff[0] == 1)
    {
        for(EN_DEV_IOCTL_IDX_OUT i = EN_DEV_IOCTL_IDX_DRY_OUT_1; i <= EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_4; i++)
        {
            DevIOCtlOutSet(i);
        }
    }
    else if(temp_buff[0] == 2)
    {
        for(EN_DEV_IOCTL_IDX_OUT i = EN_DEV_IOCTL_IDX_DRY_OUT_1; i <= EN_DEV_IOCTL_IDX_HIGH_SIDE_OUT_4; i++)
        {
            DevIOCtlOutReset(i);;
        }
    }
    if(p_buf[0] == 0x01 || p_buf[0] == 0x02)
    {
        BcuFactoryTestIOOutputTxPrep(EN_FTY_BCU_IOOUTPUT);
    }
}

/** 
* @brief    BCU IO输出检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestIOOutputTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = temp_buff[0];
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_IOOUTPUT], buf);
}

/** 
* @brief    BCU EEPROM检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestEepromTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = 1;
    buf[1] = 0;
//    if(FunctionTestDataInfo.e2prom_test)
//    {
//        buf[2] = 1;
//    }   
//    else
//    {
//        buf[2] = 2; 
//    }
    //测试假数据
    buf[2] = 1;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_EEPROM], buf);
}


/** 
* @brief    BCU EEPROM检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestEepromRxProcess(uint8_t *p_buf)
{
	BcuTextItemSet(TEST_ITEM_BCU_EEPROM);
    if(p_buf[0] == 0x01 || p_buf[0] == 0x02)
    {
        BcuFactoryTestEepromTxPrep(EN_FTY_BCU_EEPROM);
    }
}


/** 
* @brief    BCU FLASH检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestFlashTxPrep(uint8_t idx)
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
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_FLASH], buf);
}


/** 
* @brief    BCU FLASH检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestFlashRxProcess(uint8_t *p_buf)
{
	BcuTextItemSet(TEST_ITEM_BCU_FLASH);
    if(p_buf[0] == 0x01)
    {
        BcuFactoryTestFlashTxPrep(EN_FTY_BCU_FLASH);
    }
}



/** 
* @brief    BCU RTC检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestRtcTxPrep(uint8_t idx)
{
    uint8_t buf[8];

    buf[0] = gDevRtc.year;
    buf[1] = gDevRtc.year >> 8;
    buf[2] = gDevRtc.month;
    buf[3] = gDevRtc.day;
    buf[4] = gDevRtc.hour;
    buf[5] = gDevRtc.minute;
    buf[6] = gDevRtc.second;
    if(gDevRtc.year >= 0x7E8)
    {
        buf[7] = 1;
    }
    else
    {
        buf[7] = 2;
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_RTC], buf);
	
	test_tool_flag = false;
}


/** 
* @brief    BCU RTC检测接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestRtcRxProcess(uint8_t *p_buf)
{
    /*当前只判断状态结果，不记录时间*/
    BcuFactoryTestRtcTxPrep(EN_FTY_BCU_RTC);
}

/** 
* @brief    BCU CanTest发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryTestCanTestTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = 0x12;
    buf[1] = 0x23;
    buf[2] = 0x34;
    buf[3] = 0x45;
    buf[4] = 0x56;
    buf[5] = 0x67;
    buf[6] = 0x78;
    buf[7] = 0x89;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_CAN], buf);
}


/** 
* @brief    BCU CanTest接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryTestCanTestRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x11 && p_buf[1] == 0x22 && p_buf[2] == 0x33 && p_buf[3] == 0x44 &&
       p_buf[4] == 0x55 && p_buf[5] == 0x66 && p_buf[6] == 0x77 && p_buf[7] == 0x88)
    {
        BcuFactoryTestCanTestTxPrep(EN_FTY_BCU_CAN);
    }
}

/** 
* @brief    BCU Version读取命令接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuFactoryVersionReportRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x01)
    {
        BcuFactoryVersionReportTxPrep(EN_FTY_BCU_VERSION);
    }
}

/** 
* @brief    BCU 高压检测发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuFactoryVersionReportTxPrep(uint8_t idx)
{
    uint8_t buf[8];
	
	buf[0] = buf_Version[0];
	buf[1] = buf_Version[1];
	buf[2] = buf_Version[2];
	buf[3] = 0; 
	buf[4] = 0x1;	//该字节的0x01~0x1A分别代表26个字母
    buf[5] = 0;	
	buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoFactoryTestTx[EN_FTY_BCU_VERSION], buf);
}

