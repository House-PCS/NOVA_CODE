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


#include "systick.h"
#include "can_tx_rx_process.h"


//#define SLAVE_ADDR_NUM  PACK_NUM

static uint8_t slave_addr = 0;
typedef enum
{
    EN_BMU_CTL_TX = 0,
    EN_BMU_PASSIVE_BALANCE_TX,
    EN_BMU_TX_TOTAL,
}EN_CAN_BMU_TX;

typedef enum
{
    EN_BMU_STATE_RX = 0,
    EN_BMU_STATIS_VOLT_RX,
    EN_BMU_STATIS_TEMP_RX,
    EN_BMU_STATIS_OTHER_RX,
    EN_BMU_DETAIL_VOLT_RX,
    EN_BMU_DETAIL_TEMP_RX,
    EN_BMU_RX_TOTAL,
}EN_CAN_BMU_RX;

void BMUCtlTxPrep(uint8_t idx);
void BMUPassiveBalanceTxPrep(uint8_t idx);
void BMUStatRxProcess(uint8_t *p_buf);
void BMUStatisVoltRxProcess(uint8_t *p_buf);
void BMUStatisTempRxProcess(uint8_t *p_buf);
void BMUOtherRxProcess(uint8_t *p_buf);
void BMUDetailVoltRxProcess(uint8_t *p_buf);
void BMUDetailTempRxProcess(uint8_t *p_buf);


static void _TxPolling(uint8_t idx);


static CanProtoTxInfo_t _gCanProtoInfoBMUTx[] = 
{
    {0x1801F4B4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, BMUCtlTxPrep,            _TxPolling}, 
    {0x1802F4B4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, BMUPassiveBalanceTxPrep, _TxPolling}, 
};

CanProtoListTxInfo_t gCanProtoListInfoBMUTx = 
{
    .p_proto_info = _gCanProtoInfoBMUTx,
    .num = EN_BMU_TX_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoBMURx[] = 
{
    {0x1801B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUStatRxProcess,       NULL}, 
    {0x1802B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUStatisVoltRxProcess, NULL}, 
    {0x1803B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUStatisTempRxProcess, NULL}, 
    {0x1804B4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUOtherRxProcess,      NULL}, 
    {0x180AB4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUDetailVoltRxProcess, NULL}, 
    {0x180BB4F4, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, BMUDetailTempRxProcess, NULL}, 
};

CanProtoListRxInfo_t gCanProtoListInfoBMURx = 
{
    .p_proto_info = _gCanProtoInfoBMURx,
    .num = EN_BMU_RX_TOTAL
};


/** 
* @brief    控制报文发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void BMUCtlTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    /*多个从控*/
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        _gCanProtoInfoBMUTx[EN_BMU_CTL_TX].can_id.ext_pdu.ps = 0xF4 - i;
        if(gBMUCtlInfo.ctl_flag[i] != true)
        {
            continue;
        }
        buf[0] = gBMSDataInfo.bmu_io_ctl_info[i].report_enable;
        buf[1] = gBMSDataInfo.bmu_io_ctl_info[i].io_out;
        buf[2] = gBMSDataInfo.bmu_io_ctl_info[i].pwm_duty_1;
        buf[3] = gBMSDataInfo.bmu_io_ctl_info[i].pwm_duty_2;
        memset(&buf[4], 0x00, 4);
        CanProtoTxIntoQueue(&_gCanProtoInfoBMUTx[EN_BMU_CTL_TX], buf);
        gBMUCtlInfo.ctl_flag[i] = false;
    }
}



/** 
* @brief    被动均衡发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void BMUPassiveBalanceTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    
    /*多个从控*/
    for(uint8_t i = 0; i < gBMSDataInfo.bmu_num; i++)
    {
        _gCanProtoInfoBMUTx[EN_BMU_PASSIVE_BALANCE_TX].can_id.ext_pdu.ps = 0xF4 - i;
        if(gBMUCtlInfo.passive_balance_flag[i] != true)
        {
            continue;
        }
        memcpy(buf, gBMSDataInfo.bmu_passive_balance_info[i], 8);
        CanProtoTxIntoQueue(&_gCanProtoInfoBMUTx[EN_BMU_PASSIVE_BALANCE_TX], buf);
        gBMUCtlInfo.passive_balance_flag[i] = false;
    }
}


/** 
* @brief    状态信息接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUStatRxProcess(uint8_t *p_buf)
{
    uint8_t sa = _gCanProtoInfoBMURx[EN_BMU_STATE_RX].can_id.ext_pdu.sa;
    
    if(sa > 0xF4 || sa <= 0xF4 - gBMSDataInfo.bmu_num)
    {
        return;
    }
    uint8_t slave_num = 0xF4 - sa;
    
    gBMSDataInfo.bmu_io_info[slave_num].io_input_stat = p_buf[0];
    gBMSDataInfo.bmu_io_info[slave_num].io_output_stat = p_buf[1];
    gBMSDataInfo.bmu_io_info[slave_num].pwm_1_output_duty = p_buf[2];
    gBMSDataInfo.bmu_io_info[slave_num].pwm_2_output_duty = p_buf[3];
    gBMSDataInfo.bmu_io_info[slave_num].pwm_1_input_duty = p_buf[4];
    gBMSDataInfo.bmu_io_info[slave_num].pwm_2_input_duty = p_buf[5];
    gBMSDataInfo.bmu_io_info[slave_num].valid_flag = true;
    
    gBMSDataInfo.bmu_comm_last_time[slave_num] = tim_get_systime();
}

/** 
* @brief    统计电压接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUStatisVoltRxProcess(uint8_t *p_buf)
{
    uint8_t sa = _gCanProtoInfoBMURx[EN_BMU_STATIS_VOLT_RX].can_id.ext_pdu.sa;
    
    if(sa > 0xF4 || sa <= 0xF4 - gBMSDataInfo.bmu_num)
    {
        return;
    }
    uint8_t slave_num = 0xF4 - sa;
    
    gBMSDataInfo.bmu_statis_volt_info[slave_num].total_volt = ((p_buf[0] << 8) | (p_buf[1])) * 100;// 0.1V转mV
    gBMSDataInfo.bmu_statis_volt_info[slave_num].cell_max_volt_num = p_buf[2];
    gBMSDataInfo.bmu_statis_volt_info[slave_num].cell_max_volt = ((p_buf[3] << 8) | (p_buf[4]));
    gBMSDataInfo.bmu_statis_volt_info[slave_num].cell_min_volt_num = p_buf[5];
    gBMSDataInfo.bmu_statis_volt_info[slave_num].cell_min_volt = ((p_buf[6] << 8) | (p_buf[7]));
    gBMSDataInfo.bmu_statis_volt_info[slave_num].valid_flag = true;
    
    gBMSDataInfo.bmu_comm_last_time[slave_num] = tim_get_systime();
}


/** 
* @brief    统计温度接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUStatisTempRxProcess(uint8_t *p_buf)
{
    uint8_t sa = _gCanProtoInfoBMURx[EN_BMU_STATIS_TEMP_RX].can_id.ext_pdu.sa;
    
    if(sa > 0xF4 || sa <= 0xF4 - gBMSDataInfo.bmu_num)
    {
        return;
    }
    uint8_t slave_num = 0xF4 - sa;
    
    gBMSDataInfo.bmu_statis_temp_info[slave_num].cell_max_temp_num = p_buf[0];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].cell_max_temp = p_buf[1];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].cell_min_temp_num = p_buf[2];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].cell_min_temp = p_buf[3];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].pole_max_temp_num = p_buf[4];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].pole_max_temp = p_buf[5];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].pole_min_temp_num = p_buf[6];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].pole_min_temp = p_buf[7];
    gBMSDataInfo.bmu_statis_temp_info[slave_num].valid_flag = true;
    
    gBMSDataInfo.bmu_comm_last_time[slave_num] = tim_get_systime();
}


/** 
* @brief    其他信息接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUOtherRxProcess(uint8_t *p_buf)
{
    uint8_t sa = _gCanProtoInfoBMURx[EN_BMU_STATIS_OTHER_RX].can_id.ext_pdu.sa;
    
    if(sa > 0xF4 || sa <= 0xF4 - gBMSDataInfo.bmu_num)
    {
        return;
    }
    uint8_t slave_num = 0xF4 - sa;
    
    gBMSDataInfo.bmu_statis_other_info[slave_num].cell_ave_volt = ((p_buf[0] << 8) | (p_buf[1]));
    gBMSDataInfo.bmu_statis_other_info[slave_num].cell_ave_temp = p_buf[2];
    gBMSDataInfo.bmu_statis_other_info[slave_num].equalizing_plate_max_temp = p_buf[3];
    gBMSDataInfo.bmu_statis_other_info[slave_num].fault_info = ((p_buf[6] << 8) | (p_buf[7]));
    gBMSDataInfo.bmu_statis_other_info[slave_num].valid_flag = true;
    
    gBMSDataInfo.bmu_comm_last_time[slave_num] = tim_get_systime();
}


/** 
* @brief    详细电压接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUDetailVoltRxProcess(uint8_t *p_buf)
{
    uint8_t sa = _gCanProtoInfoBMURx[EN_BMU_DETAIL_VOLT_RX].can_id.ext_pdu.sa;
    
    if(sa > 0xF4 || sa <= 0xF4 - gBMSDataInfo.bmu_num)
    {
        return;
    }
    uint8_t slave_num = 0xF4 - sa;
    
    uint8_t cnt = p_buf[0];
    for(uint8_t i = 0; i < 3; i++)
    {
        if(3 * cnt + i < gBMSDataInfo.per_bmu_cell_num)
        {
            gBMSDataInfo.bmu_detail_volt_info[slave_num].cell_volt[3 * cnt + i] = ((p_buf[i * 2 + 1] << 8) | (p_buf[i * 2 + 2]));
            if(((p_buf[7] >> i) & 0x01) == 0x01)//对应通道被动均衡开启
            {
                gBMSDataInfo.bmu_detail_volt_info[slave_num].cell_balance_stat[(3 * cnt + i) / 8] |= (1 << ((3 * cnt + i) % 8));
            }
            else
            {
                gBMSDataInfo.bmu_detail_volt_info[slave_num].cell_balance_stat[(3 * cnt + i) / 8] &= ~(1 << ((3 * cnt + i) % 8));
            }
        }
    }
    if(((p_buf[7] >> 7) & 0x01) == 0x01)//完成标志
    {
        gBMSDataInfo.bmu_detail_volt_info[slave_num].valid_flag = true;
    }
    
    gBMSDataInfo.bmu_comm_last_time[slave_num] = tim_get_systime();
}


/** 
* @brief    详细温度接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void BMUDetailTempRxProcess(uint8_t *p_buf)
{
    uint8_t sa = _gCanProtoInfoBMURx[EN_BMU_DETAIL_TEMP_RX].can_id.ext_pdu.sa;
    
    if(sa > 0xF4 || sa <= 0xF4 - gBMSDataInfo.bmu_num)
    {
        return;
    }
    uint8_t slave_num = 0xF4 - sa;
    
    uint8_t cnt = p_buf[0];
    for(uint8_t i = 0; i < 7; i++)
    {
        if(7 * cnt + i < gBMSDataInfo.per_bmu_cell_temp_num)
        {
            gBMSDataInfo.bmu_detail_temp_info[slave_num].cell_temp[7 * cnt + i] = p_buf[i + 1];
        }
    }
    if(7 * (cnt + 1) >= gBMSDataInfo.per_bmu_cell_temp_num)//完成标志
    {
        gBMSDataInfo.bmu_detail_temp_info[slave_num].valid_flag = true;
    }
    
    gBMSDataInfo.bmu_comm_last_time[slave_num] = tim_get_systime();
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
        case EN_BMU_CTL_TX:
        {
            BMUCtlTxPrep(0);
            break;
        }
        case EN_BMU_PASSIVE_BALANCE_TX:
        {
            BMUPassiveBalanceTxPrep(0);
            break;
        }
        default:break;
    }
}



