/*
 * can_proto_parallel_slave.c
 *
 *  Created on: 2025年12月22日
 *      Author: Nova008226
 */

/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_parallel_slave.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-12-17
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


#include "can_comm_process.h"
#include "dev_can_comm.h"
#include "cpu_timers.h"

uint16_t CanaRXMsgData1[8];
extern uint16_t gLowSyncInInitStatus;
extern uint16_t gHighSyncInInitStatus;





typedef enum
{
    EN_TX_PARAL_TEST_ACK_1 = 0,
    EN_TX_PARAL_TEST_ACK_2,

    EN_TX_PARAL_TEST_TOTAL,
}EN_TX_CAN_PARAL_ACK_TEST;

typedef enum
{
    EN_RX_PARAL_TEST_1 = 0,
    EN_RX_PARAL_TEST_2,
    EN_RX_PARAL_TEST_TOTAL,
}EN_TX_CAN_PARAL_TEST;


static CanProtoTxInfo_t _gCanProtoInfoParallelTestTx[EN_TX_PARAL_TEST_TOTAL] =
{
    {0x18AF0180, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, true, ParallelTestAck1Prep,  NULL},     //SYNC_IN CAN0
    {0x18AF8001, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, true, ParallelTestAck2Prep,  NULL},     //SYNC_OUT CAN1
};


CanProtoListTxInfo_t gCanProtoListInfoParallelTestTx =
{
    .p_proto_info = _gCanProtoInfoParallelTestTx,
    .num = EN_TX_PARAL_TEST_TOTAL,
    .polling = NULL
};


static CanProtoRxInfo_t _gCanProtoInfoParallelTestRx[EN_RX_PARAL_TEST_TOTAL] =
{
    {0x18AF8001, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelTestAck1RxProcess,     NULL},//EN_DEV_CAN_ANY
    {0x18AF0180, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelTestAck2RxProcess,     NULL},
};

CanProtoListRxInfo_t gCanProtoListInfoParallelTestRx =
{
    .p_proto_info = _gCanProtoInfoParallelTestRx,
    .num = EN_RX_PARAL_TEST_TOTAL,
    .polling = NULL//_CanSlaveRxPollingDetect
};



#define CAN_SLAVE_TIMEOUT               10000

static uint32_t _gCanSlaveRxDetectedTimeCount = 0;


////////////////TX///////////////
/**
* @brief    并机从机响应信息1发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelTestAck1Prep(uint16_t idx)
{
    uint8_t buf[8];

    /*将业务数据赋给CAN BUF*/
    buf[0]= 0x55;
    buf[1]= 0xAA;
    buf[2]= gLowSyncInInitStatus;
    buf[3]= gLowSyncInInitStatus;
    buf[4]= 0x55;
    buf[5]= 0xAA;
    buf[6]= GPIO_ReadPin(63);
    buf[7]= GPIO_ReadPin(62);;

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelTestTx[EN_TX_PARAL_TEST_ACK_1], buf);
}



/**
* @brief    并机从机响应信息2发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelTestAck2Prep(uint16_t idx)
{
    uint8_t buf[8];

    buf[0]= 0xAA;
    buf[1]= 0x55;
    buf[2]= 0xAA;
    buf[3]= 0x55;
    buf[4]= 0xAA;
    buf[5]= 0x55;
    buf[6]= 0xAA;
    buf[7]= 0x55;



    CanProtoTxIntoQueue(&_gCanProtoInfoParallelTestTx[EN_TX_PARAL_TEST_ACK_2], buf);
}



////////////////RX///////////////
/**
* @brief    并机从机同步信息1接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelTestAck1RxProcess(uint16_t *p_buf)
{
    if((p_buf[0] == 0x55) && (p_buf[1] == 0xAA) &&\
    (p_buf[2] == 0x55) && (p_buf[3] == 0xAA) &&\
    (p_buf[4] == 0x55) && (p_buf[5] == 0xAA) &&\
    (p_buf[6] == 0x55) && (p_buf[7] == 0xAA))
    {
        //关闭按键灯
        GPIO_WritePin(31, 0);
        //HIGH_SYNC_OUT输出1
        GPIO_WritePin(0, 1);
        //LOW_SYNC_OUT输出1
        GPIO_WritePin(1, 1);
        gCanProtoListInfoParallelTestTx.p_proto_info[EN_TX_PARAL_TEST_ACK_1].data_prep(0);
    }
}

/**
* @brief    并机从机同步信息2接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelTestAck2RxProcess(uint16_t *p_buf)
{
    if((p_buf[0] == 0xAA) && (p_buf[1] == 0x55) &&\
    (p_buf[2] == 0xAA) && (p_buf[3] == 0x55) &&\
    (p_buf[4] == 0xAA) && (p_buf[5] == 0x55) &&\
    (p_buf[6] == 0xAA) && (p_buf[7] == 0x55))
    {
        //开启按键灯
        GPIO_WritePin(31, 1);
        gCanProtoListInfoParallelTestTx.p_proto_info[EN_TX_PARAL_TEST_ACK_2].data_prep(0);
    }
}

