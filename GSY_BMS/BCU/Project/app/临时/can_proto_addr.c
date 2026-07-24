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

#include "AppCfg.h"
#include "systick.h"
#include "mcu_bsp.h"
#include "can_tx_rx_process.h"

typedef enum
{
    EN_CLEAR_ADDR_TX = 0,
    EN_DIST_ADDR_TX,
    
    EN_TX_ADDR_TOTAL,
} EN_TX_CAN_ADDR;

typedef enum
{
    EN_CLEAR_ADDR_RX = 0,
    EN_DIST_ADDR_RX,
    
    EN_RX_ADDR_TOTAL,
} EN_RX_CAN_ADDR;

typedef enum
{
    EN_BMS_ADDR_CLEAR = 0,
    EN_BMS_ADDR_ALLOT,
    EN_BMS_ADDR_LATCH,
    EN_BMS_ADDR_ERR,
    EN_BMS_ADDR_OVER,
} EN_BMS_ADDR_DISTRIBUTE;

static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoAddrTx[] = 
{
    {0x1899FF99, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, false, CanAddrDistrubuteTxPrep, tx_polling}, 
     
};

CanProtoListTxInfo_t gCanProtoListInfoAddrTx = 
{
    .p_proto_info = _gCanProtoInfoAddrTx,
    .num = EN_TX_ADDR_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoAddrRx[] = 
{
    {0x18999998, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, CanAddrDistrubuteRxProcess, NULL}, 

};

CanProtoListRxInfo_t gCanProtoListInfoAddrRx = 
{
    .p_proto_info = _gCanProtoInfoAddrRx,
    .num = EN_RX_ADDR_TOTAL
};


EN_BMS_ADDR_DISTRIBUTE PackAddrState = EN_BMS_ADDR_CLEAR;

////////////////RX///////////////
//static uint8_t _gAddrOffset;//主控发下来的地址偏移，基于0xF4-_gAddrOffset，就是从控被分配的实际地址
static EN_BMS_ADDR_DISTRIBUTE gBmsAddrState = EN_BMS_ADDR_ALLOT;
static uint8_t temp_addr = 0;

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
        case 0x09:
        {
            if((p_buf[1] == temp_addr) && (p_buf[2] == 0))
            {
                temp_addr++;
                if(temp_addr >= PACK_NUM)
                {
                    gBmsAddrState = EN_BMS_ADDR_LATCH;
                }   
            }
            CanAddrDistrubuteTxPrep(EN_DIST_ADDR_TX); 
            break;
        }
        case 0x0B:
        {
            //CanProtoSlaveAddrSet(BMU_DEFAULT_ADDR - _gAddrOffset);
            _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].data_prep(0x0B);
            //delay_1ms(10 * _gAddrOffset);
            break;
        }
        default:break;
    }
}


////////////////TX///////////////
/** 
* @brief    BMU 分址发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void CanAddrDistrubuteTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    memset(buf, 7, 8);
    switch(gBmsAddrState)
    {
        case EN_BMS_ADDR_CLEAR:
        {
            static uint32_t time_count = 0;
            
            buf[0] = 0x0C;
            buf[1] = 0x00;
            buf[2] = 0x00;
            buf[3] = 0x00;
            buf[4] = 0x00;
            buf[5] = 0x00;
            buf[6] = 0x00;
            buf[7] = 0x00;
            
            CanProtoTxIntoQueue(&_gCanProtoInfoAddrTx[EN_DIST_ADDR_TX], buf);
            
            if((uint32_t)(tim_get_systime() - time_count) >= 3000)
            {
                gBmsAddrState = EN_BMS_ADDR_ALLOT;
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_BMS_ADDR_ALLOT:
        {
            buf[0] = 8;
            buf[1] = temp_addr;
            if(temp_addr == 0)
            {
                DevIOCtlOutSet(EN_DEV_IOCTL_IDX_2WAY_OUT_1);
            }
            CanProtoTxIntoQueue(&_gCanProtoInfoAddrTx[EN_DIST_ADDR_TX], buf);
            
            break;
        }
        case EN_BMS_ADDR_LATCH:
        {
            buf[0] = 0x0A;
            buf[1] = 0;
            gBmsAddrState = EN_BMS_ADDR_OVER;
            break;
        }
        case EN_BMS_ADDR_ERR:
        {
            break;
        }
        case EN_BMS_ADDR_OVER:
        {
            _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].regular_default_enable = false;
            break;
        }
        default : break;
    }  
}

static void tx_polling(uint8_t idx)
{
    switch(idx)
    {
        case EN_DIST_ADDR_TX:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoAddrTx[EN_DIST_ADDR_TX].tx_regular_period)
            {
                CanAddrDistrubuteTxPrep(EN_DIST_ADDR_TX);
                time_count = tim_get_systime();
            }
            break;
        }
        
        default:break;
    }
}
