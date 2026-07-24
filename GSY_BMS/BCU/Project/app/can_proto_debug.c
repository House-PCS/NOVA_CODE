/**
  ******************************************************************************
  * @copyright (C) COPYRIGHT  Nova
  * @file        can_proto_debug.c
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
    EN_DEBUG_HALL_BIG_CORR_COEF_TX = 0,
    EN_DEBUG_HALL_SMALL_CORR_COEF_TX,
    EN_DEBUG_TX_TOTAL,
}EN_CAN_BMU_TX;



typedef enum
{
    EN_DEBUG_CLEAR_STORAGE_RX = 0,
    EN_DEBUG_HALL_BIG_CORR_COEF_RX,
    EN_DEBUG_HALL_SMALL_CORR_COEF_RX,
    EN_DEBUG_CORR_COEF_LATCH_RX,
    EN_DEBUG_CORR_COEF_CLEAR_RX,
    EN_DEBUG_CORR_COEF_READ_RX,
    EN_DEBUG_RX_TOTAL,
}EN_CAN_BMU_RX;


void DebugHallBigCorrCoefTxPrep(uint8_t idx);
void DebugHallSmallCorrCoefTxPrep(uint8_t idx);
void DebugClearStorageRxProcess(uint8_t *p_buf);
void DebugHallBigCorrCoefRxProcess(uint8_t *p_buf);
void DebugHallSmallCorrCoefRxProcess(uint8_t *p_buf);
void DebugCorrCoefLatchRxProcess(uint8_t *p_buf);
void DebugCorrCoefClearRxProcess(uint8_t *p_buf);
void DebugCorrCoefReadRxProcess(uint8_t *p_buf);

static CanProtoTxInfo_t _gCanProtoInfoDebugTx[] = 
{
    {0x10A1FFB4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, DebugHallBigCorrCoefTxPrep,           NULL}, 
    {0x10A2FFB4, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, EN_CAN_PROTO_TX_MODE_TRIGGER, 0, false, DebugHallSmallCorrCoefTxPrep,         NULL}, 
};

CanProtoListTxInfo_t gCanProtoListInfoDebugTx = 
{
    .p_proto_info = _gCanProtoInfoDebugTx,
    .num = EN_DEBUG_TX_TOTAL
};


static CanProtoRxInfo_t _gCanProtoInfoDebugRx[] = 
{
    {0x10A0B4FF, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugClearStorageRxProcess,         NULL}, 
    {0x10A1B4FF, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugHallBigCorrCoefRxProcess,      NULL}, 
    {0x10A2B4FF, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugHallSmallCorrCoefRxProcess,    NULL}, 
    {0x10AAB4FF, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugCorrCoefLatchRxProcess,        NULL}, 
    {0x10ABB4FF, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugCorrCoefClearRxProcess,        NULL}, 
    {0x10ACB4FF, EN_DEV_CAN0, EN_DEV_CAN_TYPE_EXT, 8, 0, 0, DebugCorrCoefReadRxProcess,         NULL}, 
};

CanProtoListRxInfo_t gCanProtoListInfoDebugRx = 
{
    .p_proto_info = _gCanProtoInfoDebugRx,
    .num = EN_DEBUG_RX_TOTAL
};



/** 
* @brief    霍尔大量程电流校准系数发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void DebugHallBigCorrCoefTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t temp[4];
    
    memcpy(temp, (uint8_t *)&gCurrentInfo.big_corr_coef_k, 4);
    buf[0] = temp[3];
    buf[1] = temp[2];
    buf[2] = temp[1];
    buf[3] = temp[0];
    memcpy(temp, (uint8_t *)&gCurrentInfo.big_corr_coef_b, 4);
    buf[4] = temp[3];
    buf[5] = temp[2];
    buf[6] = temp[1];
    buf[7] = temp[0];
    
    CanProtoTxIntoQueue(&_gCanProtoInfoDebugTx[EN_DEBUG_HALL_BIG_CORR_COEF_TX], buf);
}


/** 
* @brief    霍尔小量程电流校准系数发送数据准备
* @param    idx:下发数据索引,该接口不使用
* @return   NULL
* @note:
*/
void DebugHallSmallCorrCoefTxPrep(uint8_t idx)
{
    uint8_t buf[8];
    uint8_t temp[4];
    
    memcpy(temp, (uint8_t *)&gCurrentInfo.small_corr_coef_k, 4);
    buf[0] = temp[3];
    buf[1] = temp[2];
    buf[2] = temp[1];
    buf[3] = temp[0];
    memcpy(temp, (uint8_t *)&gCurrentInfo.small_corr_coef_b, 4);
    buf[4] = temp[3];
    buf[5] = temp[2];
    buf[6] = temp[1];
    buf[7] = temp[0];
    
    CanProtoTxIntoQueue(&_gCanProtoInfoDebugTx[EN_DEBUG_HALL_SMALL_CORR_COEF_TX], buf);
}



/** 
* @brief    清除存储数据接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugClearStorageRxProcess(uint8_t *p_buf)
{
    uint16_t temp = (p_buf[0] << 8) | p_buf[1];
    
    if(temp == 0x0100)//清除FLASH下述全部信息
    {
        BMUSampleInfoClear();
        PackFactInfoClear();
        AlarmThresholdInfoClear();
        BCUFuncThresholdInfoClear();
    }
    else if(temp == 0x0101)//清除FLASH采样掩码信息
    {
        BMUSampleInfoClear();
    }
    else if(temp == 0x0102)//清除FLASH电芯出厂参数
    {
        PackFactInfoClear();
    }
    else if(temp == 0x0103)//清除FLASH故障阈值信息
    {
        AlarmThresholdInfoClear();
    }
    else if(temp == 0x0104)//清除FLASH BCU其他阈值信息
    {
        BCUFuncThresholdInfoClear();
    }
    else if(temp == 0x0200)//清除EEPROM下述全部信息
    {
        SoxInfoStorageClear();
        ElectStatistStorageClear();
    }
    else if(temp == 0x0201)//清除EEPROM SOX_INFO信息
    {
        SoxInfoStorageClear();
    }
    else if(temp == 0x0202)//清除EEPROM elect_statist信息
    {
        ElectStatistStorageClear();
    }
    else if(temp == 0x0300)//清除FLASH和EEPROM上述全部信息
    {
        BMUSampleInfoClear();
        PackFactInfoClear();
        AlarmThresholdInfoClear();
        BCUFuncThresholdInfoClear();
        SoxInfoStorageClear();
        ElectStatistStorageClear();
    }
}


/** 
* @brief    霍尔大量程电流校准系数接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugHallBigCorrCoefRxProcess(uint8_t *p_buf)
{
    float k, b;
    uint8_t temp[4];
    
    temp[0] = p_buf[3] & 0xFF;
    temp[1] = p_buf[2] & 0xFF;
    temp[2] = p_buf[1] & 0xFF;
    temp[3] = p_buf[0] & 0xFF;
    memcpy((uint8_t *)&k, temp, 4);
    temp[0] = p_buf[7] & 0xFF;
    temp[1] = p_buf[6] & 0xFF;
    temp[2] = p_buf[5] & 0xFF;
    temp[3] = p_buf[4] & 0xFF;
    memcpy((uint8_t *)&b, temp, 4);
    
    if(k >= 0.5 && k <= 2 && b <= 10 && b >= -10)//系数进行限制
    {
        gCurrentInfo.big_corr_coef_k = k;
        gCurrentInfo.big_corr_coef_b = b;
        gCurrentInfo.big_corr_coef_new_flag = true;
    }
}






/** 
* @brief    霍尔小量程电流校准系数接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugHallSmallCorrCoefRxProcess(uint8_t *p_buf)
{
    float k, b;
    uint8_t temp[4];
    
    temp[0] = p_buf[3] & 0xFF;
    temp[1] = p_buf[2] & 0xFF;
    temp[2] = p_buf[1] & 0xFF;
    temp[3] = p_buf[0] & 0xFF;
    memcpy((uint8_t *)&k, temp, 4);
    temp[0] = p_buf[7] & 0xFF;
    temp[1] = p_buf[6] & 0xFF;
    temp[2] = p_buf[5] & 0xFF;
    temp[3] = p_buf[4] & 0xFF;
    memcpy((uint8_t *)&b, temp, 4);
    
    if(k >= 0.5 && k <= 2 && b <= 10 && b >= -10)//系数进行限制
    {
        gCurrentInfo.small_corr_coef_k = k;
        gCurrentInfo.small_corr_coef_b = b;
        gCurrentInfo.small_corr_coef_new_flag = true;
    }
}


/** 
* @brief    电流校准系数锁存接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugCorrCoefLatchRxProcess(uint8_t *p_buf)
{
    if(gCurrentInfo.big_corr_coef_new_flag == true || gCurrentInfo.small_corr_coef_new_flag == true)
    {
        BCUCurrCorrCoefStorge();
    }
}



/** 
* @brief    电流校准系数清除接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugCorrCoefClearRxProcess(uint8_t *p_buf)
{
    BCUCurrCorrCoefClear();
}



/** 
* @brief    电流校准系数回读接收数据处理
* @param    p_buf:接收到的协议数据内容(8byte)
* @return   NULL
* @note:
*/
void DebugCorrCoefReadRxProcess(uint8_t *p_buf)
{
    DebugHallBigCorrCoefTxPrep(0);
    DebugHallSmallCorrCoefTxPrep(0);
}



