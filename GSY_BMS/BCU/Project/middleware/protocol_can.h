#ifndef __PROT_CAN_H_
#define __PROT_CAN_H_

//#include "PublicTypedef.h"
#include "stdbool.h"
#include "stdint.h"
#include "string.h"

#include "queue.h"


typedef enum{
    CAN_MESSAGE_DATA=0,   //数据帧
    CAN_MESSAGE_REMOTE,   //远程帧
}CAN_MESSAGE_TYPE;
typedef enum{
    CAN_ID_ST=0,   //can标准帧
    CAN_ID_EXT,   //can扩展帧
}CAN_ID_TYPE;
typedef enum{
    TX_SINGLE=0,   //单次发送
    TX_CYCLE,      //周期循环
//    FAST_CYCLE,     //周期发送，立即启动（0时刻发第一帧）
//    DELAY_CYCLE,    //周期发送，延时启动（上电延时后发送第一帧）
}CAN_SEND_TYPE;

typedef struct
{
    uint8_t  dlen;                                                    /*!< data length */
    uint8_t  data[8];                                                 /*!< receive data */
    CAN_ID_TYPE  ff;                                                      /*!< format of frame, standard or extended format */
    CAN_MESSAGE_TYPE  ft;                                                      /*!< type of frame, data or remote */
    uint32_t id;                                                   /*!< extended format frame identifier */
}CanMessageStructType;
typedef struct {
    uint8_t             p:3;
    uint8_t             r:1;
    uint8_t             dp:1;
    uint8_t             pf;
    uint8_t             ps;
    uint8_t             sa;
//    CAN_MESSAGE_TYPE    CanMessageType;   
    uint8_t             DataLen;        //发送写0时为远程帧
    CAN_ID_TYPE         CanIdType;
    CAN_SEND_TYPE       SendType;
//    bool                MessageEnble;
    uint16_t            TimeOutms;
    void (*MessageDataFun)(uint8_t);
}CanMessageCfgType;      //ASP
typedef struct {
    uint8_t             p:3;
    uint8_t             r:1;
    uint8_t             dp:1;
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
    void (*MessageDataFun)(uint8_t);
}CanMessageInfoType;      //PROT
typedef struct {
    CAN_ID_TYPE         CanIdType;
    uint8_t             p:3;
    uint8_t             r:1;
    uint8_t             dp:1;
    uint8_t             pf;
    uint8_t             ps;
    uint8_t             sa;
//    CAN_MESSAGE_TYPE    CanMessageType;   
    uint8_t             DataLen;        //发送写0时为远程帧
    uint8_t             *DataBuff;
}CanRxMessageType;

typedef struct {
    uint8_t TxMessageNum;
    uint8_t RxMessageNum;
    CanMessageInfoType *CanMessageTxInfo;
    CanMessageInfoType *CanMessageRxInfo;
    uint8_t *TxBuff;
    
//	void (*ProtInitFun)(void);
    bool (*ProtReceiveFun)(CanRxMessageType *p,uint8_t);
    void (*ProtRxTimeOutFun)(uint8_t);
    uint16_t ProtStartTime;     //协议上电延时启动时间，默认写0，不延迟
}CanProtInfoType; //指向单个协议
typedef struct {
    uint8_t ProtNum;    //协议数量
    CanProtInfoType *CanProtCfgGather; 
    Queue_t *Queue_CanRx;
    Queue_t *Queue_CanTx;
    uint8_t error;      //bit0:发送队列溢出，bit1:接收队列溢出
}CanPortProtGatherType;  //指向单个端口
typedef struct {
    uint8_t PortNum;    //端口数量
    CanPortProtGatherType *CanPortProtCfgGather;     
}CanProtGatherType;  


typedef void (*CAN_DATA_FUN)(void);
typedef void (*PROT_INIT_FUN)(CanProtInfoType* );

extern CanProtGatherType CanProtGather;


void ProtCanMain(void);

#endif
