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
    EN_TX_STATE = 0,
    EN_TX_STATS_VOLT,
    EN_TX_STATS_TEMP,
    EN_TX_STATS_OTHER,

    EN_TX_DETAIL_VOLT,
    EN_TX_DETAIL_TEMP,

    EN_TX_BMS_TOTAL,
} EN_TX_CAN_BMS;

typedef enum
{
    EN_RX_BROADCAST = 0,
    EN_RX_CTRL,
    EN_RX_BALANCE,
    
    EN_RX_BMS_TOTAL,
} EN_RX_CAN_BMS;

static void tx_polling(uint8_t idx);

static CanProtoTxInfo_t _gCanProtoInfoBmsTx[] = 
{
    {0x1801B4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, BmsTxStatePrep,           tx_polling}, 
    {0x1802B4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, BmsTxStatsVoltPrep,       tx_polling}, 
    {0x1803B4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, BmsTxStatsTempPrep,       tx_polling}, 
    {0x1804B4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, true, BmsTxStatsOtherPrep,      tx_polling}, 
                                                                                                 
    {0x180AB4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, false, BmsTxDetailVoltPrep,     tx_polling}, 
    {0x180BB4F4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_REGULAR, 1000, false, BmsTxDetailTempPrep,     tx_polling}, 
};

CanProtoListTxInfo_t gCanProtoListInfoBmsTx = 
{
    .p_proto_info = _gCanProtoInfoBmsTx,
    .num = EN_TX_BMS_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoBmsRx[] = 
{
    {0x1800F4B4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmsRxBroadCastProcess,     NULL}, 
    {0x1801F4B4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmsRxCtrlProcess,          NULL}, 
    {0x1802F4B4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BmsRxBalanceProcess,       NULL}, 

};

CanProtoListRxInfo_t gCanProtoListInfoBmsRx = 
{
    .p_proto_info = _gCanProtoInfoBmsRx,
    .num = EN_RX_BMS_TOTAL
};



////////////////TX///////////////

/** 
* @brief    BMU TxState发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmsTxStatePrep(uint8_t idx)
{
    uint8_t temp_u8 = 0, i = 0;
    uint8_t buf[8];
    
    temp_u8 = 0;//当前从控没有DI
    buf[0] = temp_u8;
    temp_u8 = 0;//当前从控没有DO
    buf[1] = temp_u8;
    buf[2] = gBMUDataInfo.fan_duty;
    buf[3] = 0;//当前从控没有PWM2输出
    buf[4] = gBMUDataInfo.fan_rd_duty;
//    buf[5] = 0;//当前从控没有PWM2输入
	buf[5] = gAFESampleInfo.balance_error_num;
//	buf[6] = 0;
//    buf[7] = 0;
	buf[6] = gAFESampleInfo.bt_error_num;
    buf[7] = gAFESampleInfo.cell_t_error_num;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoBmsTx[EN_TX_STATE], buf);
}
/** 
* @brief    BMU 电压统计 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmsTxStatsVoltPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    if(gBMUDataInfo.bmu_statis_volt_info.valid_flag != true)
    {
        return;
    }
    buf[0] = (gBMUDataInfo.bmu_statis_volt_info.total_volt / 100) >> 8;
    buf[1] = (gBMUDataInfo.bmu_statis_volt_info.total_volt / 100);
    buf[2] = gBMUDataInfo.bmu_statis_volt_info.cell_max_volt_num;
    buf[3] = (gBMUDataInfo.bmu_statis_volt_info.cell_max_volt >> 8);
    buf[4] = gBMUDataInfo.bmu_statis_volt_info.cell_max_volt;
    buf[5] = gBMUDataInfo.bmu_statis_volt_info.cell_min_volt_num;
    buf[6] = (gBMUDataInfo.bmu_statis_volt_info.cell_min_volt >> 8);
    buf[7] = gBMUDataInfo.bmu_statis_volt_info.cell_min_volt;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoBmsTx[EN_TX_STATS_VOLT], buf);
}

/** 
* @brief    BMU 温度统计 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmsTxStatsTempPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    if(gBMUDataInfo.bmu_statis_temp_info.valid_flag != true)
    {
        return;
    }
    buf[0] = gBMUDataInfo.bmu_statis_temp_info.cell_max_temp_num;
    buf[1] = gBMUDataInfo.bmu_statis_temp_info.cell_max_temp;
    buf[2] = gBMUDataInfo.bmu_statis_temp_info.cell_min_temp_num;
    buf[3] = gBMUDataInfo.bmu_statis_temp_info.cell_min_temp;
    buf[4] = gBMUDataInfo.bmu_statis_temp_info.pole_max_temp_num;
    buf[5] = gBMUDataInfo.bmu_statis_temp_info.pole_max_temp;
    buf[6] = gBMUDataInfo.bmu_statis_temp_info.pole_min_temp_num;
    buf[7] = gBMUDataInfo.bmu_statis_temp_info.pole_min_temp;

    CanProtoTxIntoQueue(&_gCanProtoInfoBmsTx[EN_TX_STATS_TEMP], buf);
}

/** 
* @brief    BMU Other 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmsTxStatsOtherPrep(uint8_t idx)
{
    uint8_t buf[8];
    
//    if(gBMUDataInfo.bmu_statis_other_info.valid_flag)
//    {
//        return;
//    }
    buf[0] = gBMUDataInfo.bmu_statis_other_info.cell_ave_volt >> 8;
    buf[1] = gBMUDataInfo.bmu_statis_other_info.cell_ave_volt;
    buf[2] = gBMUDataInfo.bmu_statis_other_info.cell_ave_temp;
    buf[3] = gBMUDataInfo.bmu_statis_other_info.equalizing_plate_max_temp;
    buf[4] = 0;
    buf[5] = 0;
    buf[6] = gBMUDataInfo.bmu_statis_other_info.fault_info >> 8;
    buf[7] = gBMUDataInfo.bmu_statis_other_info.fault_info;
    
    CanProtoTxIntoQueue(&_gCanProtoInfoBmsTx[EN_TX_STATS_OTHER], buf);
}

/** 
* @brief    BMU 单体电压 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmsTxDetailVoltPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    if(gBMUDataInfo.bmu_detail_volt_info.valid_flag != true)
    {
        return;
    }
    buf[0] = idx; 
    buf[7] = 0;
    
	for(uint8_t i = 0; i < 3; i++)
	{
		if(idx * 3 + i < gBMUDataInfo.per_bmu_cell_num)
		{
			buf[i * 2 + 1] = gBMUDataInfo.bmu_detail_volt_info.cell_volt[idx * 3 + i] >> 8;
            buf[i * 2 + 2] = gBMUDataInfo.bmu_detail_volt_info.cell_volt[idx * 3 + i];
            if(gBMUDataInfo.bmu_detail_volt_info.cell_balance_stat[idx * 3 + i] == true)
            {
                buf[7] |= (1 << i);
            }
		}
		else
		{
			buf[i * 2 + 1] = 0xFF;
            buf[i * 2 + 2] = 0xFF; 
		}
	}
    
    if(idx * 3 + 3 >= gBMUDataInfo.per_bmu_cell_num)//最后一帧
    {
        buf[7] |= (1 << 7);
    }
    
	CanProtoTxIntoQueue(&_gCanProtoInfoBmsTx[EN_TX_DETAIL_VOLT], buf);         
}

/** 
* @brief    BMU 单体温度 发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void BmsTxDetailTempPrep(uint8_t idx)
{
    uint8_t buf[8];
	
    if(gBMUDataInfo.bmu_detail_temp_info.valid_flag != true)
    {
        return;
    }
    buf[0] = idx; 
	for(uint8_t i = 0; i < 7; i++)
	{
		if(idx * 7 + i < gBMUDataInfo.per_bmu_cell_temp_num)
		{
			buf[i + 1] = gBMUDataInfo.bmu_detail_temp_info.cell_temp[idx * 7 + i];
		}
		else
		{
			buf[i + 1] = 0xFF;
		}
	}
    
	CanProtoTxIntoQueue(&_gCanProtoInfoBmsTx[EN_TX_DETAIL_TEMP], buf);   
}
////////////////RX///////////////
/** 
* @brief    BMU BroadCast接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmsRxBroadCastProcess(uint8_t *p_buf)
{
    
}
/** 
* @brief    BMU RxCtrl 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmsRxCtrlProcess(uint8_t *p_buf)
{
    if((p_buf[0] & 0x01) == 0x01)
    {
        //单体电压
        _gCanProtoInfoBmsTx[EN_TX_DETAIL_VOLT].regular_default_enable = true; 
    }
    if((p_buf[0] & 0x02) == 0x02)
    {    
        //单体温度
        _gCanProtoInfoBmsTx[EN_TX_DETAIL_TEMP].regular_default_enable = true;
    }
    
    if(gBMUDataInfo.fan_duty != p_buf[2])
    {
        gBMUDataInfo.fan_duty = p_buf[2];
        DevPwmOutDutySet(gBMUDataInfo.fan_duty);
    }
}

/** 
* @brief    BMU RxBalance 接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BmsRxBalanceProcess(uint8_t *p_buf)
{
    //BalanceCtrlSet(p_buf);
}

static void tx_polling(uint8_t idx)
{
    
    switch(idx)
    {
        case EN_TX_STATE:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBmsTx[EN_TX_STATE].tx_regular_period)
            {
                BmsTxStatePrep(EN_TX_STATE);
                time_count = tim_get_systime();
            }
            
            break;
        }
        case EN_TX_STATS_VOLT:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBmsTx[EN_TX_STATS_VOLT].tx_regular_period)
            {
                if(gBMUDataInfo.bmu_statis_volt_info.valid_flag != true)
                {
                    break;
                }
                BmsTxStatsVoltPrep(EN_TX_STATS_VOLT); 
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_STATS_TEMP:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBmsTx[EN_TX_STATS_TEMP].tx_regular_period)
            {
                if(gBMUDataInfo.bmu_statis_temp_info.valid_flag != true)
                {
                    break;
                }
                BmsTxStatsTempPrep(EN_TX_STATS_TEMP);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_STATS_OTHER:
        {
            static uint32_t time_count = 0;
            if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBmsTx[EN_TX_STATS_OTHER].tx_regular_period)
            {
//                if(gBMUDataInfo.bmu_statis_other_info.valid_flag != true)
//                {
//                    break;
//                }
                BmsTxStatsOtherPrep(EN_TX_STATS_OTHER);
                time_count = tim_get_systime();
            }
            break;
        }
        case EN_TX_DETAIL_VOLT:
        {    
            static uint32_t time_count = 0;
			static uint8_t pack_cnt = 0;
			
            if(_gCanProtoInfoBmsTx[EN_TX_DETAIL_VOLT].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBmsTx[EN_TX_DETAIL_VOLT].tx_regular_period)
                {
                    if(gBMUDataInfo.bmu_detail_volt_info.valid_flag != true)
                    {
                        break;
                    }
                    BmsTxDetailVoltPrep(pack_cnt);
                    time_count = tim_get_systime();
					pack_cnt++;
					if(pack_cnt >= (gBMUDataInfo.per_bmu_cell_num + 2) / 3)
					{
						pack_cnt = 0;
						_gCanProtoInfoBmsTx[EN_TX_DETAIL_VOLT].tx_regular_period = 1000;
					}
					else if(pack_cnt == 1)
					{
						_gCanProtoInfoBmsTx[EN_TX_DETAIL_VOLT].tx_regular_period = 10;
					}
                }
            }
            break;
        }
        case EN_TX_DETAIL_TEMP:
        {
            static uint32_t time_count = 0;
			static uint8_t pack_cnt = 0;
			
            if(_gCanProtoInfoBmsTx[EN_TX_DETAIL_TEMP].regular_default_enable)
            {
                if((uint32_t)(tim_get_systime() - time_count) >= _gCanProtoInfoBmsTx[EN_TX_DETAIL_TEMP].tx_regular_period)
                {
                    if(gBMUDataInfo.bmu_detail_temp_info.valid_flag != true)
                    {
                        break;
                    }
                    BmsTxDetailTempPrep(pack_cnt);
                    time_count = tim_get_systime();
					pack_cnt++;
					if(pack_cnt >= (gBMUDataInfo.per_bmu_cell_temp_num + 6) / 7)
					{
						pack_cnt = 0;
						_gCanProtoInfoBmsTx[EN_TX_DETAIL_TEMP].tx_regular_period = 1000;
					}
					else if(pack_cnt == 1)
					{
						_gCanProtoInfoBmsTx[EN_TX_DETAIL_TEMP].tx_regular_period = 20;
					}
                }            
            }
            break;
        }
        default:break;
    }
}