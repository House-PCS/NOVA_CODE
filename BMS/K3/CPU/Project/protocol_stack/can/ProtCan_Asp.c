#include "ProtCan_Asp.h"

//#include "can.h"
//#include "queue.h"

#include "can_protocol.h"
//CAN_MESSAGE_INFO *CanMessageTxInfo=0;


#define CAN0_PROT_NUM    1
//CAN_PROT_CFG_INFO Can0ProtCfgInfo[CAN0_PROT_NUM]={
//    {CanProtInit_Pcs,ProtReceiveFun_pcs,ProtRxTimeOut_Pcs,0},
//};
PROT_INIT_FUN ProtInin_CAN0[CAN0_PROT_NUM] =
{
    CanProtInit_Exp
};
//////////////////////PORT_CFG//////////////////////////
PROT_INIT_FUN* CanProtAspCfgInfo[CAN_PORT_NUM] =
{
    ProtInin_CAN0,
};
CAN_PROT_INFO   Can0ProtInfo[CAN0_PROT_NUM];
CAN_PORT_PROT_GATHER CanPortProtGather[CAN_PORT_NUM] =
{
    {CAN0_PROT_NUM, Can0ProtInfo, Queue_CanRx, Queue_CanTx, 0},
};
///////////////////Init////////////////////////////
void ProtCanAsp_Init()
{
    uint8_t i = 0, j = 0;
    for(i = 0; i < CAN_PORT_NUM; i++)
    {
        for(j = 0; j < CanPortProtGather[i].ProtNum; j++)
        {
            CanProtAspCfgInfo[i][j](&CanPortProtGather[i].CanProtCfgGather[j]);
        }

    }
//    for(i=0;i<CAN_PORT_NUM;i++)
//    {
//        for(j=0;j<CanPortProtGather[i].ProtNum;j++)
//        {
//            CanPortProtGather[i].CanProtCfgGather[j].;
//        }
//
//    }
    CanProtGather.PortNum = CAN_PORT_NUM;
    CanProtGather.CanPortProtCfgGather = CanPortProtGather;
}

MSH_CMD_EXPORT(ProtCanAsp_Init, -- -);