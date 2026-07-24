/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_can_test.c
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
#include "can_proto_can.h"

uint8_t gSystemPowreStatus =0;
uint8_t gAckBmsShutDownStatus =0;
static CanProtoTxInfo_t _gCanProtoInfoCanTestTx[] = 
{
    {0x18000301, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, BcuCanTestEnterTestModeTxPrep, NULL},
};

CanProtoListTxInfo_t gCanProtoListInfoCanTestTx = 
{
    .p_proto_info = _gCanProtoInfoCanTestTx,
    .num = 1
};



static CanProtoRxInfo_t _gCanProtoInfoCanTestRx[] = 
{
    {0x18000103, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuCanBmsRxProcess, NULL},
};

CanProtoListRxInfo_t gCanProtoListInfoCanTestRx = 
{
    .p_proto_info = _gCanProtoInfoCanTestRx,
    .num = 1
};




/** 
* @brief    BCU 非通信CAN发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuCanTestEnterTestModeTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    buf[0] = gAckBmsShutDownStatus;//0x12;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoCanTestTx[0], buf);
}


/** 
* @brief    BCU 非通信CAN接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuCanBmsRxProcess(uint8_t *p_buf)
{
    gSystemPowreStatus = p_buf[0];
}
uint8_t GetSystemPowerStatus(void)
{
    return gSystemPowreStatus;
}

void    CleanPowerStatus(void)
{
    gSystemPowreStatus = 0;    
}


void AckBmsRomotShutDownCtrl(uint8_t status)
{
    gAckBmsShutDownStatus = status;
    BcuCanTestEnterTestModeTxPrep(0);
}






