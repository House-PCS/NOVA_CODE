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
#include "others.h"
#include "bms_sox.h"
#include "bms_alarm_cfg.h"
#include "bms_sample_res_bsp.h"
#include "autorun.h"
#include "fpara_asp.h"

//#define SLAVE_ADDR_NUM  PACK_NUM
typedef enum
{
    EN_TX_PCS1 = 0,
    EN_TX_PCS2,
    EN_TX_PCS3,
    EN_TX_PCS4,
    EN_TX_PCS5,
    EN_TX_PCS6,
    EN_TX_PCS7,
    EN_TX_PCS8,
    EN_TX_PCS9,
    EN_TX_PCSA,
    EN_TX_PCSB,
    EN_TX_PCSC,
    EN_TX_PCSD,
    EN_TX_PCSE,
    
    EN_TX_PCS_TOTAL,
} EN_TX_CAN_PCS;
typedef enum
{
    EN_RX_PCS = 0,
    
    EN_RX_PCS_TOTAL,
} EN_RX_CAN_BCU;


static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoPcsTx[] = 
{
    {0x180150F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_01Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_02Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_03Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_04Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_05Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_06Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_07Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_08Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_09Prep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_0APrep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_0BPrep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_0CPrep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_0DPrep, tx_polling}, 
    {0x180250F1, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, PcsTx_0EPrep, tx_polling}, 
};

CanProtoListTxInfo_t gCanProtoListInfoPcsTx = 
{
    .p_proto_info = _gCanProtoInfoPcsTx,
    .num = EN_TX_PCS_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoPcsRx[] = 
{
    {0x1801F150, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, PcsRxProcess, NULL},  
};

CanProtoListRxInfo_t gCanProtoListInfoPcsRx = 
{
    .p_proto_info = _gCanProtoInfoPcsRx,
    .num = EN_RX_PCS_TOTAL
};


////////////////RX///////////////
/** 
* @brief    pcs下发并簇指令 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BcuRxStateProcess(uint8_t *p_buf)
{
    
    if((IR_EnPowerOn & 0x01) == 0)
    {
        if(p_buf[1] == 1)
        {
            IR_EnPowerOnSet(IR_EnPowerOn | 0x01);
        }
    }
    else
    {
        if(p_buf[2] == 1)
        {
            IR_EnPowerOnSet(IR_EnPowerOn & (~0x01));
        }
    }
    IR_PcsComTimeOutSet(false);

}
////////////////TX///////////////

/** 
* @brief    向pcs上报详细电芯数据 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void PcsTx_01Prep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = IR_BmsMessage->cell_voltage_max >> 8;
    buf[1] = IR_BmsMessage->cell_voltage_max;
    buf[2] = IR_BmsMessage->cell_voltage_min >> 8;
    buf[3] = IR_BmsMessage->cell_voltage_min;
    buf[4] = IR_SysInfo->soc / 10;
    buf[5] = IR_SysInfo->soh / 10;
    buf[7] = IR_SysInfo->relay_km_state;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS1], buf);
}
void PcsTx_02Prep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t temp_u16 = 0;
    
    buf[0] = IR_BmsMessage->total_voltage >> 8;
    buf[1] = IR_BmsMessage->total_voltage;
    temp_u16 = -IR_SysInfo->current;
    buf[2] = temp_u16 >> 8;
    buf[3] = temp_u16;
    buf[4] = IR_LimitCurr->charge_limit_curr >> 8;
    buf[5] = IR_LimitCurr->charge_limit_curr;
    buf[6] = IR_LimitCurr->discharge_limit_curr >> 8;
    buf[7] = IR_LimitCurr->discharge_limit_curr;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS2], buf);
}
void PcsTx_03Prep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = IR_PARA_TotalVolHigh_II >> 8;
    buf[1] = IR_PARA_TotalVolHigh_II;
    buf[2] = IR_PARA_TotalVolLow_II >> 8;
    buf[3] = IR_PARA_TotalVolLow_II;
    buf[4] = IR_BmsMessage->cell_temp_max + 40;
    buf[5] = IR_BmsMessage->cell_temp_min + 40;
    buf[6] = IR_BmsMessage->cell_temp_average + 40;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS3], buf);
}
void PcsTx_04Prep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t temp_u16 = 0;
    
    temp_u16 = (IR_BmsMessage->cell_max_pack_num - 1) * PACK_CELL_NUM + IR_BmsMessage->cell_max_num;
    buf[0] = temp_u16 >> 8;
    buf[1] = temp_u16;
    temp_u16 = (IR_BmsMessage->cell_min_pack_num - 1) * PACK_CELL_NUM + IR_BmsMessage->cell_min_num;
    buf[2] = temp_u16 >> 8;
    buf[3] = temp_u16;
    temp_u16 = (IR_BmsMessage->temp_max_pack_num - 1) * PACK_CELL_NUM + IR_BmsMessage->temp_max_num;
    buf[4] = temp_u16 >> 8;
    buf[5] = temp_u16;
    temp_u16 = (IR_BmsMessage->temp_min_pack_num - 1) * PACK_CELL_NUM + IR_BmsMessage->temp_min_num;
    buf[6] = temp_u16 >> 8;
    buf[7] = temp_u16;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS4], buf);
}
void PcsTx_05Prep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = IR_BmsMessage->cell_voltage_average>>8;
    buf[1] = IR_BmsMessage->cell_voltage_average;
    buf[2] = 0;
    buf[3] = IR_BmsSampleRes->sample_en;      //绝缘检测状态
    buf[4] = IR_BmsSampleRes->res_p>>8;
    buf[5] = IR_BmsSampleRes->res_p;
    buf[6] = IR_BmsSampleRes->res_n>>8;        
    buf[7] = IR_BmsSampleRes->res_n;
//    buf[6] = IR_BmsSampleRes->res_p>>8;      //临时调试，都上报kp值
//    buf[7] = IR_BmsSampleRes->res_p;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS5], buf);
}
void PcsTx_06Prep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t temp_u8 = 0;
    
    if(IR_SysInfo->sys_state == SYS_FAULT)
    {
        if((IR_LimitCurr->charge_limit_curr == 0) && (IR_LimitCurr->discharge_limit_curr == 0))
        {
            temp_u8 = 1;
        }
        else if(IR_LimitCurr->charge_limit_curr == 0)
        {
            temp_u8 = 2;
        }
        else if(IR_LimitCurr->discharge_limit_curr == 0)
        {
            temp_u8 = 3;
        }
    }
    else if(IR_SysInfo->sys_state == SYS_CHARGE)
    {
        temp_u8 = 4;
    }
    else if(IR_SysInfo->sys_state == SYS_DISCHARGE)
    {
        temp_u8 = 5;
    }
    buf[0] = temp_u8;
    temp_u8 = 0;
    if(IR_RelayKmState)
    {
        temp_u8 |= 1<<0;
    }
    if(IR_LimitCurr->finish_charge_flag)
    {
        temp_u8 |= 1<<1;
    }
    if(IR_LimitCurr->finish_discharge_flag)
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmI_Code)
    {
        temp_u8 |= 1<<3;
    }
    if(IR_AlarmII_Code)
    {
        temp_u8 |= 1<<4;
    }
    if(IR_AlarmIII_Code)
    {
        temp_u8 |= 1<<5;
    }
    buf[1] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmI_Code & (1<<ALARM_III_DISCHARGE_CURR_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_RES_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_CELL_VOL_DIFF))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_CELL_TEMP_DIFF))
    {
        temp_u8 |= 1<<3;
    }
    buf[2] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmI_Code & (1<<ALARM_III_TOTAL_VOL_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_TOTAL_VOL_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_TOTAL_VOL_HIGH))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_TOTAL_VOL_LOW))
    {
        temp_u8 |= 1<<3;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_CHARGE_CURR_HIGH))
    {
        temp_u8 |= 1<<7;
    }
    buf[3] = temp_u8;
    temp_u8 = 0;    //并簇前需要报0
    if(IR_AutoRunInfo->autorun_main_state == AUTORUN_MAIN_ERR)
    {
        temp_u8 = 255;
    }
    else if(IR_AutoRunInfo->autorun_main_state == AUTORUN_MAIN_RUNING)
    {
        temp_u8 = 7;
    }
    buf[4] = temp_u8;
    temp_u8 = 0;
    buf[5] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmI_Code & (1<<ALARM_III_CELL_VOL_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_CELL_VOL_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_TEMP_CHARGE_HIGH | 1<<ALARM_III_TEMP_DISCHARGE_HIGH))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmI_Code & (1<<ALARM_III_TEMP_CHARGE_LOW | 1<<ALARM_III_TEMP_DISCHARGE_LOW))
    {
        temp_u8 |= 1<<3;
    }
    buf[6] = temp_u8;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS6], buf);
}
void PcsTx_07Prep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t temp_u8 = 0;
    
    temp_u8 = 0;
    if(IR_AlarmII_Code & (1<<ALARM_III_DISCHARGE_CURR_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_RES_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_CELL_VOL_DIFF))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_CELL_TEMP_DIFF))
    {
        temp_u8 |= 1<<3;
    }
    buf[0] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmII_Code & (1<<ALARM_III_TOTAL_VOL_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_TOTAL_VOL_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_TOTAL_VOL_HIGH))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_TOTAL_VOL_LOW))
    {
        temp_u8 |= 1<<3;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_CHARGE_CURR_HIGH))
    {
        temp_u8 |= 1<<7;
    }
    buf[1] = temp_u8;
    temp_u8 = 0;
    buf[2] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmII_Code & (1<<ALARM_III_CELL_VOL_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_CELL_VOL_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_TEMP_CHARGE_HIGH | 1<<ALARM_III_TEMP_DISCHARGE_HIGH))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmII_Code & (1<<ALARM_III_TEMP_CHARGE_LOW | 1<<ALARM_III_TEMP_DISCHARGE_LOW))
    {
        temp_u8 |= 1<<3;
    }
    buf[3] = temp_u8;
    
    temp_u8 = 0;
    if(IR_AlarmIII_Code & (1<<ALARM_III_DISCHARGE_CURR_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_RES_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_CELL_VOL_DIFF))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_CELL_TEMP_DIFF))
    {
        temp_u8 |= 1<<3;
    }
    buf[4] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmIII_Code & (1<<ALARM_III_TOTAL_VOL_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_TOTAL_VOL_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_TOTAL_VOL_HIGH))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_TOTAL_VOL_LOW))
    {
        temp_u8 |= 1<<3;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_CHARGE_CURR_HIGH))
    {
        temp_u8 |= 1<<7;
    }
    buf[5] = temp_u8;
    temp_u8 = 0;
    buf[6] = temp_u8;
    temp_u8 = 0;
    if(IR_AlarmIII_Code & (1<<ALARM_III_CELL_VOL_HIGH))
    {
        temp_u8 |= 1<<0;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_CELL_VOL_LOW))
    {
        temp_u8 |= 1<<1;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_TEMP_CHARGE_HIGH | 1<<ALARM_III_TEMP_DISCHARGE_HIGH))
    {
        temp_u8 |= 1<<2;
    }
    if(IR_AlarmIII_Code & (1<<ALARM_III_TEMP_CHARGE_LOW | 1<<ALARM_III_TEMP_DISCHARGE_LOW))
    {
        temp_u8 |= 1<<3;
    }
    buf[7] = temp_u8;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS7], buf);
}
void PcsTx_08Prep(uint8_t idx)
{
    uint8_t buf[8];
    uint16_t temp_u16 = 0;
    
    temp_u16 = IR_BmsSoc->charge_capacity_kwh / 10;
    buf[0] = temp_u16>>8;
    buf[1] = temp_u16;
    temp_u16 = IR_BmsSoc->discharge_capacity_kwh / 10;
    buf[2] = temp_u16>>8;
    buf[3] = temp_u16;
    buf[4] = IR_BmsSoc->cycles >> 8;
    buf[5] = IR_BmsSoc->cycles;
    buf[6] = IR_BmsSoc->cycles >> 8;
    buf[7] = IR_BmsSoc->cycles;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS8], buf);
}
void PcsTx_09Prep(uint8_t idx)
{
    uint8_t buf[8];
    uint32_t temp_u32 = 0;
    
    temp_u32 = IR_BmsSoc->total_accum_kwh_charge / 10;
    buf[0] = temp_u32>>24;
    buf[1] = temp_u32>>16;
    buf[2] = temp_u32>>8;
    buf[3] = temp_u32;
    temp_u32 = IR_BmsSoc->total_accum_kwh_discharge / 10;
    buf[4] = temp_u32>>24;
    buf[5] = temp_u32>>16;
    buf[6] = temp_u32>>8;
    buf[7] = temp_u32;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCS9], buf);
}
void PcsTx_0APrep(uint8_t idx)
{
    uint8_t buf[8];
    uint32_t temp_u32 = 0;
    
    temp_u32 = IR_BmsSoc->accum_q_charge / 10;
    buf[0] = temp_u32>>24;
    buf[1] = temp_u32>>16;
    buf[2] = temp_u32>>8;
    buf[3] = temp_u32;
    temp_u32 = IR_BmsSoc->accum_q_discharge / 10;
    buf[4] = temp_u32>>24;
    buf[5] = temp_u32>>16;
    buf[6] = temp_u32>>8;
    buf[7] = temp_u32;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCSA], buf);
}
void PcsTx_0BPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = 40;
    buf[1] = 40;
    buf[2] = 40;
    buf[3] = 40;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = 0;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCSB], buf);
}
void PcsTx_0CPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    buf[0] = PACK_NUM>>8;
    buf[1] = PACK_NUM;
    buf[2] = CELL_NUM>>8;
    buf[3] = CELL_NUM;
    buf[4] = CELL_TEMP_NUM >> 8;
    buf[5] = CELL_TEMP_NUM;
    buf[6] = 0 >> 8;
    buf[7] = 0;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCSC], buf);
}
void PcsTx_0DPrep(uint8_t idx)
{
    uint8_t buf[8];
    static uint16_t cnt_cell = 0;
    
    _gCanProtoInfoPcsTx[EN_TX_PCSD].tx_regular_period = 10;
    buf[0] = cnt_cell>>8;
    buf[1] = cnt_cell;
    for(uint8_t i = 0; i < 3; i++)
    {
        if((cnt_cell + i) < CELL_NUM)
        {
            buf[i * 2 + 2] = IR_BmsMessage->cell_voltage[cnt_cell + i] >> 8;
            buf[i * 2 + 3] = IR_BmsMessage->cell_voltage[cnt_cell + i];
        }
        else
        {
            buf[i * 2 + 1] = 0xFF;
            buf[i * 2 + 2] = 0xFF;
            
        }       
    }   
    cnt_cell += 3;
    if(cnt_cell >= CELL_NUM)
    {
        _gCanProtoInfoPcsTx[EN_TX_PCSD].tx_regular_period = 2000;
        cnt_cell = 0;
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCSD], buf);
}
void PcsTx_0EPrep(uint8_t idx)
{
    uint8_t buf[8];
    static uint16_t cnt = 0;
    
    _gCanProtoInfoPcsTx[EN_TX_PCSE].tx_regular_period = 10;
    buf[0] = cnt>>8;
    buf[1] = cnt;
    for(uint8_t i = 0; i < 6; i++)
    {
        if((cnt + i) < CELL_TEMP_NUM)
        {
            buf[i + 2] = IR_BmsMessage->cell_temp[cnt + i] + 40;
        }
        else
        {
            buf[i + 2] = 0xFF;
        }       
    } 
    cnt += 6;
    if(cnt >= CELL_TEMP_NUM)
    {
        _gCanProtoInfoPcsTx[EN_TX_PCSE].tx_regular_period = 2000;
        cnt = 0;
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoPcsTx[EN_TX_PCSE], buf);
}
static void tx_polling(uint8_t idx)
{
    switch(idx)
    {
        case EN_TX_PCS1:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS1].tx_regular_period)
            {
                PcsTx_01Prep(EN_TX_PCS1);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS2:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS2].tx_regular_period)
            {
                PcsTx_02Prep(EN_TX_PCS2);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS3:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS3].tx_regular_period)
            {
                PcsTx_03Prep(EN_TX_PCS3);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS4:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS4].tx_regular_period)
            {
                PcsTx_04Prep(EN_TX_PCS4);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS5:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS5].tx_regular_period)
            {
                PcsTx_05Prep(EN_TX_PCS5);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS6:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS6].tx_regular_period)
            {
                PcsTx_06Prep(EN_TX_PCS6);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS7:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS7].tx_regular_period)
            {
                PcsTx_07Prep(EN_TX_PCS7);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS8:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS8].tx_regular_period)
            {
                PcsTx_08Prep(EN_TX_PCS8);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCS9:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCS9].tx_regular_period)
            {
                PcsTx_09Prep(EN_TX_PCS9);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCSA:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCSA].tx_regular_period)
            {
                PcsTx_0APrep(EN_TX_PCSA);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCSB:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCSB].tx_regular_period)
            {
                PcsTx_0BPrep(EN_TX_PCSB);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCSC:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCSC].tx_regular_period)
            {
                PcsTx_0CPrep(EN_TX_PCSC);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCSD:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCSD].tx_regular_period)
            {
                PcsTx_0DPrep(EN_TX_PCSD);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_PCSE:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoPcsTx[EN_TX_PCSE].tx_regular_period)
            {
                PcsTx_0EPrep(EN_TX_PCSE);
                time_count = tim_get_systime();
            }
            break;
        }
        
        default:break;
    }
}
