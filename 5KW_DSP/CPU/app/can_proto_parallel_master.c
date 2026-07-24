/*
 * can_proto_parallel.c
 *
 *  Created on: 2025年12月17日
 *      Author: Nova008226
 */

#include "can_comm_process.h"
#include "dev_can_comm.h"
#include "cpu_timers.h"
#include "sys_info.h"
#include "modbus_analysis.h"
#include "energy_management.h"

extern int16_t CAN_Master_Send_Master_Workmode;
extern int16_t CAN_Master_Send_Master_Antiflow;
extern int16_t CAN_Master_Send_Master_Export_Limit;
extern int16_t CAN_Master_Send_Master_Import_Limit;
extern int16_t CAN_Master_Send_Slave_AC_Set;
extern int16_t CAN_Master_Receive_Slave_AC_Output_Limit;
extern int16_t CAN_Master_Receive_Slave_AC_Input_Limit;
extern int16_t CAN_Master_Receive_Slave_AC_RealTime;
extern int16_t CAN_Master_Receive_Slave_Backup;
extern int16_t CAN_Master_Receive_Slave_PV_RealTime;
extern int16_t CAN_Master_Receive_Slave_BAT_RealTime;

extern EN_PARALLEL_OFF_NETWORK_STA geInvsta;

extern uint16_t Enoil;
extern uint16_t gSysStartFlag;

typedef enum
{
    EN_TX_PARAL_M_SYNC_1 = 0,
    EN_TX_PARAL_M_SYNC_2,
    EN_TX_PARAL_M_SYNC_3,
    EN_TX_PARAL_M_TOTAL,
} EN_TX_CAN_PARAL_M;


typedef enum
{
    EN_RX_PARAL_M_ACK_1 = 0,
    EN_RX_PARAL_M_ACK_2,
    EN_RX_PARAL_M_ACK_3,
    EN_RX_PARAL_M_ACK_4,
    EN_RX_PARAL_M_TOTAL,
} EN_TX_CAN_PARAL_ACK_M;

static void _CanMasterTxPeriodSend();

static CanProtoTxInfo_t _gCanProtoInfoParallelMasterTx[EN_TX_PARAL_M_TOTAL] =
{
    {0x18A08001, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, ParallelMasterSync1TxPrep,   NULL},
    {0x18A18001, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, ParallelMasterSync2TxPrep,   NULL},
    {0x18A28001, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, ParallelMasterSync3TxPrep,   NULL},
};


CanProtoListTxInfo_t gCanProtoListInfoParallelMasterTx =
{
    .p_proto_info = _gCanProtoInfoParallelMasterTx,
    .num = EN_TX_PARAL_M_TOTAL,
    .polling = _CanMasterTxPeriodSend
};


static CanProtoRxInfo_t _gCanProtoInfoParallelMasterRx[EN_RX_PARAL_M_TOTAL] =
{
    {0x18A00180, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelMasterAck1RxProcess,   NULL},
    {0x18A10180, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelMasterAck2RxProcess,   NULL},
    {0x18A20180, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelMasterAck3RxProcess,   NULL},
    {0x18A30180, EN_DEV_CAN1, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, ParallelMasterAck4RxProcess,   NULL},


};

CanProtoListRxInfo_t gCanProtoListInfoParallelMasterRx =
{
    .p_proto_info = _gCanProtoInfoParallelMasterRx,
    .num = EN_RX_PARAL_M_TOTAL,
    .polling = NULL
};


#define CAN_MASTER_SEND_PERIOD          10
#define CAN_MASTER_WAIT_ACK_PERIOD      1000
#define CAN_MASTER_TIMEOUT              30000 // 20260512,更改主机判断超时时间，从10秒改为30秒

typedef enum
{
    EN_CAN_MASTER_TX_IDLE,
    EN_CAN_MASTER_TX_SEND,
    EN_CAN_MASTER_TX_WAIT_ACK,
} EN_CAN_MASTER_TX_STATUS;

static bool _gCanMasterAckedFlag = false;




////////////////TX///////////////

void CanBTxSendEnable(uint16_t flag)
{
    if(flag)
    {
        for(int i = 0; i < EN_TX_PARAL_M_TOTAL; i++)
        {
            gCanProtoListInfoParallelMasterTx.p_proto_info[i].regular_default_enable = true;
        }
    }
    else
    {
        for(int i = 0; i < EN_TX_PARAL_M_TOTAL; i++)
        {
            gCanProtoListInfoParallelMasterTx.p_proto_info[i].regular_default_enable = false;
        }
    }
}
/**
* @brief    并机主机同步信息1发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelMasterSync1TxPrep(uint16_t idx)
{
    uint8_t buf[8];
    uint16_t host_status = 0;

    if((gSysStartFlag == 0) && (gSysInfoGet.inv_status == EN_INV_STANDBY))
    {
        host_status = 1;    //主机启机过程中强制给从机传离网状态
    }
    else
    {
        host_status = gSysInfoGet.inv_status;
    }
    
    /*将业务数据赋给CAN BUF*/
    buf[0]= gSysSetInfo.work_mode;
    buf[1]= gSysSetInfo.antireverseflow_enable; //防逆流开关
    buf[2]= host_status; //主机设备状态(0-待机 1-离网，2-并网)
    buf[3]= 0x00;
    buf[4]= 0x00;
    buf[5]= 0x00;
    buf[6]= 0x00;
    buf[7]= 0x00;

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelMasterTx[EN_TX_PARAL_M_SYNC_1], buf);
}



/**
* @brief    并机主机同步信息2发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelMasterSync2TxPrep(uint16_t idx)
{
    uint8_t buf[8];

    /*将业务数据赋给CAN BUF*/
    int32_t max_limit_export_power = (int32_t)gSysSetInfo.ongrid_power;
    int32_t max_limit_import_power = (int32_t)gSysSetInfo.network_power_limit;

    if(gSysInfoGet.inv_status == EN_INPUT_OFF_GRID)  //离网下发从机并网口功率为0
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
        buf[0]= (max_limit_export_power >> 24) & 0xFF;
        buf[1]= (max_limit_export_power >> 16) & 0xFF;
        buf[2]= (max_limit_export_power >> 8) & 0xFF;
        buf[3]= (max_limit_export_power >> 0) & 0xFF;

        buf[4]= (max_limit_import_power >> 24) & 0xFF;
        buf[5]= (max_limit_import_power >> 16) & 0xFF;
        buf[6]= (max_limit_import_power >> 8) & 0xFF;
        buf[7]= (max_limit_import_power >> 0) & 0xFF;
    }

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelMasterTx[EN_TX_PARAL_M_SYNC_2], buf);
}



/**
* @brief    并机主机同步信息1发送数据准备
* @param    idx:下发数据索引，不同的idx，对应下发的数据内容不一样，如对应CANID只有一种数据格式，则idx不使用，填0即可
* @return   NULL
* @note:
*/
void ParallelMasterSync3TxPrep(uint16_t idx)
{
    uint8_t buf[8];

    /*将业务数据赋给CAN BUF*/
    int32_t grid_port_power = gParallelComm.slave_set.slave_grid_port_power_set;

    buf[0]= (grid_port_power >> 24) & 0xFF;
    buf[1]= (grid_port_power >> 16) & 0xFF;
    buf[2]= (grid_port_power >> 8) & 0xFF;
    buf[3]= (grid_port_power >> 0) & 0xFF;
    buf[4]= 0x25;
    buf[5]= 0x26;
    buf[6]= 0x27;
    buf[7]= 0x28;

    CanProtoTxIntoQueue(&_gCanProtoInfoParallelMasterTx[EN_TX_PARAL_M_SYNC_3], buf);
}


////////////////RX///////////////
/**
* @brief    并机主机响应信息1接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelMasterAck1RxProcess(uint16_t *p_buf)
{
    /*解析从机响应的数据*/
    if(gSysSetInfo.parallel_mode_enable == 1)
    {
        gParallelComm.slave_info.slave_grid_port_allow_max_power = (float)(int32_t)(((int32_t)p_buf[0] << 24) | ((int32_t)p_buf[1] << 16) | ((int32_t)p_buf[2] << 8) | ((int32_t)p_buf[3] << 0));
        gParallelComm.slave_info.slave_grid_port_allow_min_power = (float)(int32_t)(((int32_t)p_buf[4] << 24) | ((int32_t)p_buf[5] << 16) | ((int32_t)p_buf[6] << 8) | ((int32_t)p_buf[7] << 0));

//      if(geInvsta == EN_GRID_CONNECT_MODE) // 20260513,修改主机可以在离网状态下进入主机模式
        {
            gSysInfoGet.parallel_mode_stat = 1; //收到从机回复，识别到自己为主机
            gParallelComm.mode = 1;
        }
        _gCanMasterAckedFlag = true;
    }
}

/**
* @brief    并机主机响应信息2接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelMasterAck2RxProcess(uint16_t *p_buf)
{
    /*解析从机响应的数据*/
    if(gSysSetInfo.parallel_mode_enable == 1)
    {
        gParallelComm.slave_info.slave_grid_port_power = (float)(int32_t)(((int32_t)p_buf[0] << 24) | ((int32_t)p_buf[1] << 16) | ((int32_t)p_buf[2] << 8) | ((int32_t)p_buf[3] << 0));
        gParallelComm.slave_info.slave_backup_power = (float)(int32_t)(((int32_t)p_buf[4] << 24) | ((int32_t)p_buf[5] << 16) | ((int32_t)p_buf[6] << 8) | ((int32_t)p_buf[7] << 0));

//      if(geInvsta == EN_GRID_CONNECT_MODE) // 20260513,修改主机可以在离网状态下进入主机模式
        {
            gSysInfoGet.parallel_mode_stat = 1; //收到从机回复，识别到自己为主机
            gParallelComm.mode = 1;
        }
        _gCanMasterAckedFlag = true;
    }
}

/**
* @brief    并机主机响应信息3接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelMasterAck3RxProcess(uint16_t *p_buf)
{
    /*解析从机响应的数据*/
    if(gSysSetInfo.parallel_mode_enable == 1)
    {
        gParallelComm.slave_info.slave_pv_power = (float)(int32_t)(((int32_t)p_buf[0] << 24) | ((int32_t)p_buf[1] << 16) | ((int32_t)p_buf[2] << 8) | ((int32_t)p_buf[3] << 0));
        gParallelComm.slave_info.slave_bat_power = (float)(int32_t)(((int32_t)p_buf[4] << 24) | ((int32_t)p_buf[5] << 16) | ((int32_t)p_buf[6] << 8) | ((int32_t)p_buf[7] << 0));

//      if(geInvsta == EN_GRID_CONNECT_MODE) // 20260513,修改主机可以在离网状态下进入主机模式
        {
            gSysInfoGet.parallel_mode_stat = 1; //收到从机回复，识别到自己为主机
            gParallelComm.mode = 1;
        }
        _gCanMasterAckedFlag = true;
    }
}


/**
* @brief    并机主机响应信息4接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void ParallelMasterAck4RxProcess(uint16_t *p_buf)
{
    /*解析从机响应的数据*/
    if(gSysSetInfo.parallel_mode_enable == 1)
    {
        gParallelComm.slave_info.slave_bat_allow_max_power = (float)(int32_t)(((int32_t)p_buf[0] << 24) | ((int32_t)p_buf[1] << 16) | ((int32_t)p_buf[2] << 8) | ((int32_t)p_buf[3] << 0));
        gParallelComm.slave_info.slave_bat_allow_min_power = (float)(int32_t)(((int32_t)p_buf[4] << 24) | ((int32_t)p_buf[5] << 16) | ((int32_t)p_buf[6] << 8) | ((int32_t)p_buf[7] << 0));

//      if(geInvsta == EN_GRID_CONNECT_MODE) // 20260513,修改主机可以在离网状态下进入主机模式
        {
            gSysInfoGet.parallel_mode_stat = 1; //收到从机回复，识别到自己为主机
            gParallelComm.mode = 1;
        }
        _gCanMasterAckedFlag = true;
    }
}


extern float master_count, master_count2,master_count3,master_count4,master_count5,master_count6,master_count7;
/**
* @brief    CAN主机按顺序发送同步包
* @param    none
* @return   none
* @note: 将数据推送至发送队列，实际由_CanProtoTxProcess发出
*/
uint32_t cantxindextest;

static void _CanMasterTxPeriodSend()
{
    static EN_CAN_MASTER_TX_STATUS status = EN_CAN_MASTER_TX_IDLE;
    static uint32_t polling_send_time_count = 0;
    static uint32_t ack_time_count = 0;
    static uint32_t timeout_time_count = 0;
    static uint8_t master_tx_idx = 1;

    if((gSysSetInfo.parallel_mode_enable == 1) && (gAlgoErr == 0))
    {
        CanBTxSendEnable(1);
    }
    else
    {
        CanBTxSendEnable(0);
    }

    switch(status)
    {
        case EN_CAN_MASTER_TX_IDLE:
        {
            polling_send_time_count = TimGetSystime();//tim_get_systime
            ack_time_count = TimGetSystime();
            timeout_time_count = TimGetSystime();
            _gCanMasterAckedFlag = false;
            if((gAlgoErr == 0) && (gSysSetInfo.parallel_mode_enable == 1) && (gSysInfoGet.parallel_mode_stat != 2) && \
                (gParallelComm.parallel_master_forbidden_flag != 1) && (Enoil == 0)) // 20260303，新增只有非油机模式才可并机通信
            {
                status = EN_CAN_MASTER_TX_SEND;
            }
            break;
        }
        case EN_CAN_MASTER_TX_SEND:
        {
            if((uint32_t)(TimGetSystime() - polling_send_time_count) > CAN_MASTER_SEND_PERIOD)
            {
                //if(gCanProtoListInfoParallelMasterTx.p_proto_info[master_tx_idx].regular_default_enable == true)
                if((gAlgoErr == 0) && (gSysSetInfo.parallel_mode_enable == 1) && (gSysInfoGet.parallel_mode_stat != 2) && \
                    (gParallelComm.parallel_master_forbidden_flag != 1) && (Enoil == 0)) // 20260303，新增只有非油机模式才可并机通信
                {
                    gCanProtoListInfoParallelMasterTx.p_proto_info[master_tx_idx].data_prep(0);
                    _gCanMasterAckedFlag = false;
                    polling_send_time_count = TimGetSystime();
                    ack_time_count = TimGetSystime();
                    status = EN_CAN_MASTER_TX_WAIT_ACK;
                }
                else//主机关掉并机使能
                {
                    status = EN_CAN_MASTER_TX_IDLE;
                    gSysInfoGet.parallel_mode_stat = 0;
                    gParallelComm.mode = 0;
                }
                cantxindextest = master_tx_idx;
                master_tx_idx++;
                if(master_tx_idx >= gCanProtoListInfoParallelMasterTx.num)
                {
                    master_tx_idx = 0;
                }
            }
            break;
        }
        case EN_CAN_MASTER_TX_WAIT_ACK:
        {
            if(_gCanMasterAckedFlag == true)
            {
                status = EN_CAN_MASTER_TX_SEND;
                timeout_time_count = TimGetSystime();
            }
            else
            {
                if((uint32_t)(TimGetSystime() - ack_time_count) > CAN_MASTER_WAIT_ACK_PERIOD)
                {
                    status = EN_CAN_MASTER_TX_SEND;
                }
            }
            break;
        }
        default:
            break;
    }
    if((uint32_t)(TimGetSystime() - timeout_time_count) > CAN_MASTER_TIMEOUT && (geInvsta == EN_GRID_CONNECT_MODE))// 20260512，新增只有在并网情况下才主机才开始并机通信30s倒计时，超时报F061并退出并机主机模式
    {
        gSysInfoGet.parallel_mode_stat = 0;
        gParallelComm.mode = 0;
        status = EN_CAN_MASTER_TX_IDLE;
    }
}





