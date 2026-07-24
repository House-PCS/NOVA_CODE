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
#include "can_main_card.h"

CanRevData_t CanRevData;


static CanProtoTxInfo_t _gCanProtoInfoCanMainCardTx[] = 
{
    {0x18000103, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, SendShutDownSignal, NULL},
};

CanProtoListTxInfo_t gCanProtoListInfoCanMainCardTx = 
{
    .p_proto_info = _gCanProtoInfoCanMainCardTx,
    .num = 1
};



static CanProtoRxInfo_t _gCanProtoInfoCanMainCardRx[] = 
{
    {0x18000301, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, RecvShutDownSignalAck, NULL},
};

CanProtoListRxInfo_t gCanProtoListInfoCanMainCardRx = 
{
    .p_proto_info = _gCanProtoInfoCanMainCardRx,
    .num = 1
};




/** 
* @brief    BCU 非通信CAN发送数据准备
* @param    shutdown ：1，开机，2：关机
* @return   NULL
* @note:
*/
void SendShutDownSignal(uint8_t shutdown)
{
    uint8_t buf[8];
    buf[0] = shutdown;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
    buf[7] = 0x00;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoCanMainCardTx[0], buf);
}


/** 
* @brief    BCU 非通信CAN接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void RecvShutDownSignalAck(uint8_t *p_buf)
{
    if(p_buf[0] == 1)
    {
        /* 关机 */
        CanRevData.bms_shut_down_signal = 1;
    }
    else if(p_buf[0] == 2)
    {
        /* 开机 */
        CanRevData.bms_shut_down_signal = 2;
    }
}











