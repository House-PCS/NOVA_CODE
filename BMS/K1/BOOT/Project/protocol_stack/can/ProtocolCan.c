#include "ProtocolCan.h"
//#include "DataProcess.h"
#include "systick.h"

CAN_PROT_GATHER CanProtGather;
uint8_t gCanTxCfgIndex;

bool ProtCanRxFun()
{
    uint8_t i = 0, j = 0, k = 0;
    bool receive = false;
    CAN_MESSAGE_STRUCT  *rxp = NULL;
    CAN_RX_MESSAGE    can_message_info;// = {0};
    memset((uint8_t *)&can_message_info,0,sizeof(CAN_RX_MESSAGE));

    for(i = 0; i < CanProtGather.PortNum; i++) //Port循环，每个端口对应一个独立的队列
    {
        if(QueueGet(CanProtGather.CanPortProtCfgGather[i].Queue_CanRx, (void *)&rxp))
        {
            can_message_info.CanIdType = rxp->ff;
            if(rxp->ff == CAN_ID_ST)
            {
                can_message_info.ps = (rxp->id >> 8) & ((uint16_t)0x7U);
                can_message_info.sa = rxp->id & ((uint16_t)0xFFU);
            }
            else if(rxp->ff == CAN_ID_EXT)
            {
                can_message_info.p = (rxp->id >> 26) & ((uint32_t)0x7U);
                can_message_info.r = (rxp->id >> 25) & ((uint32_t)0x1U);
                can_message_info.dp = (rxp->id >> 24) & ((uint32_t)0x1U);
                can_message_info.pf = (rxp->id >> 16) & ((uint32_t)0xFFU);
                can_message_info.ps = (rxp->id >> 8) & ((uint32_t)0xFFU);
                can_message_info.sa = rxp->id & ((uint32_t)0xFFU);
            }
            can_message_info.DataLen = rxp->dlen;
            can_message_info.DataBuff = rxp->data;
            receive = true;
        }
        for(j = 0; j < CanProtGather.CanPortProtCfgGather[i].ProtNum; j++) //ASP层协议数循环
        {
            for(k = 0; k < CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].RxMessageNum; k++)
            {
                if(receive)
                {
                    if(CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].ProtReceiveFun(&can_message_info, k))
                    {
                        CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageRxInfo[k].UpdateTimems = tim_get_systime();
                    }
                }

//                else
//                {
//
//                }
                if(tim_difftime(CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageRxInfo[k].UpdateTimems, tim_get_systime()) > \
                        CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageRxInfo[k].TimeOutms)
                {
                    CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].ProtRxTimeOutFun(k);
                }
            }
        }
    }
    return 1;
}
void MessageTxDataFun(uint8_t port, CAN_PROT_INFO* prot, uint8_t num)
{
    CAN_MESSAGE_STRUCT  txp;
    gCanTxCfgIndex = num;
    prot->CanMessageTxInfo[num].MessageDataFun();
    txp.id = ((uint32_t)prot->CanMessageTxInfo[num].ps) << 8     \
             | ((uint32_t)prot->CanMessageTxInfo[num].sa)     ;
    if(prot->CanMessageTxInfo[num].CanIdType == CAN_ID_EXT)
    {
        txp.id |= ((uint32_t)prot->CanMessageTxInfo[num].p) << 26    \
                  | ((uint32_t)prot->CanMessageTxInfo[num].r) << 25     \
                  | ((uint32_t)prot->CanMessageTxInfo[num].dp) << 24     \
                  | ((uint32_t)prot->CanMessageTxInfo[num].pf) << 16    ;
        txp.ff = CAN_ID_EXT;
    }
    else
    {
        txp.ff = CAN_ID_ST;
    }
    txp.dlen = prot->CanMessageTxInfo[num].DataLen;
    if(txp.dlen > 0)
    {
        memcpy(txp.data, prot->TxBuff, txp.dlen);
        txp.ft = CAN_MESSAGE_DATA;
    }
    else
    {
        txp.ft = CAN_MESSAGE_REMOTE;
    }
    QueuePut(CanProtGather.CanPortProtCfgGather[port].Queue_CanTx, &txp);
}
void ProtCanTxFun()
{
    uint8_t i = 0, j = 0, k = 0;
//    bool ret=false;
//    CAN_MESSAGE_INFO can_message_info={0};


    for(i = 0; i < CanProtGather.PortNum; i++) //Port循环，每个端口对应一个独立的队列
    {
        if(QueueIsFull(CanProtGather.CanPortProtCfgGather[i].Queue_CanTx))
        {
            ////队列满/////////
            CanProtGather.CanPortProtCfgGather[i].error |= 1 << 0;
            continue;
        }
        for(j = 0; j < CanProtGather.CanPortProtCfgGather[i].ProtNum; j++) //ASP层协议数循环
        {
            for(k = 0; k < CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].TxMessageNum; k++)
            {
                if(CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageTxInfo[k].MessageEnable)
                {                    
                    if(CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageTxInfo[k].SendType == TX_SINGLE)
                    {

                        MessageTxDataFun(i, &CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j], k);

                        CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageTxInfo[k].MessageEnable = false;
                    }
                    else
                    {
                        if(tim_difftime(CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageTxInfo[k].UpdateTimems, tim_get_systime()) > \
                                CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageTxInfo[k].TimeOutms)
                        {
                            MessageTxDataFun(i, &CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j], k);
                            CanProtGather.CanPortProtCfgGather[i].CanProtCfgGather[j].CanMessageTxInfo[k].UpdateTimems = tim_get_systime();
                            //                    break;
                        }
                    }

                }

            }
        }
    }
//    return false;
}
void ProtCan_Init()
{
//    CanProtInfo=
}
void ProtCan_Main()
{
    uint8_t i = 0, j = 0, k = 0;
    ProtCanRxFun();
    ProtCanTxFun();
}
