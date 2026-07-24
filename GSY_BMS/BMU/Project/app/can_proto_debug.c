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
#include "cmc_manage.h"
#include "fpara_asp.h"
#include "others.h"
#include "mcu_bsp.h"
#include "dev_spiflash.h"
#include "data_process.h"

typedef enum
{
    EN_TX_READ = 0,
    EN_TX_WRITE,
    
    EN_TX_DEBUG_TOTAL,
} EN_TX_CAN_DEBUG;

typedef enum
{
    EN_RX_READ = 0,
    EN_RX_WRITE,
    
    EN_RX_DEBUG_TOTAL,
} EN_RX_CAN_DEBUG;

static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoDebugTx[] = 
{
    {0x1900B4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 2, false, DebugTx_ReadPrep,  tx_polling}, 
    {0x1000B4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 2, false, DebugTx_WritePrep, NULL}, 
};

CanProtoListTxInfo_t gCanProtoListInfoDebugTx = 
{
    .p_proto_info = _gCanProtoInfoDebugTx,
    .num = EN_TX_DEBUG_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfodebugRx[] = 
{
    {0x1990F4B4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugRx_ReadProcess,  NULL}, 
    {0x1090F4B4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugRx_WriteProcess, NULL}, 
};

CanProtoListRxInfo_t gCanProtoListInfoDebugRx = 
{
    .p_proto_info = _gCanProtoInfodebugRx,
    .num = EN_RX_DEBUG_TOTAL
};

DebugReadType *DebugReadHead = NULL;
DebugWriteType *DebugWriteHead = NULL;
DebugCfgType DebugCfg = {0};

void DebugCanCfgInit(DebugReadType *const p_read, DebugWriteType *const p_write)
{   
    DebugReadHead = p_read;
    DebugWriteHead = p_write;
}


////////////////RX///////////////
/** 
* @brief    BMU BroadCast接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugRx_ReadProcess(uint8_t *p_buf)
{
    
}
/** 
* @brief    BMU RxCtrl 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugRx_WriteProcess(uint8_t *p_buf)
{

}

////////////////TX///////////////

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void DebugTx_ReadPrep(uint8_t addr)
{
    uint8_t buf[8];
       
    CanProtoTxIntoQueue(&_gCanProtoInfoDebugTx[EN_TX_DEBUG_TOTAL], buf);
}
/** 
* @brief    BMU 电压统计 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void DebugTx_WritePrep(uint8_t addr)
{
    uint8_t buf[8];
    
    buf[0] = gIR_BmsCellInfo->total_voltage>>8;
    buf[1] = gIR_BmsCellInfo->total_voltage;
    buf[2] = gIR_BmsCellInfo->CellMaxIDX;
    buf[3] = gIR_BmsCellInfo->CellVoltageMax>>8;
    buf[4] = gIR_BmsCellInfo->CellVoltageMax;
    buf[5] = gIR_BmsCellInfo->CellMinIDX;
    buf[6] = gIR_BmsCellInfo->CellVoltageMin>>8;
    buf[7] = gIR_BmsCellInfo->CellVoltageMin;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoDebugTx[EN_TX_DEBUG_TOTAL], buf);
}

static void tx_polling(uint8_t idx)
{
    
    switch(idx)
    {
        case EN_TX_READ:
        {
            static uint32_t time_count = 0;
            if(_gCanProtoInfoDebugTx[EN_TX_READ].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoDebugTx[EN_TX_READ].tx_regular_period)
                {
                    DebugTx_ReadPrep(EN_TX_READ);
                    time_count = tim_get_systime();
                }            
            }
            break;
        }
        default:break;
    }
}
