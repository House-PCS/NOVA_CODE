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


typedef enum
{
    EN_TX_MAIN = 0,

    EN_TX_PARA_TOTAL,
} EN_TX_CAN_PARA;

typedef enum
{
    EN_RX_MAIN = 0,
    
    EN_RX_PARA_TOTAL,
} EN_RX_CAN_PARA;

static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoParaTx[] = 
{
    {0x189099F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, ParaTxMainPrep, NULL}, 

};

CanProtoListTxInfo_t gCanProtoListInfoParaTx = 
{
    .p_proto_info = _gCanProtoInfoParaTx,
    .num = EN_TX_PARA_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoParaRx[] = 
{
    {0x1890FF99, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParaRxMainProcess, NULL}, 

};

CanProtoListRxInfo_t gCanProtoListInfoParaRx = 
{
    .p_proto_info = _gCanProtoInfoParaRx,
    .num = EN_RX_PARA_TOTAL
};


////////////////RX///////////////
/** 
* @brief    BMU BroadCast接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
#define DATA_BUFF_MAX   (PACK_PARA_NUM + 6)
uint8_t RxParaBuff[DATA_BUFF_MAX] = {0};
uint32_t RxFlag = 0;
void ParaRxMainProcess(uint8_t *p_buf)
{
    uint8_t temp_u8 = 0, i = 0;
    
    if((p_buf[0] == 0xF3) && (p_buf[1] == 0xF3) && (p_buf[2] == 0xF3) && (p_buf[3] == 0xF3))
    {
        if((p_buf[7] == PACK_PARA_NUM) && ((RxFlag + 1) == (1 << p_buf[6])))
        {
            memcpy((void*)&IR_BmsPara->EnParaInfo, RxParaBuff, PACK_PARA_NUM);
            FParaSaveSet(EN_FPARA_WRITE_BCU);
            CellTempValidNumcalCulate();
            _gCanProtoInfoParaTx[EN_TX_MAIN].data_prep(0);
        }
    }
    else if((p_buf[0] * 7 + 7) <= DATA_BUFF_MAX)
    {
        RxFlag |= (1 << p_buf[0]);
        memcpy(&RxParaBuff[p_buf[0] * 7], &p_buf[1], 7); 
    }
    
}

////////////////TX///////////////

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParaTxMainPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = 0x90;
    buf[1] = _gCanProtoInfoParaTx[EN_TX_MAIN].can_id.ext_pdu.sa;
    buf[2] = 0;
    buf[3] = 0;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoParaTx[EN_TX_MAIN], buf);
}

static void tx_polling(uint8_t idx)
{
    
    switch(idx)
    {
        case EN_TX_MAIN:
        {
            static uint32_t time_count = 0;
            if(_gCanProtoInfoParaTx[EN_TX_MAIN].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoParaTx[EN_TX_MAIN].tx_regular_period)
                {
                    ParaTxMainPrep(EN_TX_MAIN);
                    time_count = tim_get_systime();
                }            
            }
            break;
        }
        default:break;
    }
}
