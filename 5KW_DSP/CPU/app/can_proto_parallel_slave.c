/*
 * can_proto_parallel_slave.c
 *
 *  Created on: 2025年12月22日
 *      Author: Nova008226
 */

/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_parallel_slave.c
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2025-12-17
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


#include "can_comm_process.h"
#include "dev_can_comm.h"
#include "cpu_timers.h"
#include "sys_info.h"
#include "modbus_analysis.h"
#include "energy_management.h"

extern int16_t CAN_Slave_Send_Slave_AC_Output_Limit;
extern int16_t CAN_Slave_Send_Slave_AC_Input_Limit;
extern int16_t CAN_Slave_Send_Slave_AC_RealTime;
extern int16_t CAN_Slave_Send_Slave_Backup;
extern int16_t CAN_Slave_Send_Slave_PV_RealTime;
extern int16_t CAN_Slave_Send_Slave_BAT_RealTime;
extern int16_t CAN_Slave_Receive_Master_Workmode;
extern int16_t CAN_Slave_Receive_Master_Antiflow;
extern int16_t CAN_Slave_Receive_Master_Export_Limit;
extern int16_t CAN_Slave_Receive_Master_Import_Limit;
extern int16_t CAN_Slave_Receive_Slave_AC_Set;

extern EN_PARALLEL_OFF_NETWORK_STA geInvsta;

extern uint16_t gSyncHostStandby;   //主机透传的待机状态

typedef enum
{
    EN_TX_PARAL_S_ACK_1 = 0,
    EN_TX_PARAL_S_ACK_2,
    EN_TX_PARAL_S_ACK_3,
    EN_TX_PARAL_S_ACK_4,
    EN_TX_PARAL_S_TOTAL,
} EN_TX_CAN_PARAL_ACK_S;

typedef enum
{
    EN_RX_PARAL_S_SYNC_1 = 0,
    EN_RX_PARAL_S_SYNC_2,
    EN_RX_PARAL_S_SYNC_3,
    EN_RX_PARAL_S_TOTAL,
} EN_TX_CAN_PARAL_S;

static void _CanSlaveTxAckSend();
static void _CanSlaveRxPollingDetect();

static CanProtoTxInfo_t _gCanProtoInfoParallelSlaveTx[EN_TX_PARAL_S_TOTAL] =
{
    {0x18A00180, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, true, ParallelSlaveAck1TxPrep,  NULL},
    {0x18A10180, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, true, ParallelSlaveAck2TxPrep,  NULL},
    {0x18A20180, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, true, ParallelSlaveAck3TxPrep,  NULL},
    {0x18A30180, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, true, ParallelSlaveAck4TxPrep,  NULL},
};


CanProtoListTxInfo_t gCanProtoListInfoParallelSlaveTx =
{
    .p_proto_info = _gCanProtoInfoParallelSlaveTx,
    .num = EN_TX_PARAL_S_TOTAL,
    .polling = NULL
};


static CanProtoRxInfo_t _gCanProtoInfoParallelSlaveRx[EN_RX_PARAL_S_TOTAL] =
{
    {0x18A08001, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelSlaveAck1RxProcess,     NULL},//EN_DEV_CAN_ANY
    {0x18A18001, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelSlaveAck2RxProcess,     NULL},
    {0x18A28001, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelSlaveAck3RxProcess,     NULL},
};

CanProtoListRxInfo_t gCanProtoListInfoParallelSlaveRx =
{
    .p_proto_info = _gCanProtoInfoParallelSlaveRx,
    .num = EN_RX_PARAL_S_TOTAL,
    .polling = _CanSlaveRxPollingDetect
};



#define CAN_SLAVE_TIMEOUT               10000

static uint32_t _gCanSlaveRxDetectedTimeCount = 0;


////////////////TX///////////////
/**
* @brief    并机从机响应信息1发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/

void ParallelSlaveAck1TxPrep(uint16_t idx)
{
    uint8_t buf[8];

    /*将业务数据赋给CAN BUF*/
    //byte0-3 最大并网功率
    //byte4-7 最小并网功率

    int32_t slave_grid_port_max_power = gParallelComm.slave_info.slave_grid_port_allow_max_power;
    int32_t slave_grid_port_min_power = gParallelComm.slave_info.slave_grid_port_allow_min_power;

    if((gSysInfoGet.inv_status == EN_INV_STANDBY) || (gSysInfoGet.inv_status == EN_INPUT_OFF_GRID) || (gAlgoErr != 0)) //待机、离网、故障上传最大最小AC口功率为0
    {
        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;

        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    }
    else
    {
        buf[0] = (slave_grid_port_max_power >> 24) & 0xFF;
        buf[1] = (slave_grid_port_max_power >> 16) & 0xFF;
        buf[2] = (slave_grid_port_max_power >> 8) & 0xFF;
        buf[3] = (slave_grid_port_max_power) & 0xFF;

        buf[4] = (slave_grid_port_min_power >> 24) & 0xFF;
        buf[5] = (slave_grid_port_min_power >> 16) & 0xFF;
        buf[6] = (slave_grid_port_min_power >> 8) & 0xFF;
        buf[7] = (slave_grid_port_min_power) & 0xFF;
    }

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelSlaveTx[EN_TX_PARAL_S_ACK_1], buf);
}



/**
* @brief    并机从机响应信息2发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelSlaveAck2TxPrep(uint16_t idx)
{
    uint8_t buf[8];
    int32_t inverter_input_power = 0;
    /*将业务数据赋给CAN BUF*/
    int32_t slave_grid_port_power = gParallelComm.slave_info.slave_grid_port_power;
    int32_t slave_backup_power = gParallelComm.slave_info.slave_backup_power;

    //byte0-3 并网口实时功率
    //byte4-7 本地负载功率
    buf[0]= (slave_grid_port_power >> 24) & 0xFF;
    buf[1]= (slave_grid_port_power >> 16) & 0xFF;
    buf[2]= (slave_grid_port_power >> 8) & 0xFF;
    buf[3]= (slave_grid_port_power) & 0xFF;
    buf[4]= (slave_backup_power >> 24) & 0xFF;
    buf[5]= (slave_backup_power >> 16) & 0xFF;
    buf[6]= (slave_backup_power >> 8) & 0xFF;
    buf[7]= (slave_backup_power) & 0xFF;

    if((gSysInfoGet.inv_status == EN_INV_STANDBY) || (gSysInfoGet.inv_status == EN_INPUT_OFF_GRID) || (gAlgoErr != 0))  //待机、离网、故障上传AC口功率为0
    {
        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;
    }

    if((gSysInfoGet.inv_status == EN_INV_STANDBY) || (gAlgoErr != 0))   //待机、故障上传back up功率为0
    {
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    }
    
    CanProtoTxIntoQueue(&_gCanProtoInfoParallelSlaveTx[EN_TX_PARAL_S_ACK_2], buf);
}



/**
* @brief    并机从机响应信息3发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelSlaveAck3TxPrep(uint16_t idx)
{
    uint8_t buf[8];

    /*将业务数据赋给CAN BUF*/
    int32_t slave_pv_power = gParallelComm.slave_info.slave_pv_power;
    int32_t slave_bat_power = gParallelComm.slave_info.slave_bat_power;

    if((gSysInfoGet.inv_status == EN_INV_STANDBY) || (gAlgoErr != 0))  //待机、故障上传pv、电池功率为0
    {
        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;

        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;

    }
    else
    {
        //byte0-3 PV实时功率
        //byte4-7 电池实时功率
        buf[0]= (slave_pv_power >> 24) & 0xFF;
        buf[1]= (slave_pv_power >> 16) & 0xFF;
        buf[2]= (slave_pv_power >> 8) & 0xFF;
        buf[3]= (slave_pv_power) & 0xFF;
        buf[4]= (slave_bat_power >> 24) & 0xFF;
        buf[5]= (slave_bat_power >> 16) & 0xFF;
        buf[6]= (slave_bat_power >> 8) & 0xFF;
        buf[7]= (slave_bat_power) & 0xFF;
    }

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelSlaveTx[EN_TX_PARAL_S_ACK_3], buf);
}

/**
* @brief    并机从机响应信息4发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelSlaveAck4TxPrep(uint16_t idx)
{
    uint8_t buf[8];

    /*将业务数据赋给CAN BUF*/
    int32_t slave_bat_allow_max_power = gParallelComm.slave_info.slave_bat_allow_max_power;
    int32_t slave_bat_allow_min_power = gParallelComm.slave_info.slave_bat_allow_min_power;

    if(gAlgoErr != 0)   //故障上传电池最大最小允许功率为0
    {
        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;

        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        buf[7] = 0;
    }
    else
    {
        //byte0-3 电池最大允许功率
        //byte4-7 电池最小允许功率
        buf[0]= (slave_bat_allow_max_power >> 24) & 0xFF;
        buf[1]= (slave_bat_allow_max_power >> 16) & 0xFF;
        buf[2]= (slave_bat_allow_max_power >> 8) & 0xFF;
        buf[3]= (slave_bat_allow_max_power) & 0xFF;
        buf[4]= (slave_bat_allow_min_power >> 24) & 0xFF;
        buf[5]= (slave_bat_allow_min_power >> 16) & 0xFF;
        buf[6]= (slave_bat_allow_min_power >> 8) & 0xFF;
        buf[7]= (slave_bat_allow_min_power) & 0xFF;
    }

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelSlaveTx[EN_TX_PARAL_S_ACK_4], buf);
}

////////////////RX///////////////
/**
* @brief    并机从机同步信息1接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelSlaveAck1RxProcess(uint16_t *p_buf)
{
    ModbusReg.parallel_mode_enable = 0;
    gSysSetInfo.parallel_mode_enable = 0;
//    gSysInfoGet.parallel_mode_stat = 2;
    /*解析从机响应的数据*/
    // byte0 工作模式
    // byte1 防逆流
    if(gSysSetInfo.parallel_mode_enable != 1)//非主机模式才接收
    {
//        if(geInvsta == EN_GRID_CONNECT_MODE)//只有在并网情况下才响应主机以及将自己置为从机
        {
            gSysSetInfo.work_mode = p_buf[0];
            gSysSetInfo.antireverseflow_enable = p_buf[1];
            gSyncHostStandby = p_buf[2];
            if(ModbusReg.work_mode != gSysSetInfo.work_mode)
            {
                ModbusReg.work_mode = gSysSetInfo.work_mode;
            }
            if(ModbusReg.antireverseflow_enable != gSysSetInfo.antireverseflow_enable)
            {
                ModbusReg.antireverseflow_enable = gSysSetInfo.antireverseflow_enable;
            }
            _CanSlaveTxAckSend();
        }
    }
}

/**
* @brief    并机从机同步信息2接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelSlaveAck2RxProcess(uint16_t *p_buf)
{
    /*解析从机响应的数据*/
    // byte0-3 最大并网功率
    // byte4-7 最大入户功率
    if(gSysSetInfo.parallel_mode_enable != 1)//非主机模式才接收
    {
//        if(geInvsta == EN_GRID_CONNECT_MODE)
        {
            gSysSetInfo.ongrid_power = (float)(int32_t)(((int32_t)p_buf[0] << 24) | ((int32_t)p_buf[1] << 16) | ((int32_t)p_buf[2] << 8) | ((int32_t)p_buf[3] << 0));
            gSysSetInfo.network_power_limit = (float)(int32_t)(((int32_t)p_buf[4] << 24) | ((int32_t)p_buf[5] << 16) | ((int32_t)p_buf[6] << 8) | ((int32_t)p_buf[7] << 0));
            if(ModbusReg.ongrid_power != gSysSetInfo.ongrid_power)
            {
                ModbusReg.ongrid_power = gSysSetInfo.ongrid_power;
            }
            if(ModbusReg.network_power_limit != gSysSetInfo.network_power_limit)
            {
                ModbusReg.network_power_limit = gSysSetInfo.network_power_limit;
            }
            _CanSlaveTxAckSend();
        }
    }
}

/**
* @brief    并机从机同步信息3接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelSlaveAck3RxProcess(uint16_t *p_buf)
{
    /*解析从机响应的数据*/
    // CAN_Slave_Receive_Slave_AC_Set = (p_buf[3] | (p_buf[2] << 8));
    if(gSysSetInfo.parallel_mode_enable != 1)//非主机模式才接收
    {
//        if(geInvsta == EN_GRID_CONNECT_MODE)
        {
            gParallelComm.slave_set.slave_grid_port_power_set = (float)(int32_t)(((int32_t)p_buf[0] << 24) | ((int32_t)p_buf[1] << 16) | ((int32_t)p_buf[2] << 8) | ((int32_t)p_buf[3] << 0));
            _CanSlaveTxAckSend();
        }
    }

}




/**
* @brief    CAN从机按顺序发送响应包
* @param    none
* @return   none
* @note: 将数据推送至发送队列，实际由_CanProtoTxProcess发出
*/
static void _CanSlaveTxAckSend()
{
    static uint8_t slave_tx_idx = 0;

    gSysSetInfo.parallel_mode_enable = 0;
    ModbusReg.parallel_mode_enable = 0;

    gSysInfoGet.parallel_mode_stat = 2;//并机状态 = 2;//伪代码
    gParallelComm.mode = 2;
    _gCanSlaveRxDetectedTimeCount = TimGetSystime();
    gCanProtoListInfoParallelSlaveTx.p_proto_info[slave_tx_idx].data_prep(0);
    slave_tx_idx++;
    if(slave_tx_idx >= gCanProtoListInfoParallelSlaveTx.num)
    {
        slave_tx_idx = 0;
    }
}


/**
* @brief    CAN从机检测接收情况
* @param    none
* @return   none
* @note:
*/
static void _CanSlaveRxPollingDetect()
{
    if(gSysInfoGet.parallel_mode_stat == 2)
    {
        if((uint32_t)(TimGetSystime() - _gCanSlaveRxDetectedTimeCount) > CAN_SLAVE_TIMEOUT)
        {
            gSysInfoGet.parallel_mode_stat = 0;
            gParallelComm.mode = 0;
        }
    }
}


