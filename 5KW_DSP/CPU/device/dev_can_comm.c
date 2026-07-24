/*
 * dev_can_comm.c
 *
 *  Created on: 2025年12月17日
 *      Author: Nova008226
 */


#include "dev_can_comm.h"
#include "queue.h"
#include "can.h"

#include "can_comm_process.h"
#define DEV_CAN_REV_QUEUE_DEPTH     10   //todo

Queue_t  gDevCanRevQueue[EN_DEV_CAN_NUM];


void DevCanCommInit()
{
    for(uint8_t k = 0; k < EN_DEV_CAN_NUM; k++)
    {
        QueueInit(&gDevCanRevQueue[k], DEV_CAN_REV_QUEUE_DEPTH, sizeof(DevCanCommData_t));
    }

    CanInit();
}

/**
* @brief    CAN发送
* @param    none
* @param    none
* @return   是否发送成功
* @note:
*/
bool DevCanSend(DevCanCommData_t *pDevCanComm)
{
    uint16_t obj_id = 0;

    obj_id = MsgIdFindObjId(pDevCanComm->can_id.id);

    if(obj_id == 0)
    {
        return false;
    }

    CanTxDataCopy(pDevCanComm->data);

    if(pDevCanComm->canx == 0)
    {
        sendCANAMessage(obj_id);
    }
    else if(pDevCanComm->canx == 1)
    {
        sendCANBMessage(obj_id);
    }
    else
    {
        return false;
    }

    return true;
}


bool CanRecv(void)
{

    DevCanCommData_t dev_cana_data;

    DevCanCommData_t dev_canb_data;

    if(CANA_readMessage(CANA_BASE, CANA_RX_MSG_OBJ, &dev_cana_data) == true)
    {

        if(QueuePut(&gDevCanRevQueue[EN_DEV_CAN0], &dev_cana_data) == false)
        {
           //调试时可增加错误码，查看队列是否溢出
           return  false;
        }
    }
    if(CANB_readMessage(CANB_BASE, CANB_RX_MSG_OBJ, &dev_canb_data) == true)
    {
        if(QueuePut(&gDevCanRevQueue[EN_DEV_CAN1], &dev_canb_data) == false)
        {
           //调试时可增加错误码，查看队列是否溢出
           return  false;
        }
    }

    return  true;
}


