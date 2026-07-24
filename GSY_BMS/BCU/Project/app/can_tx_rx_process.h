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
#include "dev_spiflash.h"
//#include "sys_info.h"
#include "bmu_ctl.h"
#include "bms_data_manage.h"
#include "bcu_sox.h"
#include "high_volt_detect.h"
#include "ins_res_detect.h"
#include "temp_detect.h"
#include "alarm.h"
#include "peri_ctl_stat_api.h"
#include "sys_run.h"
#include "systick.h"
#include "crc.h"
#include "current_detect.h"
#include "data_storage_manage.h"


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




extern CanProtoListTxInfo_t gCanProtoListInfoAddrMaskTx;
extern CanProtoListRxInfo_t gCanProtoListInfoAddrMaskRx;

extern CanProtoListTxInfo_t gCanProtoListInfoBMUTx;
extern CanProtoListRxInfo_t gCanProtoListInfoBMURx;

extern CanProtoListTxInfo_t gCanProtoListInfoPCSTx;
extern CanProtoListRxInfo_t gCanProtoListInfoPCSRx;

extern CanProtoListTxInfo_t gCanProtoListInfoUpgradeTx;
extern CanProtoListRxInfo_t gCanProtoListInfoUpgradeRx;

extern CanProtoListTxInfo_t gCanProtoListInfoDebugTx;
extern CanProtoListRxInfo_t gCanProtoListInfoDebugRx;

extern CanProtoListTxInfo_t gCanProtoListInfoFactoryTestTx;
extern CanProtoListRxInfo_t gCanProtoListInfoFactoryTestRx;
	
void CanProtoInit(void);

bool CanProtoTxIntoQueue(CanProtoTxInfo_t *p_tx_info, uint8_t *p_buf);

void CanProtoProcess(void);
void CanProtoRxProcess(void);
void CanProtoTxProcess(void);



#endif

















