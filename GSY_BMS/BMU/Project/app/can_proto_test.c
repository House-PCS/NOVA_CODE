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


#define TEST_ADDR    0x55
#define MAIN_ADDR    0xAA
typedef enum
{
    EN_TX_TEST = 0,
    EN_TX_TEST_SEND,
    
    EN_TX_TEST_TOTAL,
} EN_TX_CAN_TEST;
typedef enum
{
    EN_RX_TEST = 0,
    
    EN_RX_TEST_TOTAL,
} EN_RX_CAN_TEST;

static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoTestTx[] = 
{
    
    {0x18E7AA55, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0,    false, TestTxPrep,     NULL},
    {0x18E8AA55, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, false, TestTxSendPrep, NULL}, 

};

CanProtoListTxInfo_t gCanProtoListInfoTestTx = 
{
    .p_proto_info = _gCanProtoInfoTestTx,
    .num = EN_TX_TEST_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoTestRx[] = 
{
    {0x18E755AA, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, TestRxProcess, NULL}, 

};

CanProtoListRxInfo_t gCanProtoListInfoTestRx = 
{
    .p_proto_info = _gCanProtoInfoTestRx,
    .num = EN_RX_TEST_TOTAL
};

uint8_t RxData_Test[8];
////////////////RX///////////////
/** 
* @brief    CAN TEST RX接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void TestRxProcess(uint8_t *p_buf)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        RxData_Test[i] = p_buf[i];
    }   
}

////////////////TX///////////////

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void TestTxPrep(uint8_t addr)
{
    uint8_t buf[8];
  
    for(uint8_t i = 0; i < 8; i++)
    {
        buf[i] = RxData_Test[i];
    } 
    CanProtoTxIntoQueue(&_gCanProtoInfoTestTx[EN_TX_TEST], buf);
}

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void TestTxSendPrep(uint8_t addr)
{
    uint8_t buf[8];
    memset(buf, 0xE7, 8);
    CanProtoTxIntoQueue(&_gCanProtoInfoTestTx[EN_TX_TEST_SEND], buf);
}
static void tx_polling(uint8_t idx)
{
    
    switch(idx)
    {
        case EN_TX_TEST_SEND:
        {
            static uint32_t time_count = 0;

            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoTestTx[EN_TX_TEST_SEND].tx_regular_period)
            {
                TestTxSendPrep(EN_TX_TEST_SEND);
                time_count = tim_get_systime();
            }            
            break;
        }
        default:break;
    }
}
