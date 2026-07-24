/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_addr_mask.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-09-24
  * @brief    
  ******************************************************************************
  * @attention
  * @relation
  
  * @par :
  * <table>
  * <tr><th>Date          <th>Version   <th>Author      <th>Description
  * <tr><td>2024-09-24     <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "systick.h"
#include "can_tx_rx_process.h"

typedef enum
{
    EN_ADDR_MASK_ADDR_TX,
    EN_ADDR_MASK_MASK_TX,
    EN_ADDR_MASK_TX_TOTAL,
}EN_CAN_ADDR_MASK_TX;

typedef enum
{
    EN_ADDR_MASK_ADDR_RX = 0,
    EN_ADDR_MASK_MASK_RX,
    EN_ADDR_MASK_RX_TOTAL,
}EN_CAN_ADDR_MASK_RX;

void BMUAddrMaskAddrTxPrep(uint8_t idx);
void BMUAddrMaskAddrRxProcess(uint8_t *p_buf);
void BMUAddrMaskMaskTxPrep(uint8_t idx);
void BMUAddrMaskMaskRxProcess(uint8_t *p_buf);

static void _TxPolling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoAddrMaskTx[] = 
{
    {0x1899FF99, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, BMUAddrMaskAddrTxPrep, _TxPolling}, 
    {0x1890FF99, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, BMUAddrMaskMaskTxPrep, _TxPolling},
};

CanProtoListTxInfo_t gCanProtoListInfoAddrMaskTx = 
{
    .p_proto_info = _gCanProtoInfoAddrMaskTx,
    .num = EN_ADDR_MASK_TX_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoAddrMaskRx[] = 
{
    {0x18999998, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUAddrMaskAddrRxProcess, NULL}, 
    {0x189099F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUAddrMaskMaskRxProcess, NULL}, 
};

CanProtoListRxInfo_t gCanProtoListInfoAddrMaskRx = 
{
    .p_proto_info = _gCanProtoInfoAddrMaskRx,
    .num = EN_ADDR_MASK_RX_TOTAL
};



/** 
* @brief    地址分配发送数据准备
* @param    idx:下发数据索引,0表示清除从控指令，1表示地址分配，2表示地址锁存
* @return   NULL
* @note:
*/
void BMUAddrMaskAddrTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    if(idx == 0)
    {
        buf[0] = 0x07;
        buf[1] = 0x00;
    }
    else if(idx == 1)
    {
        buf[0] = 0x08;
        buf[1] = gBMUCfgInfo.cur_addr_offset;
    }
    else if(idx == 2)
    {
        buf[0] = 0x0A;
        buf[1] = 0x00;
    }
    else
    {
        return;
    }
    memset(&buf[2], 0x00, 6);
    CanProtoTxIntoQueue(&_gCanProtoInfoAddrMaskTx[EN_ADDR_MASK_ADDR_TX], buf);
}


/** 
* @brief    地址分配响应接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUAddrMaskAddrRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x09 && p_buf[1] == gBMUCfgInfo.cur_addr_offset)//地址分配响应
    {
        gBMUCfgInfo.distr_ack = true;
    }
    else if(p_buf[0] == 0x0B)//锁存响应
    {
        uint8_t sa = _gCanProtoInfoAddrMaskRx[EN_ADDR_MASK_ADDR_RX].can_id.ext_pdu.sa;
        if(sa <= 0xF4 && sa > 0xF4 - gBMSDataInfo.bmu_sample_info.pack_num)
        {
            gBMUCfgInfo.latch_ack[0xF4 - sa] = true;
        }
    }
}


/** 
* @brief    掩码发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void BMUAddrMaskMaskTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t data_temp[64];
    uint8_t data_cnt = 0;
    uint8_t frame_num;//不包含最后一帧
    uint8_t data_temp_offset = 0;
    
    uint8_t cell_volt_mask_size = (gBMSDataInfo.bmu_sample_info.cell_volt_sample_num + 7) / 8;
    uint8_t pack_temp_mask_size = (gBMSDataInfo.bmu_sample_info.pack_temp_sample_num + 7) / 8;//要使用三次，包括电芯温度，极柱温度，正负极温度
    uint8_t balance_temp_mask_size = (gBMSDataInfo.bmu_sample_info.balance_temp_sample_num + 7) / 8;
    
    memset(data_temp, 0x00, sizeof(data_temp));
    //拷贝电芯电压mask
    memcpy(&data_temp[data_cnt], gBMSDataInfo.bmu_sample_info.cell_volt_mask, cell_volt_mask_size);
    data_cnt += cell_volt_mask_size;
    //拷贝电芯温度mask
    memcpy(&data_temp[data_cnt], gBMSDataInfo.bmu_sample_info.cell_temp_mask, pack_temp_mask_size);
    data_cnt += pack_temp_mask_size;
    //拷贝极柱温度mask
    memcpy(&data_temp[data_cnt], gBMSDataInfo.bmu_sample_info.pole_temp_mask, pack_temp_mask_size);
    data_cnt += pack_temp_mask_size;
    //拷贝正负极温度mask
    memcpy(&data_temp[data_cnt], gBMSDataInfo.bmu_sample_info.other_temp_mask, pack_temp_mask_size);
    data_cnt += pack_temp_mask_size;
    //拷贝均衡温度
    memcpy(&data_temp[data_cnt], gBMSDataInfo.bmu_sample_info.balance_temp_mask, balance_temp_mask_size);
    data_cnt += balance_temp_mask_size;
    
    //此时的data_cnt就是数据总长度
    frame_num = ((data_cnt + 6) / 7);
    
    for(uint8_t i = 0; i < frame_num; i++)
    {
        buf[0] = i;
        memcpy(&buf[1], &data_temp[data_temp_offset], 7);
        data_temp_offset += 7;
        CanProtoTxIntoQueue(&_gCanProtoInfoAddrMaskTx[EN_ADDR_MASK_MASK_TX], buf);
    }
    //最后一帧
    memset(buf, 0xF3, 4);
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = frame_num;
    buf[7] = data_cnt;
    CanProtoTxIntoQueue(&_gCanProtoInfoAddrMaskTx[EN_ADDR_MASK_MASK_TX], buf);
}


/** 
* @brief    掩码响应接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUAddrMaskMaskRxProcess(uint8_t *p_buf)
{
    if(p_buf[0] == 0x90)//mask响应
    {
        uint8_t sa = _gCanProtoInfoAddrMaskRx[EN_ADDR_MASK_MASK_RX].can_id.ext_pdu.sa;
        if(sa <= 0xF4 && sa > 0xF4 - gBMSDataInfo.bmu_sample_info.pack_num && sa == p_buf[1])
        {
            gBMUCfgInfo.mask_ack[0xF4 - sa] = true;
        }
    }
}





/** 
* @brief    数据发送轮询
* @param    idx，对应的协议项索引
* @return   NULL
* @note:
*/
static void _TxPolling(uint8_t idx)
{
    switch(idx)
    {
        case EN_ADDR_MASK_ADDR_TX:
        {
            if(gBMUCfgInfo.start_flag == true)
            {
                gBMUCfgInfo.start_flag = false;
                BMUAddrMaskAddrTxPrep(0);
            }
            else if(gBMUCfgInfo.distr_flag == true)
            {
                gBMUCfgInfo.distr_flag = false;
                BMUAddrMaskAddrTxPrep(1);
            }
            else if(gBMUCfgInfo.latch_flag == true)
            {
                gBMUCfgInfo.latch_flag = false;
                BMUAddrMaskAddrTxPrep(2);
            }
            break;
        }
        case EN_ADDR_MASK_MASK_TX:
        {
            if(gBMUCfgInfo.mask_flag == true)
            {
                gBMUCfgInfo.mask_flag = false;
                BMUAddrMaskMaskTxPrep(0);
            }
            break;
        }
        default:break;
    }
}











