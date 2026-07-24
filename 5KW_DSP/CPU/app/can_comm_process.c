/*
 * can_comm_process.c
 *
 *  Created on: 2025年12月17日
 *      Author: Nova008226
 */

#include "can_comm_process.h"
#include "queue.h"
#include "cpu_timers.h"
#include "dev_can_comm.h"
#include "sys_info.h"
#include "modbus_analysis.h"
#include "can.h"

#define CAN_PROTO_TX_INTERVAL   1 //CAN发送的间隔
#define CAN_PROTO_TX_RESEND_NUM  2 //CAN重发次数

Queue_t gCanProtoTxQueue ;//多个CAN总线数据发送用一个队列，业务不区分是哪个CAN



CanProtoListTxInfo_t *pCanProtoListInfoTx[] =
{
    &gCanProtoListInfoParallelMasterTx,
    &gCanProtoListInfoParallelSlaveTx,
    &gCanProtoListInfoParallelTestTx,

};

CanProtoListRxInfo_t *pCanProtoListInfoRx[] =
{
    &gCanProtoListInfoParallelMasterRx,
    &gCanProtoListInfoParallelSlaveRx,
    &gCanProtoListInfoParallelTestRx,

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
* @brief: CanSoftReset
* @param: void                        
* @note: 
*
*/
void CanSoftReset(void)
{
    CanInit();
}

/**
* @brief    CAN数据接收处理机制
* @param    none
* @param    none
* @return   none
* @note:
*/

static void _CanProtoRxProcess()
{
    DevCanCommData_t *p_msg;
    static uint8_t proto_type_count = 0;

    /*不同CAN接口分开处理*/
    for(uint8_t i = 0; i < EN_DEV_CAN_NUM; i++)
    {
        if(QueueGet(&gDevCanRevQueue[i], (void *)&p_msg) ==true)//
        {
            /*轮询所有InfoTx, 只有id, 帧格式, CAN通道, 以及数据长度均匹配才进入对应解析函数*/
            for(uint8_t j = 0; j < sizeof(pCanProtoListInfoRx)/sizeof(CanProtoListRxInfo_t *); j++)
            {
                for(uint8_t k = 0; k < pCanProtoListInfoRx[j]->num; k++)
                {
                    if((pCanProtoListInfoRx[j]->p_proto_info[k].canx == p_msg->canx)
                     &&(pCanProtoListInfoRx[j]->p_proto_info[k].can_type == p_msg->can_type)
                     &&(pCanProtoListInfoRx[j]->p_proto_info[k].data_len == p_msg->data_len))
                    {
                        if(pCanProtoListInfoRx[j]->p_proto_info[k].can_id.id == p_msg->can_id.id)
                        {
                            if(pCanProtoListInfoRx[j]->p_proto_info[k].data_process != NULL) // 20260429，优化代码
                            {
                                pCanProtoListInfoRx[j]->p_proto_info[k].data_process(p_msg->data);
                            }
                        }
                        else
                        {
                            if(p_msg->can_type == EN_DEV_CAN_TYPE_EXT)
                            {
                                if((p_msg->can_id.ext_pdu.ps == 0xFF)
                                 &&(pCanProtoListInfoRx[j]->p_proto_info[k].can_id.ext_pdu.pf == p_msg->can_id.ext_pdu.pf))//对于广播包也接受，仅限扩展帧
                                {
                                    if(pCanProtoListInfoRx[j]->p_proto_info[k].data_process != NULL) // 20260429，优化代码
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
    }
    /*处理协议组内各个分支轮询业务，每次处理一个版块*/
    for(uint8_t k = 0; k < pCanProtoListInfoRx[proto_type_count]->num; k++)
    {
        if(pCanProtoListInfoRx[proto_type_count]->p_proto_info[k].polling != NULL)
        {
            pCanProtoListInfoRx[proto_type_count]->p_proto_info[k].polling(k);
        }
    }
    /*处理协议组大轮询业务*/
    if(pCanProtoListInfoRx[proto_type_count]->polling != NULL)
    {
        pCanProtoListInfoRx[proto_type_count]->polling();
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
uint32_t CantxTest1, CantxTest2;
static void _CanProtoTxProcess()
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
                    CantxTest1++;
                    timeout = TimGetSystime();
                    status = EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL;
                }
                else
                {
                    timeout = TimGetSystime();
                    status = EN_CAN_PROTO_TX_STATUS_RESEND_WAIT;
                    resend_times_count = 0;
                }
            }
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL:
        {
            if((uint32_t)(TimGetSystime() - timeout) > CAN_PROTO_TX_INTERVAL)
            {
                status = EN_CAN_PROTO_TX_STATUS_IDLE;
            }
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_RESEND_WAIT:
        {
            if((uint32_t)(TimGetSystime() - timeout) > CAN_PROTO_TX_INTERVAL)
            {
                status = EN_CAN_PROTO_TX_STATUS_RESEND;
            }
            break;
        }
        case EN_CAN_PROTO_TX_STATUS_RESEND:
        {
            if(DevCanSend(p_msg) == true)
            {
                CantxTest1++;
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
            timeout = TimGetSystime();
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

    /*处理协议组内轮询业务，每次处理一个版块*/
    for(uint8_t k = 0; k < pCanProtoListInfoTx[proto_type_count]->num; k++)
    {
        if(pCanProtoListInfoTx[proto_type_count]->p_proto_info[k].polling != NULL)
        {
            pCanProtoListInfoTx[proto_type_count]->p_proto_info[k].polling(k);
        }
    }
    /*处理协议组大轮询业务*/
    if(pCanProtoListInfoTx[proto_type_count]->polling != NULL)
    {
        pCanProtoListInfoTx[proto_type_count]->polling();
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
* @brief    CAN协议业处理大轮询
* @param    none
* @return   none
* @note:
*/
void CanProtoProcess()
{
    _CanProtoRxProcess();
    _CanProtoTxProcess();
}




