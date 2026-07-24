/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_factory_test.c
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
  * <tr><td>2024-6-20      <td>V1.0.0       <td>        <td>InitialVersion
  * </table>
  ******************************************************************************
  */

#include "can_tx_rx_process.h"


typedef enum
{
    EN_DIST_ADDR_TX = 0,
    EN_DIST_MASK_TX,
    
    EN_TX_ADDR_TOTAL,
} EN_TX_CAN_ADDR;

typedef enum
{
    EN_DIST_ADDR_RX = 0,
    EN_DIST_MASK_RX,
    
    EN_RX_ADDR_TOTAL,
} EN_RX_CAN_ADDR;

static CanProtoTxInfo_t _gCanProtoInfoAddrTx[] = 
{
    {0x18999998, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, CanAddrDistrubuteTxPrep, NULL}, 
    {0x189099F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, CanAddrMaskTxPrep, NULL}, 
};

CanProtoListTxInfo_t gCanProtoListInfoAddrTx = 
{
    .p_proto_info = _gCanProtoInfoAddrTx,
    .num = EN_TX_ADDR_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoAddrRx[] = 
{
    {0x1899FF99, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, CanAddrDistrubuteRxProcess, NULL}, 
    {0x1890FF99, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, CanAddrMaskRxProcess, NULL}, 
};

CanProtoListRxInfo_t gCanProtoListInfoAddrRx = 
{
    .p_proto_info = _gCanProtoInfoAddrRx,
    .num = EN_RX_ADDR_TOTAL
};


EN_BMS_ADDR PackAddrState = BMS_ADDR_CHECK;

////////////////RX///////////////
static uint8_t _gAddrOffset;//主控发下来的地址偏移，基于0xF4-_gAddrOffset，就是从控被分配的实际地址
static bool _gAddrGetFlag = 0;//是否已经分配到地址
/** 
* @brief    BMU 分址接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void CanAddrDistrubuteRxProcess(uint8_t *p_buf)
{
    switch(p_buf[0])
    {
        case 0x07:
        {
            CanProtoSlaveAddrSet(BMU_DEFAULT_ADDR);
            DevIOCtlOutReset(EN_DEV_IOCTL_IDX_JACK_OUT_1);
            DevIOCtlOutReset(EN_DEV_IOCTL_IDX_JACK_OUT_2);
            CanProtoSlaveAddrReset();
            _gAddrGetFlag = false;
            
            break;
        }
        case 0x08: 
        {
            if(_gAddrGetFlag == false)
            {
                
                if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_1) == 0 && DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_2) == 1)//1口为前级
                {
                    _gAddrOffset = p_buf[1];
                    _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].data_prep(0x08);
                    delay_1ms(30);
                    DevIOCtlOutSet(EN_DEV_IOCTL_IDX_JACK_OUT_2);
                    _gAddrGetFlag = true;
                    
                }
                else if(DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_1) == 1 && DevIOCtlInGet(EN_DEV_IOCTL_IDX_JACK_IN_2) == 0)//2口为前级
                {
                    _gAddrOffset = p_buf[1];
                    _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].data_prep(0x08);
                    delay_1ms(30);
                    DevIOCtlOutSet(EN_DEV_IOCTL_IDX_JACK_OUT_1);
                    _gAddrGetFlag = true;
                }
            }
            break;
        }
        case 0x0A:
        {
            CanProtoSlaveAddrSet(BMU_DEFAULT_ADDR - _gAddrOffset);
            _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].data_prep(0x0A);
            delay_1ms(10 * _gAddrOffset);
            break;
        }
        default:break;
    }
}
extern uint8_t NameID;
/** 
* @brief    BMU 分址发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void CanAddrDistrubuteTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    if(idx == 0x08)
    {
        buf[0] = 0x09;
        buf[1] = _gAddrOffset;
        buf[2] = NameID; 
//		buf[2] = 0x00; 
        buf[3] = 0x00; 
        buf[4] = 0x00; 
        buf[5] = *(uint8_t *)0x08007F04;
        buf[6] = *(uint8_t *)0x08007F05; 
        buf[7] = *(uint8_t *)0x08007F06;         
    }
    else if(idx == 0x0a)
    {
        buf[0] = 0x0b;
        buf[1] = _gAddrOffset;
        buf[2] = 0;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    }
        
    CanProtoTxIntoQueue(&_gCanProtoInfoAddrTx[EN_DIST_ADDR_TX], buf);
}


/** 
* @brief    BMU 掩码接收数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void CanAddrMaskRxProcess(uint8_t *p_buf)
{
    static uint8_t data_temp[64];
    static uint32_t rx_flag = 0;
    uint8_t mask_data_num = (CELL_VOLT_SAMPLE_MAX + 7) / 8 + ((PACK_TEMP_SAMPLE_MAX + 7) / 8) * 3 + (BALANCE_TEMP_SAMPLE_MAX + 7) / 8;
                            
    if((p_buf[0] == 0xF3) && (p_buf[1] == 0xF3) && (p_buf[2] == 0xF3) && (p_buf[3] == 0xF3))
    {
        if((p_buf[7] == mask_data_num) && ((rx_flag + 1) == (1 << p_buf[6])))
        {
            memcpy((uint8_t *)&gBMUDataInfo.bmu_addr_mask_info.cell_volt_mask[0], data_temp, mask_data_num);
            _gCanProtoInfoAddrTx[EN_DIST_MASK_TX].data_prep(0);
        }
    }
    else if((p_buf[0] * 7 + 7) <= mask_data_num + 6)//+6代表最后一帧可能是一个字节(一帧7个字节)
    {
        rx_flag |= (1 << p_buf[0]);
        memcpy(&data_temp[p_buf[0] * 7], &p_buf[1], 7); 
    }
}
/** 
* @brief    BMU 掩码发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void CanAddrMaskTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = 0x90;
    buf[1] = _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].can_id.ext_pdu.sa;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;     

    CanProtoTxIntoQueue(&_gCanProtoInfoAddrTx[EN_DIST_MASK_TX], buf);
}
