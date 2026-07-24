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

//#include "prot_can_addr_bms.h"
#include "AppCfg.h"
#include "systick.h"
#include "mcu_bsp.h"
#include "can_tx_rx_process.h"
#include "bms_message.h"


//#define SLAVE_ADDR_NUM  PACK_NUM

static uint8_t slave_addr = 0;
typedef enum
{
//    EN_TX_BROADCAST =0,
    EN_TX_CTRL = 0,
    EN_TX_BALANCE,
    
    EN_TX_BCU_TOTAL,
} EN_TX_CAN_BCU;
typedef enum
{
    EN_RX_STATE = 0,
    EN_RX_STATS_VOLT,
    EN_RX_STATS_TEMP,
    EN_RX_STATS_OTHER,
    
    EN_RX_DETAIL_VOLT,
    EN_RX_DETAIL_TEMP,
    
    EN_RX_BCU_TOTAL,
} EN_RX_CAN_BCU;


static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoBcuTx[] = 
{
    {0x1801F4B4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, BcuTxCtrlPrep, NULL}, 
    {0x1802F4B4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, BcuTxBalancePrep, tx_polling}, 
};

CanProtoListTxInfo_t gCanProtoListInfoBcuTx = 
{
    .p_proto_info = _gCanProtoInfoBcuTx,
    .num = EN_TX_BCU_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoBcuRx[] = 
{
    {0x1801B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuRxStateProcess, NULL}, 
    {0x1802B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuRxStatsVoltProcess, NULL}, 
    {0x1803B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuRxStatsTempProcess, NULL}, 
    {0x1804B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuRxStatsOtherProcess, NULL}, 
    {0x180AB4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuRxDetailVoltProcess, NULL}, 
    {0x180BB4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BcuRxDetailTempProcess, NULL}, 
};

CanProtoListRxInfo_t gCanProtoListInfoBcuRx = 
{
    .p_proto_info = _gCanProtoInfoBcuRx,
    .num = EN_RX_BCU_TOTAL
};


////////////////RX///////////////
/** 
* @brief    BMU上报的状态信息 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxStateProcess(uint8_t *p_buf)
{
    slave_addr = 0xF4 - gCanProtoListInfoBcuRx.p_proto_info->can_id.ext_pdu.ps;   //第一个从控板地址0xF4开始计算
       
    gPackMessage[slave_addr].com_state |= (1 << EN_RX_STATE);

}
/** 
* @brief    BMU上报的统计电压 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxStatsVoltProcess(uint8_t *p_buf)
{
    slave_addr = 0xF4 - gCanProtoListInfoBcuRx.p_proto_info->can_id.ext_pdu.ps;
    
    gPackMessage[slave_addr].total_voltage = (p_buf[0]<<8) + p_buf[1];
    gPackMessage[slave_addr].cell_max_num = p_buf[2];
    gPackMessage[slave_addr].cell_voltage_max = (p_buf[3]<<8) + p_buf[4];
    gPackMessage[slave_addr].cell_min_num = p_buf[5];
    gPackMessage[slave_addr].cell_voltage_min = (p_buf[6]<<8) + p_buf[7];
       
    gPackMessage[slave_addr].com_state |= (1 << EN_RX_STATS_VOLT);
}

/** 
* @brief    BMU上报的统计温度 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxStatsTempProcess(uint8_t *p_buf)
{
    slave_addr = 0xF4 - gCanProtoListInfoBcuRx.p_proto_info->can_id.ext_pdu.ps;
    
    gPackMessage[slave_addr].temp_max_num     = p_buf[0];
    gPackMessage[slave_addr].cell_temp_max    = p_buf[1];
    gPackMessage[slave_addr].temp_min_num     = p_buf[2];
    gPackMessage[slave_addr].cell_temp_min    = p_buf[3];
    
    gPackMessage[slave_addr].pole_temp_max_num    = p_buf[4];
    gPackMessage[slave_addr].pole_temp_max        = p_buf[5];
    gPackMessage[slave_addr].pole_temp_min_num    = p_buf[6];
    gPackMessage[slave_addr].pole_temp_min        = p_buf[7];

    
    gPackMessage[slave_addr].com_state |= (1<<EN_RX_STATS_TEMP);
}

/** 
* @brief    BMU上报的杂项 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxStatsOtherProcess(uint8_t *p_buf)
{
    slave_addr = 0xF4 - gCanProtoListInfoBcuRx.p_proto_info->can_id.ext_pdu.ps;
    
    gPackMessage[slave_addr].cell_voltage_average = (p_buf[0]<<8) + p_buf[1];
    gPackMessage[slave_addr].cell_temp_average    = p_buf[2];
    gPackMessage[slave_addr].balance_temp_max     = p_buf[3];
    gPackMessage[slave_addr].alarm_info.val       = (p_buf[6]<<8) + p_buf[7];
    
    gPackMessage[slave_addr].com_state |= (1<<EN_RX_STATS_OTHER);
}

/** 
* @brief    BMU上报的单体电芯电压 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxDetailVoltProcess(uint8_t *p_buf)
{
    uint8_t cnt = 0;
    uint8_t temp = 0;
    slave_addr = 0xF4 - gCanProtoListInfoBcuRx.p_proto_info->can_id.ext_pdu.ps;
    
    cnt = p_buf[0];
    for(uint8_t i = 0; i < 3; i++)
    {
        if((cnt * 3 + i) < PACK_CELL_NUM)
        {
            gPackMessage[slave_addr].cell_voltage[cnt * 3 + i] = (p_buf[i * 2 + 1] << 8) + p_buf[i * 2 + 2];
        }
    }
    if(p_buf[7] & 0x80)
    {
        gPackMessage[slave_addr].com_state |= (1 << EN_RX_DETAIL_VOLT);
        temp = slave_addr + 1;
        if(temp < PACK_NUM)
        {
            BcuTxCtrlPrep(EN_TX_CTRL);
        }
    }
}

/** 
* @brief    BMU上报的单体电芯温度 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxDetailTempProcess(uint8_t *p_buf)
{
    uint8_t cnt = 0;
    slave_addr = 0xF4 - gCanProtoListInfoBcuRx.p_proto_info->can_id.ext_pdu.ps;
    
    cnt = p_buf[0];
    for(uint8_t i = 0; i < 7; i++)
    {
        if((cnt * 7 + i) < PACK_CELL_TEMP_NUM)
        {
            gPackMessage[slave_addr].cell_temp[cnt *7 + i] = p_buf[i + 1];
        }
    }
    if((cnt+1)*7 >= PACK_CELL_TEMP_NUM)
    {
        gPackMessage[slave_addr].com_state |= (1<<EN_RX_DETAIL_TEMP);
    }
}
////////////////TX///////////////

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuTxCtrlPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = 3;
    buf[1] = IR_PackCtrlInfo->do_ctrl;   //DO输出
    buf[2] = IR_PackCtrlInfo->pwm_1_duty;   //PWM1
    buf[3] = IR_PackCtrlInfo->pwm_2_duty;   //PWM2
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoBcuTx[EN_TX_CTRL], buf);
}

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BcuTxBalancePrep(uint8_t idx)
{
    uint8_t buf[8];
    
    for(uint8_t i = 0; i < 8; i++)
    {
        buf[i] = IR_PackCtrlInfo->balance_pasv_ctrl[i];
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoBcuTx[EN_TX_BALANCE], buf);
}

static void tx_polling(uint8_t idx)
{
    switch(idx)
    {
        case EN_TX_BALANCE:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBcuTx[EN_TX_BALANCE].tx_regular_period)
            {
                BcuTxBalancePrep(EN_TX_BALANCE);
                time_count = tim_get_systime();
            }
            break;
        }
        default:break;
    }
}
