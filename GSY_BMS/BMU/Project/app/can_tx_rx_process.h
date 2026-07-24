/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_tx_rx_process.h
  * @author      Xi'an NovaStar Tech Co.,LTD.
  * @version     V1.0.0
  * @date        2024-6-20
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
#ifndef __CAN_TX_RX_PROCESS_H_
#define __CAN_TX_RX_PROCESS_H_

#include "dev_can_comm.h"
#include "dev_spiflash.h"
#include "dev_pwm_detect_cfg.h"
#include "crc.h"
#include "dev_ioctl.h"
#include "flash.h"
//#include "sys_info.h"
#include "bmu_data_manage.h"


extern bool _AddrDistriSucessFlag;

typedef struct _can_proto_rx_info_stru_t
{
    union 
    {
        uint32_t            id;
        DevCanExtPdu_t      ext_pdu;
        DevCanStdPdu_t      std_pdu;
    }can_id;    
    EN_DEV_CAN              canx;
    EN_DEV_CAN_TYPE         can_type;
    uint8_t                 data_len;//用于区分是数据帧还是远程帧
    uint32_t                timeout;//超时时间，0表示不包含超时,单位ms
    uint32_t                time_count;//超时时间计数
    void                    (*data_process)(uint8_t *);//数据解析处理
    void                    (*polling)(uint8_t );//接收轮询，主要用于超时判断,
                                                 //形参表示协议idx,告诉接口是哪个协议的,增加形参,也可用于多个协议共用一个polling实现接口
}CanProtoRxInfo_t;

typedef struct _can_proto_list_rx_info_stru_t
{
    CanProtoRxInfo_t        *p_proto_info;
    uint8_t                 num;
}CanProtoListRxInfo_t;


typedef enum _can_proto_tx_mode
{
    EN_CAN_PROTO_TX_MODE_TRIGGER,//触发型，由业务发起
    EN_CAN_PROTO_TX_MODE_REGULAR,//周期型，定时自动发起
}EN_CAN_PROTO_TX_MODE;

typedef struct _can_proto_tx_info_stru_t
{
    union 
    {
        uint32_t            id;
        DevCanExtPdu_t      ext_pdu;
        DevCanStdPdu_t      std_pdu;
    }can_id;
    EN_DEV_CAN              canx;
    EN_DEV_CAN_TYPE         can_type;
    uint8_t                 data_len;//用于区分是数据帧还是远程帧
    EN_CAN_PROTO_TX_MODE    tx_mode;
    uint32_t                tx_regular_period;//周期型发送周期，触发型不涉及
    bool                    regular_default_enable;//周期发送是否使能
    void                    (*data_prep)(uint8_t );//数据准备，对于单次或者多队列单次发送，则直接执行数据推送入队列，
                                                   //对于需要多次分时发送，则进行标志位处理，由polling接口执行具体发送
                                                   //形参表示同一条CAN ID协议的第几帧，从0开始
    void                    (*polling)(uint8_t );//轮询处理机制, 用于周期性发送或者需要多次间隔发送的场景(数据量较大,队列深度不够的情况，如固件发送),
                                                 //形参表示协议idx,告诉接口是哪个协议的,增加形参,也可用于多个协议共用一个polling实现接口
}CanProtoTxInfo_t;


typedef struct _can_proto_list_tx_info_stru_t
{
    CanProtoTxInfo_t        *p_proto_info;
    uint8_t                 num;
}CanProtoListTxInfo_t;




//typedef struct _can_proto_tx_info_stru_t
//{
//    const CanProtoInfo_t *p_proto_info;
//    uint8_t              data[8];
//}CanProtoTxInfo_t;

#define CAN_PROTO_TX_QUEUE_DEPTH  20 

#define BMU_DEFAULT_ADDR  0xF4

typedef enum
{
    EN_CAN_PROTO_TX_STATUS_IDLE,//查询队列，有数据即发送
    EN_CAN_PROTO_TX_STATUS_WAITING_INTERVAL,//确保数据发送间隔
    EN_CAN_PROTO_TX_STATUS_RESEND_WAIT,//重发等待
    EN_CAN_PROTO_TX_STATUS_RESEND,//重发
    EN_CAN_PROTO_TX_STATUS_ERROR,//数据发送存在异常
}EN_CAN_PROTO_TX_STATUS;



void CanProtoInit(void);

bool CanProtoTxIntoQueue(CanProtoTxInfo_t *p_tx_info, uint8_t *p_buf);
void CanProtoSlaveAddrSet(uint8_t addr);
void CanProtoSlaveAddrReset(void);

void CanProtoProcess(void);
void CanProtoRxProcess(void);
void CanProtoTxProcess(void);

//CAN BCU
extern CanProtoListTxInfo_t gCanProtoListInfoBmsTx;
extern CanProtoListRxInfo_t gCanProtoListInfoBmsRx;


void BmsTxStatePrep(uint8_t idx);
void BmsTxStatsVoltPrep(uint8_t idx);
void BmsTxStatsTempPrep(uint8_t idx);
void BmsTxStatsOtherPrep(uint8_t idx);
void BmsTxDetailVoltPrep(uint8_t idx);
void BmsTxDetailTempPrep(uint8_t idx);


void BmsRxBroadCastProcess(uint8_t *p_buf);
void BmsRxCtrlProcess(uint8_t *p_buf);
void BmsRxBalanceProcess(uint8_t *p_buf);

//CAN ADDR
typedef enum
{
    BMS_ADDR_CHECK = 0,
    BMS_ADDR_ALLOT,
    BMS_ADDR_LATCH,
    BMS_ADDR_ERR,
    BMS_ADDR_OVER,
}EN_BMS_ADDR;

extern CanProtoListTxInfo_t gCanProtoListInfoAddrTx;
extern CanProtoListRxInfo_t gCanProtoListInfoAddrRx;
void CanAddrDistrubuteTxPrep(uint8_t idx);
void CanAddrMaskTxPrep(uint8_t idx);

void CanAddrDistrubuteRxProcess(uint8_t *p_buf);
void CanAddrMaskRxProcess(uint8_t *p_buf);
//CAN DEBUG
typedef struct _debug_read_stru_t
{
    uint8_t ch_num;
    uint8_t frames;
    uint8_t cnt;
    bool (*read_fun)(uint8_t cnt,uint8_t* buff);        //返回
    struct _debug_read_stru_t *next;
}DebugReadType;
typedef struct _debug_write_stru_t
{
    uint8_t ch_num;
//    uint8_t frames;
//    uint8_t size;
    uint8_t len;
    uint8_t *buff;
    uint32_t receive_flg;
    bool (*write_fun)(uint8_t cnt,uint8_t* buff);        //返回
    struct _debug_write_stru_t *next;
}DebugWriteType;
typedef struct _debug_cfg_stru_t
{
    uint8_t read_ch_num;
    uint8_t write_ch_num;
}DebugCfgType;

//extern CanProtoListTxInfo_t gCanProtoListInfoDebugTx;
//extern CanProtoListRxInfo_t gCanProtoListInfoDebugRx;

void DebugTx_ReadPrep(uint8_t idx);
void DebugTx_WritePrep(uint8_t idx);

void DebugRx_ReadProcess(uint8_t *p_buf);
void DebugRx_WriteProcess(uint8_t *p_buf);

void DebugCanCfgInit(DebugReadType *const p_read, DebugWriteType *const p_write);

//CAN PARA
//extern CanProtoListTxInfo_t gCanProtoListInfoParaTx;
//extern CanProtoListRxInfo_t gCanProtoListInfoParaRx;

//void ParaTxMainPrep(uint8_t idx);

//void ParaRxMainProcess(uint8_t *p_buf);

//CAN FUNCTEST

typedef struct _bmu_test_info_stru_t
{
    uint8_t     test_mode_data_buff[8];
    uint8_t     voltage_data_buff[8];
    uint8_t     temp_data_buff[8];
    uint8_t     balance_temp_data_buff[8];
    uint8_t     two_way_io_data_buff[8];
    uint8_t     io_output_data_buff[8];
    uint8_t     pwm_out_data_buff[8];
    uint8_t     io_input_data_buff[8];
    uint8_t     flash_data_buff[8];
    uint8_t     test_end_data_buff[8];
    
} BmuTestInfo_t;

extern CanProtoListTxInfo_t gCanProtoListInfoFactoryFuncTestTx;
extern CanProtoListRxInfo_t gCanProtoListInfoFactoryFuncTestRx;
void BmuFactoryTestEnterModeTxPrep(uint8_t idx);
void BmuFactoryTestVoltageTxPrep(uint8_t idx);
void BmuFactoryTestTempTxPrep(uint8_t idx);
void BmuFactoryTestBalanceTempTxPrep(uint8_t idx);
void BmuFactoryTestTwoWayIOTxPrep(uint8_t idx);
//void BmuFactoryTestDoTxPrep(uint8_t idx);
void BmuFactoryTestPwmTxPrep(uint8_t idx);
//void FuncTestTx_DiPrep(uint8_t idx);
void BmuFactoryTestFlashTxPrep(uint8_t idx);
void BmuFactoryTestVersionTxPrep(uint8_t idx);
void BmuFactoryTestEndModeTxPrep(uint8_t idx);


void BmuFactoryTestEnterModeRxPrep(uint8_t *p_buf);
void BmuFactoryTestVoltageRxPrep(uint8_t *p_buf);
void BmuFactoryTestTempRxPrep(uint8_t *p_buf);
void BmuFactoryTestBalanceTempRxPrep(uint8_t *p_buf);
void BmuFactoryTestTwoWayIORxPrep(uint8_t *p_buf);
//void BmuFactoryTestDoRxPrep(uint8_t *p_buf);
void BmuFactoryTestPwmRxPrep(uint8_t *p_buf);
//void FuncTestRx_DiProcess(uint8_t *p_buf);
void BmuFactoryTestFlashRxPrep(uint8_t *p_buf);
void BmuFactoryTestVersionRxPrep(uint8_t *p_buf);
void BmuFactoryTestEndModeRxPrep(uint8_t *p_buf);


//CAN_UPGRATE
extern CanProtoListTxInfo_t gCanProtoListInfoUpgradeTx;
extern CanProtoListRxInfo_t gCanProtoListInfoUpgradeRx;

void CanUpgradeTxPrep(uint8_t idx);

void CanUpgradeRxProcess(uint8_t *p_buf);


//CAN_TEST

void TestTxPrep(uint8_t idx);
void TestTxSendPrep(uint8_t idx);

void TestRxProcess(uint8_t *p_buf);


extern CanProtoListTxInfo_t gCanProtoListInfoTestTx;
extern CanProtoListRxInfo_t gCanProtoListInfoTestRx;



#endif







