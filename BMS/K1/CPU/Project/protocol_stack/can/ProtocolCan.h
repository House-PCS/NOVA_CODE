#ifndef __PROT_CAN_H_
#define __PROT_CAN_H_

//#include "PublicTypedef.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"

#include "queue.h"


#pragma pack(1)

typedef enum
{
    CAN_MESSAGE_DATA = 0, //数据帧
    CAN_MESSAGE_REMOTE,   //远程帧
} CAN_MESSAGE_TYPE;
typedef enum
{
    CAN_ID_ST = 0, //can标准帧
    CAN_ID_EXT,   //can扩展帧
} CAN_ID_TYPE;
typedef enum
{
    TX_SINGLE = 0, //单次发送
    TX_CYCLE,      //周期循环
//    FAST_CYCLE,     //周期发送，立即启动（0时刻发第一帧）
//    DELAY_CYCLE,    //周期发送，延时启动（上电延时后发送第一帧）
} CAN_SEND_TYPE;

typedef struct
{
    CAN_ID_TYPE  ff;                                                      /*!< format of frame, standard or extended format */
    CAN_MESSAGE_TYPE  ft;                                                      /*!< type of frame, data or remote */
    uint8_t  dlen;                                                    /*!< data length */
    uint8_t  data[8];                                                 /*!< receive data */
//    uint8_t  fi;                                                      /*!< filtering index */
//    uint16_t sfid;                                                   /*!< standard format frame identifier */
    uint32_t id;                                                   /*!< extended format frame identifier */
} CAN_MESSAGE_STRUCT;
typedef struct
{
    uint8_t             p;
    uint8_t             r;
    uint8_t             dp;
    uint8_t             pf;
    uint8_t             ps;
    uint8_t             sa;
//    CAN_MESSAGE_TYPE    CanMessageType;
    uint8_t             DataLen;        //发送写0时为远程帧
    CAN_ID_TYPE         CanIdType;
    CAN_SEND_TYPE       SendType;
//    bool                MessageEnble;
    uint16_t            TimeOutms;
    void (*MessageDataFun)(void);
} CAN_MESSAGE_CFG_INFO;     //ASP
typedef struct
{
    uint8_t             p;
    uint8_t             r;
    uint8_t             dp;
    uint8_t             pf;
    uint8_t             ps;
    uint8_t             sa;
//    CAN_MESSAGE_TYPE    CanMessageType;
    uint8_t             DataLen;        //写0时为远程帧
//    uint8_t  data[8];
    bool                MessageEnable;
    CAN_ID_TYPE         CanIdType;
    CAN_SEND_TYPE       SendType;
    uint16_t            TimeOutms;
    uint32_t            UpdateTimems;
    void (*MessageDataFun)(void);
} CAN_MESSAGE_INFO;     //PROT
typedef struct
{
    CAN_ID_TYPE         CanIdType;
    uint8_t             p;
    uint8_t             r;
    uint8_t             dp;
    uint8_t             pf;
    uint8_t             ps;
    uint8_t             sa;
//    CAN_MESSAGE_TYPE    CanMessageType;
    uint8_t             DataLen;        //发送写0时为远程帧
    uint8_t             *DataBuff;
} CAN_RX_MESSAGE;

typedef struct
{
    uint8_t TxMessageNum;
    uint8_t RxMessageNum;
    CAN_MESSAGE_INFO *CanMessageTxInfo;
    CAN_MESSAGE_INFO *CanMessageRxInfo;
    uint8_t *TxBuff;
//  void (*ProtInitFun)(void);
    bool (*ProtReceiveFun)(CAN_RX_MESSAGE *p, uint8_t);
    void (*ProtRxTimeOutFun)(uint8_t);
    uint16_t ProtStartTime;     //协议上电延时启动时间，默认写0，不延迟
} CAN_PROT_INFO; //指向单个协议
//typedef struct {
////    CAN_MESSAGE_INFO *CanMessageTxInfo;
////    CAN_MESSAGE_INFO *CanMessageRxInfo;
//  void (*ProtInitFun)(CAN_PROT_INFO* );
//    bool (*ProtReceiveFun)(CAN_MESSAGE * p,uint8_t);
//    void (*ProtRxTimeOutFun)(uint8_t);
//    uint16_t ProtStartTime;     //协议上电延时启动时间，默认写0，不延迟
//}CAN_PROT_CFG_INFO;
typedef struct
{
    uint8_t ProtNum;    //协议数量
    CAN_PROT_INFO *CanProtCfgGather;
    Queue_t *Queue_CanRx;
    Queue_t *Queue_CanTx;
    uint8_t error;      //bit0:发送队列溢出，bit1:接收队列溢出
} CAN_PORT_PROT_GATHER; //指向单个端口
typedef struct
{
    uint8_t PortNum;    //端口数量
    CAN_PORT_PROT_GATHER *CanPortProtCfgGather;
} CAN_PROT_GATHER;

#pragma pack()

typedef void (*CAN_DATA_FUN)(void);
typedef void (*PROT_INIT_FUN)(CAN_PROT_INFO* );

extern CAN_PROT_GATHER CanProtGather;
extern uint8_t gCanTxCfgIndex;

void ProtCan_Main(void);


#endif
