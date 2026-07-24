/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_tx_rx_process.c
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
#include "can_tx_rx_process.h"


#define CAN_PROTO_TX_INTERVAL   1 //CAN发送的间隔
#define CAN_PROTO_TX_RESEND_NUM  2 //CAN重发次数


Queue_t	gCanProtoTxQueue; //多个CAN总线数据发送用一个队列，业务不区分是哪个CAN

//个数通过sizeof(pCanProtoAllListInfoTx)/sizeof(CanProtoListInfo_t *)计算

CanProtoListTxInfo_t *pCanProtoListInfoTx[] = 
{
    &gCanProtoListInfoCanMainCardTx,    
};

CanProtoListRxInfo_t *pCanProtoListInfoRx[] = 
{
    &gCanProtoListInfoCanMainCardRx,   
};


/** 
* @brief    CAN协议业务初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void CanProtoInit()
{
    DevCanCommInit();
    QueueInit(&gCanProtoTxQueue, CAN_PROTO_TX_QUEUE_DEPTH, sizeof(DevCanCommData_t));
}


/** 
* @brief    CAN数据接收处理机制
* @param    none
* @param    none
* @return   none
* @note:
*/
void CanProtoRxProcess()
{
    DevCanCommData_t *p_msg;
    static uint8_t proto_type_count = 0;
 
    /*不同CAN接口分开处理*/
    for(uint8_t i = 0; i < EN_DEV_CAN_NUM; i++)
    {
        if(QueueGet(&gDevCanRevQueue[i], (void *)&p_msg) == true)
        {
            /*轮询所有InfoTx, 只有id, 帧格式, CAN通道, 以及数据长度均匹配才进入对应解析函数*/
            for(uint8_t j = 0; j < sizeof(pCanProtoListInfoRx)/sizeof(CanProtoListRxInfo_t *); j++)
            {
                for(uint8_t k = 0; k < pCanProtoListInfoRx[j]->num; k++)
                {
                    if((pCanProtoListInfoRx[j]->p_proto_info[k].canx == p_msg->canx) && 
                       (pCanProtoListInfoRx[j]->p_proto_info[k].can_type == p_msg->can_type) && 
                       (pCanProtoListInfoRx[j]->p_proto_info[k].data_len == p_msg->data_len))
                    {
                        if(pCanProtoListInfoRx[j]->p_proto_info[k].can_id.id == p_msg->can_id.id)
                        {
                            pCanProtoListInfoRx[j]->p_proto_info[k].data_process(p_msg->data);
                        }
                        else
                        {
                            if(p_msg->can_type == EN_DEV_CAN_TYPE_EXT)
                            {
                                if(pCanProtoListInfoRx[j]->p_proto_info[k].can_id.ext_pdu.ps == 0x01)//对于广播包也接受，仅限扩展帧
                                {
                                    pCanProtoListInfoRx[j]->p_proto_info[k].data_process(p_msg->data);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    /*处理轮询业务，每次处理一个版块*/
    for(uint8_t k = 0; k < pCanProtoListInfoRx[proto_type_count]->num; k++)
    {
        if(pCanProtoListInfoRx[proto_type_count]->p_proto_info[k].polling)
        {
            pCanProtoListInfoRx[proto_type_count]->p_proto_info[k].polling(k);
        }
    }
    
    proto_type_count++;
    if(proto_type_count >= sizeof(pCanProtoListInfoRx)/sizeof(CanProtoListRxInfo_t *))
    {
        proto_type_count = 0;
    }
}




/** 
* @brief    CAN数据发送处理机制
* @param    none
* @param    none
* @return   none
* @note: 该版本多个CAN总线是分时发送处理
*/
void CanProtoTxProcess()
{
    static EN_CAN_PROTO_TX_STATUS status = EN_CAN_PROTO_TX_STATUS_IDLE;
    static uint32_t timeout = 0;
    static DevCanCommData_t *p_msg;
    static uint8_t resend_times_count = 0;
    static uint8_t proto_type_count = 0;
    
    switch(status)
    {
        case EN_CAN_PROTO_TX_STATUS_IDLE:
        {
            if(QueueGet(&gCanProtoTxQueue, (void *)&p_msg) == true)
            {
                if(DevCanSend(p_msg) == true)
                {
                    timeout = tim_get_systime();
                    status = EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL;
                }
                else
                {
                    timeout = tim_get_systime();
                    status = EN_CAN_PROTO_TX_STATUS_RESEND_WAIT;
                    resend_times_count = 0;
                }
            }
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL:
        {
            if((uint32_t)(tim_get_systime() - timeout) > CAN_PROTO_TX_INTERVAL)
            {
                status = EN_CAN_PROTO_TX_STATUS_IDLE;
            }
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_RESEND_WAIT:
        {
            if((uint32_t)(tim_get_systime() - timeout) > CAN_PROTO_TX_INTERVAL)
            {
                status = EN_CAN_PROTO_TX_STATUS_RESEND;
            }
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_RESEND:
        {
            if(DevCanSend(p_msg) == true)
            {
                status = EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL;
                resend_times_count++;
            }
            else
            {
                status = EN_CAN_PROTO_TX_STATUS_RESEND_WAIT;
                resend_times_count++;
                if(resend_times_count > CAN_PROTO_TX_RESEND_NUM)
                {
                    status = EN_CAN_PROTO_TX_STATUS_ERROR;
                }
            }
            timeout = tim_get_systime();
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_ERROR:
        {
            //可加入调试信息
            
            status = EN_CAN_PROTO_TX_STATUS_IDLE;
            break;
        }
        default:break;
    }
    
    /*处理轮询业务，每次处理一个版块*/
    for(uint8_t k = 0; k < pCanProtoListInfoTx[proto_type_count]->num; k++)
    {
        if(pCanProtoListInfoTx[proto_type_count]->p_proto_info[k].polling != NULL)
        {
            pCanProtoListInfoTx[proto_type_count]->p_proto_info[k].polling(k);
        }
    }
    
    proto_type_count++;
    if(proto_type_count >= sizeof(pCanProtoListInfoTx)/sizeof(CanProtoListTxInfo_t *))
    {
        proto_type_count = 0;
    }
}






/** 
* @brief    CAN发送数据队列处理
* @param    p_tx_info:CanProtoTxInfo_t协议数据，表示要发送哪一条协议
* @param    p_buf:协议数据内容(8byte)
* @return   入队列是否成功，true表示成功
* @note:
*/
bool CanProtoTxIntoQueue(CanProtoTxInfo_t *p_tx_info, uint8_t *p_buf)
{
    DevCanCommData_t can_tx_comm;
    
    can_tx_comm.can_id.id = p_tx_info->can_id.id;
    can_tx_comm.canx = p_tx_info->canx;
    can_tx_comm.can_type = p_tx_info->can_type;
    can_tx_comm.data_len = p_tx_info->data_len;
    memcpy(can_tx_comm.data, p_buf, can_tx_comm.data_len);
    
    return QueuePut(&gCanProtoTxQueue, &can_tx_comm);
}

/** 
* @brief    CAN从机地址设置(按照地址分配情况设置自身地址)
* @param    addr: 从机自身地址
* @return   NULL
* @note:
*/
void CanProtoSlaveAddrSet(uint8_t addr)
{
    /*当需要修改某个业务地址时调用，并在下方加入相关代码*/
	
}



void CanProtoProcess()
{
    CanProtoRxProcess();
    CanProtoTxProcess();
}