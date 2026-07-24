
/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        dev_can_comm.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2023-5-16
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2023-6-9      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */
  
#include "dev_can_comm.h"


#define DEV_CAN_REV_QUEUE_DEPTH    20
  
Queue_t	gDevCanRevQueue[EN_DEV_CAN_NUM];

//static DevCanCommData_t _gDevCanRevData[EN_DEV_CAN_NUM][DEV_CAN_REV_QUEUE_DEPTH];//由于CAN队列的数据不是指针形式，而是拷贝，所以无需单独定义，每次推送完即可释放，故定义成局部变量即可



static void _DevCan0Irq0RxProcess(void);
static void _DevCan1Irq0RxProcess(void);
/** 
* @brief    CAN通信初始化
* @param    none
* @param    none
* @return   none
* @note:
*/
void DevCanCommInit()
{
    for(uint8_t k = 0; k < EN_DEV_CAN_NUM; k++)
    {
        QueueInit(&gDevCanRevQueue[k], DEV_CAN_REV_QUEUE_DEPTH, sizeof(DevCanCommData_t));
    }
    gCanHandle[EN_DEV_CAN0].irq_rx0_process = _DevCan0Irq0RxProcess;
//    gCanHandle[EN_DEV_CAN1].irq_rx0_process = _DevCan1Irq0RxProcess;
    CanBspInit();
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
    if(pDevCanComm->can_type == EN_DEV_CAN_TYPE_STD)
    {
        gCanHandle[pDevCanComm->canx].tx_frame_type = EN_CAN_FRAME_TYPE_STANDARD;
        gCanHandle[pDevCanComm->canx].tx_id = pDevCanComm->can_id.id;
    }
    else if(pDevCanComm->can_type == EN_DEV_CAN_TYPE_EXT)
    {
        gCanHandle[pDevCanComm->canx].tx_frame_type = EN_CAN_FRAME_TYPE_EXTENDED;
        gCanHandle[pDevCanComm->canx].tx_id = pDevCanComm->can_id.id;
    }
    gCanHandle[pDevCanComm->canx].tx_len = pDevCanComm->data_len;
    if(pDevCanComm->data_len == 0)
    {
        gCanHandle[pDevCanComm->canx].tx_frame_kind = EN_CAN_FRAME_KIND_REMOTE;
    }
    else
    {
        gCanHandle[pDevCanComm->canx].tx_frame_kind = EN_CAN_FRAME_KIND_DATA;
        memcpy(gCanHandle[pDevCanComm->canx].tx_buf, pDevCanComm->data, gCanHandle[pDevCanComm->canx].tx_len);
    }
    return CanSend(&gCanHandle[pDevCanComm->canx]);
    
}




/** 
* @brief    CAN0_FIFO0中断接收 
* @param    端口
* @return   none
* @note:
*/
static void _DevCan0Irq0RxProcess()
{
    DevCanCommData_t dev_can_data;
    dev_can_data.canx = EN_DEV_CAN0;
    
    dev_can_data.can_id.id = gCanHandle[EN_DEV_CAN0].rx_id;
    if(gCanHandle[EN_DEV_CAN0].rx_frame_type == EN_CAN_FRAME_TYPE_STANDARD)
    {
        dev_can_data.can_type = EN_DEV_CAN_TYPE_STD;
    }
    else if(gCanHandle[EN_DEV_CAN0].rx_frame_type == EN_CAN_FRAME_TYPE_EXTENDED)
    {
        dev_can_data.can_type = EN_DEV_CAN_TYPE_EXT;
    }
    
    if(gCanHandle[EN_DEV_CAN0].tx_frame_kind == EN_CAN_FRAME_KIND_DATA)
    {
        dev_can_data.data_len = gCanHandle[EN_DEV_CAN0].rx_len;
        memcpy(dev_can_data.data, gCanHandle[EN_DEV_CAN0].rx_buf, gCanHandle[EN_DEV_CAN0].rx_len);
    }
    else if(gCanHandle[EN_DEV_CAN0].tx_frame_kind == EN_CAN_FRAME_KIND_REMOTE)
    {
        dev_can_data.data_len = 0;
    }
    
    if(QueuePut(&gDevCanRevQueue[EN_DEV_CAN0], &dev_can_data) == false)
    {
        //调试时可增加错误码，查看队列是否溢出
        
        return;
    }
}


/** 
* @brief    CAN1_FIFO0中断接收 
* @param    端口
* @return   none
* @note:
*/
static void _DevCan1Irq0RxProcess()
{
//    DevCanCommData_t dev_can_data;
//    dev_can_data.canx = EN_DEV_CAN1;
//    
//    dev_can_data.can_id.id = gCanHandle[EN_DEV_CAN1].rx_id;
//    if(gCanHandle[EN_DEV_CAN1].rx_frame_type == EN_CAN_FRAME_TYPE_STANDARD)
//    {
//        dev_can_data.can_type = EN_DEV_CAN_TYPE_STD;
//    }
//    else if(gCanHandle[EN_DEV_CAN1].rx_frame_type == EN_CAN_FRAME_TYPE_EXTENDED)
//    {
//        dev_can_data.can_type = EN_DEV_CAN_TYPE_EXT;
//    }
//    
//    if(gCanHandle[EN_DEV_CAN1].tx_frame_kind == EN_CAN_FRAME_KIND_DATA)
//    {
//        dev_can_data.data_len = gCanHandle[EN_DEV_CAN1].rx_len;
//        memcpy(dev_can_data.data, gCanHandle[EN_DEV_CAN1].rx_buf, gCanHandle[EN_DEV_CAN1].rx_len);
//    }
//    else if(gCanHandle[EN_DEV_CAN1].tx_frame_kind == EN_CAN_FRAME_KIND_REMOTE)
//    {
//        dev_can_data.data_len = 0;
//    }
//    
//    if(QueuePut(&gDevCanRevQueue[EN_DEV_CAN1], &dev_can_data) == false)
//    {
//        //调试时可增加错误码，查看队列是否溢出
//        
//        return;
//    }
}



