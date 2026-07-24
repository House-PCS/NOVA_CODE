/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_tx_rx_process.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-6-20
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
#ifndef __CAN_TX_RX_PROCESS_H_
#define __CAN_TX_RX_PROCESS_H_

#include "dev_can_comm.h"
#include "sys_info.h"

typedef struct _can_proto_rx_info_stru_t
{
    union 
    {
        uint32_t            id;
        DevCanExtPdu_t      ext_pdu;
        DevCanStdPdu_t      std_pdu;
    }can_id;    
    EN_DEV_CAN              canx;
    EN_DEV_CAN_TYPE         can_type;
    uint8_t                 data_len;//用于区分是数据帧还是远程帧
    uint32_t                timeout;//超时时间，0表示不包含超时,单位ms
    uint32_t                time_count;//超时时间计数
    void                    (*data_process)(uint8_t *);//数据解析处理
    void                    (*polling)(uint8_t );//接收轮询，主要用于超时判断,
                                                 //形参表示协议idx,告诉接口是哪个协议的,增加形参,也可用于多个协议共用一个polling实现接口
}CanProtoRxInfo_t;

typedef struct _can_proto_list_rx_info_stru_t
{
    CanProtoRxInfo_t        *p_proto_info;
    uint8_t                 num;
}CanProtoListRxInfo_t;


typedef enum _can_proto_tx_mode
{
    EN_CAN_PROTO_TX_MODE_TRIGGER,//触发型，由业务发起
    EN_CAN_PROTO_TX_MODE_REGULAR,//周期型，定时自动发起
}EN_CAN_PROTO_TX_MODE;

typedef struct _can_proto_tx_info_stru_t
{
    union 
    {
        uint32_t            id;
        DevCanExtPdu_t      ext_pdu;
        DevCanStdPdu_t      std_pdu;
    }can_id;
    EN_DEV_CAN              canx;
    EN_DEV_CAN_TYPE         can_type;
    uint8_t                 data_len;//用于区分是数据帧还是远程帧
    EN_CAN_PROTO_TX_MODE    tx_mode;
    uint32_t                tx_regular_period;//周期型发送周期，触发型不涉及
	bool                    regular_default_enable;//周期发送是否使能
    void                    (*data_prep)(uint8_t );//数据准备，对于单次或者多队列单次发送，则直接执行数据推送入队列，
                                                   //对于需要多次分时发送，则进行标志位处理，由polling接口执行具体发送
                                                   //形参表示同一条CAN ID协议的第几帧，从0开始
    void                    (*polling)(uint8_t );//轮询处理机制, 用于周期性发送或者需要多次间隔发送的场景(数据量较大,队列深度不够的情况，如固件发送),
                                                 //形参表示协议idx,告诉接口是哪个协议的,增加形参,也可用于多个协议共用一个polling实现接口
}CanProtoTxInfo_t;


typedef struct _can_proto_list_tx_info_stru_t
{
    CanProtoTxInfo_t        *p_proto_info;
    uint8_t                 num;
}CanProtoListTxInfo_t;




#define CAN_PROTO_TX_QUEUE_DEPTH  20 

typedef enum
{
    EN_CAN_PROTO_TX_STATUS_IDLE,//查询队列，有数据即发送
    EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL,//确保数据发送间隔
    EN_CAN_PROTO_TX_STATUS_RESEND_WAIT,//重发等待
    EN_CAN_PROTO_TX_STATUS_RESEND,//重发
    EN_CAN_PROTO_TX_STATUS_ERROR,//数据发送存在异常
}EN_CAN_PROTO_TX_STATUS;




extern CanProtoListTxInfo_t gCanProtoListInfoCanMainCardTx;
extern CanProtoListTxInfo_t gCanProtoListInfoFactoryTestTx;

extern CanProtoListRxInfo_t gCanProtoListInfoCanMainCardRx;
extern CanProtoListRxInfo_t gCanProtoListInfoFactoryTestRx;

void CanProtoInit(void);

bool CanProtoTxIntoQueue(CanProtoTxInfo_t *p_tx_info, uint8_t *p_buf);

void CanProtoProcess(void);
void CanProtoRxProcess(void);
void CanProtoTxProcess(void);



//CAN TEST
void SendShutDownSignal(uint8_t idx);
void RecvShutDownSignalAck(uint8_t *p_buf);


//BCU  FACTORY TEST
void BcuFactoryTestEnterTestModeTxPrep(uint8_t idx);
void BcuFactoryTestExitTestModeTxPrep(uint8_t idx);
void BcuFactoryTestHighVoltageTxPrep(uint8_t idx);
void BcuFactoryTestInsTestTxPrep(uint8_t idx);
void BcuFactoryTestCurDivTxPrep(uint8_t idx);
void BcuFactoryTestHallTxPrep(uint8_t idx);
void BcuFactoryTestTempTxPrep(uint8_t idx);
void BcuFactoryTestIOInputTxPrep(uint8_t idx);
void BcuFactoryTestIOOutputTxPrep(uint8_t idx);
void BcuFactoryTestEepromTxPrep(uint8_t idx);
void BcuFactoryTestFlashTxPrep(uint8_t idx);
void BcuFactoryTestRtcTxPrep(uint8_t idx);


void BcuFactoryTestEnterTestModeRxProcess(uint8_t *p_buf);
void BcuFactoryTestExitTestModeRxProcess(uint8_t *p_buf);
void BcuFactoryTestHighVoltageRxProcess(uint8_t *p_buf);
void BcuFactoryTestInsTestRxProcess(uint8_t *p_buf);
void BcuFactoryTestCurDivRxProcess(uint8_t *p_buf);
void BcuFactoryTestHallRxProcess(uint8_t *p_buf);
void BcuFactoryTestTempRxProcess(uint8_t *p_buf);
void BcuFactoryTestIOInputRxProcess(uint8_t *p_buf);
void BcuFactoryTestIOOutputRxProcess(uint8_t *p_buf);
void BcuFactoryTestEepromRxProcess(uint8_t *p_buf);
void BcuFactoryTestFlashRxProcess(uint8_t *p_buf);
void BcuFactoryTestRtcRxProcess(uint8_t *p_buf);

//BMU  FACTORY TEST
void BmuFactoryTestEnterTestModeTxPrep(uint8_t idx);
void BmuFactoryTestExitTestModeTxPrep(uint8_t idx);
void BmuFactoryTestVoltageTxPrep(uint8_t idx);
void BmuFactoryTestTempTxPrep(uint8_t idx);
void BmuFactoryTestBalanceTempTxPrep(uint8_t idx);
void BmuFactoryTestIOInoutTxPrep(uint8_t idx);
void BmuFactoryTestIOOutputTxPrep(uint8_t idx);
void BmuFactoryTestPWMTxPrep(uint8_t idx);
void BmuFactoryTestIOInputTxPrep(uint8_t idx);
void BmuFactoryTestFlashTxPrep(uint8_t idx);


void BmuFactoryTestEnterTestModeRxProcess(uint8_t *p_buf);
void BmuFactoryTestExitTestModeRxProcess(uint8_t *p_buf);
void BmuFactoryTestVoltageRxProcess(uint8_t *p_buf);
void BmuFactoryTestTempRxProcess(uint8_t *p_buf);
void BmuFactoryTestBalanceTempRxProcess(uint8_t *p_buf);
void BmuFactoryTestIOInoutRxProcess(uint8_t *p_buf);
void BmuFactoryTestIOOutputRxProcess(uint8_t *p_buf);
void BmuFactoryTestPWMRxProcess(uint8_t *p_buf);
void BmuFactoryTestIOInputRxProcess(uint8_t *p_buf);
void BmuFactoryTestFlashRxProcess(uint8_t *p_buf);
#endif

















